import sys, pickle, zlib, zstd, gzip, bz2
import http.client, urllib
from enum import Enum
from collections import namedtuple
import protobuf.sra_pb2

from joblib import Parallel, delayed

ColumnDef = namedtuple( 'ColumnDef', 'comp level group' )
GroupDef = namedtuple( 'GroupDef', 'comp level cutoff cols' )
TableDef = namedtuple( 'TableDef', 'columns groups' )
DatabaseDef = namedtuple( 'DatabaseDef', 'name tables' )

TableMetaDef = namedtuple( 'TableMetaDef', 'name schema blobmap' )

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
        col = protobuf.sra_pb2.Column()
        # we handle only strings and arrays of int!
        # no other data-type( single int / float / etc )!
        for cell in data:
            pb_cell = protobuf.sra_pb2.Cell()
            if type( cell ) == str:
                pb_cell.str_value = cell
            else: # array of int
                for i in cell:
                    pb_cell.int_values.i.append( i )
            col.cells.append( pb_cell )
        return col.SerializeToString()

    def serialize_blob( self, data ) :
        pb2_blob = protobuf.sra_pb2.Group()
        for k, v in data.items() :
            pb2_blob.names.append( k )
            pb2_blob.encoded_columns.append( v )
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
    outdir ... path to write serialized data-files ( caller created it )
    accession ..... name of the run ( ie SRR000001 )
    schema ... dictionaly of columns, and dictionay of groups
"""

class table_writer:
    def __init__( self, outdir : str, accession : str, schema : TableDef ) :
        self.outdir = outdir
        self.accession = accession
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
        self.meta = TableMetaDef( self.accession, self.schema, dict() )

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

"""
    outdir ... path to write tables ( caller created it, but not the subdirs )
    name ..... name of the run aka accession ( ie SRR000001 )
"""
class db_writer:
    def __init__( self, outdir : str, accession : str ) :
        self.outdir = outdir
        self.accession = accession
        self.schema = []

    def make_table_writer( self, table_name : str, schema : TableDef ) :
        #create a directory for the table
        sub_path = f"{self.outdir}/{table_name}/"
        shutil.rmtree( sub_path, ignore_errors=True )
        os.mkdir( sub_path )
        return table_writer( sub_path, self.accession, schema )
