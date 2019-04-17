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

#include <cassert>
#include <climits>
#include <collections/Hash.hpp>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <vector>

namespace VDB3 {
template <typename KEY, typename VALUE> class HashTable {
public:
    //    HashTable ( size_t initial_capacity = 0 ) {}
    //    virtual ~HashTable ();
    //    operator ==,!=

    void reserve ( size_t capacity ) noexcept { rehash ( capacity ); }

    bool empty () const noexcept __attribute__ ( ( warn_unused_result ) )
    {
        return count_ == 0;
    }
    size_t size () const noexcept __attribute__ ( ( warn_unused_result ) )
    {
        return count_;
    }

    void clear () noexcept
    {
        buckets_.clear ();
        count_ = 0;
        load_ = 0;
    }

    bool contains ( const KEY &k ) const noexcept
        __attribute__ ( ( warn_unused_result ) )
    {
        return ( findbucket ( k ) != ULONG_MAX );
    }

    size_t count ( const KEY &k ) const noexcept
        __attribute__ ( ( warn_unused_result ) )
    {
        if ( contains ( k ) )
            return 1;
        else
            return 0;
    }

    VALUE get ( const KEY &k ) const
        __attribute__ ( ( warn_unused_result ) ) // can throw std::out_of_range
    {
        size_t bucketid = findbucket ( k );
        if ( bucketid == ULONG_MAX ) throw std::out_of_range ( "no such key" );
        return buckets_[bucketid].value;
    }

    void insert ( const KEY &k, const VALUE &v ) noexcept
    {
        const uint64_t hash = Hash ( k ) | ( BUCKET_VALID | BUCKET_VISIBLE );
        insert ( hash, k, v );
    }

    // insert_or_assign

    bool erase ( const KEY &k ) noexcept
    {
        size_t bucketid = findbucket ( k );
        if ( bucketid == ULONG_MAX ) return false;
        buckets_[bucketid].hashandbits = BUCKET_VALID;
        --count_;
        return true;
    }

private:
    static const uint64_t BUCKET_VALID = 1ul << 63u;
    static const uint64_t BUCKET_VISIBLE = 1ul << 62u;

    // Returns index to found bucket or LONG_MAX
    size_t findbucket ( const KEY k ) const
    {
        if ( UNLIKELY ( count_ == 0 ) ) return ULONG_MAX;
        const uint64_t hash = Hash ( k ) | ( BUCKET_VALID | BUCKET_VISIBLE );
        const size_t mask = buckets_.capacity () - 1;
        assert ( buckets_.capacity () > 0 );
        assert ( mask != 0 );
        assert ( ( ( mask + 1 ) & mask ) == 0 ); // mask+1 is a mask
        size_t bucketidx = hash;
        size_t triangle = 0;
        while ( 1 ) {
            bucketidx &= mask;
            assert ( bucketidx <= buckets_.size () );
            const auto &bucket = buckets_[bucketidx];
            if ( !( bucket.hashandbits & BUCKET_VALID ) ) return ULONG_MAX;

            if ( bucket.hashandbits == hash ) {
                // hash hit, probability very low (2^-62) that this is an actual
                // miss, but we have to check.
                if ( LIKELY ( bucket.key == k ) ) return bucketidx;
            }
            // To improve lookups when hash function has poor distribution, we
            // use quadratic probing with a triangle sequence: 0,1,3,6,10...
            // This will allow complete coverage on a % 2^N hash table.
            ++triangle;
            bucketidx += ( triangle * ( triangle + 1 ) / 2 );
        }
    }

    int uint64_msbit ( uint64_t self ) __attribute__ ( ( const ) )
    {
        if ( self == 0 ) return -1;
        return 63 - __builtin_clzll ( self );
    }

    void rehash ( size_t newcapacity )
    {
        // fprintf ( stderr, "    rehash(%lu) was %lu, count=%lu", newcapacity,
        // buckets_.capacity (), count_ );
        newcapacity = std::max ( count_, newcapacity );
        const size_t minsize = 16;
        newcapacity = std::max ( minsize, newcapacity );

        const auto lg2 = uint64_msbit ( newcapacity | 1u );
        newcapacity = 1uL << lg2;
        // fprintf ( stderr, " resizing to %lu\n", newcapacity );

        std::vector<HashBucket> newbuckets ( newcapacity );
        const auto oldbuckets = buckets_;
        buckets_ = std::move ( newbuckets );
        load_ = 0;
        count_ = 0;

        for ( const auto &b : newbuckets ) assert ( b.hashandbits == 0 );

        for ( size_t i = 0; i != oldbuckets.size (); ++i ) {
            const auto &bucket = oldbuckets[i];
            /*
            fprintf ( stderr, "\nold bucket %lu hash=%lx ", i,
            bucket.hashandbits ); if ( bucket.hashandbits & BUCKET_VALID )
                fprintf ( stderr, "VALID " );
            if ( bucket.hashandbits & BUCKET_VISIBLE )
                fprintf ( stderr, "VISIBLE " );
*/
            if ( bucket.hashandbits & BUCKET_VISIBLE ) {
                insert ( bucket.hashandbits, bucket.key, bucket.value );
            }
        }
        // fprintf ( stderr, "rehashed\n" );
    }

    void insert ( uint64_t hash, const KEY &k, const VALUE &v ) noexcept
    {
        // fprintf ( stderr, "load=%lu, capa=%lu\n", load_, buckets_.capacity ()
        // );
        bool grow = load_ >= ( buckets_.capacity () * 5 / 8 );
        if ( UNLIKELY ( grow ) ) {
            //            fprintf ( stderr, "grow time load=%lu, count=%lu\n",
            //            load_, count_ );
            if ( ( load_ + 1 ) / ( count_ + 1 ) == 1 ) {
                // Expand
                rehash ( buckets_.capacity () * 2 );
            } else {
                // lots of deletes, just rehash table at existing size
                rehash ( buckets_.capacity () );
            }
        }

        assert ( buckets_.capacity () > 0 );
        const size_t mask = buckets_.capacity () - 1;
        assert ( mask != 0 );
        assert ( ( ( mask + 1 ) & mask ) == 0 ); // mask+1 is a mask
        size_t bucketidx = hash;
        size_t triangle = 0;
        while ( 1 ) {
            bucketidx &= mask;
            assert ( bucketidx <= buckets_.size () );
            auto &bucket = buckets_[bucketidx];
            // fprintf ( stderr, "insert hash=%lx, mask=%lx, bucketidx=%lu\n",
            // hash, mask, bucketidx );

            if ( !( bucket.hashandbits & BUCKET_VALID ) ) { // insert
                bucket.hashandbits = hash;
                bucket.key = k;
                bucket.value = v;

                ++count_;
                ++load_;

                // fprintf ( stderr, "inserted hash=%lx\n", hash );
                return;
            }

            // replacement
            if ( bucket.hashandbits == hash ) {
                if ( LIKELY ( bucket.key == k ) ) {
                    bucket.value = v;
                    // fprintf ( stderr, "          replaced %lx\n", hash );
                }
                return;
            }

            // probe
            ++triangle;
            // fprintf ( stderr, "probe %lu  ", triangle );
            bucketidx += ( triangle * ( triangle + 1 ) / 2 );
        }
    }


    struct HashBucket {
        uint64_t hashandbits;
        KEY key;
        VALUE value;
        //        HashBucket () { fprintf ( stderr, "HashBucket ctor %lx\n",
        //        hashandbits ); } ~HashBucket () { fprintf ( stderr,
        //        "HashBucket dtor %lx\n", hashandbits ); }
    };

    std::vector<HashBucket> buckets_ {};

    size_t count_ = 0;
    size_t load_ = 0; /* Included invisible buckets */

    // @TODO: Iterators, Persist, Allocators
};
} // namespace VDB3
