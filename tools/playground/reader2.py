#!/usr/bin/env python3

import os, argparse, shutil, run2

if __name__ == '__main__' :
    parser = argparse.ArgumentParser()
    parser.add_argument( 'addr', nargs = 1, metavar='URL or path', help='input URL or directory', type=str )
    parser.add_argument( '-N', '--count', metavar='rows', help='how many reads', type=int, dest='count' )
    parser.add_argument( '-U', '--url', help='read from a URL', action='store_true', dest='url', default=False )
    args = parser.parse_args()

    try :
        #print( "reading from : {}".format( args.inputdir[ 0 ] ) )
        mode = run2.AccessMode.URL if args.url else run2.AccessMode.FileSystem
        reader = run2.run_reader( args.addr[0], [ "READ", "QUALITY", "NAME" ], mode )
        done = False
        row_count = 0
        while not done :
            if args.count != None and row_count >= args.count :
                break

            if not reader.next_row() :
                break

            row_count += 1

            read = reader.get("READ")
            qual = reader.get("QUALITY")
            name = reader.get("NAME")
            print( f"@{reader.name()}.{row_count} {name} length={len(read)}")
            print( read )
            print( f"+{reader.name()}.{row_count} {name} length={len(qual)}")
            print( qual )

    except KeyboardInterrupt :
        print( "^C" )
