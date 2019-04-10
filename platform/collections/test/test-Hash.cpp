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

#include <collections/Hash.hpp>

#include <cstdlib>
#include <cstring>
#include <map>
#include <set>
#include <stdexcept>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

using namespace std;
using namespace VDB3;

TEST ( Hash, Basic)
{
    const char *str = "Tu estas probando este hoy, no manana";
    size_t size = strlen ( str );

    uint64_t hash = Hash ( str, size );
    ASSERT_NE ( hash, (uint64_t)0 );

    uint64_t hash2 = Hash ( str, size );
    ASSERT_EQ ( hash, hash2 );

    uint64_t hash3 = HashCStr ( str );
    ASSERT_EQ ( hash, hash3 );
}

TEST_CASE (Hash, String)
{
    std::string str="This space for rent";
    uint64_t hash = Hash(str);
    ASSERT_NE (hash, 0);
    ASSERT_EQ (Hash(str), Hash(str));
}

TEST_CASE (Hash, Integer)
{
    int i=random();
    uint64_t hash=Hash(i);
    ASSERT_EQ(hash, Hash(i));
    i++;
    ASSERT_NE(hash, Hash(i));
}

TEST_CASE (Hash, Float)
{
    float a=0.0,b=-0.0;
    ASSERT_EQ(Hash(a), Hash(b));
}

TEST_CASE (Hash, Double)
{
    double a=0.0,b=-0.0;
    ASSERT_EQ(Hash(a), Hash(b));
}

TEST_CASE ( Hash_unique )
{
    const char *str = "Tu estas probando este hoy, no manana";
    size_t size = strlen ( str );

    uint64_t hash1 = Hash ( str, size );
    uint64_t hash2 = Hash ( str, size - 1 );
    ASSERT_NE ( hash1, hash2 );
}

TEST_CASE ( Hash_Adjacent )
{
    uint64_t hash1, hash2, diff;

    uint64_t val = 0x1234567890ABCDE0;

    hash1 = Hash ( (char *)&val, 8 );
    ++val;
    hash2 = Hash ( (char *)&val, 8 );
    diff = labs ( hash2 - hash1 );
    ASSERT_LE ( diff, (uint64_t)7 );

    const char *str1 = "string01";
    const char *str2 = "string02";
    size_t size = strlen ( str1 );
    hash1 = Hash ( str1, size );
    hash2 = Hash ( str2, size );
    diff = labs ( hash2 - hash1 ) & 0xfffff;
    if ( diff > 7 ) {
        fprintf ( stderr, "%lx %lx\n", hash1, hash2 );
        ASSERT_LE ( diff, (uint64_t)7 );
    }

    str1 = "str01";
    str2 = "str02";
    size = strlen ( str1 );
    hash1 = Hash ( str1, size );
    hash2 = Hash ( str2, size );
    diff = labs ( hash2 - hash1 );
    ASSERT_LE ( diff, (uint64_t)7 );
}

TEST_CASE ( Hash_Collide )
{
    // We fill a buffer with random bytes, and then increment each byte once
    // and verify no collisions occur for all lengths.
    char buf[37];
    for ( size_t l = 0; l != sizeof ( buf ); l++ ) buf[l] = (char)random ();

    std::set<uint64_t> set;

    size_t inserts = 0;
    size_t collisions = 0;
    for ( size_t l = 0; l != sizeof ( buf ); l++ )
        for ( size_t j = 0; j != l; j++ )
            for ( size_t k = 0; k != 255; k++ ) {
                buf[j] += 1;
                uint64_t hash = Hash ( buf, l );
                size_t count = set.count ( hash );
                if ( count ) {
                    collisions++;
                    fprintf ( stderr,
                        "Collision at %lu on hash of len %lu is %lx: "
                        "%lu elements %lx\n",
                        j, l, hash, set.size (), *(uint64_t *)buf );
                }
                set.insert ( hash );
                inserts++;
            }
    ASSERT_EQ ( inserts, set.size () );
}

TEST_CASE ( Hash_Speed )
{
    char key[8192];
    unsigned long loops = 1000000;

    for ( uint64_t i = 0; i != sizeof ( key ); i++ ) key[i] = random ();

    long len = 4;
    while ( len < 10000 ) {
        stopwatch ();
        for ( unsigned long i = 0; i != loops; i++ ) Hash ( key, len );

        unsigned long us = stopwatch ();
        unsigned long hps = 1000000 * loops / us;
        unsigned long mbps = hps * len / 1048576;
        printf ( "Hash %lu %lu us elapsed (%lu hash/sec, %lu Mbytes/sec)\n",
            len, us, hps, mbps );

        len *= 2;
    }
}

