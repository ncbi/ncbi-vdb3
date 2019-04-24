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

#include <memory/RawMemoryBlock.hpp>

// memset, memmove
#include <cstring>

using namespace VDB3;

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
    memset( m_ptr . get(), int ( filler ), size() );
}

RawMemoryBlock RawMemoryBlock :: clone() const
{
    RawMemoryBlock ret ( getMgr(), m_size );
    memmove ( ret . getPtr() . get(), m_ptr . get(), m_size );
    return ret;
}