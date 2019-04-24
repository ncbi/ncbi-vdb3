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

#include <memory/PinnedMemoryMgr.hpp>

#include <sys/mman.h>
#include <cassert>

#include <memory/PrimordialMemoryMgr.hpp>

using namespace VDB3;

PinnedMemoryMgr :: MemoryLockerItf :: ~MemoryLockerItf ()
{
}

/**
* Default memory locker. Uses mlock/munlock.
*/
class PosixLocker : public PinnedMemoryMgr :: MemoryLockerItf
{
public:
    PosixLocker(){}
    virtual ~PosixLocker () {}
    /**
     * Locks memory in the address range starting at ptr and continuing for bytes.
     * @param ptr pointer to the start of the address range
     * @param bytes size of the address range
     * @exception ???
     */
    virtual void lock( MemoryManagerItf :: pointer ptr, MemoryManagerItf :: size_type bytes )
    {
        int res = mlock ( ptr, bytes );
        if ( res != 0 )
        {
            throw std :: logic_error ( "mlock() failed" );
        }
    }
    /**
     * Unocks memory in the address range starting at ptr and continuing for bytes.
     * @param ptr pointer to the start of the address range
     * @param bytes size of the address range
     * @exception ???
     */
    virtual void unlock( MemoryManagerItf :: pointer ptr, MemoryManagerItf :: size_type bytes )
    {
        int res = munlock ( ptr, bytes );
        if ( res != 0 )
        {
            throw std :: logic_error ( "munlock() failed" );
        }
    }
};

PosixLocker linuxLocker; ///< the default memory locker

PinnedMemoryMgr :: PinnedMemoryMgr( TrackingMemoryManagerItf * base_mgr, MemoryLockerItf * locker )
:   TrackingBypassMemoryManager ( base_mgr ),
    m_locker ( locker == nullptr ? linuxLocker : * locker)
{
}

PinnedMemoryMgr :: ~PinnedMemoryMgr()
{
}

PinnedMemoryMgr :: pointer
PinnedMemoryMgr :: allocate ( size_type bytes )
{
    pointer ret = baseMgr () . allocate ( bytes );
    if ( ret != nullptr ) // nullptr can happen if bytes == 0
    {
        m_locker . lock ( ret, bytes );
    }
    return ret;
}

PinnedMemoryMgr :: pointer
PinnedMemoryMgr :: reallocate ( pointer old_ptr, size_type new_size )
{
    if ( old_ptr == nullptr )
    {
        return allocate ( new_size );
    }

    size_t old_size = baseMgr () . getBlockSize ( old_ptr ); // will throw if bad block

    pointer new_ptr =  baseMgr () . reallocate ( old_ptr, new_size );
    if ( old_ptr != new_ptr )
    {   // unpin the old block
        m_locker . unlock ( old_ptr, old_size );
    }

    if ( new_ptr != nullptr )
    {   // pin the new block
        m_locker . lock ( new_ptr, new_size );
    }

    return new_ptr;
}

void
PinnedMemoryMgr :: deallocate ( pointer ptr, size_type bytes ) noexcept
{
    if ( ptr == nullptr )
    {
        return;
    }

    try // this method is noexcept as required by the interface
    {
        size_t size = baseMgr () . getBlockSize ( ptr ); // will throw if bad block
        m_locker . unlock ( ptr, size );
        baseMgr () . deallocate ( ptr, size );
    }
    catch (...)
    {
        //TODO: log and ignore
    }
}