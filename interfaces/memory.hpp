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
#ifndef _hpp_vdb3_memory_
#define _hpp_vdb3_memory_

#include <memory>

namespace VDB3
{

typedef unsigned char byte_t;
typedef size_t bytes_t;

class MemoryBlockItf;

class MemoryManagerItf
{
public:
    typedef size_t size_type;
    typedef void * pointer;
    typedef const void * const_pointer;
    // typedef MemoryBlockItf & reference;
    // typedef const MemoryBlockItf & const_reference;

public:
    virtual ~MemoryManagerItf() = 0;

    // STL allocator-like methods
    virtual pointer allocate ( size_type ) = 0;
    virtual pointer allocate ( size_type, byte_t fill ) = 0;
    // extra overloads for shared/pinned
    virtual pointer allocate_shared ( size_type, byte_t fill ) = 0;
    virtual pointer allocate_shared ( size_type, int fd, off_t offset ) = 0;
    virtual pointer allocate_pinned ( size_type ) = 0;
    virtual pointer allocate_pinned ( size_type, byte_t fill ) = 0;
    // shared *and* pinned?

    virtual pointer reallocate ( pointer, size_type ) = 0;

    // takes care of unsharing/unpinning
    virtual void deallocate ( pointer ) = 0;

    virtual size_type max_size() const noexcept = 0;

// additionally, in requiements for STL allocator
    template <class U, class... Args> void construct (U* p, Args&&... args);
    template <class U> void destroy (U* p);
    // pointer address ( reference x ) const noexcept;
    // const_pointer address ( const_reference x ) const noexcept;
//

    // quotas etc
    virtual size_type quota() const noexcept = 0;
    virtual bool update_quota( size_type min_extension ) const = 0;
    virtual size_type total_free() const noexcept= 0;
    virtual size_type total_used() const noexcept= 0;
    virtual size_type max_free() const noexcept= 0;
};

class PrimordialMemoryMgr : public MemoryManagerItf
{
public:
    PrimordialMemoryMgr();
    virtual ~PrimordialMemoryMgr();

    // STL allocator-like methods
    virtual pointer allocate ( size_type );
    virtual pointer allocate ( size_type, byte_t fill );
    // extra overloads for shared/pinned
    virtual pointer allocate_shared ( size_type, byte_t fill );
    virtual pointer allocate_shared ( size_type, int fd, off_t offset );
    virtual pointer allocate_pinned ( size_type );
    virtual pointer allocate_pinned ( size_type, byte_t fill );
    // shared *and* pinned?

    virtual pointer reallocate ( pointer, size_type );

    // takes care of unsharing/unpinning
    virtual void deallocate ( pointer );

    virtual size_type max_size() const noexcept;

    // quotas etc
    virtual size_type quota() const noexcept;
    virtual bool update_quota( size_type min_extension ) const;
    virtual size_type total_free() const noexcept;
    virtual size_type total_used() const noexcept;
    virtual size_type max_free() const noexcept;
/*
    virtual RawMemoryBlock alloc ( const bytes_t & size );
    virtual void free ( RawMemoryBlock& );

    virtual bytes_t quota() const noexcept;
    virtual bytes_t avail() const noexcept;
*/
protected:
    PrimordialMemoryMgr(const MemoryManagerItf&);
    PrimordialMemoryMgr & operator = (const PrimordialMemoryMgr&);
};

///////////////////////////////////////////////

class MemoryBlockItf
{
public:
    virtual bytes_t size() const = 0;

protected:
    MemoryBlockItf(MemoryManagerItf&);
    MemoryBlockItf(const MemoryBlockItf&);
    virtual ~MemoryBlockItf() = 0;

    MemoryManagerItf& getMgr() const { return * mgr; }

    template < typename T > class Deleter
    {
    public:
        Deleter(MemoryManagerItf& p_mgr) : m_mgr ( p_mgr ) {}
        void operator() (T * p) const
        {
            p -> ~T();
            m_mgr . deallocate ( p );
        }
    private:
        MemoryManagerItf & m_mgr;
    };

private:
    MemoryBlockItf & operator = (const MemoryBlockItf&);

