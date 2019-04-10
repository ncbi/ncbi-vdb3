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

#include <cassert>
#include <endian.h>
#include <HashTable.hpp>
#include <cstdbool>
#include <cstdio>
#include <cstdlib>
#include <string>
#define __STDC_FORMAT_MACROS
#include <cinttypes>

#undef memcpy
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define LIKELY(x) __builtin_expect(!!(x), 1)

namespace VDB3
{
    class HashTable {
        HashTable (size_t initial_capacity=0)
        {

        }
        virtual ~HashTable::HashTable()
        {

        }

        size_t size(void) const noexcept;

        void erase() noexcept;
        size_t count(KEY k) const noexcept;
        void insert(KEY k, VALUE v);

        void reserve(size_t capcity);

        float load_factor() const noexcept;
        float max_load_factor() const noexcept;
        void max_laod_factor(float factor);

        V get(KEY k) const noexcept;

        void erase(K) noexcept;
    }

};

#define BUCKET_VALID ((uint64_t)(1ULL << 63u))
#define BUCKET_VISIBLE ((uint64_t)(1ULL << 62u))

/*                          H A S H T B L 1 */
#define MAGIC ((uint64_t)(0x4841534854424C31ULL))

#define cstr KHT_key_type_cstr
#define raw  KHT_key_type_raw

#if 0
static double getloadfactor(const KHashTable* self)
{
    if (self == NULL) return 0.0;
    if (self->num_buckets == 0) return 0.0;

    double load_factor = (double)self->load / (double)self->num_buckets;
    return load_factor;
}

static void dump(const KHashTable* self)
{
    assert(self != NULL);
    size_t bucket_size = self->bucket_size;

    if (self->num_buckets > 64) return;
    fprintf(stderr, "-- table has %ld/%ld %ld\n", self->count, self->load,
            self->num_buckets);
    fprintf(stderr, "bucket_size=%ld\n", bucket_size);
    fprintf(stderr, "key_size=%ld\n", self->key_size);
    for (size_t bucket = 0; bucket != self->num_buckets; bucket++) {
        const char* bucketptr = (char*)self->buckets + (bucket * bucket_size);
        const char* hashptr = bucketptr;
        const char* keyptr = bucketptr + 8;
        uint64_t buckethash;
        memcpy(&buckethash, hashptr, 8);
        fprintf(stderr, "   bucket %03ld hash %lx", bucket, buckethash);
        if (buckethash & BUCKET_VALID) fprintf(stderr, " val");
        if (buckethash & BUCKET_VISIBLE) fprintf(stderr, " vis");
        uint64_t key = 0;
        memcpy(&key, keyptr, self->key_size);
        fprintf(stderr, " key=%lx", key);
        fprintf(stderr, "\n");
    }
}
#endif

static rc_t rehash ( KHashTable *self, size_t capacity )
{
    assert ( self != NULL );

    self->iterator = -1; /* Invalidate any current iterators */

    if ( capacity < self->count ) capacity = self->count;

    uint64_t lg2 = (uint64_t)uint64_msbit ( capacity | 1u );
    capacity = 1ULL << lg2;

    void *old_buckets = self->buckets;
    size_t old_num_buckets = self->num_buckets;

    /* Note that because we reserve two highest bits of hash, table will not
     * benefit from being expanded beyond 2**62 entries (~4 exabytes).
     * But since linux kernels circa 2018 can't support more than 4PB
     * physical/128PB virtual memory (52/57 bits), this isn't a concern,
     * calloc will fail long before this point.
     * Note: Benchmarks show 5% slower if we attempt to round up bucket sizes
     * for better alignment. */
    size_t bucket_size = self->bucket_size;
    void *new_buckets = calloc ( 1, capacity * bucket_size );
    if ( !new_buckets ) {
        return RC ( rcCont, rcTrie, rcInserting, rcMemory, rcExhausted );
    }

    self->num_buckets = capacity;
    self->buckets = new_buckets;
    self->mask = capacity - 1;
    self->count = 0;
    self->load = 0;

    if ( old_buckets ) {
        hashkey_type old_key_type = self->key_type;
        self->key_type = hashkey_raw;

        for ( size_t bucket = 0; bucket != old_num_buckets; bucket++ ) {
            const char *bucketptr
                = (char *)old_buckets + ( bucket * bucket_size );
            const char *hashptr = bucketptr;
            const char *keyptr = bucketptr + 8;
            const char *valueptr = bucketptr + 8 + self->key_size;
            uint64_t buckethash;
            memcpy ( &buckethash, hashptr, 8 );
            if ( ( buckethash & BUCKET_VALID )
                 && ( buckethash & BUCKET_VISIBLE ) ) {
                KHashTableAdd ( self, keyptr, buckethash, valueptr );
            }
        }
        free ( old_buckets );
        self->key_type = old_key_type;
    }

    return 0;
}

