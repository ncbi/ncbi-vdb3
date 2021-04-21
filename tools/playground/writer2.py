#!/usr/bin/env python3

import os, sys, vdb, argparse, pickle, shutil, write_lib, pickle

def extract_name( full_path : str ) -> str :
    ret = os.path.basename( full_path.strip('/') )
    ret = os.path.splitext( ret )[ 0 ]
    return ret

ReadDef1="READ"
ReadDef2="(INSDC:2na:packed)READ"

def copy_table( tbl, first : int, count : int, outdir : str, accession : str ) -> bool :
    col_names = [
        ReadDef1,
        "READ_LEN",
        "READ_START",
        "READ_TYPE",
        "SPOT_GROUP",
        "(INSDC:quality:text:phred_33)QUALITY",
        "NAME"
    ]

    # specify compression: none(default), zlib, gzip, zstd, ... + level (with a default)
    # can compress entire blobs but saw only ~1% gain if all columns are comressed
    cutoff = 16*1024*1024
    #level=22
    level=3
    compression = ("zstd", level)
    #compression = ("gzip", level) # same as zlib
    #compression = ("bz2", level)
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
        r = cols[ ReadDef1 ].Read( row )
        #in case we are reading packed data ( ReadDef2 ):
        #rp = cols[ ReadDef2 ].Read_bin( row )[ 2 ]
        #writer.write_cell( 'READ', rp, len( rp ) )
        writer.write_cell( 'READ', r, len( r ) )

        rl = cols[ 'READ_LEN' ].Read( row )
        writer.write_cell( 'READ_LEN', rl, len( rl ) )

        rs = cols[ 'READ_START' ].Read( row )
        writer.write_cell( 'READ_START', rs, len( rs ) )

        rt = cols[ 'READ_TYPE' ].Read( row )
        writer.write_cell( 'READ_TYPE', rt, len( rt ) )

        sg = cols[ 'SPOT_GROUP' ].Read( row )
        writer.write_cell( 'SPOT_GROUP', sg, len( sg ) )

        q = cols[ '(INSDC:quality:text:phred_33)QUALITY' ].Read( row )
        writer.write_cell( 'QUALITY', q, len( q ) )

        n = cols[ 'NAME' ].Read( row )
        writer.write_cell( 'NAME', n, len( n ) )

        writer.close_row()

    writer.finish()
    return True

def copy_database( db, outdir : str, accession : str ) -> bool :
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
