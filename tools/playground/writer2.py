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

def copy_aligned( db, db_writer ) -> bool :
    # copy the alignment-table ( READ, QUALITY, SEQ_ID, READ_ID, ORIENTATION )
    tbl_schema = write_lib.TableDef(
        {  # columns
            "READ"          :    write_lib.ColumnDef( compression[0], compression[1], "g1" ),
            "QUALITY"           : write_lib.ColumnDef( compression[0], compression[1], "g2" ),
            "SEQ_SPOT_ID"       : write_lib.ColumnDef( compression[0], compression[1], "g3" ),
            "SEQ_READ_ID"       : write_lib.ColumnDef( compression[0], compression[1], "g3" ),
            "REF_ORIENTATION"   : write_lib.ColumnDef( compression[0], compression[1], "g3" ),
            "SEQ_NAME"   : write_lib.ColumnDef( compression[0], compression[1], "g4" ),
            "REF_POS"   : write_lib.ColumnDef( compression[0], compression[1], "g4" ),
        },
        {   # column groups
            "g1" : write_lib.GroupDef( compression[0], compression[1], cutoff, [ "READ" ] ),
            "g2" : write_lib.GroupDef( compression[0], compression[1], cutoff, [ "QUALITY" ] ),
            "g3" : write_lib.GroupDef( compression[0], compression[1], cutoff, [ "SEQ_SPOT_ID", "SEQ_READ_ID", "REF_ORIENTATION" ] ),
            "g4" : write_lib.GroupDef( compression[0], compression[1], cutoff, [ "SEQ_NAME", "REF_POS" ] ),
        }
    )

    tbl = db.OpenTable( "PRIMARY_ALIGNMENT" )
    col_names = [ ReadDef1, QualDef, "SEQ_SPOT_ID", "SEQ_READ_ID", "REF_ORIENTATION", "SEQ_NAME", "REF_POS" ]
    cols = tbl.CreateCursor().OpenColumns( col_names )
    first_row, row_count = cols[ col_names[ 0 ] ].row_range()

    tbl_writer = db_writer.make_table_writer( "ALIGN", tbl_schema )
    if tbl_writer != None :
        for row in vdb.xrange( first_row, first_row + row_count ) :
            copy_cell( cols, tbl_writer, row, ReadDef1, 'READ' )
            copy_cell( cols, tbl_writer, row, QualDef, 'QUALITY' )
            copy_cell( cols, tbl_writer, row, 'SEQ_SPOT_ID', 'SEQ_SPOT_ID' )
            copy_cell( cols, tbl_writer, row, 'SEQ_READ_ID', 'SEQ_READ_ID' )
            copy_bool_cell( cols, tbl_writer, row, 'REF_ORIENTATION', 'REF_ORIENTATION' )
            copy_cell( cols, tbl_writer, row, 'SEQ_NAME', 'SEQ_NAME' )
            copy_cell( cols, tbl_writer, row, 'REF_POS', 'REF_POS' )
            tbl_writer.close_row()

        tbl_writer.finish()
        return True
    return False

