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

/**
 * Unit tests for hashtables
 */

#include <collections/Hash.hpp>
#include <collections/HashTable.hpp>
#include <collections/Random.hpp>
#include <map>
#include <set>
#include <sys/time.h>
#include <unordered_map>
#include <unordered_set>

#include <gtest/gtest.h>
using namespace VDB3;

TEST ( HashTable, Init )
{
    HashTable<int, int> htab;
    ASSERT_EQ ( htab.size (), 0 );
}

TEST ( HashTable, Basic )
{
    std::string str1 = "Tu estas probando este hoy, no manana";
    std::string str2 = "Tu estas probando este hoy, no mananX";

    HashTable<std::string, int> hmap;
    ASSERT_EQ ( hmap.size (), 0 );
    ASSERT_EQ ( hmap.empty (), true );

    hmap.clear ();
    ASSERT_EQ ( hmap.empty (), true );
    ASSERT_EQ ( hmap.contains ( "foo" ), false );
    ASSERT_EQ ( hmap.count ( "foo" ), 0 );
    ASSERT_EQ ( hmap.erase ( "foo" ), false );

    hmap.insert ( str1, 3 );
    ASSERT_EQ ( hmap.size (), 1 );
    ASSERT_EQ ( hmap.empty (), false );
    ASSERT_EQ ( hmap.count ( str1 ), 1 );
    int val = 999;
    ASSERT_NO_THROW ( val = hmap.get ( str1 ) );
    ASSERT_NE ( val, 999 );
    ASSERT_EQ ( hmap.get ( str1 ), 3 );

    hmap.insert ( str2, 4 );
    ASSERT_EQ ( hmap.size (), 2 );
    ASSERT_EQ ( hmap.empty (), false );
    ASSERT_EQ ( hmap.count ( str2 ), 1 );
    ASSERT_EQ ( hmap.get ( str2 ), 4 );
    ASSERT_NO_THROW ( val = hmap.get ( str2 ) );

    ASSERT_EQ ( hmap.erase ( str1 ), true );
    ASSERT_EQ ( hmap.size (), 1 );
    ASSERT_EQ ( hmap.empty (), false );
    ASSERT_EQ ( hmap.count ( str1 ), 0 );
    val = -1;
    ASSERT_THROW ( val = hmap.get ( str1 ), std::out_of_range );
    ASSERT_EQ ( val, -1 );
    ASSERT_EQ ( hmap.count ( str2 ), 1 );
    ASSERT_EQ ( hmap.get ( str2 ), 4 );

    hmap.clear ();
    ASSERT_EQ ( hmap.empty (), true );
    ASSERT_EQ ( hmap.size (), 0 );
    ASSERT_EQ ( hmap.contains ( str1 ), false );
    ASSERT_EQ ( hmap.count ( str1 ), 0 );
    ASSERT_THROW ( val = hmap.get ( str1 ), std::out_of_range );
    ASSERT_EQ ( val, -1 );
    ASSERT_EQ ( hmap.erase ( str1 ), false );
    ASSERT_EQ ( hmap.contains ( str2 ), false );
    ASSERT_EQ ( hmap.count ( str2 ), 0 );
    ASSERT_EQ ( hmap.erase ( str2 ), false );
    ASSERT_THROW ( val = hmap.get ( str2 ), std::out_of_range );
    ASSERT_EQ ( val, -1 );
}

TEST ( HashTable, Dups )
{
    std::string str1 = "Tu estas probando este hoy, no manana";
    std::string str2 = "Tu estas probando este hoy, no mananX";

    HashTable<std::string, int> hmap;
    ASSERT_EQ ( hmap.size (), 0 );
    ASSERT_EQ ( hmap.empty (), true );

    hmap.insert ( str1, 1 );
    hmap.insert ( str2, 2 );
    hmap.insert ( str1, 3 );
    hmap.insert ( str2, 4 );
    hmap.insert ( str1, 5 );
    ASSERT_EQ ( hmap.contains ( "foo" ), false );
    ASSERT_EQ ( hmap.size (), 2 );
    ASSERT_EQ ( hmap.get ( str1 ), 5 );
    ASSERT_EQ ( hmap.get ( str2 ), 4 );
}