TEST_CASE ( string_hash_Speed )
{
    char key[8192];
    unsigned long loops = 1000000;

    for ( uint64_t i = 0; i != sizeof ( key ); i++ ) key[i] = random ();

    long len = 4;
    while ( len < 10000 ) {
        stopwatch ();
        for ( uint64_t i = 0; i != loops; i++ ) string_hash ( key, len );

        unsigned long us = stopwatch ();
        unsigned long hps = 1000000 * loops / us;
        unsigned long mbps = hps * len / 1048576;
        printf (
            "string_hash %lu %lu us elapsed (%lu hash/sec, %lu Mbytes/sec)\n",
            len, us, hps, mbps );

        len *= 2;
    }
}

TEST_CASE ( std_hash_Speed )
{
    unsigned long loops = 1000000;
    string str = "1234";

    std::size_t hash = 0;
    long len = 4;
    while ( len < 10000 ) {
        stopwatch ();
        for ( uint64_t i = 0; i != loops; i++ )
            hash += std::hash<std::string> {}( str );

        unsigned long us = stopwatch () + 1;
        unsigned long hps = 1000000 * loops / us;
        unsigned long mbps = hps * len / 1048576;
        printf (
            "std::hash %lu %lu us elapsed (%lu hash/sec, %lu Mbytes/sec)\n",
            len, us, hps, mbps );

        len *= 2;
        str += str;
    }
}

TEST_CASE ( hash_hamming )
{
    char key[100];
    uint64_t mask = 0xfff;
    uint64_t hash_collisions[mask + 1];
    uint64_t khash_collisions[mask + 1];
    uint64_t rhash_collisions[mask + 1];

    for ( uint64_t i = 0; i != mask + 1; i++ ) {
        hash_collisions[i] = 0;
        khash_collisions[i] = 0;
        rhash_collisions[i] = 0;
    }

    const char *foo1 = "ABCDE1";
    const char *foo2 = "ABCDE2";

    printf ( "khash of %s is %lx, %s is %lx\n", foo1,
        Hash ( foo1, strlen ( foo1 ) ), foo2,
        Hash ( foo2, strlen ( foo2 ) ) );
    printf ( "string_hash of %s is %u, %s is %u\n", foo1,
        string_hash ( foo1, strlen ( foo1 ) ), foo2,
        string_hash ( foo2, strlen ( foo2 ) ) );
    for ( uint64_t i = 0; i != 10000000; i++ ) {
        sprintf ( key, "ABCD%lu", i );
        uint64_t hash = string_hash ( key, strlen ( key ) );
        hash &= mask;
        hash_collisions[hash] = hash_collisions[hash] + 1;

        hash = Hash ( key, strlen ( key ) );
        hash &= mask;
        khash_collisions[hash] = khash_collisions[hash] + 1;

        hash = random ();
        hash &= mask;
        rhash_collisions[hash] = rhash_collisions[hash] + 1;
    }

    uint64_t hash_max = 0;
    uint64_t khash_max = 0;
    uint64_t rhash_max = 0;
    for ( uint64_t i = 0; i != mask; i++ ) {
        if ( hash_collisions[i] > hash_max ) hash_max = hash_collisions[i];
        if ( khash_collisions[i] > khash_max ) khash_max = khash_collisions[i];
        if ( rhash_collisions[i] > rhash_max ) rhash_max = rhash_collisions[i];
    }

    printf ( "string_hash longest probe is %lu\n", hash_max );
    printf ( "khash longest probe is %lu\n", khash_max );
    printf ( "rhash longest probe is %lu\n", rhash_max );
}

TEST ( PinnedMemoryMgr, Instantiate )
{
    PinnedMemoryMgr mgr ( nullptr );
}

TEST ( PinnedMemoryMgr, Allocate )
{
    PinnedMemoryMgr mgr ( nullptr );
    MemoryManagerItf::pointer p = mgr.allocate(1);
    ASSERT_NE ( nullptr, p );
    mgr.deallocate(p, 1);
}

/**
* Fake locker class for testing. Keeps track of currently "pinned" blocks
*/
class TestLocker : public PinnedMemoryMgr :: MemoryLockerItf
{
public:
    TestLocker(){}
    virtual ~TestLocker () {}
    /**
     * Add an address range to the list of "pinned" ranges
     * @param ptr pointer to the start of the address range
     * @param bytes size of the address range
     */
    virtual void lock( MemoryManagerItf :: pointer ptr, MemoryManagerItf :: size_type bytes )
    {
        blocks [ ptr ] = bytes;
    }
    /**
     * Remove an address range from the list of "pinned" ranges
     * @param ptr pointer to the start of the address range
     * @param bytes size of the address range
     */
    virtual void unlock( MemoryManagerItf :: pointer ptr, MemoryManagerItf :: size_type bytes )
    {
        assert ( bytes == blocks [ ptr ] );
        blocks . erase ( ptr );
    }

