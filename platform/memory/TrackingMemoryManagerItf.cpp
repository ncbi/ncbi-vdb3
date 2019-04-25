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

using namespace std;
using namespace VDB3;

static MemoryMgr primMgr = make_shared < PrimordialMemoryMgr > ();

//////// TrackingMemoryManager

TrackingMemoryManager :: TrackingMemoryManager( MemoryMgr baseMgr )
:   m_baseMgr( baseMgr ),
    m_blocks( * m_baseMgr )
{
}

TrackingMemoryManager :: TrackingMemoryManager ()
:   m_baseMgr( primMgr ),
    m_blocks( * m_baseMgr )
{
}

TrackingMemoryManager :: ~TrackingMemoryManager ()
{
}

void *
TrackingMemoryManager :: allocateBlock( bytes_t bytes )
{
    return m_baseMgr -> allocateBlock( bytes );
}

void *
TrackingMemoryManager :: reallocateBlock( void * block, bytes_t cur_size, bytes_t new_size )
{
    return m_baseMgr -> reallocateBlock( block, cur_size, new_size );
}

void
TrackingMemoryManager :: deallocateBlock( void * block, bytes_t size ) noexcept
{
    m_baseMgr -> deallocateBlock( block, size );
}

TrackingMemoryManager :: pointer
TrackingMemoryManager :: allocate( size_type bytes )
{
    pointer ret = m_baseMgr -> allocate( bytes );
    if ( ret != nullptr )
    {
        m_blocks [ ret ] = bytes;
    }
    return ret;
}

TrackingMemoryManager :: pointer
TrackingMemoryManager :: reallocate( pointer ptr, size_type new_size )
{
    Blocks::const_iterator it = m_blocks . end();
    if ( ptr != nullptr )
    {
        it = m_blocks . find( ptr );
        if ( it == m_blocks . end () )
        {
            throw std :: logic_error( "TrackingMemoryManager :: getBlockSize () called with an unknown block" ); //TODO: replace with a VDB3 exception
        }
    }

    pointer ret = m_baseMgr -> reallocate( ptr, new_size );
    if ( ret != nullptr )
    {
        if ( ret != ptr && it != m_blocks . end () )
        {
            m_blocks . erase( it );
        }
        m_blocks [ ret ] = new_size;
    }
    else if ( it != m_blocks . end () )
    {
        m_blocks . erase( ptr );
    }

    return ret;
}

void
TrackingMemoryManager :: deallocate( pointer ptr, size_type bytes ) noexcept
{
    Blocks::const_iterator it = m_blocks . find(ptr);
    if ( it != m_blocks . end () )
    {
        m_baseMgr -> deallocate( ptr, bytes );
        m_blocks . erase( it );
    }
}

TrackingMemoryManager :: size_type
TrackingMemoryManager :: getBlockSize( const_pointer ptr ) const
{
    Blocks::const_iterator it = m_blocks . find(ptr);
    if ( it == m_blocks . end () )
    {
        throw std :: logic_error( "TrackingMemoryManager :: getBlockSize () called with an unknown block" ); //TODO: replace with a VDB3 exception
    }
    else
    {
        return it -> second;
    }
}

void
TrackingMemoryManager :: setBlockSize( const_pointer ptr, size_type size )
{
    m_blocks [ ptr ] = size;
}

//////// TrackingBypassMemoryManager

static TrackingMemoryMgr trackingMgr = make_shared < TrackingMemoryManager > ();

TrackingBypassMemoryManager :: TrackingBypassMemoryManager( TrackingMemoryMgr baseMgr )
:   m_baseMgr( baseMgr )
{
}

TrackingBypassMemoryManager :: TrackingBypassMemoryManager ()
:   m_baseMgr( trackingMgr )
{
}

TrackingBypassMemoryManager :: ~TrackingBypassMemoryManager ()
{
}

void *
TrackingBypassMemoryManager :: allocateBlock( bytes_t bytes )
{
    return m_baseMgr -> allocateBlock( bytes );
}

void *
TrackingBypassMemoryManager :: reallocateBlock( void * block, bytes_t cur_size, bytes_t new_size )
{
    return m_baseMgr -> reallocateBlock( block, cur_size, new_size );
}

void
TrackingBypassMemoryManager :: deallocateBlock( void * block, bytes_t size ) noexcept
{
    m_baseMgr -> deallocateBlock( block, size );
}

TrackingBypassMemoryManager :: pointer
TrackingBypassMemoryManager :: allocate( size_type bytes )
{
    return m_baseMgr -> allocate( bytes );
}

TrackingBypassMemoryManager :: pointer
TrackingBypassMemoryManager :: reallocate( pointer ptr, size_type new_size )
{
    return m_baseMgr -> reallocate( ptr, new_size );
}

void
TrackingBypassMemoryManager :: deallocate( pointer ptr, size_type bytes ) noexcept
{
    m_baseMgr -> deallocate( ptr, bytes );
}

TrackingBypassMemoryManager :: size_type
TrackingBypassMemoryManager :: getBlockSize( const_pointer ptr ) const
{
    return m_baseMgr -> getBlockSize( ptr );
}

void
TrackingBypassMemoryManager :: setBlockSize( const_pointer ptr, size_type size )
{
    m_baseMgr -> setBlockSize( ptr, size );
}


