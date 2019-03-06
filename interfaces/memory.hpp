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

enum class byte : unsigned char {} ;    //  std::byte in C++17
typedef byte    byte_t;

typedef size_t  bytes_t;

class MemoryBlockItf;

/**
 *  Interface for a memory manager, providing allocation/deallocation for untyped memory blocks.
 *
 *  Provides declarations that satisfy STL requirements for an Allocator:
 *      typedef size_type;
 *      typedef pointer;
 *      typedef const_pointer;
 *      pointer allocate ( size_type bytes );
 *      void deallocate ( pointer ptr, size_type bytes );
 *  Other requirements (e.g. copy construction) are to be satisfied by sub-classes.
 *
 *  TODO: alignment issues.
 */
class MemoryManagerItf
{
public:
    /**
     * Size of a memory block, in bytes.
     */
    typedef size_t size_type;

    /**
     * Address of a modifiable memory block.
     */
    typedef void * pointer;

    /**
     * Address of a non-modifiable memory block.
     */
    typedef const void * const_pointer;

public:
    virtual ~MemoryManagerItf() = 0;

    /**
     * Allocate a block of given size
     * @param bytes requested block size in bytes. Can be 0.
     * @return pointer to the allocated block. nullptr if the specified size was 0
     * @exception TODO: ??? if insufficient memory
     */
    virtual pointer allocate ( size_type bytes ) = 0;

    /**
     * Change the size of a block, possibly reallocating it.
     * @param ptr pointer to the block being reallocated. Can be nullptr, in which case a new block of size 'bytes' will be allocated. The pointer must have been returned by the same instance of MemoryManagerItf and not previously deallocated.
     * @param new_size requested new block size in bytes. Can be 0, resulting in deallocaiton of the block.
     * @return pointer to the reallocated block. May be different from ptr. If the block has been moved, the area pointed to by ptr will be deallocated. nullptr if the specified size was 0.
     * @exception TODO: ??? if insufficient memory. The original block is left untouched; it is not freed or moved.
     */
    virtual pointer reallocate ( pointer ptr, size_type new_size ) = 0;

    /**
     * Deallocate a block.
     * @param ptr pointer to the block being reallocated. The pointer must have been returned by the same instance of MemoryManagerItf and not previously deallocated. Can be nullptr, which is ignored.
     * @param bytes block size in bytes. Must match the value previously passed to allocate.
     */
    virtual void deallocate ( pointer ptr, size_type bytes ) noexcept = 0;
};

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

/**
 *  Memory manager making sure that all new blocks abd deallocated are filled with specific byte values.
 */
class FillingMemoryMgr : public MemoryManagerItf
{
public:
    /**
     * Default value to fill newly allocated blocks with
     */
    static const byte_t DefaultFiller = byte_t ( 0 );
    /**
     * Default value to fill dellocated blocks with
     */
    static const byte_t DefaultTrash  = byte_t ( 0 );

public:
    /**
     * Constructor
     * @param base_mgr optional pointer to a memory manager to handle allocation/deallocation. TODO: If not specified, ??? will be used.
     * @param fill_byte immediately after allocation, the block will be filled with this value
     * @param trash_byte immediately before dealocation, the block will be filled with this value
    */
    FillingMemoryMgr( MemoryManagerItf * base_mgr = nullptr, byte_t fill_byte = DefaultFiller, byte_t trash_byte = DefaultTrash );

    virtual ~FillingMemoryMgr();

    /**
     * Byte value used to fill newly allocated blocks
     * @return Byte value used to fill newly allocated blocks
    */
    byte_t fillByte () const;

    /**
     * Byte value used to fill deallocated blocks
     * @return Byte value used to fill deallocated blocks
    */
    byte_t trashByte () const;

public: // inherited from MemoryManagerItf
    /**
     * Allocate a block of given size and fill it with fill_byte
     * @param bytes see MemoryManagerItf
     * @return see MemoryManagerItf
     * @exception see MemoryManagerItf
     */
    virtual pointer allocate ( size_type bytes );

    /**
     * Change the size of a block, possibly reallocating it. Any extra bytes will be filled with fill_byte.
     * @param ptr see MemoryManagerItf
     * @param new_size see MemoryManagerItf
     * @return see MemoryManagerItf
     * @exception see MemoryManagerItf
     */
    virtual pointer reallocate ( pointer ptr, size_type new_size );

