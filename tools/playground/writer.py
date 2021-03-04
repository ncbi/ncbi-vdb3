#!/usr/bin/env python3

import os, vdb, argparse, pickle, shutil, run

def copy_table( tbl, first : int, count : int, cutoff : int, outdir : str, name : str ) :
    col_names = [ "READ", "(INSDC:quality:text:phred_33)QUALITY", "NAME" ]
    col_schema = [ "READ", "QUALITY", "NAME" ]

    cols = tbl.CreateCursor().OpenColumns( col_names )

    first_row, row_count = cols[ col_names[ 0 ] ].row_range()
    if first != None : first_row = first
    if count != None : row_count = count

    writer = run.run_writer( outdir, name, cutoff, col_schema )
    for row in vdb.xrange( first_row, first_row + row_count ) :
        r = cols[ 'READ' ].Read( row )
        q = cols[ '(INSDC:quality:text:phred_33)QUALITY' ].Read( row )
        n = cols[ 'NAME' ].Read( row )
        bytes_written = ( len( r ) + len( q ) + len( n ) )
        writer.append_row( ( r, q, n ), bytes_written )
    writer.close_writing()

if __name__ == '__main__' :
    parser = argparse.ArgumentParser()
    parser.add_argument( 'accession', nargs=1, type=str )
    parser.add_argument( '-X', '--first', metavar='row-id', help='first row-id', type=int, dest='first' )
    parser.add_argument( '-N', '--count', metavar='rows', help='how many reads', type=int, dest='count' )
    parser.add_argument( '-R', '--readlib', metavar='path', help='read library', type=str, dest='readlib' )
    parser.add_argument( '-C', '--cutoff', metavar='size', help='bytes per blob-group', type=int, dest='cutoff', default=16*1024*1024 )
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
            copy_table( rd_tbl, args.first, args.count, args.cutoff, args.outdir, args.accession[ 0 ] )

    except vdb.vdb_error as e :
        print( e )
    except KeyboardInterrupt :
        print( "^C" )