def copy_unaligned( db, db_writer ) -> bool :
    # copy the unaligned reads from the sequence-table ( READ, QUALITY, SEQ_ID, READ_ID )
    tbl_schema = write_lib.TableDef(
        {  # columns
            "READ"              : write_lib.ColumnDef( compression[0], compression[1], "g1" ),
            "QUALITY"           : write_lib.ColumnDef( compression[0], compression[1], "g2" ),
            "SEQ_SPOT_ID"       : write_lib.ColumnDef( compression[0], compression[1], "g3" ),
            "SEQ_READ_ID"       : write_lib.ColumnDef( compression[0], compression[1], "g3" ),
        },
        {   # column groups
            "g1" : write_lib.GroupDef( compression[0], compression[1], cutoff, [ "READ" ] ),
            "g2" : write_lib.GroupDef( compression[0], compression[1], cutoff, [ "QUALITY" ] ),
            "g3" : write_lib.GroupDef( compression[0], compression[1], cutoff, [ "SEQ_SPOT_ID", "SEQ_READ_ID" ] )
        }
    )
    tbl = db.OpenTable( "SEQUENCE" )
    col_names = [ ReadDef1, QualDef, "PRIMARY_ALIGNMENT_ID", "READ_LEN", "READ_START" ]
    cols = tbl.CreateCursor().OpenColumns( col_names )
    first_row, row_count = cols[ col_names[ 0 ] ].row_range()

    tbl_writer = db_writer.make_table_writer( "UNALIGN", tbl_schema )
    if tbl_writer != None :
        for row in vdb.xrange( first_row, first_row + row_count ) :
            prim_al_ids = cols["PRIMARY_ALIGNMENT_ID" ].Read( row )
            if len( prim_al_ids ) == 2 and ( prim_al_ids[ 0 ] == 0 or prim_al_ids[ 1 ] == 0 ) :
                #if either of the prim-align-ids is zero, we have unaligned reads in this row
                read = cols[ ReadDef1 ].Read( row )
                qual = cols[ QualDef ].Read( row )
                read_start = cols[ "READ_START" ].Read( row )
                read_len = cols[ "READ_LEN" ].Read( row )
                if prim_al_ids[ 0 ] == 0 :
                    #the first read is unaligned
                    start = read_start[ 0 ]
                    end = start + read_len[ 0 ]
                    read_0 = read[ start : end ]
                    qual_0 = qual[ start : end ]
                    tbl_writer.write_cell( "READ", read_0, len( read_0 ) )
                    tbl_writer.write_cell( "QUALITY", qual_0, len( qual_0 ) )
                    tbl_writer.write_cell( "SEQ_SPOT_ID", [ row ], 1 )
                    tbl_writer.write_cell( "SEQ_READ_ID", [ 1 ], 1 )
                    tbl_writer.close_row()
                if prim_al_ids[ 1 ] == 0 :
                    #the second read is unaligned
                    start = read_start[ 1 ]
                    end = start + read_len[ 1 ]
                    read_1 = read[ start : end ]
                    qual_1 = qual[ start : end ]
                    tbl_writer.write_cell( "READ", read_1, len( read_1 ) )
                    tbl_writer.write_cell( "QUALITY", qual_1, len( qual_1 ) )
                    tbl_writer.write_cell( "SEQ_SPOT_ID", [ row ], 1 )
                    tbl_writer.write_cell( "SEQ_READ_ID", [ 2 ], 1 )
                    tbl_writer.close_row()
        tbl_writer.finish()
        return True
    return False

def copy_ref( db, db_writer ) -> bool :
    # copy reference table ( READ, SEQ_ID, SEQ_START )
    # TODO: optionally include PRIMARY_ALIGNMENT_IDS / SECONDARY_ALIGNMENT_IDS
    tbl_schema = write_lib.TableDef(
        {  # columns
            "READ"              : write_lib.ColumnDef( compression[0], compression[1], "g1" ),
            "SEQ_ID"            : write_lib.ColumnDef( compression[0], compression[1], "g2" ),
            "SEQ_START"            : write_lib.ColumnDef( compression[0], compression[1], "g2" ),
        },
        {   # column groups
            "g1" : write_lib.GroupDef( compression[0], compression[1], cutoff, [ "READ" ] ),
            "g2" : write_lib.GroupDef( compression[0], compression[1], cutoff, [ "SEQ_ID", "SEQ_START" ] ),
        }
    )

    tbl = db.OpenTable( "REFERENCE" )
    col_names = [ "READ", "SEQ_ID", "SEQ_START" ]
    cols = tbl.CreateCursor().OpenColumns( col_names )
    first_row, row_count = cols[ col_names[ 0 ] ].row_range()

    tbl_writer = db_writer.make_table_writer( "REFERENCE", tbl_schema )
    if tbl_writer != None :
        for row in vdb.xrange( first_row, first_row + row_count ) :
            copy_cell( cols, tbl_writer, row, 'READ', 'READ' )
            copy_cell( cols, tbl_writer, row, 'SEQ_ID', 'SEQ_ID' )
            copy_cell( cols, tbl_writer, row, 'SEQ_START', 'SEQ_START' )
            tbl_writer.close_row()
        tbl_writer.finish()
        return True
    return False

def copy_database( db, outdir : str, accession : str ) -> bool :
    db_writer = write_lib.db_writer( outdir, accession )
    if db_writer == None :
        return False
    if not copy_aligned( db, db_writer ) :
        return False
    if not copy_unaligned( db, db_writer ) :
        return False
    if not copy_ref( db, db_writer ) :
        return False

    db_writer.finish()
    return True

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
