import sys, pickle, zlib, zstd, gzip, bz2
import http.client, urllib
from enum import Enum
from collections import namedtuple
from recordtype import recordtype
import protobuf.sra_pb2

from joblib import Parallel, delayed
from time import perf_counter

GroupDef = namedtuple( 'GroupDef', 'comp level cutoff cols' )

ReadTimes = recordtype( 'ReadTimes', 'dnld, decomp, deser' )

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

    def read_meta( self ):
        if self.url.scheme == "https" :
            self.conn = http.client.HTTPSConnection( self.url.netloc )
        else :
            self.conn = http.client.HTTPConnection( self.url.netloc )
        self.conn.request("GET", f"{self.path}meta")
        return self.conn.getresponse().read()

    def read( self, group_name : str, blob_nr : int ):
        if self.url.scheme == "https" :
            self.conn = http.client.HTTPSConnection( self.url.netloc )
        else :
            self.conn = http.client.HTTPConnection( self.url.netloc )
        self.conn.request("GET", f"{self.path}{group_name}.{blob_nr}")
        ret = self.conn.getresponse().read()
        return ret

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
        self.t_col = ReadTimes( 0.0, 0.0, 0.0 )   # ( dnld, decomp, deser )
        self.t_blob = ReadTimes( 0.0, 0.0, 0.0 )

    def total_rows( self ) :
        last = self.row_map[-1]
        return last[0]+last[1]

    def deserialize_column( self, data ) :
        t_start = perf_counter() 
        pb2_col = protobuf.sra_pb2.Column()
        pb2_col.ParseFromString( data )
        ret = list()
        for cell in pb2_col.cells:
            celltype = cell.WhichOneof('Data')
            if celltype == 'str_value' :
                ret.append( cell.str_value )
            else:
                l = list()
                for i in cell.int_values.i:
                    l.append( i )
                ret.append( l )
        self.t_col.deser = self.t_col.deser + ( perf_counter() - t_start )
        return ret

    def deserialize_blob( self, data ) :
        t_start = perf_counter() 
        pb2_blob = protobuf.sra_pb2.Group()
        pb2_blob.ParseFromString( data )
        ret = dict()
        for i in range( len(pb2_blob.names) ):
            ret[ pb2_blob.names[ i ] ] = pb2_blob.encoded_columns[ i ]
        self.t_blob.deser += ( perf_counter() - t_start )
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
            t_start = perf_counter()
            data = self.blob_reader.read( self.name, blob_nr )
            self.t_blob.dnld += ( perf_counter() - t_start )
        except Exception as e:
            sys.stderr.write( f"{e}\n" )
            data = None
        if data != None :
            t_start = perf_counter()
            decomp = self.decompress( data, self.compression )
            self.t_blob.decomp += ( perf_counter() - t_start )
            deserialized = self.deserialize_blob( decomp )
            blob = dict()
            for k, v in deserialized.items() :
                compression = self.column_meta[ k ].comp
                t_start = perf_counter()
                decomp = self.decompress( v, compression )
                self.t_col.decomp += ( perf_counter() - t_start )
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

class ParallelMode( Enum ) :
    Sequential = 0
    Threads = 1

class table_reader:
    def __init__( self,
                  addr : str,       # path or url
                  wanted : list,    # list of column-names to consider (empty/None: all)
                  access_mode : AccessMode = AccessMode.FileSystem,
                  parallel_mode : ParallelMode = ParallelMode.Sequential  ) :

        # meta : ( name, schema, blob-map )
        br = self.make_reader( addr, access_mode )
        self.meta = pickle.loads( br.read_meta() )

        self.groups = dict() # group name -> group_reader
        self.total_rows = None
        for group_name, groupdef in self.meta.schema.groups.items() :
            if self.is_group_wanted( groupdef, wanted ) :
                br = self.make_reader( addr, access_mode )
                self.groups[ group_name ] = group_reader( group_name, groupdef, br,
                    self.meta.blobmap[group_name], self.meta.schema.columns )
                if self.total_rows == None :
                    self.total_rows = self.groups[ group_name ].total_rows()
                else:
                    if not self.total_rows == self.groups[ group_name ].total_rows() :
                        raise Exception( "inconsistent total_rows across column groups" )
        self.cur_row = 0

        #setup the parallel-mode...
        self.pc = None
        if parallel_mode == ParallelMode.Threads :
            self.pc = Parallel( n_jobs = len( self.groups.items()), prefer="threads" )

    def report_times( self, detailed : bool ) :
        if detailed :
            for name, group in self.groups.items() :
                print( f"for group: {name}", file=sys.stderr )
                print( f"\tcolum-times: {group.t_col}", file=sys.stderr )
                print( f"\tblob-times : {group.t_blob}", file=sys.stderr, flush=True )
        else :
            c  = read_lib.ReadTimes( 0.0, 0.0, 0.0 )
            b = read_lib.ReadTimes( 0.0, 0.0, 0.0 )
            for _, group in self.groups.items() :
                c.dnld += group.t_col.dnld
                c.decomp += group.t_col.decomp
                c.deser += group.t_col.deser
                b.dnld += group.t_blob.dnld
                b.decomp += group.t_blob.decomp
                b.deser += group.t_blob.deser
            print( f"colum-times: {c}", file=sys.stderr )
            print( f"blob-times : {b}", file=sys.stderr, flush=True )

    def make_reader( self, addr : str, access_mode : AccessMode ) :
        if access_mode == AccessMode.FileSystem :
            return blob_file_reader( addr )
        return blob_http_reader( addr )

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
        if self.pc != None :
            self.pc( delayed( group.set_window )( start, count ) for _, group in self.groups.items() )
        else :
            for _, group in self.groups.items() :
                group.set_window( start, count ) # ask the groups to load all these rows

        res = self.total_rows - start
        return max( 0, min( count, res ) )

    def group_of_column( self, col : str ) :
        gr_name = self.meta.schema.columns[col].group # dragon here! can throw...
        return self.groups[gr_name]     # dragon here! can throw...

    #attention: row has to be zero based ( implicitly written that way by the writer )
    def get( self, row : int, name : str ) :
        # another dragon: group throws eventually a out of range
        return self.group_of_column( name ).get( row, name )

class database_reader:
    def __init__( self,
                  addr : str,       # path or url
                  access_mode : AccessMode = AccessMode.FileSystem,
                  parallel_mode : ParallelMode = ParallelMode.Sequential
                ) :
        self.addr = addr
        self.access_mode = access_mode
        self.parallel_mode = parallel_mode

    def make_table_reader( self, table_name : str, wanted : list ) :
        sub_addr = f"{self.addr}/{table_name}/"
        #here we could read the meta-file and verify that the requested table-name is in it
        return table_reader( sub_addr, wanted, self.access_mode, self.parallel_mode )
