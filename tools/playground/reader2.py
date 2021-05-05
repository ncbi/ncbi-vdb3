#!/usr/bin/env python3

import sys, os, argparse, shutil, read_lib
import threading, subprocess

complement = str.maketrans( "ACGT", "TGCA", "" )

def read_aligned( db_reader, window : int, rows : int, timing : bool, just_download: bool ) :
    tbl_reader = db_reader.make_table_reader( 'ALIGN', [ 'READ','QUALITY','SEQ_SPOT_ID','SEQ_READ_ID','REF_ORIENTATION' ], just_download )
    if tbl_reader == None :
        print( "error: read_aligned().make_table_reader() failed", file=sys.stderr, flush=True )
        sys.exit( 3 )
    done = False
    start_row = 0
    while not done :
        loaded = tbl_reader.set_window( start_row, window )
        if loaded < 1 :
            done = True
        accession = tbl_reader.name() # gets it from meta
        for row in range( start_row, start_row + loaded ) :
            if row > rows :
                done = True
                break
            if just_download :
                continue

            read = tbl_reader.get( row, "READ" )
            qual = tbl_reader.get( row, "QUALITY" )
            spot_id = tbl_reader.get( row, "SEQ_SPOT_ID" )
            #read_id = tbl_reader.get( row, "SEQ_READ_ID" )[0]
            orient = tbl_reader.get( row, "REF_ORIENTATION" )
            if read == None :
                print( f"error: read_aligned().get(READ) failed in row #{row}", file=sys.stderr, flush=True )
                done = True
                sys.exit( 3 )
                break
            if qual == None :
                print( f"error: read_aligned().get(QUALITY) failed in row #{row}", file=sys.stderr, flush=True )
                done = True
                sys.exit( 3 )
                break
            if spot_id == None :
                print( f"error: read_aligned().get(SEQ_SPOT_ID) failed in row #{row}", file=sys.stderr, flush=True )
                done = True
                sys.exit( 3 )
                break
            else :
                spot_id = spot_id[ 0 ]
            if orient == None :
                print( f"error: read_aligned().get(REF_ORIENTATION) failed in row #{row}", file=sys.stderr, flush=True )
                done = True
                sys.exit( 3 )
                break
            else :
                orient = orient[ 0 ]

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

    if timing :
        return tbl_reader.report_times( True )

def read_unaligned( db_reader, window : int, rows : int, timing : bool, just_download: bool ) :
    tbl_reader = db_reader.make_table_reader( 'UNALIGN', [ 'READ','QUALITY','SEQ_SPOT_ID','SEQ_READ_ID' ], just_download )
    if tbl_reader == None :
        print( "error: read_aligned().make_table_reader() failed", file=sys.stderr, flush=True )
    done = False
    start_row = 0
    while not done :
        loaded = tbl_reader.set_window( start_row, window )
        if loaded < 1 :
            done = True
        accession = tbl_reader.name() # gets it from meta
        for row in range( start_row, start_row + loaded ) :
            if row > rows :
                done = True
                break
            if just_download :
                continue

            read = tbl_reader.get( row, "READ" )
            qual = tbl_reader.get( row, "QUALITY" )
            spot_id = tbl_reader.get( row, "SEQ_SPOT_ID" )[0]
            #read_id = tbl_reader.get( row, "SEQ_READ_ID" )[0]
            if read == None or qual == None :
                done = True
                sys.exit( 3 )
                break
            print( f"@{accession}.{spot_id} {spot_id} length={len(read)}")
            print( read )
            print( f"+{accession}.{spot_id} {spot_id} length={len(qual)}")
            print( qual )
        start_row += loaded

    if timing :
        return tbl_reader.report_times( True )

