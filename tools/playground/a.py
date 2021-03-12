#!/usr/bin/env python3

import vdb, pickle, zstd, random

lookup = {
    0 : 'A',
    1 : 'C',
    2 : 'G',
    3 : 'T'
}

def num_to_bases( i ) : 
    a0 = ( i >> 6 ) & 3
    a1 = ( i >> 4 ) & 3
    a2 = ( i >> 2 ) & 3
    a3 = i & 3
    return lookup[ a0 ] + lookup[ a1 ] + lookup[ a2 ] + lookup[ a3 ]

def gen_lookup( ) :
    res = {}
    for i in range( 0, 256 ) :
        res[ i ] = num_to_bases( i )
    return res

def packed_to_str( lookup, packed ) :
    res = ''
    for i in packed[ 2 ] :
        res += lookup[ i ]
        if len( res ) == len( packed[ 2 ] ) * 4 :
            # this is the last byte, eventually use less
            bits_to_use = packed[ 0 ] * packed[ 1 ]
            left = bits_to_use % 8
            if left > 0 :
                to_clip = ( 8 - left ) // 2
                res = res[ :-to_clip ]
    return res

def t1() :
    lookup = gen_lookup()

    mgr = vdb.manager( vdb.OpenMode.Read )
    tbl = mgr.OpenTable( './SRR000001' )

    col_names = [
        "READ",
        "QUALITY",
        "(INSDC:2na:packed)READ"
    ]

    cols = tbl.CreateCursor().OpenColumns( col_names )

    for row in range( 2, 3 ) :
        r1 = cols[ 'READ' ].Read( row )
        print( r1 )

        q = cols[ 'QUALITY' ].Read( row )
        print( q )

        r2 = cols[ '(INSDC:2na:packed)READ' ].Read( row )
        print( r2 )
        s2 = packed_to_str( lookup, r2 )
        print( s2 )
        #if not r1 == s2 :
        #    print( "problem" )

def t2() :
    l = list()
    for x in range( 0, 10000 ) :
        a = bytearray()
        for i in range( 0, 20 ) :
            a.append( random.randrange( 0, 256 ) )
        l.append( a )

    ba = bytearray()
    for x in l :
        for x1 in x :
            ba.append( x1 )

    print( len( ba ) )
    p = pickle.dumps( ba )

    print( len( p ) )
    c = zstd.compress( p, 9 )
    print( len( c ) )

if __name__ == '__main__' :
    t2()
