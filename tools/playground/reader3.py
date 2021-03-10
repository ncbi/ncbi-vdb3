#!/usr/bin/env python3

import sys, os, argparse, run2, random

if __name__ == '__main__' :
    parser = argparse.ArgumentParser()
    parser.add_argument( 'addr', nargs = 1, metavar='URL or path', help='input URL or directory', type=str )
    parser.add_argument( '-U', '--url', help='read from a URL', action='store_true', dest='url', default=False )
    args = parser.parse_args()

    try :
        #print( "reading from : {}".format( args.inputdir[ 0 ] ) )
        mode = run2.AccessMode.URL if args.url else run2.AccessMode.FileSystem
        reader = run2.run_reader( args.addr[0], [ "READ", "QUALITY", "NAME" ], mode )
        total_rows = reader.total_rows

        rows = list( range( 0, total_rows- 1, 10000 ) )
        random.shuffle( rows )

        for start_row in rows :
            loaded = reader.set_window( start_row, 100 )
            for i in range( 0, 10 ) :
                row = random.randint( start_row, start_row + loaded - 1 )
                name = reader.get( row, "NAME" )
                print( f"{reader.name()}.{row+1} {name}")

    except KeyboardInterrupt :
        print( "^C" )