def read_reference( db_reader, window : int, rows : int, timing : bool, just_download: bool ) :
    tbl_reader = db_reader.make_table_reader( 'REFERENCE', [ "READ", "SEQ_ID", "SEQ_START" ], just_download )
    if tbl_reader == None :
        print( "error: read_aligned().make_table_reader() failed", file=sys.stderr, flush=True )
    done = False
    start_row = 0
    total = 0
    while not done :
        loaded = tbl_reader.set_window( start_row, window )
        if loaded < 1 :
            done = True
        for row in range( start_row, start_row + loaded ) :
            if row > rows :
                done = True
                break
            if just_download :
                continue
            total += len( tbl_reader.get( row, "READ" ) )
            tbl_reader.get( row, "SEQ_ID" )
            tbl_reader.get( row, "SEQ_START" )
        start_row += loaded

    if timing :
        return tbl_reader.report_times( True )

def read_database( path, access_mode, parallel_mode, window : int, rows : int, timing : bool, just_download: bool ) :
    db_reader = read_lib.database_reader( path, access_mode, parallel_mode )
    t1 = read_aligned( db_reader, window, rows, timing, just_download )
    t2 = read_unaligned( db_reader, window, rows, timing, just_download )
    if timing :
        print( "download {:.2f}".format(t1+t2), file=sys.stderr, flush=True )

def read_database_ref( path, access_mode, parallel_mode, window : int, rows : int, timing : bool, just_download: bool ) :
    db_reader = read_lib.database_reader( path, access_mode, parallel_mode )
    total = read_reference( db_reader, window, rows, timing, just_download )
    if timing :
        print( "download {:.2f}".format(total), file=sys.stderr, flush=True )

#--------------------------------------------------------------------------------------------------------

def read_table( path, access_mode, parallel_mode, window : int, rows : int, timing : bool, just_download: bool ) :
    reader = read_lib.table_reader( path, [ "READ", "QUALITY", "NAME" ], access_mode, parallel_mode, just_download )

    done = False
    start_row = 0

    while not done :
        loaded = reader.set_window( start_row, window )
        if loaded < 1 :
            done = True

        accession = reader.name() # gets it from meta

        for row in range( start_row, start_row + loaded ) :
            if row > rows :
                done = True
                break
            if just_download :
                continue
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

    if timing :
        return reader.report_times( True )

#--------------------------------------------------------------------------------------------------------

if __name__ == '__main__' :
    parser = argparse.ArgumentParser()
    parser.add_argument( 'addr', nargs = 1, metavar='URL or path', help='input URL or directory', type=str )
    parser.add_argument( '-U', '--url', help='read from a URL', action='store_true', dest='url', default=False )
    parser.add_argument( '-p', '--parallel', help='download column groups in parallel', dest='parallel', default=False, action='store_true' )
    parser.add_argument( '-W', '--window', metavar='window', help='window-size in bytes', type=int, dest='window', default=50000 )
    parser.add_argument( '-N', '--rows', metavar='rows', help='stop after x rows', type=int, dest='rows', default=sys.maxsize )
    parser.add_argument( '-D', '--database', help='read a database', dest='db', default=False, action='store_true' )
    parser.add_argument( '-R', '--reference', help='read reference table in a thread', dest='ref', default=False, action='store_true' )
    parser.add_argument( '-T', '--timing', help='report timing', dest='timing', default=False, action='store_true' )
    parser.add_argument( '-J', '--just-download', help='just download', dest='download', default=False, action='store_true' )

    args = parser.parse_args()

    try :
        access_mode = read_lib.AccessMode.URL if args.url else read_lib.AccessMode.FileSystem

        parallel_mode = read_lib.ParallelMode.Sequential
        if args.parallel :
            parallel_mode = read_lib.ParallelMode.Threads

        if args.db :
            read_database( args.addr[ 0 ], access_mode, parallel_mode, args.window, args.rows, args.timing, args.download )
        elif args.ref :
            read_database_ref( args.addr[ 0 ], access_mode, parallel_mode, args.window, args.rows, args.timing, args.download )
        else :
            read_table( args.addr[ 0 ], access_mode, parallel_mode, args.window, args.rows, args.timing, args.download )

    except KeyboardInterrupt :
        print( "^C" )
