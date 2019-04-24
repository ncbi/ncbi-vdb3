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

#include <memory/TrackingMemoryManagerItf.hpp>

namespace VDB3
{

/**
 *  Memory manager using system heap allocation functions (malloc/realloc/free).
 */
class PrimordialMemoryMgr : public MemoryManagerItf
{
public:
    PrimordialMemoryMgr();

    virtual ~PrimordialMemoryMgr();

public: // inherited from MemoryManagerItf

    virtual pointer allocate ( size_type bytes );

    virtual pointer reallocate ( pointer ptr, size_type new_size) ;

    virtual void deallocate ( pointer ptr, size_type bytes ) noexcept;

private:
    /**
     * Copy constructor - deleted
     * @param that source
     */
    PrimordialMemoryMgr(const MemoryManagerItf & that);

    /**
     * Assignment - deleted
     * @param that source
     * @return *this
     */
    PrimordialMemoryMgr & operator = (const PrimordialMemoryMgr & that);
};

} // namespace VDB3