rc_t KHashTableMake ( KHashTable **self, size_t key_size,
                      size_t value_size, size_t capacity, double max_load_factor,
                      hashkey_type key_type )
{
    if ( self == NULL )
        return RC ( rcCont, rcTrie, rcConstructing, rcParam, rcInvalid );

    *self = NULL;

    if ( max_load_factor < 0 || max_load_factor >= 1.0 )
        return RC ( rcCont, rcTrie, rcConstructing, rcParam, rcInvalid );

    if ( key_size == 0 )
        return RC ( rcCont, rcTrie, rcConstructing, rcParam, rcInvalid );

    if ( key_type == hashkey_cstr && key_size != sizeof ( char * ) )
        return RC ( rcCont, rcTrie, rcConstructing, rcParam, rcInvalid );

    if ( key_type == hashkey_raw && key_size > sizeof ( void * ) )
        return RC ( rcCont, rcTrie, rcConstructing, rcParam, rcInvalid );

    if ( capacity <= 16 ) capacity = 16;

    KHashTable *kht = (KHashTable *)malloc ( sizeof ( KHashTable ) );
    if ( kht == NULL )
        return RC ( rcCont, rcTrie, rcConstructing, rcMemory, rcExhausted );
    kht->key_size = key_size;
    kht->value_size = value_size;
    kht->key_type = key_type;
    if ( max_load_factor == 0.0 )
        kht->max_load_factor = 0.6;
    else
        kht->max_load_factor = max_load_factor;
    kht->bucket_size = 8 + kht->key_size + kht->value_size;
    kht->buckets = NULL;
    kht->count = 0;

    rc_t rc = rehash ( kht, capacity );

    if ( rc ) {
        free ( kht );
        return rc;
    }

    *self = kht;

    return 0;
}

/* Serialized format is little-endian:
 * uint64_t MAGIC // Used to detect endian switch
 * uint64_t key_size
 * uint64_t value_size
 * uint64_t count
 * uint64_t key_type
 * Followed by count entries of:
 *     keyhash
 *     if key_type is hashkey_raw:
 *        key
 *     else if key_type is hashkey_cstr:
 *        length of key, rounded to multiple of 8 bytes
 *        key
 *     if value_size>0
 *        value
 */
rc_t KHashTableLoad ( KHashTable **self, const KDataBuffer *inbuf )
{
    rc_t rc;

    if ( self == NULL || inbuf == NULL )
        return RC ( rcCont, rcTrie, rcConstructing, rcParam, rcNull );

    *self = NULL;

    uint64_t *pos = (uint64_t *)inbuf->base;
    uint64_t inmagic = *pos++;
    /* fprintf ( stderr, "inmagic is %" PRIx64 "\n", inmagic ); */
    if ( inmagic != MAGIC ) {
#if LINUX
        if ( inmagic == be64toh ( MAGIC ) )
            return RC (
                       rcCont, rcTrie, rcConstructing, rcByteOrder, rcNotAvailable );
#endif
        return RC ( rcCont, rcTrie, rcConstructing, rcFormat, rcUnrecognized );
    }
    /* fprintf ( stderr, "magic OK\n" ); */

    size_t inkey_size = *pos++;
    size_t invalue_size = *pos++;
    size_t incount = *pos++;
    hashkey_type inkey_type = *pos++;

    /* fprintf ( stderr, "inkey size is %zu\n", inkey_size ); */
    /* fprintf ( stderr, "invalue size is %zu\n", invalue_size ); */
    /* fprintf ( stderr, "incount is %zu\n", incount ); */
    /* fprintf ( stderr, "inkey_type is %u\n", inkey_type ); */

    /* if ( inkey_type == hashkey_cstr ) fprintf ( stderr, "hashkey_cstr\n" );
    */
    rc = KHashTableMake (
                         self, inkey_size, invalue_size, incount, 0.0, inkey_type );
    if ( rc ) return rc;

    void *key;
    void *value = NULL;

    for ( size_t cnt = 0; cnt != incount; ++cnt ) {
        uint64_t hash;
        if ( inkey_type == hashkey_raw ) {
            hash = *pos++;
            key = (void *)( pos )++;
            /* fprintf ( stderr, "hashkey_raw key is %zx\n", *(uint64_t *)key );
            */
        } else {
            hash = *pos++;
            size_t keylen = *pos++;
            key = pos++;
            while ( keylen > 8 ) {
                pos++;
                keylen -= 8;
            }

            /* fprintf ( stderr, "hashkey_cstr key is '%s'\n", (const char *)key
             * ); */
        }

        if ( invalue_size ) value = (void *)*pos++;

        /* fprintf ( stderr, "hash is  %zx\n", hash ); */
        rc = KHashTableAdd ( *self, key, hash, &value );
        if ( rc ) return rc;
    }

    return rc;
}