TEST ( HashTable, HashTableMapInts )
{
    HashTable<uint64_t, uint64_t> hmap;

    for ( uint64_t i = 0; i != 100; i++ ) { ASSERT_EQ ( hmap.count ( i ), 0 ); }

    size_t count = 0;
    for ( uint64_t i = 0; i != 100; i++ ) {
        uint64_t j = i * 3;
        hmap.insert ( i, j );
        ++count;
        ASSERT_EQ ( hmap.size (), count );
    }

    for ( uint64_t i = 0; i != 100; i++ ) {
        ASSERT_EQ ( hmap.contains ( i ), true );
        ASSERT_EQ ( hmap.get ( i ), i * 3 );
    }

    for ( uint64_t i = 0; i != 10000; i++ ) {
        uint64_t key = i + 9999999;
        ASSERT_EQ ( hmap.contains ( key ), false );
    }
}

TEST ( HashTable, HashTableMapStrings )
{
    Random r;
    HashTable<std::string, uint64_t> hmap;
    std::map<std::string, uint64_t> stlmap;

    for ( auto i = 0; i != 1000; ++i ) {
        size_t l = r.randint ( 1, 33 );
        std::string k = r.randbytes ( l );
        auto pair = std::make_pair ( k, l );
        hmap.insert ( k, l );
        stlmap.insert ( pair );
    }

    ASSERT_EQ ( hmap.size (), stlmap.size () );

    for ( auto const &x : stlmap ) {
        ASSERT_EQ ( hmap.contains ( x.first ), true );
        ASSERT_EQ ( hmap.get ( x.first ), x.second );
        uint64_t val;
        ASSERT_NO_THROW ( val = hmap.get ( x.first ) );
        ASSERT_EQ ( val, x.second );
    }
}

TEST ( HashTable, HashTableMapInts2 )
{
    HashTable<uint64_t, uint64_t> hmap;

    size_t count = 0;
    for ( uint64_t i = 0; i != 1000; i++ ) {
        uint64_t j = i * 3;
        hmap.insert ( i, j );
        count++;
        ASSERT_EQ ( hmap.size (), count );
    }

    for ( uint64_t i = 0; i != 1000; i++ ) {
        ASSERT_EQ ( hmap.contains ( i ), true );
        ASSERT_EQ ( hmap.get ( i ), i * 3 );
    }

    for ( uint64_t i = 0; i != 1000; i++ ) {
        ASSERT_EQ ( hmap.contains ( i + 999999 ), false );
    }
}

TEST ( HashTable, HashTableMapValid )
{
    Random r;
    HashTable<uint64_t, uint64_t> hmap;
    std::map<uint64_t, uint64_t> map;

    const uint64_t loops = 10000;
    for ( uint64_t i = 0; i != loops; ++i ) {
        uint64_t key = r.randint ( 0, loops );
        uint64_t value = i;

        auto pair = std::make_pair ( key, value );
        map.erase ( key );
        map.insert ( pair );
        hmap.insert ( key, value );
        ASSERT_EQ ( hmap.contains ( key ), true );
        ASSERT_EQ ( hmap.get ( key ), value );
    }

    size_t mapcount = map.size ();
    size_t hmapcount = hmap.size ();
    ASSERT_EQ ( mapcount, hmapcount );

    for ( uint64_t i = 0; i != loops; ++i ) {
        uint64_t key = r.randint ( 0, loops );
        bool hfound = hmap.contains ( key );

        uint64_t hvalue = 9999;
        auto mapfound = map.find ( key );
        if ( mapfound == map.end () ) {
            ASSERT_THROW ( hvalue = hmap.get ( key ), std::out_of_range );
            ASSERT_EQ ( hvalue, 9999 );
            ASSERT_EQ ( hfound, false );
        } else {
            ASSERT_EQ ( hfound, true );
            hvalue = hmap.get ( key );
            uint64_t mvalue = mapfound->second;
            ASSERT_EQ ( hvalue, mvalue );
        }
    }
}

