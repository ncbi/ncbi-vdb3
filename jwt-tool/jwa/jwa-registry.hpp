/*==============================================================================
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
 *  Author: Kurt Rodarmer
 *
 * ===========================================================================
 *
 */

#pragma once

#include <ncbi/jwa.hpp>

#include <set>
#include <map>

namespace ncbi
{

    /**
     * @class JWARegistry
     * @brief a global algorithm registry
     */
    class JWARegistry
    {
    public:

        bool acceptJWKAlgorithm ( const String & kty, const String & alg ) const noexcept;
        bool acceptJWSAlgorithm ( const String & alg ) const noexcept;

        JWASignerRef getSigner ( const String & alg ) const;
        JWAVerifierRef getVerifier ( const String & alg ) const;

        void registerSigner ( const String & alg, const JWASignerRef & signer ) noexcept;
        void registerVerifier ( const String & alg, const JWAVerifierRef & verifier ) noexcept;

        String mapCurveNameToNIST ( const String & crv ) const noexcept;
        String mapCurveNameToSECG ( const String & crv ) const noexcept;
        String mapCurveNameToANSI ( const String & crv ) const noexcept;

        void doNothing () noexcept;

        JWARegistry () noexcept;
        ~ JWARegistry () noexcept;

    private:

        void makeMaps () const noexcept;

        struct Maps
        {
            Maps ();
            ~ Maps ();

            // acceptance of signature and verification algorithm names
            // these are NOT necessarily symmetrical
            std :: set < String > sign_accept;
            std :: set < String > verify_accept;

            // signature and verification algorithms
            std :: map < String, JWASignerRef > signers;
            std :: map < String, JWAVerifierRef > verifiers;

            // mapping from key-type to supported algorithm set
            std :: map < String, std :: set < String > > key_accept;

            // mappings from curve names to standard namespace
            std :: map < String, String > crv_to_nist;
            std :: map < String, String > crv_to_secg;
            std :: map < String, String > crv_to_ansi;

            // metrics on curve, algorithm and key-type names
            // gathered from constants when the mappings are created
            count_t jws_alg_max_length;
            count_t jwk_typ_max_length;
            count_t crv_name_max_length;
            bool jws_alg_is_ascii;
            bool jwk_typ_is_ascii;
            bool crv_name_is_ascii;

        };

        static bool testJWSAlgorithm ( const String & alg, const Maps * cmaps );
        static bool testCurveName ( const String & crv, const Maps * cmaps );

        mutable Maps * maps;

    };

    extern JWARegistry gJWARegistry;

    void includeJWA_none ( bool always_false );
    void includeJWA_hmac ( bool always_false );
    void includeJWA_rsa ( bool always_false );
    void includeJWA_ecdsa ( bool always_false );

}
