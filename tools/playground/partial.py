#!/usr/bin/env python3

import http.client, urllib, argparse, random

files = [
( 5248534, "g1.0" ),
( 5247719, "g1.1" ),
( 5248380, "g1.10" ),
( 5251053, "g1.11" ),
( 5250098, "g1.12" ),
( 5248322, "g1.13" ),
( 5251161, "g1.14" ),
( 5254714, "g1.15" ),
( 5253934, "g1.16" ),
( 5249195, "g1.17" ),
( 5255772, "g1.18" ),
( 5250748, "g1.19" ),
( 5250283, "g1.2" ),
( 5254629, "g1.20" ),
( 5252781, "g1.21" ),
( 5249821, "g1.22" ),
( 5247527, "g1.23" ),
( 3519044, "g1.24" ),
( 5247703, "g1.3" ),
( 5251736, "g1.4" ),
( 5253495, "g1.5" ),
( 5249766, "g1.6" ),
( 5240256, "g1.7" ),
( 5235125, "g1.8" ),
( 5250447, "g1.9" ),
( 8643259, "g2.0" ),
( 8654640, "g2.1" ),
( 8537339, "g2.10" ),
( 8505877, "g2.11" ),
( 8537940, "g2.12" ),
( 8554205, "g2.13" ),
( 8531562, "g2.14" ),
( 8550426, "g2.15" ),
( 8520641, "g2.16" ),
( 8575602, "g2.17" ),
( 8563926, "g2.18" ),
( 8574731, "g2.19" ),
( 8650861, "g2.2" ),
( 8543981, "g2.20" ),
( 8583518, "g2.21" ),
( 8523578, "g2.22" ),
( 8558561, "g2.23" ),
( 5734011, "g2.24" ),
( 8649323, "g2.3" ),
( 8695499, "g2.4" ),
( 8670748, "g2.5" ),
( 8682954, "g2.6" ),
( 8665515, "g2.7" ),
( 8551577, "g2.8" ),
( 8522054, "g2.9" ),
( 4088557, "g3.0" ),
( 4177000, "g3.1" ),
( 3749486, "g3.2" ),
( 3704867, "g3.3" ),
( 3715074, "g3.4" ),
( 906690, "g3.5" ),
( 10512, "g4.0" ),
( 1076,  "meta" ),
]

SPECIAL_BASE="https://sra-download.be-md.ncbi.nlm.nih.gov/sos3/access-comp/"

ACC="https://sra-pub-run-odp.s3.amazonaws.com/sra/SRR4156255/SRR4156255"

WHOLE_FILE=( 361647833 , "single_file" )

def make_connection( url ) :
    if url.scheme == "https" :
        return http.client.HTTPSConnection( url.netloc )
    return http.client.HTTPConnection( url.netloc )
    
def dnld_full_files( lst ) :
    url = urllib.parse.urlparse( SPECIAL_BASE + lst[0][1] )
    conn = make_connection( url )
    for count, filename in lst :
        url = urllib.parse.urlparse( SPECIAL_BASE +filename )
        conn.request( "GET", url.path )
        data = conn.getresponse().read()
        print( f"{filename}: filesize = {count}, downloaded = { len(data) }" )

def partial_request( conn, url, start, end ) :
    hdrs = { "Range" : f"bytes={start}-{end}" }
    conn.request( "GET", url.path, headers=hdrs )
    return conn.getresponse().read()    

def dnld_partials( full_url : str, lst ) -> bool :
    url = urllib.parse.urlparse( full_url )
    conn = make_connection( url )
    start = 0
    for count, _ in lst :
        end = start + count - 1
        data = partial_request( conn, url, start, end )
        print( f"start={start}, count={count}, dnld={len(data)}" )
        start += count

def dnld_partials2( full_url : str, lst ) -> bool :
    url = urllib.parse.urlparse( full_url )
    conn = make_connection( url )
    for start, count in lst :
        end = start + count - 1
        data = partial_request( conn, url, start, end )
        print( f"start={start}, count={count}, dnld={len(data)}" )

if __name__ == '__main__' :
    parser = argparse.ArgumentParser()
    parser.add_argument( '-p', '--partial', help='partial reads from a single file',
        dest='partial', default=False, action='store_true' )
    parser.add_argument( '-r', '--random', help='random order reads',
        dest='random', default=False, action='store_true' )
    parser.add_argument( '-a', '--amazon', help='read from amazon',
        dest='amazon', default=False, action='store_true' )

    args = parser.parse_args()

    try :
        if args.partial :
            start = 0
            l = list()
            for count,_ in files :
                l.append( ( start, count ) )
                start += count

            if args.random :
                print( "random partial requests from single file" )
                random.seed()
                random.shuffle( l )
            else :
                print( "sequential partial requests from single file" )

            if args.amazon :
                full_url = ACC 
            else :
                full_url = SPECIAL_BASE + WHOLE_FILE[ 1 ]
            print( f"downloading from {full_url}" )
            dnld_partials2( full_url, l )                
        else :
            if args.random :
                print( "downloading full files in random order" )
                random.seed()
                random.shuffle( files )            
            else :
                print( "downloading full files" )
            dnld_full_files( files )
            
    except KeyboardInterrupt :
        print( "^C" )