TEST ( HashTable, HashTableMapDeletes )
{
    HashTable<int64_t, int64_t> hmap;
    std::map<int64_t, int64_t> map;

    int64_t threshold = random ();  // Delete some fraction of keys
    int64_t threshold2 = random (); // Delete some fraction of keys

    for ( int64_t iter = 0; iter != 3; ++iter ) {
        const int64_t loops = random () % 10000;
        for ( int i = 0; i != loops; ++i ) {
            int64_t key = random () % loops;
            int64_t value = i;

            auto pair = std::make_pair ( key, value );
            if ( random () > threshold2 ) {
                map.erase ( key );
                map.insert ( pair );

                hmap.erase ( key );
                hmap.insert ( key, value );
                ASSERT_EQ ( hmap.contains ( key ), true );
                ASSERT_EQ ( hmap.get ( key ), value );
            }

            if ( random () > threshold ) {
                map.erase ( key );
                hmap.erase ( key );
            }

            size_t mapcount = map.size ();
            size_t hmapcount = hmap.size ();
            ASSERT_EQ ( mapcount, hmapcount );
        }

        for ( int i = 0; i != loops; ++i ) {
            int64_t key = random () % loops;
            int64_t hvalue = 0;
            bool hfound = hmap.contains ( key );

            auto mapfound = map.find ( key );
            if ( mapfound == map.end () ) {
                ASSERT_EQ ( hfound, false );
            } else {
                ASSERT_EQ ( hfound, true );
                hvalue = hmap.get ( key );
                int64_t mvalue = mapfound->second;
                ASSERT_EQ ( hvalue, mvalue );
            }
        }
    }
}

TEST ( HashTable, HashTableMapSmallKeys )
{
    HashTable<uint16_t, uint16_t> hmap;
    std::map<uint16_t, uint16_t> map;

    const int loops = 1000;
    for ( uint16_t i = 0; i != loops; ++i ) {
        auto key = static_cast<uint16_t> ( random () % loops );
        uint16_t value = i;

        auto pair = std::make_pair ( key, value );
        map.erase ( key );
        map.insert ( pair );
        hmap.insert ( key, value );
    }

    size_t mapcount = map.size ();
    size_t hmapcount = hmap.size ();
    ASSERT_EQ ( mapcount, hmapcount );

    for ( int i = 0; i != loops; ++i ) {
        auto key = static_cast<uint16_t> ( random () % loops );
        uint16_t hvalue = 0;
        bool hfound = hmap.contains ( key );

        auto mapfound = map.find ( key );
        if ( mapfound == map.end () ) {
            ASSERT_EQ ( hfound, false );
        } else {
            ASSERT_EQ ( hfound, true );
            hvalue = hmap.get ( key );
            uint16_t mvalue = mapfound->second;
            ASSERT_EQ ( hvalue, mvalue );
        }
    }
}

TEST ( HashTable, HashTableMapReserve )
{
    HashTable<uint16_t, uint16_t> hmap;
    auto capacity = static_cast<size_t> ( random () % 20000 );
    hmap.reserve ( capacity );
    std::map<uint16_t, uint16_t> map;

    const uint16_t loops = 1000;
    for ( uint16_t i = 0; i != loops; ++i ) {
        capacity = static_cast<size_t> ( random () % 20000 );
        hmap.reserve ( capacity );

        auto key = static_cast<uint16_t> ( random () % loops );
        uint16_t value = i;

        auto pair = std::make_pair ( key, value );
        map.erase ( key );
        map.insert ( pair );
        hmap.insert ( key, value );
    }

    capacity = static_cast<size_t> ( random () % 20000 );
    hmap.reserve ( capacity );

    size_t mapcount = map.size ();
    size_t hmapcount = hmap.size ();
    ASSERT_EQ ( mapcount, hmapcount );

    for ( int i = 0; i != loops; ++i ) {
        auto key = static_cast<uint16_t> ( random () % loops );
        uint16_t hvalue = 0;
        bool hfound = hmap.contains ( key );

        auto mapfound = map.find ( key );
        if ( mapfound == map.end () ) {
            ASSERT_EQ ( hfound, false );
        } else {
            ASSERT_EQ ( hfound, true );
            hvalue = hmap.get ( key );
            uint16_t mvalue = mapfound->second;
            ASSERT_EQ ( hvalue, mvalue );
        }
    }
}