    typedef std::map < MemoryManagerItf :: pointer, MemoryManagerItf :: size_type > Blocks; ///< block start -> block size
    Blocks blocks; ///< currently pinned blocks
};

TEST ( PinnedMemoryMgr, CustomLocker_Pin )
{
    TestLocker tl;
    PinnedMemoryMgr mgr ( nullptr, & tl );
    MemoryManagerItf :: pointer p = mgr . allocate ( 1 );
    ASSERT_EQ ( 1, tl . blocks [ p ] );

    mgr . deallocate ( p, 1 );
}

TEST ( PinnedMemoryMgr, CustomLocker_Unpin )
{
    TestLocker tl;
    PinnedMemoryMgr mgr ( nullptr, & tl );
    MemoryManagerItf :: pointer p = mgr . allocate ( 1 );

    mgr . deallocate ( p, 1 );
    ASSERT_EQ ( tl . blocks . end (), tl . blocks . find ( p ) );
}

TEST ( PinnedMemoryMgr, CustomLocker_Alloc_0_size )
{
    TestLocker tl;
    PinnedMemoryMgr mgr ( nullptr, & tl );
    MemoryManagerItf :: pointer p = mgr . allocate ( 0 );
    ASSERT_EQ ( nullptr, p );
    ASSERT_EQ ( 0, tl . blocks . size () );
}

TEST ( PinnedMemoryMgr, CustomLocker_Realloc )
{
    TestLocker tl;
    PinnedMemoryMgr mgr ( nullptr, & tl );
    MemoryManagerItf :: pointer p1 = mgr . allocate ( 1 );

    // should call unlock(p1, 1), lock(p2, 100)
    const size_t NewSize = 100;
    MemoryManagerItf :: pointer p2 = mgr . reallocate ( p1, NewSize );
    ASSERT_NE ( p1, p2 );
    ASSERT_EQ ( NewSize, tl . blocks [ p2 ] );
    ASSERT_EQ ( tl . blocks . end (), tl . blocks . find ( p1 ) );

    mgr . deallocate ( p2, NewSize );
    ASSERT_EQ ( 0, tl . blocks . size () );
}

TEST ( PinnedMemoryMgr, CustomLocker_Realloc_0_size )
{
    TestLocker tl;
    PinnedMemoryMgr mgr ( nullptr, & tl );
    MemoryManagerItf :: pointer p1 = mgr . allocate ( 1 );
    MemoryManagerItf :: pointer p2 = mgr . reallocate ( p1, 0 );
    ASSERT_EQ ( nullptr, p2 );
    ASSERT_EQ ( 0, tl . blocks . size () );
}

static PrimordialMemoryMgr primMgr;

/**
* Under valgrind, the default heap manager always moves the block on reallocate()
* In order to test scenarios when reallocaiton does not move the block, we will use this
* simple manager.
*/
class NoMoveReallocMgr : public TrackingMemoryManager
{ // never moves the block on reallocate
public:
    virtual pointer reallocate ( pointer ptr, size_type new_size )
    {   // do not move
        setBlockSize ( ptr, new_size );
        return ptr;
    }
};

TEST ( PinnedMemoryMgr, CustomLocker_Realloc_same_size )
{
    TestLocker tl;
    NoMoveReallocMgr nmm;
    PinnedMemoryMgr mgr ( &nmm, & tl );
    const size_t Size = 100;
    MemoryManagerItf :: pointer p1 = mgr . allocate ( Size );
    MemoryManagerItf :: pointer p2 = mgr . reallocate ( p1, Size );
    ASSERT_EQ ( p1, p2 );
    ASSERT_EQ ( 1, tl . blocks . size () );

    mgr . deallocate ( p2, Size ); // or could be p1
    ASSERT_EQ ( 0, tl . blocks . size () );
}

TEST ( PinnedMemoryMgr, CustomLocker_Realloc_shrink )
{
    TestLocker tl;
    NoMoveReallocMgr nmm;
    PinnedMemoryMgr mgr ( &nmm, & tl );
    MemoryManagerItf :: pointer p1 = mgr . allocate ( 100 );
    const size_t NewSize = 99;
    MemoryManagerItf :: pointer p2 = mgr . reallocate ( p1, NewSize );
    ASSERT_EQ ( p1, p2 );   // hope the PrimordialHeapMgr did not move the memory block
    ASSERT_EQ ( NewSize, tl . blocks [ p1 ] );

    mgr . deallocate ( p2, NewSize ); // or could be p1
    ASSERT_EQ ( 0, tl . blocks . size () );
}