    /**
     * Deallocate a block. Before deallocation the block will be filled with trash_byte.
     * @param ptr see MemoryManagerItf
     * @param bytes see MemoryManagerItf
     */
    virtual void deallocate ( pointer ptr, size_type bytes ) noexcept;
};

/**
 *  Memory manager creating memory blocks (possibly memory-mapped files) that can be shared between processes.
 */
class SharedMemoryMgr : public MemoryManagerItf
{
public:
    /**
     * Constructor
     * @param base_mgr optional pointer to a memory manager to handle post-allocation and pre-deallocation (e.g. filling/trashing)
    */
    SharedMemoryMgr( MemoryManagerItf * base_mgr = nullptr );

    virtual ~SharedMemoryMgr();

public: // inherited from MemoryManagerItf

    /**
     * Allocate an anonymous shared memory block.
     * @param bytes see MemoryManagerItf
     * @return see MemoryManagerItf
     * @exception see MemoryManagerItf
     */
    virtual pointer allocate ( size_type bytes );

    virtual pointer reallocate ( pointer ptr, size_type new_size );

    virtual void deallocate ( pointer ptr, size_type bytes ) noexcept;

public:
    /**
     * Memory-map a (portion of a) file.
     * @param bytes length of the mapped portion, in bytes
     * @param fd file descriptor of an open file to be memory-mapped
     * @param offset offset of the mapped portion, in bytes from the beginning of the file
     * @param shared true if the block is to be shared between processes
     * @return see MemoryManagerItf
     * @exception TODO ??? if failed
     */
    pointer allocate ( size_type bytes, int fd, off_t offset = 0, bool shared = false );
};

/**
 *  Memory manager creating non-swappable memory blocks
 */
class PinnedMemoryMgr : public MemoryManagerItf
{
public:
    /**
     * Constructor
     * @param base_mgr optional pointer to a memory manager to handle post-allocation and pre-deallocation (e.g. filling/trashing)
    */
    PinnedMemoryMgr( MemoryManagerItf * base_mgr = nullptr );

    virtual ~PinnedMemoryMgr();

public: // inherited from MemoryManagerItf

    /**
     * Allocate a non-swappable memory block.
     * @param bytes see MemoryManagerItf
     * @return see MemoryManagerItf
     * @exception see MemoryManagerItf
     */
    virtual pointer allocate ( size_type bytes );

    virtual pointer reallocate ( pointer ptr, size_type new_size );

    virtual void deallocate ( pointer ptr, size_type bytes ) noexcept;
};

/**
 *  Memory manager tracking a specified allocation quota
 */
class QuotaMemoryMgr : public MemoryManagerItf
{
public:
    /**
     * Constructor
     * @param base_mgr optional pointer to a memory manager to handle post-allocation and pre-deallocation (e.g. filling/trashing)
     * @param quota maximum number of bytes to be allocated by this instance at any given time
    */
    QuotaMemoryMgr( MemoryManagerItf * base_mgr, size_type quota );

    virtual ~QuotaMemoryMgr();

public: // inherited from MemoryManagerItf

    /**
     * Allocate a memory block against the quota.
     * @param bytes see MemoryManagerItf
     * @return see MemoryManagerItf
     * @exception see MemoryManagerItf
     */
    virtual pointer allocate ( size_type bytes );

    virtual pointer reallocate ( pointer ptr, size_type new_size );

    virtual void deallocate ( pointer ptr, size_type bytes ) noexcept;

public:
    /**
     * Maximum total amount of memory to be allocated through this instance.
     * @return Maximum total amount of memory to be allocated through this instance, in bytes
     */
    size_type quota() const noexcept;

    /**
     * A virtual method giving a subclass a chance to increase the quota once it has been exhausted.
     * @param min_extension minimum acceptable extension of the quota, in bytes
     * @return true if the quota extension has been granted; the new value of the quota is available throgh quota()
     */
    virtual bool update_quota( size_type min_extension ) { return false; }

    /**
     * Current amount of memory available for allocation.
     * @return Currently available memory, in bytes
     */
    size_type total_free() const noexcept;

    /**
     * Current amount of memory allocated through this instance.
     * @return Currently allocated memory, in bytes
     */
    size_type total_used() const noexcept;

