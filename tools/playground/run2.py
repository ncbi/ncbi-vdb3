import pickle, zlib, zstd, gzip
import http.client, urllib

class group_writer:
    def __init__( self, name : str, attrs, column_meta, outdir : str ) :
        self.name = name
        self.attrs = attrs
        self.column_meta = column_meta
        self.cutoff = attrs["cutoff"]

        self.outdir = outdir
        self.file_nr = 0
        self.columns = self.attrs["cols"] # list(str)
        self.compression = self.attrs[ "comp" ]
        self.level = self.attrs[ "level" ]

        self.clear_blob()
        self.start_row = 0
        self.row_count = 0
        self.bytes_written = 0

    def clear_blob( self ) :
        self.blob = dict() # column name -> list() of values
        for c in self.columns :
            self.blob[c] = list()

    def write_cell( self, col_name : str, value, val_size : int ) :
        self.blob[ col_name ].append(value)
        self.bytes_written += val_size

    def close_row( self, blob_map : list ) : # list( ( start, count ) )
        self.row_count += 1
        for _, c in self.blob.items() :
            while len(c) < self.row_count :
                c.append(None)
        if self.bytes_written > self.cutoff :
            self.flush_blob( blob_map )

    def compress( self, src, compression : str, level : int ) :
        if compression == 'zlib' :
            return zlib.compress( pickle.dumps( src ), level )
        elif compression == 'gzip' :
            return gzip.compress( pickle.dumps( src ), level )
        elif compression == 'zstd' :
            return zstd.compress( pickle.dumps( src ), level )
        return pickle.dumps( src )

    def flush_blob( self, blob_map : list ) : # list( ( start, count ) )
        fname = f"{self.outdir}/{self.name}.{self.file_nr}"
        #compress each column in the blob seperately
        compressed = dict()
        for c in self.columns :
            compression = self.column_meta[ c ][ 'comp' ]
            level = self.column_meta[ c ][ 'level' ]
            compressed[c] = self.compress( self.blob[ c ], compression, level )

        to_write = self.compress( compressed, self.compression, self.level )
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
    def __init__( self, outdir : str, name : str, schema ) :
        self.outdir = outdir
        self.name = name
        self.schema = schema
        self.initialize()

    def initialize( self ) :
        self.meta = ( self.name, self.schema, dict() ) # row_group -> list( (start, count) )

        self.groups = dict() # group name -> group_writer
        for k,v in self.schema[1].items() :
            self.groups[ k ] = group_writer( k, v, self.schema[0], self.outdir )
            self.meta[2][k] = list()

    def group_of_column( self, col : str ) :
        gr_name = self.schema[0][col]["group"]
        return self.groups[gr_name]

    def write_cell( self, column_name : str, value, value_size : int ) :
        gr_writer = self.group_of_column( column_name )
        gr_writer.write_cell( column_name, value, value_size )

    def close_row( self ) :
        for k, v in self.groups.items() :
            v.close_row( self.meta[2][k] )

    def finish( self ) :
        for k, v in self.groups.items() :
            v.flush_blob( self.meta[2][k] )
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
        self.conn = http.client.HTTPConnection(self.url.netloc)

    def read_meta( self ):
        self.conn.request("GET", f"{self.path}meta")
        return self.conn.getresponse().read()

    def read( self, group_name : str, blob_nr : int ):
        self.conn.request("GET", f"{self.path}{group_name}.{blob_nr}")
        return self.conn.getresponse().read()

class group_reader:
    def __init__( self, name : str, attrs, blob_reader, row_map, column_meta ) : # list( (start, count) )
        self.name = name
        self.attrs = attrs
        self.blob_reader = blob_reader
        self.row_map = row_map
        self.column_meta = column_meta
        self.compression = self.attrs[ 'comp' ]
        self.blob_nr = 0

        self.row_count = 0
        self.relative_row_nr = 0

        self.blob = None

    def total_rows( self ) :
        last = self.row_map[-1]
        return last[0]+last[1]

    def decompress( self, src, compression : str ) :
        if compression == 'zlib' :
            return pickle.loads( zlib.decompress( src ) )
        elif compression == 'gzip' :
            return pickle.loads( gzip.decompress( src ) )
        elif compression == 'zstd' :
            return pickle.loads( zstd.decompress( src ) )
        return pickle.loads( src )

    def load_blob( self ) :
        decompressed = self.decompress( self.blob_reader.read( self.name, self.blob_nr ), self.compression )
        self.blob = dict()
        for k, v in decompressed.items() :
            compression = self.column_meta[ k ][ 'comp' ]
            self.blob[ k ] = self.decompress( v, compression )
        self.row_count = self.row_map[ self.blob_nr ] [ 1 ]
        self.relative_row_nr = 0

        self.blob_nr += 1

    def next_row( self ) :
        if self.blob == None :
            self.load_blob()
            return

        if self.relative_row_nr == self.row_count - 1:
            self.load_blob()
            return

        self.relative_row_nr += 1

    def get( self, col : str ):
        return self.blob[col][self.relative_row_nr]

class run_reader:
    def __init__( self,
                  is_dir : bool, # false = a url
                  addr : str  ) :
        if is_dir :
            br = blob_file_reader( addr )
        else:
            br = blob_http_reader( addr )

        self.meta = pickle.loads( br.read_meta() )

        self.groups = dict() # group name -> group_reader
        self.total_rows = None
        for k,v in self.meta[1][1].items() :
            self.groups[ k ] = group_reader( k, v, br, self.meta[2][k], self.meta[1][0] )
            if self.total_rows == None :
                self.total_rows = self.groups[ k ].total_rows()
            else:
                if not self.total_rows == self.groups[ k ].total_rows() :
                    raise "inconsistent total_rows across column groups"

        self.cur_row = 0

    def name( self ):
        return self.meta[0]

    def next_row( self ) :
        self.cur_row += 1
        if self.cur_row > self.total_rows :
            return False

        for _,v in self.groups.items() :
            v.next_row()
        return True

    def group_of_column( self, col : str ) :
        gr_name = self.meta[1][0][col]["group"]
        return self.groups[gr_name]

    def get( self, name : str ) :
        return self.group_of_column(name).get(name)
