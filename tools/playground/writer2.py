#!/usr/bin/env python3

import os, vdb, argparse, pickle, shutil, run2, pickle

def extract_name( full_path : str ) -> str :
    ret = os.path.basename( full_path.strip('/') )
    ret = os.path.splitext( ret )[ 0 ]
    return ret

ReadDef1="READ"
ReadDef2="(INSDC:2na:packed)READ"

def copy_table( tbl, first : int, count : int, outdir : str, name : str ) :
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
    #
    # VDB2 stores reads as 2na packed, we do ASCII text, for now
    #DefaultCutoff = 128 * 1024 * 1024
    cutoff = 16*1024*1024
    tbl_schema = run2.SchemaDef(
        {  # columns
            "READ"          : run2.ColumnDef( "zstd", 9, "g1" ),
            "QUALITY"       : run2.ColumnDef( "zstd", 9, "g3" ),
            "NAME"          : run2.ColumnDef( "zstd", 9, "g3" ),

            "READ_LEN"      : run2.ColumnDef( "zstd", 3, "g2" ),
            "READ_START"    : run2.ColumnDef( "zstd", 3, "g2" ),
            "READ_TYPE"     : run2.ColumnDef( "zstd", 3, "g2" ),
            "SPOT_GROUP"    : run2.ColumnDef( "zstd", 3, "g2" )
        },
        {   # column groups
            "g1" : run2.GroupDef( "zstd", 9, cutoff, [ "READ" ] ),
            "g2" : run2.GroupDef( "zstd", 9, cutoff, [ "READ_LEN", "READ_START", "READ_TYPE", "SPOT_GROUP" ] ),
            "g3" : run2.GroupDef( "zstd", 9, cutoff, [ "QUALITY", "NAME" ] ),
        }
    )

    cols = tbl.CreateCursor().OpenColumns( col_names )

    first_row, row_count = cols[ col_names[ 0 ] ].row_range()
    if first != None : first_row = first
    if count != None : row_count = count

    writer = run2.run_writer( outdir, name, tbl_schema )
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

if __name__ == '__main__' :
    parser = argparse.ArgumentParser()
    parser.add_argument( 'accession', nargs=1, type=str )
    parser.add_argument( '-X', '--first', metavar='row-id', help='first row-id', type=int, dest='first' )
    parser.add_argument( '-N', '--count', metavar='rows', help='how many reads', type=int, dest='count' )
    parser.add_argument( '-R', '--readlib', metavar='path', help='read library', type=str, dest='readlib' )
    parser.add_argument( '-O', '--output', metavar='path', help='output directory', type=str, dest='outdir', default='out' )
    args = parser.parse_args()

    try :
        print( "making a copy of : {}".format( args.accession[ 0 ] ) )

        mgr = vdb.manager( vdb.OpenMode.Read,  args.readlib )

        rd_tbl = None
        pt = mgr.PathType( args.accession[ 0 ] )
        if pt == vdb.PathType.Database :
            rd_tbl = mgr.OpenDB( args.accession[ 0 ] ).OpenTable( "SEQUENCE" ) #object is a database
        elif pt == vdb.PathType.Table :
            rd_tbl = mgr.OpenTable( args.accession[ 0 ] ) #object is a table
        else :
            print( "%s is not an SRA-object"%( args.accession[ 0 ] ) )

        #eventually create the output-dir and clear its content
        shutil.rmtree( args.outdir, ignore_errors=True )
        os.mkdir( args.outdir )

        if rd_tbl != None :
            name = extract_name( args.accession[ 0 ] )
            print( f"name = {name}" )
            copy_table( rd_tbl, args.first, args.count, args.outdir, name )

    except vdb.vdb_error as e :
        print( e )
    except KeyboardInterrupt :
        print( "^C" )
