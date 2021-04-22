#!/usr/bin/env python3

import os, sys, vdb, argparse, pickle, shutil, write_lib, pickle

def extract_name( full_path : str ) -> str :
    ret = os.path.basename( full_path.strip('/') )
    ret = os.path.splitext( ret )[ 0 ]
    return ret

ReadDef1="READ"
ReadDef2="(INSDC:2na:packed)READ"
QualDef="(INSDC:quality:text:phred_33)QUALITY"

cutoff = 16*1024*1024

# specify compression: none(default), zlib, gzip, zstd, ... + level (with a default)
# can compress entire blobs but saw only ~1% gain if all columns are comressed
level = 3
compression = ( "zstd", level )

def copy_cell( src, dst, row, src_name, dst_name ) :
    data = src[ src_name ].Read( row )
    dst.write_cell( dst_name, data, len( data ) )

def copy_bool_cell( src, dst, row, src_name, dst_name ) :
    b = src[ src_name ].Read( row )
    if b[ 0 ] :
        dst.write_cell( dst_name, [1], 1 )
    else :
        dst.write_cell( dst_name, [0], 1 )

def copy_table( tbl, first : int, count : int, outdir : str, accession : str ) -> bool :
    col_names = [
        ReadDef1,
        "READ_LEN",
        "READ_START",
        "READ_TYPE",
        "SPOT_GROUP",
        QualDef,
        "NAME"
    ]

    tbl_schema = write_lib.TableDef(
        {  # columns
            "READ"          : write_lib.ColumnDef( compression[0], compression[1], "g1" ),
            "QUALITY"       : write_lib.ColumnDef( compression[0], compression[1], "g2" ),
            "NAME"          : write_lib.ColumnDef( compression[0], compression[1], "g3" ),

            "READ_LEN"      : write_lib.ColumnDef( compression[0], compression[1], "g4" ),
            "READ_START"    : write_lib.ColumnDef( compression[0], compression[1], "g4" ),
            "READ_TYPE"     : write_lib.ColumnDef( compression[0], compression[1], "g4" ),
            "SPOT_GROUP"    : write_lib.ColumnDef( compression[0], compression[1], "g4" )
        },
        {   # column groups
            "g1" : write_lib.GroupDef( compression[0], compression[1], cutoff, [ "READ" ] ),
            "g2" : write_lib.GroupDef( compression[0], compression[1], cutoff, [ "QUALITY" ] ),
            "g3" : write_lib.GroupDef( compression[0], compression[1], cutoff, [ "NAME" ] ),
            "g4" : write_lib.GroupDef( compression[0], compression[1], cutoff, [ "READ_LEN", "READ_START", "READ_TYPE", "SPOT_GROUP" ] ),
        }
    )

    cols = tbl.CreateCursor().OpenColumns( col_names )

    first_row, row_count = cols[ col_names[ 0 ] ].row_range()
    if first != None : first_row = first
    if count != None : row_count = count

    writer = write_lib.table_writer( outdir, accession, tbl_schema )
    for row in vdb.xrange( first_row, first_row + row_count ) :
        copy_cell( cols, writer, row, ReadDef1, 'READ' )
        copy_cell( cols, writer, row, 'READ_LEN', 'READ_LEN' )
        copy_cell( cols, writer, row, 'READ_START', 'READ_START' )
        copy_cell( cols, writer, row, 'READ_TYPE', 'READ_TYPE' )
        copy_cell( cols, writer, row, 'SPOT_GROUP', 'SPOT_GROUP' )
        copy_cell( cols, writer, row, QualDef, 'QUALITY' )
        copy_cell( cols, writer, row, 'NAME', 'NAME' )
        writer.close_row()

    writer.finish()
    return True

