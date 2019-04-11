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
#include <endian.h>
#include <cmath>
#include <cstdbool>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <x86intrin.h>
#include <cstring>
#include <string>
#define __STDC_FORMAT_MACROS
#include <cinttypes>

#undef memcpy

#if __BYTE_ORDER == __BIG_ENDIAN
#error "Big endian not tested"
#endif

#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define LIKELY(x) __builtin_expect(!!(x), 1)

namespace VDB3
{
    // Fast, locality preserving, not cryptographically secure hash function.
    // Platform depedendent, will not give same results on big-endian.
    
    static inline uint64_t rotr(const uint64_t x, int k) {
        return (x << (64 - k) | (x >> k));
    }


    static uint64_t hashmix(uint64_t hash, uint64_t high, uint64_t low, uint64_t mul )
    {
        uint64_t masked=low & 0xFCULL;
        uint64_t h1=(high+masked)*mul;
        uint64_t h2=rotr(h1,47U);
        return hash+h2+low*2;
    }

    uint64_t Hash ( const char *s, size_t len )
    {
        /* Some primes between 2^63 and 2^64 for various uses. */
        static const uint64_t k0 = 0xc3a5c85c97cb3127ULL;
        static const uint64_t k1 = 0xb492b66fbe98f273ULL;
        static const uint64_t k2 = 0x9ae16a3b2f90404fULL;

        uint64_t hash=0;

        if (UNLIKELY(len >= 32))
        {
            __m128i h1;
            __m128i h2;
            h1=_mm_setzero_si128();
            h2=_mm_setzero_si128();
            const __m128i * s128=reinterpret_cast<const __m128i *>(s);
            while (len >= 32)
            {
                h1+=_mm_loadu_si128(s128++);
                h2+=_mm_loadu_si128(s128++);
                h1=_mm_aesenc_si128(h1,h2);

                len-=32;
                s+=32;
            }
            //h1^=h2;
            hash=_mm_cvtsi128_si64(h1);
        }

        assert(len < 32);

        switch(len&31)
        {
            uint8_t b1;
            uint16_t w1, w2;
            uint32_t l1, l2;
            uint64_t ll1, ll2;
            uint64_t last;
          case 0:
            return hash;
          case 1:
            memcpy(&b1, s, sizeof(b1));
            return hashmix(hash,0,b1,k0);
          case 2:
            memcpy(&w1, s, sizeof(w1));
            return hashmix(hash, 0, w1, k1);
          case 3:
            memcpy(&w1, s, sizeof(w1));
            memcpy(&w2, s+1, sizeof(w2));
            return hashmix(hash, w1, w2, k2);
          case 4:
            memcpy(&l1, s, sizeof(l1));
            return hashmix(hash, 0, l1, k0);
          case 5:
            memcpy(&l1, s, sizeof(l1));
            memcpy(&l2, s+1, sizeof(l1));
            return hashmix(hash, l1, l2, k1);
          case 6:
            memcpy(&l1, s, sizeof(l1));
            memcpy(&l2, s+2, sizeof(l1));
            return hashmix(hash, l1, l2, k2);
          case 7:
            memcpy(&l1, s, sizeof(l1));
            memcpy(&l2, s+3, sizeof(l1));
            return hashmix(hash, l1, l2, k0);
            break;
          case 8: // locality preserving for 64 bit ints
            memcpy(&ll1, s, sizeof(ll1));
            ll2=(ll1 & 0x3FFFFFFFFFFFFFFCUL);
            hash+=rotr(ll2*k1,47);
            hash+=ll1;
            hash+=(ll1 >> 62);
            return hash;
          case 9:
            memcpy(&ll1, s, sizeof(ll1));
            memcpy(&ll2, s+1, sizeof(ll2));
            return hashmix(hash, ll1, ll2, k0);
          case 10:
            memcpy(&ll1, s, sizeof(ll1));
            memcpy(&ll2, s+2, sizeof(ll2));
            return hashmix(hash, ll1, ll2, k0);
          case 11:
            memcpy(&ll1, s, sizeof(ll1));
            memcpy(&ll2, s+3, sizeof(ll2));
            return hashmix(hash, ll1, ll2, k0);
          case 12:
            memcpy(&ll1, s, sizeof(ll1));
            memcpy(&ll2, s+4, sizeof(ll2));
            return hashmix(hash, ll1, ll2, k0);
          case 13:
            memcpy(&ll1, s, sizeof(ll1));
            memcpy(&ll2, s+5, sizeof(ll2));
            return hashmix(hash, ll1, ll2, k0);
          case 14:
            memcpy(&ll1, s, sizeof(ll1));
            memcpy(&ll2, s+6, sizeof(ll2));
            return hashmix(hash, ll1, ll2, k0);
          case 15:
            memcpy(&ll1, s, sizeof(ll1));
            memcpy(&ll2, s+7, sizeof(ll2));
            return hashmix(hash, ll1, ll2, k0);
          case 16:
            memcpy(&ll1, s, sizeof(ll1));
            memcpy(&ll2, s+8, sizeof(ll2));
            return hashmix(hash, ll1, ll2, k0);
          case 17:
            memcpy(&ll1, s, sizeof(ll1));
            memcpy(&ll2, s+8, sizeof(ll2));
            hash+=ll1+ll2;
            memcpy(&ll2, s+9, sizeof(ll2));
            return hashmix(hash, ll1, ll2, k0);
          case 18:
            memcpy(&ll1, s, sizeof(ll1));
            memcpy(&ll2, s+8, sizeof(ll2));
            hash+=ll1+ll2;
            memcpy(&ll2, s+10, sizeof(ll2));
            return hashmix(hash, ll1, ll2, k0);
          case 19:
            memcpy(&ll1, s, sizeof(ll1));
            memcpy(&ll2, s+8, sizeof(ll2));
            hash+=ll1+ll2;
            memcpy(&ll2, s+11, sizeof(ll2));
            return hashmix(hash, ll1, ll2, k0);
          case 20:
            memcpy(&ll1, s, sizeof(ll1));
            memcpy(&ll2, s+8, sizeof(ll2));
            hash+=ll1+ll2;
            memcpy(&ll2, s+12, sizeof(ll2));
            return hashmix(hash, ll1, ll2, k0);
          case 21:
            memcpy(&ll1, s, sizeof(ll1));
            memcpy(&ll2, s+8, sizeof(ll2));
            hash+=ll1+ll2;
            memcpy(&ll2, s+13, sizeof(ll2));
            return hashmix(hash, ll1, ll2, k0);
          case 22:
            memcpy(&ll1, s, sizeof(ll1));
            memcpy(&ll2, s+8, sizeof(ll2));
            hash+=ll1+ll2;
            memcpy(&ll2, s+14, sizeof(ll2));
            return hashmix(hash, ll1, ll2, k0);
          case 23:
            memcpy(&ll1, s, sizeof(ll1));
            memcpy(&ll2, s+8, sizeof(ll2));
            hash+=ll1+ll2;
            memcpy(&ll2, s+15, sizeof(ll2));
            return hashmix(hash, ll1, ll2, k0);
          case 24:
            memcpy(&ll1, s, sizeof(ll1));
            memcpy(&ll2, s+8, sizeof(ll2));
            hash+=ll1+ll2;
            memcpy(&ll2, s+16, sizeof(ll2));
            return hashmix(hash, ll1, ll2, k0);
          case 25:
            memcpy(&ll1, s, sizeof(ll1));
            memcpy(&ll2, s+8, sizeof(ll2));
            hash+=ll1+ll2;
            memcpy(&ll1, s+16, sizeof(ll1));
            memcpy(&ll2, s+17, sizeof(ll2));
            return hashmix(hash, ll1, ll2, k0);
          case 26:
            memcpy(&ll1, s, sizeof(ll1));
            memcpy(&ll2, s+8, sizeof(ll2));
            hash+=ll1+ll2;
            memcpy(&ll1, s+16, sizeof(ll1));
            memcpy(&ll2, s+18, sizeof(ll2));
            return hashmix(hash, ll1, ll2, k0);
          case 27:
            memcpy(&ll1, s, sizeof(ll1));
            memcpy(&ll2, s+8, sizeof(ll2));
            hash+=ll1+ll2;
            memcpy(&ll1, s+16, sizeof(ll1));
            memcpy(&ll2, s+19, sizeof(ll2));
            return hashmix(hash, ll1, ll2, k0);
          case 28:
            memcpy(&ll1, s, sizeof(ll1));
            memcpy(&ll2, s+8, sizeof(ll2));
            hash+=ll1+ll2;
            memcpy(&ll1, s+16, sizeof(ll1));
            memcpy(&ll2, s+20, sizeof(ll2));
            return hashmix(hash, ll1, ll2, k0);
          case 29:
            memcpy(&ll1, s, sizeof(ll1));
            memcpy(&ll2, s+8, sizeof(ll2));
            hash+=ll1+ll2;
            memcpy(&ll1, s+16, sizeof(ll1));
            memcpy(&ll2, s+21, sizeof(ll2));
            return hashmix(hash, ll1, ll2, k0);
          case 30:
            memcpy(&ll1, s, sizeof(ll1));
            memcpy(&ll2, s+8, sizeof(ll2));
            hash+=ll1+ll2;
            memcpy(&ll1, s+16, sizeof(ll1));
            memcpy(&ll2, s+22, sizeof(ll2));
            return hashmix(hash, ll1, ll2, k0);
          case 31:
            memcpy(&ll1, s, sizeof(ll1));
            memcpy(&ll2, s+8, sizeof(ll2));
            hash+=ll1+ll2;
            memcpy(&ll1, s+16, sizeof(ll1));
            memcpy(&ll2, s+23, sizeof(ll2));
            return hashmix(hash, ll1, ll2, k0);
        }
    }

    uint64_t Hash (const std::string & str)
    {
        return Hash(str.data(), str.size());
    }

    uint64_t Hash (int i)
    {
        return Hash(reinterpret_cast<const char*>(&i),sizeof(i));
    }

    uint64_t Hash (unsigned long long l)
    {
        return Hash(reinterpret_cast<const char*>(&l),sizeof(l));
    }

    uint64_t Hash (float f)
    {
        // Handle 0.0==-0.0
        float pf=fabs(f);
        return Hash(reinterpret_cast<const char *>(&pf),sizeof(pf));
    }

    uint64_t Hash (double d)
    {
        // Handle 0.0==-0.0
        double pd=fabs(d);
        return Hash(reinterpret_cast<const char*>(&pd),sizeof(pd));
    }


}
