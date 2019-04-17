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
#include <string>

// c++20 defines [[likely]] [[unlikely]]
#define UNLIKELY( x ) __builtin_expect ( !!( x ), 0 )
#define LIKELY( x ) __builtin_expect ( !!( x ), 1 )

namespace VDB3 {
uint64_t Hash ( const char *s, size_t len ) noexcept
    __attribute__ ( ( pure, warn_unused_result ) );
uint64_t Hash ( const std::string &str ) noexcept __attribute__ ( ( pure ) );
uint64_t Hash ( int i ) noexcept __attribute__ ( ( pure ) );
uint64_t Hash ( long int i ) noexcept __attribute__ ( ( pure ) );
uint64_t Hash ( unsigned long long l ) noexcept __attribute__ ( ( pure ) );
uint64_t Hash ( uint64_t l ) noexcept __attribute__ ( ( pure ) );
uint64_t Hash ( float f ) noexcept __attribute__ ( ( pure ) );
uint64_t Hash ( double d ) noexcept __attribute__ ( ( pure ) );
} // namespace VDB3
