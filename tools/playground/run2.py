import pickle, zlib

class group_writer:
    def __init__( self, name : str, attrs, outdir : str ) :
        self.name = name
        self.attrs = attrs
        self.cutoff = attrs["cutoff"]

        self.outdir = outdir
        self.file_nr = 0
        self.columns = self.attrs["cols"] # list(str)

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

    def flush_blob( self, blob_map : list ) : # list( ( start, count ) )
        fname = f"{self.outdir}/{self.name}.{self.file_nr}"
        with open( fname, "wb" ) as f :
             f.write( pickle.dumps( self.blob ) )
        self.file_nr += 1
        self.clear_blob()

        blob_map.append( ( self.start_row, self.row_count ) )
        self.start_row += self.row_count
        self.row_count = 0
        self.bytes_written = 0

    #     compressed = dict()
    #     for k, v in self.blob.items() :
    #         compressed[ k ] = zlib.compress( pickle.dumps( v ), 9 )
    #     with open( fname, "wb" ) as f :
    #         f.write( pickle.dumps( compressed ) )
    #     self.meta[2].append( ( self.blob_start_row, len( self.blob[ self.schema[ 0 ] ] ) ) )
    #     for c in self.schema :
    #         self.blob[ c ] = list()
    #     self.blob_nr += 1
    #     self.blob_start_row = self.current_row
    #     self.bytes_written = 0


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
            self.groups[ k ] = group_writer( k, v, self.outdir )
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


class group_reader:
    def __init__( self, name : str, attrs, indir : str, row_map ) : # list( (start, count) )
        self.name = name
        self.attrs = attrs
        self.indir = indir
        self.row_map = row_map
        self.blob_nr = 0

        self.row_count = 0
        self.relative_row_nr = 0

        self.blob = None

    def total_rows( self ) :
        last = self.row_map[-1]
        return last[0]+last[1]

    def load_blob( self ) :
        fname = f"{self.indir}/{self.name}.{self.blob_nr}"
        with open( fname, "rb" ) as f :
            self.blob = pickle.loads( f.read() )

        #     compressed = pickle.loads( f.read() )
        # self.blob = dict()
        # for k, v in compressed.items() :
        #     self.blob[ k ] = pickle.loads( zlib.decompress( v ) )
        # self.relative_row_nr = 0

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
    def __init__( self, indir : str ) :
        self.indir = indir
        self.meta = pickle.load( open( f"{self.indir}/meta", "rb" ) )

        self.groups = dict() # group name -> group_reader
        self.total_rows = None
        for k,v in self.meta[1][1].items() :
            self.groups[ k ] = group_reader( k, v, self.indir, self.meta[2][k] )
            if self.total_rows == None :
                self.total_rows = self.groups[ k ].total_rows()
            else:
                if not self.total_rows == self.groups[ k ].total_rows() :
                    raise "hell, inconsistent total_rows across column groups"

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
