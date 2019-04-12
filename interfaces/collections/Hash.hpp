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
#include <cmath>
#include <cstdbool>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <endian.h>
#include <string>
#include <x86intrin.h>
#define __STDC_FORMAT_MACROS
#include <cinttypes>

#undef memcpy

#if __BYTE_ORDER == __BIG_ENDIAN
#error "Big endian not tested"
#endif

#define UNLIKELY( x ) __builtin_expect ( !!( x ), 0 )
#define LIKELY( x ) __builtin_expect ( !!( x ), 1 )

namespace VDB3 {
// Fast, locality preserving, not cryptographically secure hash function.
// Platform depedendent, will not give same results on big-endian.
// References:
// * https://github.com/rurban/smhasher/
// * https://bigdata.uni-saarland.de/publications/p249-richter.pdf
uint64_t Hash ( const char *s, size_t len ) noexcept
    __attribute__ ( ( const ) );
uint64_t Hash ( const std::string &str ) noexcept __attribute__ ( ( const ) );
uint64_t Hash ( int i ) noexcept __attribute__ ( ( const ) );
uint64_t Hash ( long int i ) noexcept __attribute__ ( ( const ) );
uint64_t Hash ( unsigned long long l ) noexcept __attribute__ ( ( const ) );
uint64_t Hash ( uint64_t l ) noexcept __attribute__ ( ( const ) );
uint64_t Hash ( float f ) noexcept __attribute__ ( ( const ) );
uint64_t Hash ( double d ) noexcept __attribute__ ( ( const ) );


static inline uint64_t rotr ( const uint64_t x, int k )
{
    return ( x << ( 64 - k ) | ( x >> k ) );
}

/*
static uint64_t hashmix (
    uint64_t hash, uint64_t high, uint64_t low, uint64_t mul )
{
    uint64_t masked = low & 0xfcu // locality preserving
    uint64_t h1 = ( high + masked ) * mul;
    uint64_t h2 = rotr ( h1, 47U );
    return hash + h2 + low * 2; // locality restricting
}
*/
uint64_t Hash ( const char *s, size_t len ) noexcept
{
    static const uint64_t lowbits = 0xfcffffffffffffffu;
    // Some primes between 2^63 and 2^64 for various uses.
    static const uint64_t k0 = 0xc3a5c85c97cb3127;
    // static const uint64_t k1 = 0xb492b66fbe98f273;
    // static const uint64_t k2 = 0x9ae16a3b2f90404f;

    uint64_t hash = 0; // maybe seed?

    if ( UNLIKELY ( len >= 32 ) ) {
        // High bandwidth mode
        __m128i h1 = _mm_setzero_si128 ();
        __m128i h2 = _mm_setzero_si128 ();
        __m128i h3 = _mm_setzero_si128 ();
        while ( len >= 32 ) {
            len -= 32;
            const __m128i *s128 = reinterpret_cast<const __m128i *> ( s );
            h1 += _mm_loadu_si128 ( s128 );
            h2 += _mm_loadu_si128 ( s128 + 1 );
            h3 += _mm_aesenc_si128 ( h1, h2 );
            s += 32;
        }
        h3 = _mm_aesenc_si128 ( h3, h1 );
        h3 = _mm_aesenc_si128 ( h3, h2 );
        hash = static_cast<uint64_t> ( _mm_cvtsi128_si64 ( h3 ) );
        // fprintf ( stderr, "bighash is %lx\n", hash );
    }

    assert ( len < 32 );

    // Goal is to have only one unpredictable branch and to use the minimum
    // number of cache loads
    // @FIX: g++ simply won't believe 0<=len<=31
    switch ( len & 31 ) {
    case 0: {
        return hash;
    }
    case 1: {
        uint8_t b1;
        memcpy ( &b1, s, sizeof ( b1 ) );
        hash += b1 * 2u;
        return hash;
    }
    case 2: {
        uint16_t w1;
        memcpy ( &w1, s, sizeof ( w1 ) );
        const uint8_t b1 = static_cast<uint8_t> ( w1 >> 8 );
        w1 &= static_cast<uint16_t> ( lowbits >> 48 );
        hash += static_cast<uint64_t> ( w1 ) * k0;
        hash *= k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    case 3: {
        uint16_t w1;
        uint16_t w2;
        memcpy ( &w1, s, sizeof ( w1 ) );
        memcpy ( &w2, s + 1, sizeof ( w2 ) );
        const uint8_t b1 = static_cast<uint8_t> ( w2 >> 8 );
        w2 &= static_cast<uint16_t> ( lowbits >> 48 );
        hash += static_cast<uint64_t> ( w1 + w2 ) * k0;
        hash *= k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    case 4: {
        uint32_t l1;
        memcpy ( &l1, s, sizeof ( l1 ) );
        const uint8_t b1 = static_cast<uint8_t> ( l1 >> 24 );
        l1 &= static_cast<uint32_t> ( lowbits >> 32 );
        hash += l1 * k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    case 5: {
        uint32_t l1, l2;
        memcpy ( &l1, s, sizeof ( l1 ) );
        memcpy ( &l2, s + 1, sizeof ( l2 ) );
        const uint8_t b1 = static_cast<uint8_t> ( l2 >> 24 );
        // printf ( "l1 is %x\nl2 is %x,b1=%x\n", l1, l2, b1 );
        l2 &= static_cast<uint32_t> ( lowbits >> 32 );
        // printf ( "l2 is now %x,b1=%x\n", l2, b1 );
        hash += ( l1 + l2 ) * k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        // printf ( "hash is %lx\n", hash );
        return hash;
    }
    case 6: {
        uint32_t l1, l2;
        memcpy ( &l1, s, sizeof ( l1 ) );
        memcpy ( &l2, s + 2, sizeof ( l2 ) );
        const uint8_t b1 = static_cast<uint8_t> ( l2 >> 24 );
        l2 &= static_cast<uint32_t> ( lowbits >> 32 );
        hash += ( l1 + l2 ) * k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    case 7: {
        uint32_t l1, l2;
        memcpy ( &l1, s, sizeof ( l1 ) );
        memcpy ( &l2, s + 3, sizeof ( l2 ) );
        const uint8_t b1 = static_cast<uint8_t> ( l2 >> 24 );
        l2 &= static_cast<uint32_t> ( lowbits >> 32 );
        hash += ( l1 + l2 ) * k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    case 8: { // locality preserving for 64 bit ints
        uint64_t ll1, ll2;
        memcpy ( &ll1, s, sizeof ( ll1 ) );
        ll2 = ( ll1 & 0x3FFFFFFFFFFFFFFCU );
        // printf("ll1 is %lx\nll2 is %lx\n", ll1, ll2);
        hash += rotr ( ll2 * k0, 47 );
        hash += ( ll1 << 1 );
        hash += ( ll1 >> 62 );
        return hash;
    }
    case 9: {
        uint64_t ll1, ll2;
        memcpy ( &ll1, s, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 1, sizeof ( ll2 ) );
        const uint8_t b1 = static_cast<uint8_t> ( ll2 >> 56 );
        ll2 &= lowbits;
        hash += ( ll1 + ll2 ) * k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    case 10: {
        uint64_t ll1, ll2;
        memcpy ( &ll1, s, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 2, sizeof ( ll2 ) );
        const uint8_t b1 = static_cast<uint8_t> ( ll2 >> 56 );
        ll2 &= lowbits;
        hash += ( ll1 + ll2 ) * k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    case 11: {
        uint64_t ll1, ll2;
        memcpy ( &ll1, s, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 3, sizeof ( ll2 ) );
        const uint8_t b1 = static_cast<uint8_t> ( ll2 >> 56 );
        ll2 &= lowbits;
        hash += ( ll1 + ll2 ) * k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    case 12: {
        uint64_t ll1, ll2;
        memcpy ( &ll1, s, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 4, sizeof ( ll2 ) );
        const uint8_t b1 = static_cast<uint8_t> ( ll2 >> 56 );
        ll2 &= lowbits;
        hash += ( ll1 + ll2 ) * k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    case 13: {
        uint64_t ll1, ll2;
        memcpy ( &ll1, s, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 5, sizeof ( ll2 ) );
        const uint8_t b1 = static_cast<uint8_t> ( ll2 >> 56 );
        ll2 &= lowbits;
        hash += ( ll1 + ll2 ) * k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    case 14: {
        uint64_t ll1, ll2;
        memcpy ( &ll1, s, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 6, sizeof ( ll2 ) );
        const uint8_t b1 = static_cast<uint8_t> ( ll2 >> 56 );
        ll2 &= lowbits;
        hash += ( ll1 + ll2 ) * k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    case 15: {
        uint64_t ll1, ll2;
        memcpy ( &ll1, s, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 7, sizeof ( ll2 ) );
        const uint8_t b1 = static_cast<uint8_t> ( ll2 >> 56 );
        ll2 &= lowbits;
        hash += ( ll1 + ll2 ) * k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    case 16: {
        uint64_t ll1, ll2;
        memcpy ( &ll1, s, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 8, sizeof ( ll2 ) );
        const uint8_t b1 = static_cast<uint8_t> ( ll2 >> 56 );
        ll2 &= lowbits;
        hash += ( ll1 + ll2 ) * k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    case 17: {
        uint64_t ll1, ll2;
        memcpy ( &ll1, s, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 8, sizeof ( ll2 ) );
        hash += ll1;
        hash += ll2;
        memcpy ( &ll2, s + 9, sizeof ( ll2 ) );
        const uint8_t b1 = static_cast<uint8_t> ( ll2 >> 56 );
        ll2 &= lowbits;
        hash += ll2;
        hash *= k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    case 18: {
        uint64_t ll1, ll2;
        memcpy ( &ll1, s, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 8, sizeof ( ll2 ) );
        hash += ll1;
        hash += ll2;
        memcpy ( &ll2, s + 10, sizeof ( ll2 ) );
        const uint8_t b1 = static_cast<uint8_t> ( ll2 >> 56 );
        ll2 &= lowbits;
        hash += ll2;
        hash *= k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    case 19: {
        uint64_t ll1, ll2;
        memcpy ( &ll1, s, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 8, sizeof ( ll2 ) );
        hash += ll1;
        hash += ll2;
        memcpy ( &ll2, s + 11, sizeof ( ll2 ) );
        const uint8_t b1 = static_cast<uint8_t> ( ll2 >> 56 );
        ll2 &= lowbits;
        hash += ll2;
        hash *= k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    case 20: {
        uint64_t ll1, ll2;
        memcpy ( &ll1, s, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 8, sizeof ( ll2 ) );
        hash += ll1;
        hash += ll2;
        memcpy ( &ll2, s + 12, sizeof ( ll2 ) );
        const uint8_t b1 = static_cast<uint8_t> ( ll2 >> 56 );
        ll2 &= lowbits;
        hash += ll2;
        hash *= k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    case 21: {
        uint64_t ll1, ll2;
        memcpy ( &ll1, s, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 8, sizeof ( ll2 ) );
        hash += ll1;
        hash += ll2;
        memcpy ( &ll2, s + 13, sizeof ( ll2 ) );
        const uint8_t b1 = static_cast<uint8_t> ( ll2 >> 56 );
        ll2 &= lowbits;
        hash += ll2;
        hash *= k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    case 22: {
        uint64_t ll1, ll2;
        memcpy ( &ll1, s, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 8, sizeof ( ll2 ) );
        hash += ll1;
        hash += ll2;
        memcpy ( &ll2, s + 14, sizeof ( ll2 ) );
        const uint8_t b1 = static_cast<uint8_t> ( ll2 >> 56 );
        ll2 &= lowbits;
        hash += ll2;
        hash *= k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    case 23: {
        uint64_t ll1, ll2;
        memcpy ( &ll1, s, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 8, sizeof ( ll2 ) );
        hash += ll1;
        hash += ll2;
        memcpy ( &ll2, s + 15, sizeof ( ll2 ) );
        const uint8_t b1 = static_cast<uint8_t> ( ll2 >> 56 );
        ll2 &= lowbits;
        hash += ll2;
        hash *= k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    case 24: {
        uint64_t ll1, ll2;
        memcpy ( &ll1, s, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 8, sizeof ( ll2 ) );
        hash += ll1;
        hash += ll2;
        memcpy ( &ll2, s + 16, sizeof ( ll2 ) );
        const uint8_t b1 = static_cast<uint8_t> ( ll2 >> 56 );
        ll2 &= lowbits;
        hash += ll2;
        hash *= k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    case 25: {
        uint64_t ll1, ll2;
        memcpy ( &ll1, s, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 8, sizeof ( ll2 ) );
        hash += ll1;
        hash += ll2;
        memcpy ( &ll1, s + 16, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 17, sizeof ( ll2 ) );
        const uint8_t b1 = static_cast<uint8_t> ( ll2 >> 56 );
        ll2 &= lowbits;
        hash += ll1;
        hash += ll2;
        hash *= k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    case 26: {
        uint64_t ll1, ll2;
        memcpy ( &ll1, s, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 8, sizeof ( ll2 ) );
        hash += ll1;
        hash += ll2;
        memcpy ( &ll1, s + 16, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 18, sizeof ( ll2 ) );
        const uint8_t b1 = static_cast<uint8_t> ( ll2 >> 56 );
        ll2 &= lowbits;
        hash += ll1;
        hash += ll2;
        hash *= k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    case 27: {
        uint64_t ll1, ll2;
        memcpy ( &ll1, s, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 8, sizeof ( ll2 ) );
        hash += ll1;
        hash += ll2;
        memcpy ( &ll1, s + 16, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 19, sizeof ( ll2 ) );
        const uint8_t b1 = static_cast<uint8_t> ( ll2 >> 56 );
        ll2 &= lowbits;
        hash += ll1;
        hash += ll2;
        hash *= k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    case 28: {
        uint64_t ll1, ll2;
        memcpy ( &ll1, s, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 8, sizeof ( ll2 ) );
        hash += ll1;
        hash += ll2;
        memcpy ( &ll1, s + 16, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 20, sizeof ( ll2 ) );
        const uint8_t b1 = static_cast<uint8_t> ( ll2 >> 56 );
        ll2 &= lowbits;
        hash += ll1;
        hash += ll2;
        hash *= k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    case 29: {
        uint64_t ll1, ll2;
        memcpy ( &ll1, s, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 8, sizeof ( ll2 ) );
        hash += ll1;
        hash += ll2;
        memcpy ( &ll1, s + 16, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 21, sizeof ( ll2 ) );
        const uint8_t b1 = static_cast<uint8_t> ( ll2 >> 56 );
        ll2 &= lowbits;
        hash += ll1;
        hash += ll2;
        hash *= k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    case 30: {
        uint64_t ll1, ll2;
        memcpy ( &ll1, s, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 8, sizeof ( ll2 ) );
        hash += ll1;
        hash += ll2;
        memcpy ( &ll1, s + 16, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 22, sizeof ( ll2 ) );
        const uint8_t b1 = static_cast<uint8_t> ( ll2 >> 56 );
        ll2 &= lowbits;
        hash += ll1;
        hash += ll2;
        hash *= k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    case 31: {
        uint64_t ll1, ll2;
        memcpy ( &ll1, s, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 8, sizeof ( ll2 ) );
        hash += ll1;
        hash += ll2;
        memcpy ( &ll1, s + 16, sizeof ( ll1 ) );
        memcpy ( &ll2, s + 23, sizeof ( ll2 ) );
        const uint8_t b1 = static_cast<uint8_t> ( ll2 >> 56 );
        ll2 &= lowbits;
        hash += ll1;
        hash += ll2;
        hash *= k0;
        hash = rotr ( hash, 33 );
        hash += b1 * 2u;
        return hash;
    }
    default: // Shouldn't be able to reach this
        __builtin_unreachable ();
        break;
    }
    __builtin_unreachable ();
}

uint64_t Hash ( const std::string &str ) noexcept
{
    return Hash ( str.data (), str.size () );
}

uint64_t Hash ( int i ) noexcept
{
    return Hash ( reinterpret_cast<const char *> ( &i ), sizeof ( i ) );
}

uint64_t Hash ( long int i ) noexcept
{
    return Hash ( reinterpret_cast<const char *> ( &i ), sizeof ( i ) );
}

uint64_t Hash ( unsigned long long l ) noexcept
{
    return Hash ( reinterpret_cast<const char *> ( &l ), sizeof ( l ) );
}

uint64_t Hash ( uint64_t l ) noexcept
{
    return Hash ( reinterpret_cast<const char *> ( &l ), sizeof ( l ) );
}

uint64_t Hash ( float f ) noexcept
{
    // Handle 0.0==-0.0
    float pf = fabsf ( f );
    return Hash ( reinterpret_cast<const char *> ( &pf ), sizeof ( pf ) );
}

uint64_t Hash ( double d ) noexcept
{
    // Handle 0.0==-0.0
    double pd = fabs ( d );
    return Hash ( reinterpret_cast<const char *> ( &pd ), sizeof ( pd ) );
}


} // namespace VDB3
