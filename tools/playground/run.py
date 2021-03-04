import pickle

class run_writer:
    def __init__( self, outdir : str, name : str, cutoff : int, schema ) :
        self.outdir = outdir
        self.cutoff = cutoff
        self.blob_start_row = 0
        self.bytes_written = 0
        self.current_row = 0
        self.blob_nr = 0
        self.blob = list()
        self.meta = ( name, schema, list() )

    def write_blob( self ) :
        fname = f"{self.outdir}/blob.{self.blob_nr}"
        pickle.dump( self.blob, open( fname, "wb" ) )
        self.meta[2].append( ( self.blob_start_row, len( self.blob ) ) )
        #print( f"{fname} : ({self.blob_start_row},{len( self.blob )}) bytes={self.bytes_written}" )

    def append_row( self, data, byte_count : int ) :
        if self.bytes_written >= self.cutoff :
            self.write_blob()
            self.blob = list()
            self.blob_nr += 1
            self.blob_start_row = self.current_row
            self.bytes_written = 0
        self.current_row += 1
        self.bytes_written += byte_count
        self.blob.append( data )

    def close_writing( self ) :
        if self.bytes_written > 0 :
            self.write_blob()
        pickle.dump( self.meta, open( f"{self.outdir}/meta", "wb" ) )

class run_reader:
    def __init__( self, indir : str ) :
        self.indir = indir
        self.meta = pickle.load( open( f"{self.indir}/meta", "rb" ) )
        self.schema = self.meta[1]
        self.blob_nr = 0
        self.last_row = 0
        self.load_blob()
        self.cur_row = None

    def name( self ):
        return self.meta[0]

    def load_blob( self ) :
        # print(self.meta)
        self.blob_meta = self.meta[2] [ self.blob_nr ]
        self.blob = pickle.load( open( f"{self.indir}/blob.{self.blob_nr}", "rb" ) )
        # print( f"{self.indir}/blob.{self.blob_nr}" )

    def next_row( self ) :
        if self.last_row < self.blob_meta[0] + self.blob_meta[1] :
            self.cur_row = self.blob[ self.last_row - self.blob_meta[0] ]
            self.last_row += 1
            return True

        self.blob_nr += 1
        if self.blob_nr >= len(self.meta[2]) :
            return False

        self.load_blob()
        # print(f"self.last_row={self.last_row}" )
        self.cur_row = self.blob[ self.last_row - self.blob_meta[0] ]
        self.last_row += 1
        return True

    def get( self, name : str ) :
        idx = self.schema.index(name)
        if idx != None :
            return self.cur_row[idx]
        return None