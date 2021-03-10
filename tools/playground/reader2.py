#!/usr/bin/env python3

import sys, os, argparse, shutil, run2

if __name__ == '__main__' :
    parser = argparse.ArgumentParser()
    parser.add_argument( 'addr', nargs = 1, metavar='URL or path', help='input URL or directory', type=str )
    parser.add_argument( '-U', '--url', help='read from a URL', action='store_true', dest='url', default=False )
    args = parser.parse_args()

    try :
        #print( "reading from : {}".format( args.inputdir[ 0 ] ) )
        mode = run2.AccessMode.URL if args.url else run2.AccessMode.FileSystem
        reader = run2.run_reader( args.addr[0], [ "READ", "QUALITY", "NAME" ], mode )

        done = False
        start_row = 0
        window_size = 50000

        while not done :
            loaded = reader.set_window( start_row, window_size )
            #sys.stderr.write( f"start_row:{start_row} loaded:{loaded}\n" )
            if loaded < 1 :
                done = True

            for row in range( start_row, start_row + loaded ) :
                read = reader.get( row, "READ" )
                qual = reader.get( row, "QUALITY" )
                name = reader.get( row, "NAME" )
                print( f"@{reader.name()}.{row+1} {name} length={len(read)}")
                print( read )
                print( f"+{reader.name()}.{row+1} {name} length={len(qual)}")
                print( qual )

            start_row += loaded

    except KeyboardInterrupt :
        print( "^C" )