    /**
     * Maximum size of a block currently available for allocation.
     * @return MMaximum size of a block currently available for allocation, in bytes.
     */
    size_type max_free() const noexcept;
};

///////////////////////////////////////////////

/**
 *  Interface for a memory block.
 */
class MemoryBlockItf
{
public:
    /**
     * Size of the memory block, in bytes.
     * @return Size of the memory block, in bytes.
     */
    virtual bytes_t size() const = 0;

protected:
    /**
     * Constructor.
     * @param that source
     */
    MemoryBlockItf ( MemoryManagerItf & that );

    /**
     * Copy constructor.
     * @param that source
     */
    MemoryBlockItf ( const MemoryBlockItf & that );

    virtual ~MemoryBlockItf() = 0;

    /**
     * Access to the memory manager associated with the block.
     * @return instance of memory manager that allocated this block
     */
    MemoryManagerItf& getMgr() const { return * mgr; }

    /**
    *  Deleter template, used to wrap MemoryBlockItf in "allocator" classes passed to STL containers.
    */
    template < typename T > class Deleter
    {
    public:
        /**
         * Constructor.
         * @param p_mgr instance of memory manager to be used for deallocation
         */
        Deleter ( MemoryManagerItf & p_mgr ) : m_mgr ( p_mgr ) {}

        /**
         * Call a destructor on the argument and pass its memory to the associated memory manager for deallocation.
         * @param p pointer to the block to be deallocated
         */
        void operator() ( T * p ) const
        {
            p -> ~T();
            m_mgr . deallocate ( p, sizeof ( T ) );
        }

    private:
        MemoryManagerItf & m_mgr; ///< the memory manager instance to be used for deallocation
    };

private:
    /**
     * Assignment - disabled
     * @param that source
     * @return *this
     */
    MemoryBlockItf & operator = ( const MemoryBlockItf & that );

    MemoryManagerItf * mgr; ///< the memory manager instance to be used for deallocation; TODO: convert to shared_ptr?
};

/**
 *  Raw memory block, a sequence of bytes. Reference-counted.
 */
class RawMemoryBlock : public MemoryBlockItf
{
public:
    /**
     * Constructor
     * @param mgr instance of a memory manager that allocated this block. Will be used for deallocation.
     * @param size size of the block
    */
    RawMemoryBlock ( MemoryManagerItf & mgr, size_t size );

    /**
     * Copy constructor.
     * @param that source
     */
    RawMemoryBlock( const RawMemoryBlock & that );

    virtual ~RawMemoryBlock();

public: // inherited from MemoryBlockItf

    virtual bytes_t size() const;

public:
    /**
     * Access the underlying bytes for read/write.
     * @return underlying bytes
    */
    byte_t * data() noexcept { return m_ptr . get (); }

    /**
     * Access the underlying bytes for read.
     * @return underlying bytes
    */
    const byte_t * data() const noexcept { return m_ptr . get (); }

    /**
     * Number of references to this block.
     * @return Number of references to this block.
    */
    unsigned long refcount () const noexcept { return ( unsigned long ) m_ptr . use_count(); }

    /**
     * Fill the block with specified value.
     * @param filler vallue to fill the block with
    */
    void fill ( byte_t filler );

    /**
     * Create a copy of the block, with reference count 1.
     * @return a copy of the block, with reference count 1.
    */
    RawMemoryBlock clone() const;

protected:
    /**
     * Shared pointer to the sequence of bytes
    */
    typedef std :: shared_ptr < byte_t > PtrType;

    /**
     * Read/write access to the shared pointer
     * @return the shared pointer
    */
    PtrType getPtr() { return m_ptr; }

    /**
     * Read access to the shared pointer
     * @return the shared pointer
    */
    const PtrType getPtr() const { return m_ptr; }

private:
    /**
     * Assignment - deleted
     * @param that source
     * @return *this
    */
    RawMemoryBlock & operator = ( const RawMemoryBlock & that );

    size_t m_size;  ///< size of the block, in bytes
    PtrType m_ptr;  ///< pointer to the underlying bytes
};

/**
 *  A raw memory block with exactly one owner.
 */
