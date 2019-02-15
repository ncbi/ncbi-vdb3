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

#include "../memory.cpp"

#include <gtest/gtest.h>

using namespace std;

// PrimordialMemoryMgr

TEST ( PrimordialMemoryMgr, Allocate_Deallocate )
{
    VDB3::PrimordialMemoryMgr pm;
    auto ptr = pm . allocate ( 1 );
    ASSERT_NE ( nullptr, ptr );
    pm . deallocate ( ptr );
}

TEST ( PrimordialMemoryMgr, Allocate_WithFill )
{
    VDB3::PrimordialMemoryMgr pm;

    const byte_t Filler = 0x5a;
    auto ptr = pm.allocate ( 2 , Filler );
    ASSERT_NE ( nullptr, ptr );
    ASSERT_EQ ( Filler, * ( byte_t * ) ptr );
    ASSERT_EQ ( Filler, * ( ( byte_t * ) ptr + 1 ) );

    pm . deallocate ( ptr );
}

class PrimMgr : public ::testing::Test
{
protected:
    //! SetUp
    void SetUp() override
    {
    }
    //! TearDown
    void TearDown() override
    {
    }

    const size_t Size = 13;
    const byte_t Filler1 = 0x5a;
    const byte_t Filler2 = 0x25;

    VDB3::PrimordialMemoryMgr pm;
};

// RawMemoryBlock

TEST_F ( PrimMgr, Raw_Allocate_Deallocate_Size )
{
    RawMemoryBlock rmb ( pm, Size );
    ASSERT_NE ( nullptr, rmb . data() );
    ASSERT_EQ ( Size, rmb . size() );
    // dtor will deallocate
}

TEST_F ( PrimMgr, Raw_Copy_Share_Fill )
{
    RawMemoryBlock rmb1 ( pm, Size );
    auto rmb2 = rmb1;
    ASSERT_EQ ( Size, rmb2 . size() );
    rmb1.fill ( Filler1 );
    ASSERT_EQ ( Filler1, rmb2.data() [ 0 ] );
    ASSERT_EQ ( Filler1, rmb2.data() [ 1 ] );
}

TEST_F ( PrimMgr, Raw_Clone )
{
    RawMemoryBlock rmb1 ( pm, Size );
    auto rmb2 = rmb1 . clone();
    rmb2.fill ( Filler2 );
    ASSERT_EQ ( Size, rmb2 . size() );
    rmb1.fill ( Filler1 ); // does not affect rmb2:
    ASSERT_EQ ( Filler2, rmb2.data() [ 0 ] );
    ASSERT_EQ ( Filler2, rmb2.data() [ 1 ] );
}

TEST_F ( PrimMgr, Raw_RefCount )
{
    RawMemoryBlock rmb1 ( pm, Size );
    ASSERT_EQ ( 1, rmb1 . refcount () );

    auto rmb2 = rmb1;
    ASSERT_EQ ( 2, rmb1 . refcount () );
    ASSERT_EQ ( 2, rmb2 . refcount () );
}

// UniqueRawMemoryBlock

TEST_F ( PrimMgr, UniqueRaw_Allocate_Deallocate_Size )
{
    UniqueRawMemoryBlock rmb ( pm, Size );
    ASSERT_NE ( nullptr, rmb . data() );
    ASSERT_EQ ( Size, rmb . size() );
}

TEST_F ( PrimMgr, UniqueRaw_Clone_Fill )
{
    UniqueRawMemoryBlock rmb1 ( pm, Size );
    auto rmb2 = rmb1 . clone();
    rmb2.fill ( Filler2 );
    ASSERT_EQ ( Size, rmb2 . size() );
    rmb1.fill ( Filler1 ); // does not affect rmb2:
    ASSERT_EQ ( Filler2, rmb2.data() [ 0 ] );
    ASSERT_EQ ( Filler2, rmb2.data() [ 1 ] );
}

// TypedMemoryBlock

class C
{
public:
    C(int m1, float m2) : mem1(m1), mem2(m2){}
    C(const C& that) : mem1(that.mem1), mem2(that.mem2){}
    int mem1;
    float mem2;
};

TEST_F ( PrimMgr, Typed_Allocate_Deallocate_Size_Data )
{
    TypedMemoryBlock<C> tmb ( pm, 1, 2.3f );
    ASSERT_EQ ( ( bytes_t ) sizeof ( C ), tmb . size() );
    ASSERT_EQ ( 1, tmb . data() . mem1 );
    ASSERT_EQ ( 2.3f, tmb . data() . mem2 );
}

TEST_F ( PrimMgr, Typed_Copy )
{
    TypedMemoryBlock<C> tmb1 ( pm, 1, 2.3f );
    TypedMemoryBlock<C> tmb2 ( tmb1 );
    ASSERT_EQ ( tmb1 . data() . mem1, tmb2 . data() . mem1 );
    ASSERT_EQ ( tmb1 . data() . mem2, tmb2 . data() . mem2 );
    // same using conversion operator
    ASSERT_EQ ( ( ( C & ) tmb1 ) . mem1, ( ( C & ) tmb2 ) . mem1 );
    ASSERT_EQ ( ( ( C & ) tmb1 ) . mem2, ( ( C & ) tmb2 ) . mem2 );
}

TEST_F ( PrimMgr, Typed_Share )
{
    TypedMemoryBlock<C> tmb1 ( pm, 1, 2.3f );
    TypedMemoryBlock<C> tmb2 ( tmb1 );
    ( ( C & ) tmb1 ) . mem1 = 2;
    ASSERT_EQ ( tmb1 . data() . mem1, tmb2 . data() . mem1 );
}

TEST_F ( PrimMgr, Typed_Clone )
{
    TypedMemoryBlock<C> tmb1 ( pm, 1, 2.3f );
    auto tmb2 = tmb1 . clone();
    ( ( C & ) tmb1 ) . mem1 = 2;
    ASSERT_EQ ( 1, tmb2 . data() . mem1 ); // did not change
}

//////////////////////////////////////////////////////////

int main ( int argc, char **argv )
{
    ::testing::InitGoogleTest ( & argc, argv );
    return RUN_ALL_TESTS();
}