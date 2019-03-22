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

#include <memory/FillingMemoryMgr.hpp>

#include "MemoryManagerItf_Test.hpp"
#include "TrackingMemoryManagerItf_Test.hpp"

using namespace std;
using namespace VDB3;

// FillingMemoryMgr

// inteface conformance
INSTANTIATE_TYPED_TEST_SUITE_P(FillingMemoryMgr_ItfConformance, MemoryManagerItf_Test, FillingMemoryMgr);
INSTANTIATE_TYPED_TEST_SUITE_P(FillingMemoryMgr_TrackingItfConformance, TrackingMemoryManagerItf_Test, FillingMemoryMgr);

TEST ( FillingMemoryMgr, InstantiateDefaults_fillByte_trashByte )
{
    FillingMemoryMgr mgr;
    ASSERT_EQ ( FillingMemoryMgr::DefaultFiller, mgr.fillByte() );
    ASSERT_EQ ( FillingMemoryMgr::DefaultTrash,  mgr.trashByte() );
}

TEST ( FillingMemoryMgr, Instantiate )
{
    const byte_t Filler = byte_t ( 1 );
    const byte_t Trash = byte_t ( 2 );
    FillingMemoryMgr mgr ( nullptr, Filler, Trash );
    ASSERT_EQ ( Filler, mgr.fillByte() );
    ASSERT_EQ ( Trash, mgr.trashByte() );
}

TEST ( FillingMemoryMgr, Allocate_WithFill )
{
    const byte_t Filler = byte_t ( 0x5a );
    FillingMemoryMgr mgr ( nullptr, Filler );

    auto ptr = mgr.allocate ( 2 );
    ASSERT_NE ( nullptr, ptr );
    ASSERT_EQ ( Filler, * ( byte_t * ) ptr );
    ASSERT_EQ ( Filler, * ( ( byte_t * ) ptr + 1 ) );

    mgr . deallocate ( ptr, 2 );
}

TEST ( FillingMemoryMgr, Rellocate_GrowWithFill )
{
    const byte_t Filler = byte_t ( 0x5a );
    FillingMemoryMgr mgr ( nullptr, Filler );

    auto ptr = mgr.allocate ( 2 );
    ptr = mgr.reallocate( ptr, 4 );
    ASSERT_EQ ( Filler, * ( byte_t * ) ptr );
    ASSERT_EQ ( Filler, * ( ( byte_t * ) ptr + 1 ) );
    ASSERT_EQ ( Filler, * ( ( byte_t * ) ptr + 2 ) );
    ASSERT_EQ ( Filler, * ( ( byte_t * ) ptr + 3 ) );

    mgr . deallocate ( ptr, 4 );
}

/**
 * A memory manager for testing filling of blocks.
 * Holds on to memory blocks on deallocation so that their contents can be examined
*/
class FreeOnceMgr : public MemoryManagerItf
{
public:
    /**
     * Constructor
     * @param p_size - total size of memory for all allocated blocks
     */
    FreeOnceMgr( size_t p_size ) : memory ( ( byte_t *) malloc ( p_size ) ), size ( p_size ), next_free ( 0 ) {}
    virtual ~FreeOnceMgr() { free ( memory ); }
    virtual pointer allocate ( size_type bytes )
    {
        assert ( next_free + bytes <  )
        pointer ret = memory + next_free;
        next_free += bytes;
        return ret;
    }
    virtual pointer reallocate ( pointer ptr, size_type new_size ) { assert(false); }
    virtual void deallocate ( pointer ptr, size_type bytes ) noexcept {}

    byte_t * memory; ///< memory segment to allocate blocks from
    size_t size; ///< size of 'memory'
    size_type next_free; ///< next allocated block will start at this offset into 'memory'
};

TEST ( FillingMemoryMgr, Rellocate_ShrinkWithTrash )
{
    const byte_t Filler = byte_t ( 0x5a );
    const byte_t Trash = byte_t ( 0xde );

    FreeOnceMgr fom(1000);
    TrackingMemoryManager tm ( & fom );
    FillingMemoryMgr mgr ( & tm, Filler, Trash );

    auto ptr = mgr.allocate ( 4 );
    auto new_ptr = mgr.reallocate( ptr, 2 );
    // this while block may be deallocated, or trailing portion trashed in place
    ASSERT_EQ ( Trash, * ( ( byte_t * ) ptr + 2 ) );
    ASSERT_EQ ( Trash, * ( ( byte_t * ) ptr + 3 ) );

    mgr . deallocate ( new_ptr, 2 );
}

TEST ( FillingMemoryMgr, Rellocate_MoveWithTrash )
{
    const byte_t Filler = byte_t ( 0x5a );
    const byte_t Trash = byte_t ( 0xde );

    FreeOnceMgr fom(1000);
    TrackingMemoryManager tm ( & fom );
    FillingMemoryMgr mgr ( & tm, Filler, Trash );

    auto ptr = mgr.allocate ( 2 );
    auto new_ptr = mgr.reallocate( ptr, 4 ); // this will always move the block
    ASSERT_NE ( ptr, new_ptr ); // make sure move
    ASSERT_EQ ( Trash, * ( ( byte_t * ) ptr ) );
    ASSERT_EQ ( Trash, * ( ( byte_t * ) ptr + 1 ) );

    mgr . deallocate ( new_ptr, 4 );
}

TEST ( FillingMemoryMgr, Deallocate_WithTrash )
{
    const byte_t Filler = byte_t ( 0x5a );
    const byte_t Trash = byte_t ( 0xde );

    FreeOnceMgr fom(1000);
    TrackingMemoryManager tm ( & fom );
    FillingMemoryMgr mgr ( & tm, Filler, Trash );

    auto ptr = mgr.allocate ( 2 );
    mgr . deallocate ( ptr, 2 );
    ASSERT_EQ ( Trash, * ( byte_t * ) ptr );
    ASSERT_EQ ( Trash, * ( ( byte_t * ) ptr + 1 ) );
}
