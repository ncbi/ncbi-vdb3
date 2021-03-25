import sys, pickle, zlib, zstd, gzip, bz2
import http.client, urllib
from enum import Enum
from collections import namedtuple
import protobuf.sra_pb2

ColumnDef = namedtuple( 'ColumnDef', 'comp level group' )
GroupDef = namedtuple( 'GroupDef', 'comp level cutoff cols' )
SchemaDef = namedtuple( 'SchemaDef', 'columns groups' )

MetaDef = namedtuple( 'MetaDef', 'name schema blobmap' )

class SerializationMode( Enum ) :
    Pickle = 0
    Protobuf = 1

ser_mode = SerializationMode.Pickle

class group_writer:
    def __init__( self, name : str, groupdef : GroupDef, col_defs, outdir : str ) :
        self.name = name
        self.col_defs = col_defs
        self.outdir = outdir
        self.file_nr = 0

        self.compression = groupdef.comp
        self.level = groupdef.level
        self.cutoff = groupdef.cutoff
        self.column_names = groupdef.cols # list(str)

        self.clear_blob()
        self.start_row = 0
        self.row_count = 0
        self.bytes_written = 0

    def clear_blob( self ) :
        self.blob = dict() # column name -> list() of values
        for c in self.column_names :
            self.blob[ c ] = list()

    def write_cell( self, col_name : str, value, val_size : int ) :
        self.blob[ col_name ].append( value )
        self.bytes_written += val_size

    def close_row( self, blob_map : list ) : # list( ( start, count ) )
        self.row_count += 1

        #this is an insurance against the case that the user did not
        #write a value into each column
        for _, c in self.blob.items() :
            while len(c) < self.row_count :
                c.append( None )

        if self.bytes_written > self.cutoff :
            self.flush_blob( blob_map )

    def serialize_column( self, name : str, data ) : # returns bytearray
        if ser_mode == SerializationMode.Pickle:
            return pickle.dumps( data )
        else:
            col = protobuf.sra_pb2.Column()
            for cell in data:
                pb_cell = protobuf.sra_pb2.Cell()
                if type(cell) == str:
                    pb_cell.str_value = cell
                else: # array of int
                    for i in cell:
                        pb_cell.int_values.i.append( i )
                # else: a single int, float etc (TODO)
                col.cells.append( pb_cell )
            return col.SerializeToString()

    def serialize_blob( self, data ) :
        if ser_mode == SerializationMode.Pickle:
            return pickle.dumps( data )
        else:
            pb2_blob = protobuf.sra_pb2.Group()
            for k, v in data.items() :
                pb2_blob.names.append(k)
                pb2_blob.encoded_columns.append(v)
            return pb2_blob.SerializeToString()

    def compress( self, src, compression : str, level : int ) : # src is a bytearray
        if compression == 'zlib' :
            return zlib.compress( src, level )
        elif compression == 'gzip' :
            return gzip.compress( src, level )
        elif compression == 'zstd' :
            return zstd.compress( src, level )
        elif compression == 'bz2' :
            return bz2.compress( src, level )
        return src

    def flush_blob( self, blob_map : list ) : # list( ( start, count ) )
        fname = f"{self.outdir}/{self.name}.{self.file_nr}"
        #compress each column in the blob seperately
        compressed = dict()
        for c in self.column_names :
            compression = self.col_defs[ c ].comp
            level = self.col_defs[ c ].level
            serialized = self.serialize_column( c, self.blob[ c ] )
            compressed[c] = self.compress( serialized, compression, level )

        serialized = self.serialize_blob( compressed )
        to_write = self.compress( serialized, self.compression, self.level )
        with open( fname, "wb" ) as f :
            f.write( to_write )

        self.file_nr += 1
        self.clear_blob()

        blob_map.append( ( self.start_row, self.row_count ) )
        self.start_row += self.row_count
        self.row_count = 0
        self.bytes_written = 0

