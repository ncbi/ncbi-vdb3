import pickle

class runobj:
    def __init__( self, outdir : str, cutoff : int ) :
        self.outdir = outdir
        self.cutoff = cutoff
        self.blob_start_row = 0
        self.bytes_written = 0
        self.current_row = 0
        self.blob_nr = 0
        self.blob = list()
        self.meta = list()

    def write_blob( self ) :
        fname = f"{self.outdir}/blob.{self.blob_nr}"
        pickle.dump( self.blob, open( fname, "wb" ) )
        self.meta.append( ( self.blob_start_row, len( self.blob ) ) )
        print( f"{fname} : ({self.blob_start_row},{len( self.blob )}) bytes={self.bytes_written}" ) 

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