#if 0
TEST (HashTable, HashTableSetPersist )
{
    rc_t rc;

    KHashTable *hset;
    rc = KHashTableMake ( &hset, 4, 0, 0, 0.0, hashkey_raw );
    ASSERT_RC ( rc );

    std::set<uint32_t> set;

    uint64_t hash = (uint64_t)random (); // Test probing
    const int loops = 3000;
    for ( int i = 0; i != loops; ++i ) {
        uint32_t key = 123 + random () % loops;
        uint32_t value = i;

        set.erase ( key );
        set.insert ( key );
        rc = KHashTableAdd ( hset, (void *)&key, hash, NULL );
        ASSERT_RC ( rc );
        bool hfound
            = KHashTableFind ( hset, (void *)&key, hash, (void *)&value );
        ASSERT_EQ ( hfound, true );
    }

    uint32_t key = 0xababab;
    rc = KHashTableAdd ( hset, (void *)&key, hash, NULL );
    ASSERT_RC ( rc );
    set.insert ( key );
    key = 0xacacac;
    rc = KHashTableAdd ( hset, (void *)&key, hash, NULL );
    ASSERT_RC ( rc );
    set.insert ( key );

    KDataBuffer db;
    KDataBuffer wdb;
    rc = KDataBufferMakeBytes ( &db, 1 );
    ASSERT_RC ( rc );
    rc = KDataBufferMakeWritable ( &db, &wdb );
    ASSERT_RC ( rc );
    KDataBufferWhack ( &db );
    rc = KHashTableSave ( hset, &wdb );
    ASSERT_RC ( rc );
    KHashTableDispose ( hset, NULL, NULL, NULL );
    hset = NULL;

    rc = KHashTableLoad ( &hset, &wdb );
    ASSERT_RC ( rc );

    bool hfound;
    hfound = KHashTableFind ( hset, (void *)&key, hash, NULL );
    ASSERT_EQ ( hfound, true );
    key = 0x9999999;
    hfound = KHashTableFind ( hset, (void *)&key, hash, NULL );
    ASSERT_EQ ( hfound, false );

    size_t setcount = set.size ();
    size_t hsetcount = KHashTableCount ( hset );
    ASSERT_EQ ( setcount, hsetcount );

    for ( int i = 0; i != loops; ++i ) {
        uint32_t key = random () % loops;
        int hvalue = i;
        hfound = KHashTableFind ( hset, (void *)&key, hash, &hvalue );

        auto setfound = set.find ( key );
        if ( setfound == set.end () ) {
            ASSERT_EQ ( hfound, false );
        } else {
            ASSERT_EQ ( hfound, true );
            ASSERT_EQ ( hvalue, i );
        }
    }

    KDataBufferWhack ( &wdb );
    KHashTableDispose ( hset, NULL, NULL, NULL );
}