class UniqueRawMemoryBlock : public MemoryBlockItf
{
public:
    /**
     * Constructor
     * @param mgr instance of a memory manager that allocated this block. Will be used for deallocation.
     * @param size size of the block
    */
    UniqueRawMemoryBlock ( MemoryManagerItf & mgr, size_t size );

    /**
     * Move constructor.
     * @param that source
     */
    UniqueRawMemoryBlock ( UniqueRawMemoryBlock && that );

    virtual ~UniqueRawMemoryBlock();

public: // inherited from MemoryBlockItf

    virtual bytes_t size () const;

public:

    /**
     * Change the size of the block
     * @param new_size new size of the block
     * @exception TODO: ??? if reallocaiton failed
     */
    void resize ( bytes_t new_size );

    /**
     * Read/write access to the bytes
     * @return underlying bytes
     */
    byte_t * data() noexcept { return m_ptr . get (); }

    /**
     * Read access to the bytes
     * @return underlying bytes
     */
    const byte_t * data() const noexcept { return m_ptr . get (); }

    /**
     * Fill out the block with specified value
     * @param filler vallue to fill the block with
     */
    void fill ( byte_t filler );

    /**
     * Create a copy of the block
     * @return a copy of the block
     * @exception TODO: ??? if failed
     */
    UniqueRawMemoryBlock clone () const;

protected:
    /**
     * Uniquely owned pointer to the sequence bytes
    */
    typedef std :: unique_ptr < byte_t, Deleter < byte_t > > PtrType;

    /**
     * Read/write access to the pointer
     * @return the pointer
    */
    PtrType & getPtr() { return m_ptr; }

    /**
     * Read access to the pointer
     * @return the pointer
    */
    const PtrType & getPtr() const { return m_ptr; }

private:
    /**
     * Assignment - deleted
     * @param that source
     * @return *this
    */
    UniqueRawMemoryBlock & operator = ( const UniqueRawMemoryBlock & that );

    size_t m_size;  ///< size of the block, in bytes
    PtrType m_ptr;  ///< pointer to the underlying bytes
};

/**
 * A typed, reference-counted memory block.
 * TODO: move method implementations to an adjunct header
 */
template <typename T> class TypedMemoryBlock : public MemoryBlockItf
{
public:
    /**
     * Constructor
     * @param p_mgr instance of a memory manager to allocate this block. Will also be used for deallocation.
     * @param p_args arguments to be passed to T's constructor
    */
    template<typename... Args> TypedMemoryBlock( MemoryManagerItf & p_mgr, const Args&& ... p_args )
    :   MemoryBlockItf ( p_mgr ),
        m_ptr ( ( T * ) getMgr() . allocate ( sizeof ( T ) ), Deleter<T> ( getMgr() ) )
    {
        new ( m_ptr.get() ) T ( p_args ...);
    }

    /**
     * Copy constructor.
     * @param that source
     */
    TypedMemoryBlock( const TypedMemoryBlock & that )
    :   MemoryBlockItf ( that . getMgr() ),
        m_ptr ( that . m_ptr )
    {
    }

    virtual ~TypedMemoryBlock()
    {
    }

public: // inherited from MemoryBlockItf

    virtual bytes_t size() const { return sizeof ( T ); }

public:

    /**
     * Conversion to the base type
     * @return underlying value of T
     */
    operator T& () { return * m_ptr . get (); }

    /**
     * Conversion to the base type, read-only
     * @return underlying value of T
     */
    operator const T& () const  { return * m_ptr . get (); }

    /**
     * Read/write access to the typed value
     * @return underlying value of T
     */
    T& data() noexcept { return * m_ptr . get (); }

    /**
     * Read access to the typed value
     * @return underlying value of T
     */
    const T& data() const noexcept { return * m_ptr . get (); }

    /**
     * Copy-construct a new typed value with reference count of 1.
     * T is required to define a copy constructor
     * @return a copy with reference count of 1.
     * @exception TODO: ??? if failed
     */
    TypedMemoryBlock clone() const
    {
        return TypedMemoryBlock ( getMgr(), std::forward<const T>( data() ) );
    };

private:
    /**
     * Assignment - deleted.
     * @param that source
     * @return *this
     */
    TypedMemoryBlock & operator = ( const TypedMemoryBlock & that );

    std :: shared_ptr < T > m_ptr; ///< pointer to the shared block
};

} // namespace VDB3

#endif
