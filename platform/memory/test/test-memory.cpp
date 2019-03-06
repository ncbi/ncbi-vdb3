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
    pm . deallocate ( ptr, 1 );
}

/**
 *  Test fixture class encapsulating PrimordialMemoryMgr.
 */
class PrimMgr : public ::testing::Test
{
protected:

    const size_t Size = 13; ///< common block size
    const byte_t Filler1 = byte_t ( 0x5a ); ///< a filler byte
    const byte_t Filler2 = byte_t ( 0x25 ); ///< another filler byte

    VDB3::PrimordialMemoryMgr pm;   ///< the underlying memory manager
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

    {
        auto rmb2 = rmb1;
        ASSERT_EQ ( 2, rmb1 . refcount () );
        ASSERT_EQ ( 2, rmb2 . refcount () );
    }
    ASSERT_EQ ( 1, rmb1 . refcount () );
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

/**
 *  A user class for testing TypedMemoryBlock<>
 */
class C
{
public:
    /**
     * Constructor.
     * @param m1 some type
     * @param m2 another type
     */
    C(int m1, float m2) : mem1(m1), mem2(m2) {}

    /**
     * Copy constructor.
     * @param that source
     */
    C(const C& that) : mem1(that.mem1), mem2(that.mem2){}

    /**
     * Destructor.
     */
    ~C() { ++ dtor_called; }

    int mem1;   ///< data member
    float mem2; ///< another data member

    static uint32_t dtor_called;    ///< incremented with every call to the destructor
};

uint32_t C :: dtor_called = 0;

TEST_F ( PrimMgr, Typed_Allocate_Deallocate_Size_Data )
{
    C :: dtor_called = 0;
    {
        TypedMemoryBlock<C> tmb ( pm, 1, 2.3f );
        ASSERT_EQ ( ( bytes_t ) sizeof ( C ), tmb . size() );
        ASSERT_EQ ( 1, tmb . data() . mem1 );
        ASSERT_EQ ( 2.3f, tmb . data() . mem2 );
    }
    ASSERT_EQ ( 1, C :: dtor_called );
}

TEST_F ( PrimMgr, Typed_Copy )
{
    C :: dtor_called = 0;
    {
        TypedMemoryBlock<C> tmb1 ( pm, 1, 2.3f );
        TypedMemoryBlock<C> tmb2 ( tmb1 ); // a new reference to C
        ASSERT_EQ ( tmb1 . data() . mem1, tmb2 . data() . mem1 );
        ASSERT_EQ ( tmb1 . data() . mem2, tmb2 . data() . mem2 );
        // same using conversion operator
        ASSERT_EQ ( ( ( C & ) tmb1 ) . mem1, ( ( C & ) tmb2 ) . mem1 );
        ASSERT_EQ ( ( ( C & ) tmb1 ) . mem2, ( ( C & ) tmb2 ) . mem2 );
    }
    ASSERT_EQ ( 1, C :: dtor_called ); // only 1 C existed
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
    C :: dtor_called = 0;
    {
        TypedMemoryBlock<C> tmb1 ( pm, 1, 2.3f );
        auto tmb2 = tmb1 . clone(); // a new C object
        ( ( C & ) tmb1 ) . mem1 = 2;
        ASSERT_EQ ( 1, tmb2 . data() . mem1 ); // did not change
    }
    ASSERT_EQ ( 2, C :: dtor_called ); // 2 Cs existed
}

// TEST ( FillingMemoryMgr, Allocate_WithFill )
// {
//     const byte_t Filler = byte_t ( 0x5a );
//     VDB3::FillingMemoryMgr pm( Filler ); etc. - add trash byte
//     auto ptr = pm.allocate ( 2 , Filler );
//     ASSERT_NE ( nullptr, ptr );
//     ASSERT_EQ ( Filler, * ( byte_t * ) ptr );
//     ASSERT_EQ ( Filler, * ( ( byte_t * ) ptr + 1 ) );

//     pm . deallocate ( ptr, 2 );
// }

//////////////////////////////////////////////////////////

int main ( int argc, char **argv )
{
    ::testing::InitGoogleTest ( & argc, argv );
    return RUN_ALL_TESTS();
}