TEST (HashTable, HashTableMapValidPersist )
{
    rc_t rc;

    KHashTable *hmap;
    rc = KHashTableMake ( &hmap, 4, 4, 0, 0.0, hashkey_raw );
    ASSERT_RC ( rc );

    std::map<uint32_t, uint32_t> map;

    uint64_t hash = (uint64_t)random (); // Test probing
    const int loops = 10000;
    for ( int i = 0; i != loops; ++i ) {
        uint32_t key = random () % loops;
        uint32_t value = i;

        auto pair = std::make_pair ( key, value );
        map.erase ( key );
        map.insert ( pair );
        rc = KHashTableAdd ( hmap, (void *)&key, hash, (void *)&value );
        ASSERT_RC ( rc );
        bool hfound
            = KHashTableFind ( hmap, (void *)&key, hash, (void *)&value );
        ASSERT_EQ ( hfound, true );
    }

    KDataBuffer db;
    KDataBuffer wdb;
    rc = KDataBufferMakeBytes ( &db, 1 );
    ASSERT_RC ( rc );
    rc = KDataBufferMakeWritable ( &db, &wdb );
    ASSERT_RC ( rc );
    KDataBufferWhack ( &db );
    rc = KHashTableSave ( hmap, &wdb );
    ASSERT_RC ( rc );
    KHashTableDispose ( hmap, NULL, NULL, NULL );
    hmap = NULL;

    rc = KHashTableLoad ( &hmap, &wdb );
    ASSERT_RC ( rc );

    size_t mapcount = map.size ();
    size_t hmapcount = KHashTableCount ( hmap );
    ASSERT_EQ ( mapcount, hmapcount );

    for ( int i = 0; i != loops; ++i ) {
        uint32_t key = random () % loops;
        uint32_t hvalue = 0;
        bool hfound = KHashTableFind ( hmap, (void *)&key, hash, &hvalue );

        auto mapfound = map.find ( key );
        // fprintf ( stderr, "#%d, key=%d %d mapfound=%d hfound=%d\n", i, key,
        //    mapfound != map.end (), mapfound, hfound );
        if ( mapfound == map.end () ) {
            ASSERT_EQ ( hfound, false );
        } else {
            uint32_t mvalue = mapfound->second;
            ASSERT_EQ ( hfound, true );
            ASSERT_EQ ( hvalue, mvalue );
        }
    }

    KDataBufferWhack ( &wdb );
    KHashTableDispose ( hmap, NULL, NULL, NULL );
}

TEST (HashTable, HashTableMapIterator )
{
    const int loops = 10000;
    rc_t rc;

    KHashTable *hmap;
    rc = KHashTableMake ( &hmap, 4, 4, loops, 0.0, hashkey_raw );
    ASSERT_RC ( rc );
    uint32_t key;
    uint32_t value;

    std::map<uint32_t, uint32_t> map;
    for ( int iter = 0; iter != 2; ++iter ) {
        for ( int i = 0; i != loops; ++i ) {
            key = random () % loops;
            value = i;
            uint64_t hash = KHash ( (char *)&key, 4 );

            auto pair = std::make_pair ( key, value );
            map.erase ( key );
            map.insert ( pair );
            rc = KHashTableAdd ( hmap, (void *)&key, hash, (void *)&value );
            ASSERT_RC ( rc );

            size_t mapcount = map.size ();
            size_t hmapcount = KHashTableCount ( hmap );
            ASSERT_EQ ( mapcount, hmapcount );
        }
        for ( int i = 0; i != loops; ++i ) {
            key = random () % loops;
            uint64_t hash = KHash ( (char *)&key, 4 );

            map.erase ( key );
            KHashTableDelete ( hmap, (void *)&key, hash );
            bool found = KHashTableFind ( hmap, (void *)&key, hash, NULL );
            ASSERT_EQ ( found, false );

            size_t mapcount = map.size ();
            size_t hmapcount = KHashTableCount ( hmap );
            ASSERT_EQ ( mapcount, hmapcount );
        }
        for ( int i = 0; i != loops; ++i ) {
            key = random () % loops;
            value = random ();
            uint64_t hash = KHash ( (char *)&key, 4 );

            auto pair = std::make_pair ( key, value );
            map.erase ( key );
            map.insert ( pair );
            rc = KHashTableAdd ( hmap, (void *)&key, hash, (void *)&value );
            ASSERT_RC ( rc );

            size_t mapcount = map.size ();
            size_t hmapcount = KHashTableCount ( hmap );
            ASSERT_EQ ( mapcount, hmapcount );
        }

        size_t founds = 0;
        key = loops + 1;
        KHashTableIteratorMake ( hmap );
        while ( KHashTableIteratorNext ( hmap, &key, &value, NULL ) ) {
            auto mapfound = map.find ( key );
            if ( mapfound == map.end () ) {
                // fprintf ( stderr, "no key=%d\n", key );
                ASSERT_EQ ( true, false );
            } else {
                uint32_t mvalue = mapfound->second;
                ASSERT_EQ ( value, mvalue );
                ++founds;
            }
        }
        size_t mapcount = map.size ();
        size_t hmapcount = KHashTableCount ( hmap );
        ASSERT_EQ ( founds, mapcount );
        ASSERT_EQ ( founds, hmapcount );

        KHashTableIteratorMake ( hmap );
        while ( KHashTableIteratorNext ( hmap, &key, NULL, NULL ) ) {
            map.erase ( key );
            uint64_t hash = KHash ( (char *)&key, 4 );
            KHashTableDelete ( hmap, (void *)&key, hash );
        }
        mapcount = map.size ();
        hmapcount = KHashTableCount ( hmap );
        ASSERT_EQ ( mapcount, hmapcount );
        ASSERT_EQ ( mapcount, (size_t)0 );
    }
    KHashTableDispose ( hmap, NULL, NULL, NULL );
}
#endif