def copy_database( db, outdir : str, accession : str ) -> bool :
    # 1.step copy the alignment-table ( READ, QUALITY, SEQ_ID, READ_ID )
    col_names = [ ReadDef1, QualDef, "SEQ_SPOT_ID", "SEQ_READ_ID", "REF_ORIENTATION" ]
    tbl_schema = write_lib.TableDef(
        {  # columns
            "READ"          :    write_lib.ColumnDef( compression[0], compression[1], "g1" ),
            "QUALITY"           : write_lib.ColumnDef( compression[0], compression[1], "g2" ),
            "SEQ_SPOT_ID"       : write_lib.ColumnDef( compression[0], compression[1], "g3" ),
            "SEQ_READ_ID"       : write_lib.ColumnDef( compression[0], compression[1], "g3" ),
            "REF_ORIENTATION"   : write_lib.ColumnDef( compression[0], compression[1], "g3" ),
        },
        {   # column groups
            "g1" : write_lib.GroupDef( compression[0], compression[1], cutoff, [ "READ" ] ),
            "g2" : write_lib.GroupDef( compression[0], compression[1], cutoff, [ "QUALITY" ] ),
            "g3" : write_lib.GroupDef( compression[0], compression[1], cutoff, [ "SEQ_SPOT_ID", "SEQ_READ_ID", "REF_ORIENTATION" ] )
        }
    )
    tbl = db.OpenTable( "PRIMARY_ALIGNMENT" )
    cols = tbl.CreateCursor().OpenColumns( col_names )
    first_row, row_count = cols[ col_names[ 0 ] ].row_range()

    db_writer = write_lib.db_writer( outdir, accession )
    if db_writer != None :
        tbl_writer = db_writer.make_table_writer( "ALIGN", tbl_schema )
        if tbl_writer != None :
            for row in vdb.xrange( first_row, first_row + row_count ) :
                copy_cell( cols, tbl_writer, row, ReadDef1, 'READ' )
                copy_cell( cols, tbl_writer, row, QualDef, 'QUALITY' )
                copy_cell( cols, tbl_writer, row, 'SEQ_SPOT_ID', 'SEQ_SPOT_ID' )
                copy_cell( cols, tbl_writer, row, 'SEQ_READ_ID', 'SEQ_READ_ID' )
                copy_bool_cell( cols, tbl_writer, row, 'REF_ORIENTATION', 'REF_ORIENTATION' )
                tbl_writer.close_row()

            tbl_writer.finish()
            return True
        db_writer.finish()
    return False

def process_table( args, mgr ) -> bool :
    tbl = mgr.OpenTable( args.accession[ 0 ] )
    if tbl != None :
        #create the output-dir and clear its content
        shutil.rmtree( args.outdir, ignore_errors=True )
        os.mkdir( args.outdir )
        accession = extract_name( args.accession[ 0 ] )
        return copy_table( tbl, args.first, args.count, args.outdir, accession )
    return False

def process_database( args, mgr ) -> int :
    db = mgr.OpenDB( args.accession[ 0 ] )
    if db != None :
        tablelist = db.ListTbl()
        if 'SEQUENCE' in tablelist and 'PRIMARY_ALIGNMENT' in tablelist :
            shutil.rmtree( args.outdir, ignore_errors=True )
            os.mkdir( args.outdir )
            accession = extract_name( args.accession[ 0 ] )
            return copy_database( db, args.outdir, accession )
        else:
            print( "this db cannot be processed" )
            return False
    return False


if __name__ == '__main__' :
    parser = argparse.ArgumentParser()
    parser.add_argument( 'accession', nargs=1, type=str )
    parser.add_argument( '-X', '--first', metavar='row-id', help='first row-id', type=int, dest='first' )
    parser.add_argument( '-N', '--count', metavar='rows', help='how many reads', type=int, dest='count' )
    parser.add_argument( '-R', '--readlib', metavar='path', help='read library', type=str, dest='readlib' )
    parser.add_argument( '-O', '--output', metavar='path', help='output directory', type=str, dest='outdir', default='out' )
    args = parser.parse_args()

    success = False
    try :
        print( "making a copy of : {}".format( args.accession[ 0 ] ) )

        mgr = vdb.manager( vdb.OpenMode.Read,  args.readlib )
        pt = mgr.PathType( args.accession[ 0 ] )
        if pt == vdb.PathType.Database :
            success = process_database( args, mgr )
        elif pt == vdb.PathType.Table :
            success = process_table( args, mgr )
        else :
            print( f"{args.accession[ 0 ]} is not an SRA-object ( {pt} )" )

    except vdb.vdb_error as e :
        print( e )
        sys.exit( 2 )
    except KeyboardInterrupt :
        print( "^C" )
        sys.exit( 3 )

    if success :
        sys.exit( 0 )
    sys.exit( 1 )