rc_t KHashTableSave ( KHashTable *self, KDataBuffer *outbuf )
{
    rc_t rc = 0;

    if ( self == NULL )
        return RC ( rcCont, rcTrie, rcPersisting, rcParam, rcNull );

    if ( !KDataBufferWritable ( outbuf ) )
        return RC ( rcCont, rcTrie, rcPersisting, rcParam, rcInvalid );

    Vector bytes;
    VectorInit ( &bytes, 0, 1024 );

    rc = VectorAppend ( &bytes, NULL, (void *)MAGIC );
    if ( rc ) return rc;
    rc = VectorAppend ( &bytes, NULL, (void *)self->key_size );
    if ( rc ) return rc;
    rc = VectorAppend ( &bytes, NULL, (void *)self->value_size );
    if ( rc ) return rc;
    rc = VectorAppend ( &bytes, NULL, (void *)self->count );
    if ( rc ) return rc;
    rc = VectorAppend ( &bytes, NULL, (void *)self->key_type );
    if ( rc ) return rc;

    void *key;
    void *value;
    uint64_t keyhash;

    self->iterator = -1; /* Invalidate any current iterators */
    KHashTableIteratorMake ( self );
    while ( KHashTableIteratorNext ( self, &key, &value, &keyhash ) ) {
        if ( self->key_type == hashkey_raw ) {
            rc = VectorAppend ( &bytes, NULL, (void *)keyhash );
            if ( rc ) return rc;

            rc = VectorAppend ( &bytes, NULL, key );
            if ( rc ) return rc;
        } else {
            /* fprintf ( stderr, "saving key='%s'\n", (char *)key ); */
            size_t keylen = strlen ( key );

            /* uint64_t hash = KHash ( key, keylen ); */
            /* fprintf ( stderr, "hash was %zu\n", hash ); */
            rc = VectorAppend ( &bytes, NULL, (void *)keyhash );
            if ( rc ) return rc;

            size_t l8 = ( keylen + 1u + 7u ) & ~7u;
            rc = VectorAppend ( &bytes, NULL, (void *)( l8 ) );
            if ( rc ) return rc;

            void *kstr = malloc ( l8 );
            memmove ( kstr, key, keylen + 1 );

            uint64_t *p = kstr;
            size_t i = 0;
            while ( l8 ) {
                rc = VectorAppend ( &bytes, NULL, (void *)p[i++] );
                if ( rc ) {
                    free ( kstr );
                    return rc;
                }
                l8 -= 8;
            }
            free ( kstr );
        }

        if ( self->value_size ) {
            rc = VectorAppend ( &bytes, NULL, value );
            if ( rc ) return rc;
        }
    }

    KDataBufferResize ( outbuf, VectorLength ( &bytes ) * sizeof ( void * ) );
    memmove (
             outbuf->base, bytes.v, VectorLength ( &bytes ) * sizeof ( void * ) );
    /* for ( uint32_t i = 0; i != VectorLength ( &bytes ); ++i ) {
       fprintf ( stderr, "%3d %p\n", i, VectorGet ( &bytes, i ) );
       }
       fprintf ( stderr, "Table saved %d\n", VectorLength ( &bytes ) );
       */

    VectorWhack ( &bytes, NULL, NULL );
    return rc;
}

void KHashTableDispose ( KHashTable *self,
                         void ( *keywhack ) ( void *item, void *data ),
                         void ( *valuewhack ) ( void *item, void *data ), void *data )
{
    if ( self == NULL ) return;

    /* TODO: for ... */
    if ( keywhack != NULL ) {}
    if ( valuewhack != NULL ) {}
    free ( self->buckets );
    memset ( self, 0, sizeof ( KHashTable ) );
    self->iterator = -1;
    free ( self );
}

size_t KHashTableCount ( const KHashTable *self )
{
    if ( self != NULL ) return self->count;
    return 0;
}