static double stopwatch ( double start = 0.0 ) ATTRWARNUNUSED;
static double stopwatch ( double start )
{
    struct timeval tv_cur = {};
    gettimeofday ( &tv_cur, nullptr );
    double finish = static_cast<double> ( tv_cur.tv_sec )
        + static_cast<double> ( tv_cur.tv_usec ) / 1000000.0;
    if ( start == 0.0 ) return finish;
    double elapsed = finish - start;
    if ( elapsed == 0.0 ) elapsed = 1 / 1000000000.0;
    return elapsed;
}

static std::vector<uint64_t> make_benchkeys ()
{
    Random r;
    std::vector<uint64_t> benchkeys;
    size_t sz = 1u << 24u;
    for ( size_t i = 0; i != sz; ++i ) benchkeys.push_back ( i );
    for ( size_t i = 0; i != sz; ++i ) benchkeys.push_back ( r () );

    return benchkeys;
}

TEST ( HashTable, Benchmark_stdSetBench )
{
    const uint64_t loops = 1000000;
    auto benchkeys = make_benchkeys ();
    std::set<uint64_t> hset;

    for ( size_t numelem = 4; numelem <= benchkeys.size (); numelem *= 2 ) {
        hset.clear ();

        double start = stopwatch ();
        for ( size_t i = 0; i != numelem; ++i ) {
            auto idx = i;
            hset.insert ( benchkeys[idx] );
        }
        size_t sz = hset.size ();
        ASSERT_EQ ( sz, numelem );
        printf ( "std::set " );
        double elapsed = stopwatch ( start );
        printf ( "required %.1fs to insert %lu\n", elapsed, numelem );

        start = stopwatch ();
        uint64_t c = 0;
        for ( uint64_t loop = 0; loop != loops; loop++ ) {
            c += hset.count ( loop );
        }
        elapsed = stopwatch ( start );
        printf ( "Found %lu,", c );
        double lps = static_cast<double> ( loops ) / elapsed;
        printf ( "numelem=%lu\t%.1f Mlookups/sec, ", numelem, lps / 1000000.0 );

        start = stopwatch ();
        c = 0;
        for ( uint64_t loop = 0; loop != loops; loop++ ) {
            c += hset.count ( benchkeys[loop] );
        }
        elapsed = stopwatch ( start );
        printf ( "Random found %lu,", c );
        lps = static_cast<double> ( loops ) / elapsed;
        printf ( "\t%.1f Mlookups/sec, ", lps / 1000000.0 );
        printf ( "\n" );
    }
    printf ( "\n" );
}

