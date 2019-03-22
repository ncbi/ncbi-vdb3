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

#include <memory/MemoryManagerItf.hpp>

namespace VDB3
{

/**
 *  Interface for a memory block.
 */
class MemoryBlockItf
{
public:
    /**
     * Size of the memory block, in bytes.
     * @return Size of the memory block, in bytes.
     */
    virtual bytes_t size() const = 0;

protected:
    /**
     * Constructor.
     * @param that source
     */
    MemoryBlockItf ( MemoryManagerItf & that );

    /**
     * Copy constructor.
     * @param that source
     */
    MemoryBlockItf ( const MemoryBlockItf & that );

    virtual ~MemoryBlockItf() = 0;

    /**
     * Access to the memory manager associated with the block.
     * @return instance of memory manager that allocated this block
     */
    MemoryManagerItf& getMgr() const { return * mgr; }

    /**
    *  Deleter template, used to wrap MemoryBlockItf in "allocator" classes passed to STL containers.
    */
    template < typename T > class Deleter
    {
    public:
        /**
         * Constructor.
         * @param p_mgr instance of memory manager to be used for deallocation
         */
        Deleter ( MemoryManagerItf & p_mgr ) : m_mgr ( p_mgr ) {}

        /**
         * Call a destructor on the argument and pass its memory to the associated memory manager for deallocation.
         * @param p pointer to the block to be deallocated
         */
        void operator() ( T * p ) const
        {
            p -> ~T();
            m_mgr . deallocate ( p, sizeof ( T ) );
        }

    private:
        MemoryManagerItf & m_mgr; ///< the memory manager instance to be used for deallocation
    };

private:
    /**
     * Assignment - disabled
     * @param that source
     * @return *this
     */
    MemoryBlockItf & operator = ( const MemoryBlockItf & that );

    MemoryManagerItf * mgr; ///< the memory manager instance to be used for deallocation; TODO: convert to shared_ptr?
};

} // namespace VDB3

