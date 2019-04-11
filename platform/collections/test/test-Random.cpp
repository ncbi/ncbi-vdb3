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

#include <collections/Random.hpp>

#include <cstdlib>
#include <cstring>
#include <set>
#include <set>
#include <stdexcept>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <utility>
#include <vector>
#include <unordered_set>

#include <gtest/gtest.h>

using namespace std;
using namespace VDB3;

TEST ( Random, Basic)
{
    Random r;

    uint64_t r1=r();
    uint64_t r2=r();

    ASSERT_NE(r1,r2);
    ASSERT_NE(r1,0);
    ASSERT_NE(r1,123);

    r.seed(3);
    uint64_t r3=r();
    ASSERT_NE(r1,r3);
    ASSERT_NE(r2,r3);
    ASSERT_EQ(r3,123);
}

TEST ( Random, Range)
{
    Random r;

    for (int i=0; i!=1000; ++i)
    {
        uint64_t x=r.randint(5,6);
        ASSERT_LE(x,6);
        ASSERT_GE(x,5);
    }
}

TEST ( Random, Dups)
{
    Random r;
    std::unordered_set<uint64_t> set;
    size_t ins=1000;
    for ( size_t i=0; i!=ins; ++i)
    {
        uint64_t x=r();
        ASSERT_EQ(set.count(x),0);
        set.insert(x);
    }
    ASSERT_EQ(set.size(),ins);
}

TEST (Random, Bytes)
{
    Random r;
    std::unordered_set<string> set;

    size_t ins=50;
    for (size_t i=0; i!=ins; ++i)
    {
        std::string bytes=r.randbytes(i);
        ASSERT_EQ(bytes.size(),i);
        ASSERT_EQ(set.count(bytes),0);
        set.insert(bytes);
    }
    ASSERT_EQ(set.size(), ins);
}