TEST ( HashTable, Benchmark_stdunorderedSetBench )
{
    const uint64_t loops = 1000000;
    auto benchkeys = make_benchkeys ();
    std::unordered_set<uint64_t> hset;

    for ( size_t numelem = 4; numelem <= benchkeys.size (); numelem *= 2 ) {
        hset.clear ();

        double start = stopwatch ();
        for ( size_t i = 0; i != numelem; ++i ) {
            auto idx = i;
            hset.insert ( benchkeys[idx] );
        }
        size_t sz = hset.size ();
        ASSERT_EQ ( sz, numelem );
        printf ( "std::unordered_set " );
        double elapsed = stopwatch ( start );
        printf ( "required %.1fs to insert %lu\n", elapsed, numelem );

        start = stopwatch ();
        uint64_t c = 0;
        for ( uint64_t loop = 0; loop != loops; loop++ ) {
            c += hset.count ( loop );
        }
        elapsed = stopwatch ( start );
        printf ( "Found %lu,", c );
        double lps = static_cast<double> ( loops ) / elapsed;
        printf ( "numelem=%lu\t%.1f Mlookups/sec, ", numelem, lps / 1000000.0 );

        start = stopwatch ();
        c = 0;
        for ( uint64_t loop = 0; loop != loops; loop++ ) {
            c += hset.count ( benchkeys[loop] );
        }
        elapsed = stopwatch ( start );
        printf ( "Random found %lu,", c );
        lps = static_cast<double> ( loops ) / elapsed;
        printf ( "\t%.1f Mlookups/sec, ", lps / 1000000.0 );
        printf ( "\n" );
    }
    printf ( "\n" );
}

TEST ( HashTable, Benchmark_HashMap )
{
    const uint64_t loops = 1000000;
    auto benchkeys = make_benchkeys ();
    HashTable<uint64_t, uint64_t> hmap;

    for ( size_t numelem = 4; numelem <= benchkeys.size (); numelem *= 2 ) {
        hmap.clear ();

        double start = stopwatch ();
        for ( size_t i = 0; i != numelem; ++i ) {
            auto idx = i;
            hmap.insert ( benchkeys[idx], i );
        }
        size_t sz = hmap.size ();
        ASSERT_EQ ( sz, numelem );
        printf ( "VDB3::HashTable " );
        double elapsed = stopwatch ( start );
        printf ( "required %.1fs to insert %lu\n", elapsed, numelem );

        start = stopwatch ();
        uint64_t c = 0;
        for ( uint64_t loop = 0; loop != loops; loop++ ) {
            c += hmap.count ( loop );
        }
        elapsed = stopwatch ( start );
        printf ( "Found %lu,", c );
        double lps = static_cast<double> ( loops ) / elapsed;
        printf ( "numelem=%lu\t%.1f Mlookups/sec, ", numelem, lps / 1000000.0 );

        start = stopwatch ();
        c = 0;
        size_t n = benchkeys.size () / 2;
        for ( uint64_t loop = 0; loop != loops; loop++ ) {
            c += hmap.count ( benchkeys[n] );
            ++n;
        }
        elapsed = stopwatch ( start );
        printf ( "Random found %lu,", c );
        lps = static_cast<double> ( loops ) / elapsed;
        printf ( "\t%.1f Mlookups/sec, ", lps / 1000000.0 );
        printf ( "\n" );
    }
    printf ( "\n" );
}

TEST ( HashTable, Benchmark_Varying )
{
    // https://martin.ankerl.com/2019/04/01/hashmap-benchmarks-03-02-result-RandomDistinct2/
    Random r;
    double start, elapsed;
    int checksum = 0;
    start = stopwatch ();
    {
        std::unordered_map<uint64_t, int> map;
        for ( size_t i = 0; i < 50000000; ++i ) {
            checksum += ++map[r.randint ( 0, 25000000 )];
        }
        map.clear ();
        elapsed = stopwatch ( start );
        printf ( "stdmap took %.1f sec\n", elapsed );
    }

    start = stopwatch ();
    {
        HashTable<uint64_t, int> map;
        for ( size_t i = 0; i < 50000000; ++i ) {
            uint64_t idx = r.randint ( 0, 25000000 );
            if ( map.contains ( idx ) ) {
                int v = map.get ( idx );
                checksum += v;
                map.insert ( idx, v + 1 );
            } else
                map.insert ( idx, 0 );
        }
        map.clear ();
        elapsed = stopwatch ( start );
        printf ( "HashTable took %.1f sec\n", elapsed );
        printf ( "%d\n", checksum );
    }
}