/*
   Since no C++ templates, buckets is going to be variable sized struct:
   {
   u64 keyhash; // we use bit 63 to indicate validity, bit 62 for active (not
   deleted)
   u8[key_size] key;
   u8[value_size] value; // Will not be present for sets
   }
   */

bool KHashTableFind (
                     const KHashTable *self, const void *key, uint64_t keyhash, void *value )

{
    if ( self == NULL || self->buckets == NULL ) return false;

    keyhash |= ( BUCKET_VALID | BUCKET_VISIBLE );
    uint64_t bucket = keyhash;
    const uint64_t mask = self->mask;
    const char *buckets = (const char *)self->buckets;
    const size_t bucket_size = self->bucket_size;
    const hashkey_type key_type = self->key_type;
    const size_t key_size = self->key_size;
    const size_t value_size = self->value_size;
    size_t triangle = 0;
#ifdef DEBUG
    const size_t init_bucket = bucket & mask;
#endif
    while ( 1 ) {
        bucket &= mask;
        const char *bucketptr = buckets + ( bucket * bucket_size );
        const char *hashptr = bucketptr;
        uint64_t buckethash;
        memcpy ( &buckethash, hashptr, 8 );

        if ( !( buckethash & BUCKET_VALID ) ) /* reached invalid bucket */
            return false;

        if ( buckethash == keyhash )
            /* hash hit, probability very low (2^-62) that this is an actual miss,
             * but we have to check. */
        {
            bool found;
            const char *keyptr = bucketptr + 8;

            if ( key_type == hashkey_cstr ) {
                char *p;
                memcpy ( &p, keyptr, sizeof ( char * ) );
                found = ( strcmp ( p, key ) == 0 );
            } else {
                if ( key_size == 8 )
                    found = ( memcmp ( keyptr, key, 8 ) == 0 );
                else if ( key_size == 4 )
                    found = ( memcmp ( keyptr, key, 4 ) == 0 );
                else
                    found = ( memcmp ( keyptr, key, key_size ) == 0 );
            }

            if ( found ) {
                if ( value && value_size ) {
                    const char *valueptr = bucketptr + 8 + key_size;
                    if ( value_size == 8 )
                        memcpy ( value, valueptr, 8 );
                    else if ( value_size == 4 )
                        memcpy ( value, valueptr, 4 );
                    else
                        memcpy ( value, valueptr, value_size );
                }

                return true;
            }
        }

        /* To improve lookups when hash function has poor distribution, we use
         * quadratic probing with a triangle sequence: 0,1,3,6,10...
         * Supposedly this will allow complete coverage on a % 2^N hash table.
         */
        ++triangle;
        bucket += ( triangle * ( triangle + 1 ) / 2 );
#ifdef DEBUG
        assert ( ( bucket & mask ) != init_bucket );
#endif
    }
}

rc_t KHashTableAdd (
                    KHashTable *self, const void *key, uint64_t keyhash, const void *value )
{
    if ( self == NULL || self->buckets == NULL )
        return RC ( rcCont, rcTrie, rcInserting, rcParam, rcInvalid );

    keyhash |= ( BUCKET_VALID | BUCKET_VISIBLE );
    uint64_t bucket = keyhash;
    const uint64_t mask = self->mask;
    char *buckets = (char *)self->buckets;
    const size_t bucket_size = self->bucket_size;
    const hashkey_type key_type = self->key_type;
    const size_t key_size = self->key_size;
    const size_t value_size = self->value_size;
    size_t triangle = 0;

    while ( 1 ) {
        bucket &= mask;
        char *bucketptr = buckets + ( bucket * bucket_size );
        char *hashptr = bucketptr;
        char *keyptr = bucketptr + 8;
        char *valueptr = bucketptr + 8 + key_size;
        uint64_t buckethash;
        memcpy ( &buckethash, bucketptr, 8 );

        if ( !( buckethash & BUCKET_VALID ) ) /* reached invalid bucket */
        {
            memcpy ( hashptr, &keyhash, 8 );

            if ( key_type == hashkey_cstr )
                memcpy ( keyptr, &key, sizeof ( &keyptr ) );
            else {
                if ( key_size == 8 )
                    memcpy ( keyptr, key, 8 );
                else if ( key_size == 4 )
                    memcpy ( keyptr, key, 4 );
                else
                    memcpy ( keyptr, key, key_size );
            }

            if ( value_size ) {
                if ( value_size == 8 )
                    memcpy ( valueptr, value, 8 );
                else if ( value_size == 4 )
                    memcpy ( valueptr, value, 4 );
                else
                    memcpy ( valueptr, value, value_size );
            }

            self->count++;
            self->load++;

            double load_factor = (double)self->load / (double)self->num_buckets;

            if ( load_factor > self->max_load_factor ) {
                if ( (double)self->count / (double)self->load > 0.5 )
                    return rehash ( self, self->num_buckets * 2 );
                /* lots of deletes, just rehash table at existing size */
                return rehash ( self, self->num_buckets );
            }

            return 0;
        }

        if ( buckethash == keyhash ) /* hash hit */
        {
            bool found;
            if ( key_type == hashkey_cstr ) {
                char *p;
                memcpy ( &p, keyptr, sizeof ( char * ) );
                found = ( strcmp ( p, key ) == 0 );
            } else {
                if ( key_size == 8 )
                    found = ( memcmp ( keyptr, key, 8 ) == 0 );
                else if ( key_size == 4 )
                    found = ( memcmp ( keyptr, key, 4 ) == 0 );
                else
                    found = ( memcmp ( keyptr, key, key_size ) == 0 );
            }

            if ( found ) {
                /* replacement */
                if ( value_size ) memcpy ( valueptr, value, value_size );
                return 0;
            }
        }

        ++triangle;
        bucket += ( triangle * ( triangle + 1 ) / 2 );
    }
}