"""
blobs :
    -----------G1---------------        ------------G2------------------
    READ        QUALITY     NAME        SPOTGROUP   READ_START  READ_LEN
    R[1]        Q[1]        N[1]
    R[2]        Q[2]        N[2]
    ..          ..          ..
"""
"""
    outdir ... path to write pickeld data-files ( caller created it )
    name ..... name of the run aka accession ( ie SRR000001 )
    cutoff ... max bytes in a group-blob ( pre compression, sum of all cells )
    schema ... flat list of column-names ( ide ['READ','QUALITY','NAME'] )
"""
class run_writer:
    def __init__( self, outdir : str, name : str, schema : SchemaDef ) :
        self.outdir = outdir
        self.name = name
        self.schema = schema
        self.initialize()

    def initialize( self ) :
        #schema is tuple of 2 dictionaries:
        #   (
        #       { column_name : ColumnDef },
        #       { group_name : GroupDef }
        #   )
        #
        #blob-map is a dictionary: row_group -> list( (start, count) )
        self.meta = MetaDef( self.name, self.schema, dict() )

        self.groups = dict() # group name -> group_writer
        for k, v in self.schema.groups.items() :
            self.groups[ k ] = group_writer( k, v, self.schema.columns, self.outdir )
            self.meta.blobmap[k] = list()

    def group_of_column( self, col : str ) :
        gr_name = self.schema.columns[col].group
        return self.groups[gr_name]

    def write_cell( self, column_name : str, value, value_size : int ) :
        gr_writer = self.group_of_column( column_name )
        gr_writer.write_cell( column_name, value, value_size )

    def close_row( self ) :
        for k, v in self.groups.items() :
            v.close_row( self.meta.blobmap[k] )

    def finish( self ) :
        for k, v in self.groups.items() :
            v.flush_blob( self.meta.blobmap[k] )
        pickle.dump( self.meta, open( f"{self.outdir}/meta", "wb" ) )

class blob_file_reader:
    def __init__( self, indir : str ) :
        self.indir = indir

    def read_meta( self ):
        with open( f"{self.indir}/meta", "rb" ) as f :
            return f.read()

    def read( self, group_name : str, blob_nr : int ):
        fname = f"{self.indir}/{group_name}.{blob_nr}"
        with open( fname, "rb" ) as f :
            return f.read()

class blob_http_reader:
    def __init__( self, url : str ) :
        self.url = urllib.parse.urlparse( url )
        self.path = self.url.path
        if self.path == "" or self.path[-1] != '/':
            self.path += "/"
        if self.url.scheme == "https" :
            self.conn = http.client.HTTPSConnection( self.url.netloc )
        else :
            self.conn = http.client.HTTPConnection( self.url.netloc )

    def read_meta( self ):
        self.conn.request("GET", f"{self.path}meta")
        return self.conn.getresponse().read()

    def read( self, group_name : str, blob_nr : int ):
        self.conn.request("GET", f"{self.path}{group_name}.{blob_nr}")
        return self.conn.getresponse().read()

#encapsulates a list of dictionaries
class group_blob:
    def __init__( self, blob, first : int, count : int ) :
        self.blob = blob
        self.first = first
        self.last = first + count - 1
        self.count = count

    def get( self, row : int, column_name : str ) :
        if row < self.first or row > self.last :
            raise Exception( f"group_blob out of range: {row}" )
        return self.blob[ column_name ][ row - self.first ] #another dragon...

