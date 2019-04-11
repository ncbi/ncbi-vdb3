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

#include <cstdint>
#include <cstdlib>
#include <vector>

namespace VDB3 {
template <class KEY, class VALUE> class HashTable {
public:
    //    HashTable ( size_t initial_capacity = 0 ) {}
    //    virtual ~HashTable ();

    size_t size ( void ) const noexcept { return count_; }

    void erase () noexcept;
    size_t count ( KEY k ) const noexcept { return 0; }
    void insert ( KEY k, VALUE v );

    void reserve ( size_t capcity );

    float load_factor () const noexcept;
    float max_load_factor () const noexcept;
    void max_load_factor ( float factor );

    VALUE get ( KEY k ) const noexcept;

    void erase ( KEY k ) noexcept;

private:
    struct bucket {
        uint64_t hashandbits;
        KEY key;
        VALUE value;
    };

    std::vector<bucket> buckets_;

    uint64_t mask_ = 0;
    size_t num_buckets_ = 0; /* Always a power of 2 */
    size_t count_ = 0;
    size_t load_ = 0; /* Included invisible buckets */
    double max_load_factor_ = 0;

    // @TODO: Iterators, Persist, Allocators
};
} // namespace VDB3
