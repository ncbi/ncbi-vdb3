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

#include <memory/TrackingMemoryManagerItf.hpp>

#include <cassert>
#include <stdexcept>

#include <memory/PrimordialMemoryMgr.hpp>

using namespace VDB3;

static PrimordialMemoryMgr primMgr;

//////// TrackingMemoryManager

TrackingMemoryManager :: TrackingMemoryManager( MemoryManagerItf * baseMgr )
:   m_baseMgr ( baseMgr == nullptr ? primMgr : * baseMgr ),
    m_blocks ( m_baseMgr )
{
}

TrackingMemoryManager :: ~TrackingMemoryManager()
{
}

TrackingMemoryManager :: pointer
TrackingMemoryManager :: allocate ( size_type bytes )
{
    pointer ret = m_baseMgr . allocate ( bytes );
    if ( ret != nullptr )
    {
        m_blocks [ ret ] = bytes;
    }
    return ret;
}

TrackingMemoryManager :: pointer
TrackingMemoryManager :: reallocate ( pointer ptr, size_type new_size )
{
    pointer ret = m_baseMgr . reallocate ( ptr, new_size );
    if ( ret != nullptr )
    {
        if ( ret != ptr )
        {
            m_blocks . erase ( ptr );
        }
        m_blocks [ ret ] = new_size;
    }
    else
    {
        m_blocks . erase ( ptr );
    }

    return ret;
}

void
TrackingMemoryManager :: deallocate ( pointer ptr, size_type bytes ) noexcept
{
    m_baseMgr . deallocate ( ptr, bytes );
    m_blocks . erase ( ptr );
}

TrackingMemoryManager :: size_type
TrackingMemoryManager :: getBlockSize ( const_pointer ptr ) const
{
    Blocks::const_iterator it = m_blocks . find(ptr);
    if ( it == m_blocks . end() )
    {
        throw std :: logic_error ( "TrackingMemoryManager :: getBlockSize() called with an unknown block" ); //TODO: replace with a VDB3 exception
    }
    else
    {
        return it -> second;
    }
}

void
TrackingMemoryManager :: setBlockSize ( const_pointer ptr, size_type size )
{
    m_blocks [ ptr ] = size;
}

//////// TrackingBypassMemoryManager

static TrackingMemoryManager trackingMgr;

TrackingBypassMemoryManager :: TrackingBypassMemoryManager ( TrackingMemoryManagerItf * baseMgr )
:   m_baseMgr ( baseMgr == nullptr ? trackingMgr : * baseMgr )
{
}

TrackingBypassMemoryManager :: ~TrackingBypassMemoryManager ()
{
}

TrackingBypassMemoryManager :: pointer
TrackingBypassMemoryManager :: allocate ( size_type bytes )
{
    return m_baseMgr . allocate ( bytes );
}

TrackingBypassMemoryManager :: pointer
TrackingBypassMemoryManager :: reallocate ( pointer ptr, size_type new_size )
{
    return m_baseMgr . reallocate ( ptr, new_size );
}

void
TrackingBypassMemoryManager :: deallocate ( pointer ptr, size_type bytes ) noexcept
{
    m_baseMgr . deallocate ( ptr, bytes );
}

TrackingBypassMemoryManager :: size_type
TrackingBypassMemoryManager :: getBlockSize ( const_pointer ptr ) const
{
    return m_baseMgr . getBlockSize ( ptr );
}

void
TrackingBypassMemoryManager :: setBlockSize ( const_pointer ptr, size_type size )
{
    m_baseMgr . setBlockSize ( ptr, size );
}