class group_reader:
    def __init__( self, name : str, groupdef : GroupDef, blob_reader, row_map, column_meta ) : # list( (start, count) )
        self.name = name
        self.groupdef = groupdef
        self.blob_reader = blob_reader
        self.row_map = row_map  # list of start,count - tuples
        self.column_meta = column_meta
        self.compression = self.groupdef.comp
        self.blobs = dict()     # here are the blob-groups, keyed by blob_nr

    def total_rows( self ) :
        last = self.row_map[-1]
        return last[0]+last[1]

    def deserialize_column( self, data ) :
        if ser_mode == SerializationMode.Pickle:
            return pickle.loads( data )
        else:
            pb2_col = protobuf.sra_pb2.Column()
            pb2_col.ParseFromString( data )
            ret = list()
            for cell in pb2_col.cells:
                if cell.WhichOneof('Data') == 'str_value':
                    ret.append(cell.str_value)
                else:
                    l = list()
                    for i in cell.int_values:
                        l.append(i)
                    ret.append(l)
            return ret

    def deserialize_blob( self, data ) :
        if ser_mode == SerializationMode.Pickle:
            return pickle.loads( data )
        else:
            pb2_blob = protobuf.sra_pb2.Group()
            pb2_blob.ParseFromString( data )
            ret = dict()
            for i in range( len(pb2_blob.names) ):
                ret[ pb2_blob.names[ i ] ] = pb2_blob.encoded_columns[ i ]
            return ret

    def decompress( self, src, compression : str ) :
        if compression == 'zlib' :
            return zlib.decompress( src )
        elif compression == 'gzip' :
            return gzip.decompress( src )
        elif compression == 'zstd' :
            return zstd.decompress( src )
        return src

    def load_blob( self, blob_nr : int ) -> group_blob :
        try :
            data = self.blob_reader.read( self.name, blob_nr )
        except Exception as e:
            sys.stderr.write( f"{e}\n" )
            data = None
        if data != None :
            decomp = self.decompress( data, self.compression )
            deserialized = self.deserialize_blob( decomp )
            blob = dict()
            for k, v in deserialized.items() :
                compression = self.column_meta[ k ].comp
                decomp = self.decompress( v, compression )
                blob[ k ] = self.deserialize_column( decomp )
            return group_blob( blob, self.row_map[ blob_nr ][ 0 ], self.row_map[ blob_nr ][ 1 ] )
        return None

    #attention: row has to be zero based ( implicitly written that way by the writer )
    def row_2_blobnr( self, row : int ) -> int :
        blob_nr = 0
        for start, count in self.row_map :  # linear search for now...
            if row >= start and row < start + count :
                return blob_nr
            blob_nr += 1
        return None

    #returns the number of available rows in the window ( count, except the last one )
    def set_window( self, start : int, count : int ) :
        first_blob_nr = self.row_2_blobnr( start )
        last_blob_nr = self.row_2_blobnr( start + count - 1 )
        if first_blob_nr == None :
            return
        if last_blob_nr == None :
            last_blob_nr = self.row_2_blobnr( self.total_rows() - 1 )
        to_load = list()
        for blob_nr in range( first_blob_nr, last_blob_nr + 1 ) :
            to_load.append( blob_nr )
        s1 = set( to_load )
        s2 = set( self.blobs.keys() )
        s_to_load = s1.difference( s2 )
        s_to_drop = s2.difference( s1 )
        for blob_nr in s_to_drop :
            self.blobs.pop( blob_nr )
        for blob_nr in s_to_load :
            self.blobs[ blob_nr ] = self.load_blob( blob_nr )

    def get( self, row : int, col_name : str ):
        blob_nr = self.row_2_blobnr( row )
        data = self.blobs[ blob_nr ]
        if data != None :
            return data.get( row, col_name )
        return None

class AccessMode( Enum ) :
    FileSystem = 0
    URL = 1

class run_reader:
    def __init__( self,
                  addr : str,       # path or url
                  wanted : list,    # list of column-names to consider (empty/None: all)
                  mode : AccessMode = AccessMode.FileSystem ) :
        if mode == AccessMode.FileSystem :
            br = blob_file_reader( addr )
        else:
            br = blob_http_reader( addr )

        # meta : ( name, schema, blob-map )
        self.meta = pickle.loads( br.read_meta() )

        self.groups = dict() # group name -> group_reader
        self.total_rows = None
        for group_name, groupdef in self.meta.schema.groups.items() :
            if self.is_group_wanted( groupdef, wanted ) :
                self.groups[ group_name ] = group_reader( group_name, groupdef, br,
                    self.meta.blobmap[group_name], self.meta.schema.columns )
                if self.total_rows == None :
                    self.total_rows = self.groups[ group_name ].total_rows()
                else:
                    if not self.total_rows == self.groups[ group_name ].total_rows() :
                        raise Exception( "inconsistent total_rows across column groups" )
        self.cur_row = 0

    def is_group_wanted( self, groupdef : GroupDef, wanted : list ) -> bool :
        if wanted == None :
            return True
        if len( wanted ) == 0 :
            return True
        set1 = set( groupdef.cols )
        set2 = set( wanted )
        return len( set1 & set2 ) > 0

    def name( self ):
        return self.meta.name

    #returns the number of available rows in the window ( count, except the last one )
    #attention: start has to be zero based ( implicitly written that way by the writer )
    def set_window( self, start : int, count : int ) -> int :
        for _, group in self.groups.items() :
            group.set_window( start, count ) # ask the groups to load all these rows
        res = self.total_rows - start
        return max( 0, min( count, res ) )

    #def next_row( self ) :
    #    self.cur_row += 1
    #    if self.cur_row > self.total_rows :
    #        return False
    #    for _,v in self.groups.items() :
    #        v.next_row()
    #    return True

    def group_of_column( self, col : str ) :
        gr_name = self.meta.schema.columns[col].group # dragon here! can throw...
        return self.groups[gr_name]     # dragon here! can throw...

    #attention: row has to be zero based ( implicitly written that way by the writer )
    def get( self, row : int, name : str ) :
        # another dragon: group throws eventually a out of range
        return self.group_of_column( name ).get( row, name )
