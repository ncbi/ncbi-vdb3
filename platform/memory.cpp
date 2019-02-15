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

#include <memory.hpp>

#include <cstring>
#include <cassert>

using namespace VDB3;

/////////////// MemoryManagerItf

MemoryManagerItf::~MemoryManagerItf()
{
}

/////////////// PrimordialMemoryMgr

PrimordialMemoryMgr::PrimordialMemoryMgr()
{
}

PrimordialMemoryMgr::~PrimordialMemoryMgr()
{
}

MemoryManagerItf::pointer PrimordialMemoryMgr::allocate ( size_type size )
{
    //TODO: throw if nullptr
    return ( pointer ) malloc ( size );
}

MemoryManagerItf::pointer PrimordialMemoryMgr::allocate ( size_type size, byte_t fill )
{
    auto ret = allocate ( size );
    memset ( ret, fill, size );
    return ret;
}

MemoryManagerItf::pointer PrimordialMemoryMgr::allocate_shared ( size_type, byte_t fill )           { assert(false); return nullptr; }
MemoryManagerItf::pointer PrimordialMemoryMgr::allocate_shared ( size_type, int fd, off_t offset )  { assert(false); return nullptr; }
MemoryManagerItf::pointer PrimordialMemoryMgr::allocate_pinned ( size_type )                        { assert(false); return nullptr; }
MemoryManagerItf::pointer PrimordialMemoryMgr::allocate_pinned ( size_type, byte_t fill )           { assert(false); return nullptr; }

void
PrimordialMemoryMgr::deallocate ( pointer p )
{
    free ( p );
}

MemoryManagerItf::pointer
PrimordialMemoryMgr:: reallocate ( pointer p, size_type s )
{
    //TODO: throw if nullptr
    return realloc(p, s);
}

MemoryManagerItf::size_type PrimordialMemoryMgr::max_size() const noexcept      { assert(false); return 0; }

MemoryManagerItf::size_type PrimordialMemoryMgr::quota() const noexcept         { assert(false); return 0; }
bool PrimordialMemoryMgr::update_quota( size_type min_extension ) const         { assert(false); return false; }
MemoryManagerItf::size_type PrimordialMemoryMgr::total_free() const noexcept    { assert(false); return 0; }
MemoryManagerItf::size_type PrimordialMemoryMgr::total_used() const noexcept    { assert(false); return 0; }
MemoryManagerItf::size_type PrimordialMemoryMgr::max_free() const noexcept      { assert(false); return 0; }

/////////////// MemoryBlockItf

MemoryBlockItf :: MemoryBlockItf ( MemoryManagerItf & p_mgr )
: mgr ( & p_mgr )
{
}

MemoryBlockItf :: MemoryBlockItf ( const MemoryBlockItf & that )
: mgr ( that.mgr )
{
}

MemoryBlockItf :: ~MemoryBlockItf()
{
}

/////////////// RawMemoryBlock

RawMemoryBlock :: RawMemoryBlock ( MemoryManagerItf & p_mgr, size_t p_size )
:   MemoryBlockItf ( p_mgr ),
    m_size ( p_size )
{
    auto ptr = ( byte_t * ) getMgr() . allocate ( m_size );
    m_ptr . reset ( ptr, Deleter<byte_t>( getMgr() ) );
}

RawMemoryBlock :: RawMemoryBlock( const RawMemoryBlock & that )
:   MemoryBlockItf ( that . getMgr() ),
    m_size ( that . size () ),
    m_ptr ( that . getPtr () )
{
}

RawMemoryBlock :: ~RawMemoryBlock()
{
}

bytes_t RawMemoryBlock :: size() const
{
    return m_size;
}

void RawMemoryBlock :: fill(byte_t filler)
{
    memset( m_ptr . get(), filler, size() );
}

RawMemoryBlock RawMemoryBlock :: clone() const
{
    RawMemoryBlock ret ( getMgr(), m_size );
    memmove ( ret . getPtr() . get(), m_ptr . get(), m_size );
    return ret;
}

/////////////// UniqueRawMemoryBlock

UniqueRawMemoryBlock :: UniqueRawMemoryBlock ( MemoryManagerItf & p_mgr, size_t p_size )
:   MemoryBlockItf ( p_mgr ),
    m_size ( p_size ),
    m_ptr ( ( byte_t * ) getMgr() . allocate ( m_size ), Deleter<byte_t>( getMgr() ) )
{
}

UniqueRawMemoryBlock :: ~UniqueRawMemoryBlock()
{
}

bytes_t UniqueRawMemoryBlock :: size() const
{
    return m_size;
}

void UniqueRawMemoryBlock :: fill(byte_t filler)
{
    memset( m_ptr . get(), filler, size() );
}

UniqueRawMemoryBlock UniqueRawMemoryBlock :: clone() const
{
    UniqueRawMemoryBlock ret ( getMgr(), m_size );
    memmove ( ret . getPtr() . get(), m_ptr . get(), m_size );
    return ret;
}

