# ===========================================================================
#
#                            PUBLIC DOMAIN NOTICE
#               National Center for Biotechnology Information
#
#  This software/database is a "United States Government Work" under the
#  terms of the United States Copyright Act.  It was written as part of
#  the author's official duties as a United States Government employee and
#  thus cannot be copyrighted.  This software/database is freely available
#  to the public for use. The National Library of Medicine and the U.S.
#  Government have not placed any restriction on its use or reproduction.
#
#  Although all reasonable efforts have been taken to ensure the accuracy
#  and reliability of the software and data, the NLM and the U.S.
#  Government do not and cannot warrant the performance or results that
#  may be obtained by using this software or data. The NLM and the U.S.
#  Government disclaim all warranties, express or implied, including
#  warranties of performance, merchantability or fitness for any particular
#  purpose.
#
#  Please cite the author in any work or product based on this material.
#
# ===========================================================================
##DAS_DIBI_DOH
cmake_minimum_required ( VERSION 2.8 )

set ( CMAKE_VERBOSE_MAKEFILE on )

## Project name
project ( SERVER )

set ( OUT_ROOT "${PROJECT_SOURCE_DIR}/out" )

if ( ${UNIX} )
    set ( OUT_OS "unix" )
elseif ( ${APPLE} )
    set ( OUT_OS "mac" )
else ( ${UNIX} )
    set ( OUT_OS "win" )
endif ( ${UNIX} )

if ( "${CMAKE_BUILD_TYPE}" STREQUAL "Release" )
    set ( OUT_BLD "Release" )
else ()
    set ( OUT_BLD "Debug" )
endif ()

set ( OUT_DIR "${OUT_ROOT}/${OUT_OS}_${OUT_BLD}" )
MESSAGE ( STATUS "OUT_DIR: " ${OUT_DIR} )

set ( LIBRARY_OUTPUT_PATH "${OUT_DIR}/lib" )
set ( EXECUTABLE_OUTPUT_PATH "${OUT_DIR}/bin" )
MESSAGE ( STATUS "LIBRARY_OUTPUT_PATH: " ${LIBRARY_OUTPUT_PATH} )
MESSAGE ( STATUS "EXECUTABLE_OUTPUT_PATH: " ${EXECUTABLE_OUTPUT_PATH} )

add_definitions ( -DARCH )
add_definitions ( -DARCHBITS=64 )
add_definitions ( -Dx86_64 )
add_definitions ( -DUNIX )
add_definitions ( -std=c++11 )

add_subdirectory ( src )
add_subdirectory ( app )

# include_directories ( "$(CMAKE_SOURCE_DIR)/itf" )
