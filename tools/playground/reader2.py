#!/usr/bin/env python3

import sys, os, argparse, shutil, read_lib

if __name__ == '__main__' :
    parser = argparse.ArgumentParser()
    parser.add_argument( 'addr', nargs = 1, metavar='URL or path', help='input URL or directory', type=str )
    parser.add_argument( '-U', '--url', help='read from a URL', action='store_true', dest='url', default=False )
    parser.add_argument( '-p', '--parallel', help='download column groups in parallel', dest='parallel', default=False, action='store_true' )
    parser.add_argument( '-W', '--window', metavar='window', help='window-size in bytes', type=int, dest='window', default=50000 )
    args = parser.parse_args()

    try :
        access_mode = read_lib.AccessMode.URL if args.url else read_lib.AccessMode.FileSystem

        parallel_mode = read_lib.ParallelMode.Sequential
        if args.parallel :
            parallel_mode = read_lib.ParallelMode.Threads

        reader = read_lib.table_reader( args.addr[0], [ "READ", "QUALITY", "NAME" ], access_mode, parallel_mode )

        done = False
        start_row = 0

        while not done :
            loaded = reader.set_window( start_row, args.window )
            if loaded < 1 :
                done = True

            for row in range( start_row, start_row + loaded ) :
                read = reader.get( row, "READ" )
                qual = reader.get( row, "QUALITY" )
                name = reader.get( row, "NAME" )
                if read == None or qual == None or name == None :
                    done = True
                    sys.exit( 3 )
                    break
                print( f"@{reader.name()}.{row+1} {name} length={len(read)}")
                print( read )
                print( f"+{reader.name()}.{row+1} {name} length={len(qual)}")
                print( qual )

            start_row += loaded

    except KeyboardInterrupt :
        print( "^C" )
