#!/usr/bin/env python3

import os, argparse, shutil, run2

if __name__ == '__main__' :
    parser = argparse.ArgumentParser()
    parser.add_argument( 'inputdir', nargs = 1, metavar='path', help='input directory', type=str )
    parser.add_argument( '-N', '--count', metavar='rows', help='how many reads', type=int, dest='count' )
    args = parser.parse_args()

    try :
        #print( "reading from : {}".format( args.inputdir[ 0 ] ) )

        reader = run2.run_reader( args.inputdir[0] )
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
