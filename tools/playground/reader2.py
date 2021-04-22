#!/usr/bin/env python3

import sys, os, argparse, shutil, read_lib

complement = str.maketrans( "ACGT", "TGCA", "" )

def read_database( path, access_mode, parallel_mode, window ) :
    db_reader = read_lib.database_reader( path, access_mode, parallel_mode )
    tbl_reader = db_reader.make_table_reader( 'ALIGN', [ 'READ','QUALITY','SEQ_SPOT_ID','SEQ_READ_ID' ] )

    done = False
    start_row = 0

    while not done :
        loaded = tbl_reader.set_window( start_row, window )
        if loaded < 1 :
            done = True

        accession = tbl_reader.name() # gets it from meta

        for row in range( start_row, start_row + loaded ) :
            read = tbl_reader.get( row, "READ" )
            qual = tbl_reader.get( row, "QUALITY" )
            spot_id = tbl_reader.get( row, "SEQ_SPOT_ID" )[0]
            read_id = tbl_reader.get( row, "SEQ_READ_ID" )[0]
            orient = tbl_reader.get( row, "REF_ORIENTATION" )[0]
            if read == None or qual == None :
                done = True
                sys.exit( 3 )
                break
            if orient > 0 :
                #reverse-compliment READ
                read = read[::-1]
                read = read.translate( complement )
                #reverse QUALITY
                qual = qual[::-1]
            print( f"@{accession}.{spot_id} {spot_id} length={len(read)}")
            print( read )
            print( f"+{accession}.{spot_id} {spot_id} length={len(qual)}")
            print( qual )

        start_row += loaded


#--------------------------------------------------------------------------------------------------------

def read_table( path, access_mode, parallel_mode, window ) :
    reader = read_lib.table_reader( path, [ "READ", "QUALITY", "NAME" ], access_mode, parallel_mode )

    done = False
    start_row = 0

    while not done :
        loaded = reader.set_window( start_row, window )
        if loaded < 1 :
            done = True

        accession = reader.name() # gets it from meta

        for row in range( start_row, start_row + loaded ) :
            read = reader.get( row, "READ" )
            qual = reader.get( row, "QUALITY" )
            name = reader.get( row, "NAME" )
            if read == None or qual == None or name == None :
                done = True
                sys.exit( 3 )
                break
            print( f"@{accession}.{row+1} {name} length={len(read)}")
            print( read )
            print( f"+{accession}.{row+1} {name} length={len(qual)}")
            print( qual )

        start_row += loaded

#--------------------------------------------------------------------------------------------------------

if __name__ == '__main__' :
    parser = argparse.ArgumentParser()
    parser.add_argument( 'addr', nargs = 1, metavar='URL or path', help='input URL or directory', type=str )
    parser.add_argument( '-U', '--url', help='read from a URL', action='store_true', dest='url', default=False )
    parser.add_argument( '-p', '--parallel', help='download column groups in parallel', dest='parallel', default=False, action='store_true' )
    parser.add_argument( '-W', '--window', metavar='window', help='window-size in bytes', type=int, dest='window', default=50000 )
    parser.add_argument( '-D', '--database', help='read a database', dest='db', default=False, action='store_true' )    
    args = parser.parse_args()

    try :
        access_mode = read_lib.AccessMode.URL if args.url else read_lib.AccessMode.FileSystem

        parallel_mode = read_lib.ParallelMode.Sequential
        if args.parallel :
            parallel_mode = read_lib.ParallelMode.Threads

        if args.db :
            read_database( args.addr[ 0 ], access_mode, parallel_mode, args.window )
        else :
            read_table( args.addr[ 0 ], access_mode, parallel_mode, args.window )

    except KeyboardInterrupt :
        print( "^C" )