    MemoryManagerItf * mgr;
};

class RawMemoryBlock : public MemoryBlockItf
{
public:
    RawMemoryBlock ( MemoryManagerItf & mgr, size_t size );
    RawMemoryBlock( const RawMemoryBlock & );

    virtual ~RawMemoryBlock();

    virtual bytes_t size() const;
    // void resize ( bytes_t new_size ); // cannot do on a shared_ptr

          byte_t * data()       noexcept { return m_ptr . get (); }
    const byte_t * data() const noexcept { return m_ptr . get (); }

    unsigned long refcount () const noexcept { return ( unsigned long ) m_ptr . use_count(); }

    void fill ( byte_t );

    RawMemoryBlock clone() const; // creates a new block with refcount 1

protected:
    typedef std :: shared_ptr < byte_t > PtrType;
    PtrType getPtr() { return m_ptr; }
    const PtrType getPtr() const { return m_ptr; }

private:
    RawMemoryBlock & operator = (const RawMemoryBlock&);

    size_t m_size;
    PtrType m_ptr;
};

class UniqueRawMemoryBlock : public MemoryBlockItf
{
public:
    UniqueRawMemoryBlock ( MemoryManagerItf & mgr, size_t size );
    UniqueRawMemoryBlock( UniqueRawMemoryBlock && ); // move
    virtual ~UniqueRawMemoryBlock();

    virtual bytes_t size() const;
    void resize ( bytes_t new_size );

          byte_t * data()       noexcept { return m_ptr . get (); }
    const byte_t * data() const noexcept { return m_ptr . get (); }

    void fill ( byte_t );

    UniqueRawMemoryBlock clone() const;

protected:
    typedef std :: unique_ptr < byte_t, Deleter<byte_t> > PtrType;

    PtrType & getPtr() { return m_ptr; }
    const PtrType & getPtr() const { return m_ptr; }

private:
    UniqueRawMemoryBlock & operator = (const UniqueRawMemoryBlock&);

    size_t m_size;
    PtrType m_ptr;
};

template <typename T> class TypedMemoryBlock : public MemoryBlockItf
{
public:
    template<typename... Args> TypedMemoryBlock( MemoryManagerItf& p_mgr, const Args&& ... p_args )
    :   MemoryBlockItf ( p_mgr ),
        m_ptr ( ( T * ) getMgr() . allocate ( sizeof ( T ) ), Deleter<T> ( getMgr() ) )
    {
        new ( m_ptr.get() ) T ( p_args ...);
    }

    TypedMemoryBlock( const TypedMemoryBlock & that )
    :   MemoryBlockItf ( that . getMgr() ),
        m_ptr ( that . m_ptr )
    {
    }

    virtual ~TypedMemoryBlock()
    {
    }

    virtual bytes_t size() const { return sizeof ( T ); }

    // to base type
    operator T& ()          { return * m_ptr . get (); }
    operator const T& ()    { return * m_ptr . get (); }
    // better name "get"? not have at all? (the conversion operator does the job)
          T& data()       noexcept { return * m_ptr . get (); }
    const T& data() const noexcept { return * m_ptr . get (); }

    TypedMemoryBlock clone() const
    {
        return TypedMemoryBlock ( getMgr(), std::forward<const T>( data() ) ); // requires copy ctor from T
    };

protected:
    TypedMemoryBlock & operator = (const TypedMemoryBlock&);

    std :: shared_ptr < T > m_ptr;
};

template <typename T> class RefCountTypedMemoryBlock : public std :: shared_ptr < TypedMemoryBlock < T > >
{
public:
    RefCountTypedMemoryBlock();
    RefCountTypedMemoryBlock( const RefCountTypedMemoryBlock & );
};

} // namespace VDB3

#endif
