import pickle, zlib

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
    def __init__( self, outdir : str, name : str, cutoff : int, schema ) :
        self.outdir = outdir
        self.name = name
        self.cutoff = cutoff
        self.schema = schema
        self.initialize()

    def initialize( self ) :
        self.blob_start_row = 0
        self.bytes_written = 0
        self.current_row = 0
        self.blob_nr = 0
        self.blob = dict()
        self.row = dict()
        for c in self.schema :
            self.blob[ c ] = list()
            self.row[ c ] = None
        self.meta = ( self.name, self.schema, list() )

    def open_row( self ) :
        for _, v in self.row.items() :
            v = None

    def write_cell( self, column_name : str, value, value_size : int ) :
        if column_name in self.row.keys() :
            self.row[ column_name ] = value
            self.bytes_written += value_size
        else :
            raise f"unknown column {column_name}"

    def close_row( self ) :
        if self.bytes_written >= self.cutoff :
            self.flush_blob()
        for k, v in self.row.items() :
            self.blob[ k ].append( v )
            self.bytes_written
        self.current_row += 1

    def flush_blob( self ) :
        fname = f"{self.outdir}/blob.{self.blob_nr}"
        compressed = dict()
        for k, v in self.blob.items() :
            compressed[ k ] = zlib.compress( pickle.dumps( v ), 9 )
        with open( fname, "wb" ) as f :
            f.write( pickle.dumps( compressed ) )
        self.meta[2].append( ( self.blob_start_row, len( self.blob[ self.schema[ 0 ] ] ) ) )
        for c in self.schema :
            self.blob[ c ] = list()
        self.blob_nr += 1
        self.blob_start_row = self.current_row
        self.bytes_written = 0

    def finish( self ) :
        if self.bytes_written > 0 :
            self.flush_blob()
        pickle.dump( self.meta, open( f"{self.outdir}/meta", "wb" ) )

class run_reader:
    def __init__( self, indir : str ) :
        self.indir = indir
        self.meta = pickle.load( open( f"{self.indir}/meta", "rb" ) )
        self.schema = self.meta[1]
        self.blob_nr = 0
        self.blob = None

    def name( self ):
        return self.meta[0]

    def schema( self ) :
        return self.schema

    def load_blob( self ) :
        self.blob_meta = self.meta[2] [ self.blob_nr ]
        fname = f"{self.indir}/blob.{self.blob_nr}"
        with open( fname, "rb" ) as f :
            compressed = pickle.loads( f.read() )
        self.blob = dict()
        for k, v in compressed.items() :
            self.blob[ k ] = pickle.loads( zlib.decompress( v ) )
        self.relative_row_nr = 0

    def next_row( self ) :
        if self.blob == None :
            self.load_blob()
            return True

        self.relative_row_nr += 1
        if self.relative_row_nr < self.blob_meta[ 1 ] :
            return True

        self.blob_nr += 1
        if self.blob_nr >= len( self.meta[ 2 ] ) :
            return False

        self.load_blob()
        return True

    def get( self, name : str ) :
        if name in self.blob.keys() :
            return self.blob[ name ][ self.relative_row_nr ]
        return None