bool KHashTableDelete (
                       KHashTable *self, const void *key, uint64_t keyhash )
{
    if ( self == NULL || self->buckets == NULL ) return false;

    keyhash |= ( BUCKET_VALID | BUCKET_VISIBLE );
    uint64_t bucket = keyhash;
    const uint64_t mask = self->mask;
    char *buckets = (char *)self->buckets;
    const size_t bucket_size = self->bucket_size;
    const hashkey_type key_type = self->key_type;
    const size_t key_size = self->key_size;
    size_t triangle = 0;

    while ( 1 ) {
        bucket &= mask;
        char *bucketptr = buckets + ( bucket * bucket_size );
        char *hashptr = bucketptr;
        uint64_t buckethash;
        memcpy ( &buckethash, hashptr, 8 );

        if ( !( buckethash & BUCKET_VALID ) ) /* reached invalid bucket */
            return false;

        if ( buckethash == keyhash )
            /* hash hit, probability very low (2^-62) that this is an actual miss,
             * but we have to check. */
        {
            bool found;
            const char *keyptr = bucketptr + 8;

            if ( key_type == hashkey_cstr ) {
                char *p;
                memcpy ( &p, keyptr, sizeof ( char * ) );
                found = ( strcmp ( p, key ) == 0 );
            } else {
                if ( key_size == 8 )
                    found = ( memcmp ( keyptr, key, 8 ) == 0 );
                else if ( key_size == 4 )
                    found = ( memcmp ( keyptr, key, 4 ) == 0 );
                else
                    found = ( memcmp ( keyptr, key, key_size ) == 0 );
            }

            if ( found ) {
                buckethash = BUCKET_VALID;
                memcpy ( hashptr, &buckethash, 8 );

                self->count--;
                return true;
            }
        }

        ++triangle;
        bucket += ( triangle * ( triangle + 1 ) / 2 );
    }
}

rc_t KHashTableReserve ( KHashTable *self, size_t capacity )
{
    return rehash ( self, capacity );
}

void KHashTableIteratorMake ( KHashTable *self )
{
    if ( self != NULL ) self->iterator = 0;
}

bool KHashTableIteratorNext (
                             KHashTable *self, void *key, void *value, uint64_t *keyhash )
{
    if ( self == NULL || self->iterator == -1 ) return false;

    char *buckets = (char *)self->buckets;
    const size_t bucket_size = self->bucket_size;

    const size_t key_size = self->key_size;
    const size_t value_size = self->value_size;

    while ( 1 ) {
        if ( (size_t)self->iterator >= self->num_buckets ) {
            self->iterator = -1;
            return false;
        }

        char *bucketptr = buckets + ( self->iterator * bucket_size );
        char *hashptr = bucketptr;
        const char *keyptr = bucketptr + 8;
        const char *valueptr = bucketptr + 8 + self->key_size;
        uint64_t buckethash;
        memcpy ( &buckethash, hashptr, 8 );

        ++self->iterator;

        if ( ( buckethash & BUCKET_VALID )
             && ( buckethash & BUCKET_VISIBLE ) ) {
            memcpy ( key, keyptr, key_size );
            if ( value && value_size ) memcpy ( value, valueptr, value_size );
            if ( keyhash ) *keyhash = buckethash;
            return true;
        }
    }
}

}
