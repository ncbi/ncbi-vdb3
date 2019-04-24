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

#include <memory/UniqueRawMemoryBlock.hpp>

#include <memory/PrimordialMemoryMgr.hpp>

#include <gtest/gtest.h>

using namespace std;
using namespace VDB3;

/**
 *  Test fixture class encapsulating PrimordialMemoryMgr.
 */
class UniqueRawMemoryBlock_Fixture : public ::testing::Test
{
protected:

    const size_t Size = 13; ///< common block size
    const byte_t Filler1 = byte_t ( 0x5a ); ///< a filler byte
    const byte_t Filler2 = byte_t ( 0x25 ); ///< another filler byte

    VDB3::PrimordialMemoryMgr pm;   ///< the underlying memory manager
};

// UniqueRawMemoryBlock

TEST_F ( UniqueRawMemoryBlock_Fixture, UniqueRaw_Allocate_Deallocate_Size )
{
    UniqueRawMemoryBlock rmb ( pm, Size );
    ASSERT_NE ( nullptr, rmb . data() );
    ASSERT_EQ ( Size, rmb . size() );
}

TEST_F ( UniqueRawMemoryBlock_Fixture, UniqueRaw_Clone_Fill )
{
    UniqueRawMemoryBlock rmb1 ( pm, Size );
    auto rmb2 = rmb1 . clone();
    rmb2.fill ( Filler2 );
    ASSERT_EQ ( Size, rmb2 . size() );
    rmb1.fill ( Filler1 ); // does not affect rmb2:
    ASSERT_EQ ( Filler2, rmb2.data() [ 0 ] );
    ASSERT_EQ ( Filler2, rmb2.data() [ 1 ] );
}
