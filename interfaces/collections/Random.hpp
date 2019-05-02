/*===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 */

#pragma once

#include <cassert>
#include <cstring>
#include <random>
#include <string>
#include <x86intrin.h>
#define __STDC_FORMAT_MACROS
#include <cinttypes>

#undef memcpy // Code never copies overlapping regions

namespace VDB3 {
class Random final // Very fast RNG
{
public:
    /**
     * Fast random number generator
     */

    /**
     * Default constructor, securely seeded
     */
    Random ()
    {
        std::random_device r;
        // returns unsigned int, so we need to grab a few
        state1_ = r ();
        state1_ <<= 32;
        state1_ |= r ();
        state2_ = r ();
        state2_ <<= 32;
        state2_ |= r ();
        next ();
    }

    /**
     * Seeded constructor, useful if repeatability needed
     * @param value seed
     */
    explicit Random ( uint64_t value ) { seed ( value ); }

    /**
     * Reseed
     * @param value seed
     */
    void seed ( uint64_t value )
    {
        state1_ = value;
        state2_ = 1;
        next ();
    }

    /* Return next random value
     * @return uint64_t
     */
    uint64_t operator() () { return next (); }

    /**
     * Returns start <= randint <= stop
     * @param start lower bound
     * @param stop higher bound
     * Fast but biased, see http://www.pcg-random.org/posts/bounded-rands.html
     * Use std::uniform_int_distribution if higher quality is desired
     */
    uint64_t randint ( uint64_t start, uint64_t stop )
    {
        assert ( start <= stop );
        uint64_t r = next ();
        return start + ( r % ( stop - start + 1 ) );
    }

    /// Returns a double between 0.0 and 1.0
    double randdouble ( void )
    {
        uint64_t r = next ();
        return static_cast<double> ( r >> 11u )
            * ( 1.0 / ( UINT64_C ( 1 ) << 53u ) );
    }

    /// Returns random bytes, not ASCII
    std::string randbytes ( size_t num )
    {
        std::string buf;
        buf.reserve ( num );
        while ( num >= sizeof ( uint64_t ) ) {
            uint64_t r = next ();
            buf.append ( reinterpret_cast<const char *> ( &r ), sizeof ( r ) );
            num -= sizeof ( uint64_t );
        }

        while ( num-- ) { buf.push_back ( static_cast<char> ( next () ) ); }

        return buf;
    }

    /// Returns UUID4 string
    std::string uuid4 ()
    {
        std::string uuid; //(36,' ');
        uuid.reserve ( 36 );
        // RFC4122:
        // Set the two most significant bits (bits 6 and 7) of the
        // clock_seq_hi_and_reserved to zero and one, respectively.
        //
        // Set the four most significant bits (bits 12 through 15) of the
        // time_hi_and_version field to the 4-bit version number from
        // Section 4.1.3.
        //
        // Set all the other bits to randomly (or pseudo-randomly) chosen
        // values.

        uint64_t r = next ();
        uint64_t c;

        // time-low(4) '-' time-mid(2) '-' time-high-and-version(2) '-'
        // clock-seq-and-reserve(1) clock-seq-low(1) '-' node(6)

        appendhex ( uuid, r >> 0 );  // time-low0
        appendhex ( uuid, r >> 8 );  // time-low1
        appendhex ( uuid, r >> 16 ); // time-low2
        appendhex ( uuid, r >> 24 ); // time-low3
        uuid += '-';

        appendhex ( uuid, r >> 32 ); // time-mid0
        appendhex ( uuid, r >> 40 ); // time-mid1
        uuid += '-';

        c = r >> 48;
        c |= 0x40;
        c &= 0x4f;
        appendhex ( uuid, c );       // time-high-and0
        appendhex ( uuid, r >> 56 ); // time-high-and1
        uuid += '-';

        r = next ();
        c = r >> 0;
        c |= 0x80;
        c &= 0xbf;
        appendhex ( uuid, c );      // clock-and0
        appendhex ( uuid, r >> 8 ); // clock-seq0
        uuid += '-';

        appendhex ( uuid, r >> 16 ); // node0
        appendhex ( uuid, r >> 24 ); // node1
        appendhex ( uuid, r >> 32 ); // node2
        appendhex ( uuid, r >> 40 ); // node3
        appendhex ( uuid, r >> 48 ); // node4
        appendhex ( uuid, r >> 56 ); // node5

        return uuid;
    }


private:
    void appendhex ( std::string &str, uint64_t w )
    {
        static const char bighex[]
            = "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f"
              "202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f"
              "404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f"
              "606162636465666768696a6b6c6d6e6f707172737475767778797a7b7c7d7e7f"
              "808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9f"
              "a0a1a2a3a4a5a6a7a8a9aaabacadaeafb0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
              "c0c1c2c3c4c5c6c7c8c9cacbcccdcecfd0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
              "e0e1e2e3e4e5e6e7e8e9eaebecedeeeff0f1f2f3f4f5f6f7f8f9fafbfcfdfef"
              "f";
        char buf[16];
        w &= 0xff;
        assert ( w <= 255 );
        memcpy ( buf, &bighex[2 * w], 2 );
        str.append ( buf, 2 );
    }

    uint64_t state1_ = 0, state2_ = 0;

    static inline uint64_t rotl ( const uint64_t x, int k )
    {
        return ( x << k ) | ( x >> ( 64 - k ) );
    }

    uint64_t next ( void )
    {
        // xoroshiro128plus
        const uint64_t s0 = state1_;
        uint64_t s1 = state2_;
        const uint64_t result = s0 + s1;

        s1 ^= s0;
        state1_ = rotl ( s0, 24 ) ^ s1 ^ ( s1 << 16 );
        state2_ = rotl ( s1, 37 );

        return result;
    }
};
} // namespace VDB3
