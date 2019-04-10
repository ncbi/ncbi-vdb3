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
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <x86intrin.h>
#include <random>
#include <string>
#define __STDC_FORMAT_MACROS
#include <cinttypes>
#include <fstream>


namespace VDB3
{
    class Random // Very fast RNG
    {
      public:
          Random()
          {
              std::random_device r;
              state1_=r();
              state2_=r();
          }

          Random(uint64_t value)
          {
              seed(value);
          }

          void seed(uint64_t value)
          {
              state1_=seed;
              state2_=0;
              getoctet();
          }

          uint64_t operator()()
          {
              return getoctet();
          }

          // Returns start <= randint <= stop
          // Fast but biased, see http://www.pcg-random.org/posts/bounded-rands.html
          // Use std::uniform_int_distribution if higher quality is desired
          uint64_t randint(uint64_t start, uint64_t stop)
          {
              assert(start<=stop);
              uint64_t r=getoctet();
              return start+(r%(stop-start+1));
          }

          double randdouble(void)
          {
              uint64_t r=getoctet();
              return (r >> 11u) * (1.0 / (UINT64_C(1) << 53));
          }

          std::string randbytes(size_t num)
          {
              std::string buf;
              buf.reserve(num);
              while (num > 8 )
              {
                  uint64_t r=getoctet();
                  buf.append(&r,sizeof(r));
                  num-=8;
              }

              while (num)
              {
                  buf.push_back(getoctet())
              }

              return buf;
          }


      private:
          uint64_t state1_, state2_;

          uint64_t getoctet(void)
          {
              // xorshiro128plus
              const uint64_t s0 = state1_;
              uint64_t s1 = state2_;
              const uint64_t result = s0 + s1;

              s1 ^= s0;
              state1_ = rotl(s0, 24) ^ s1 ^ (s1 << 16);
              state2_ = rotl(s1, 37);

              return result;

          }

    }
}
