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

#include <ncbi/jwa.hpp>
#include "jwa-registry.hpp"

namespace ncbi
{

    JWARegistry gJWARegistry;

    bool JWARegistry :: acceptJWKAlgorithm ( const String & kty, const String & alg ) const noexcept
    {
        makeMaps ();

        try
        {
            const Maps * cmaps = maps;

            if ( kty . isEmpty () )
                return false;
            if ( cmaps -> jwk_typ_is_ascii && ! kty . isAscii () )
                return false;
            if ( kty . length () > cmaps -> jwk_typ_max_length )
                return false;

            auto it1 = cmaps -> key_accept . find ( kty );
            if ( it1 != cmaps -> key_accept . cend () )
            {
                const std :: set < String > & accept = it1 -> second;
                auto it2 = accept . find ( alg );
                if ( it2 != accept . cend () )
                    return true;
            }
        }
        catch ( ... )
        {
        }

        return false;
    }

    bool JWARegistry :: acceptJWSAlgorithm ( const String & alg ) const noexcept
    {
        makeMaps ();

        try
        {
            const Maps * cmaps = maps;

            if ( ! testJWSAlgorithm ( alg, cmaps ) )
                return false;

            auto it1 = cmaps -> verify_accept . find ( alg );
            return it1 != cmaps -> verify_accept . cend ();
        }
        catch ( ... )
        {
        }

        return false;
    }

    JWASignerRef JWARegistry :: getSigner ( const String & alg ) const
    {
        makeMaps ();
        const Maps * cmaps = maps;

        if ( testJWSAlgorithm ( alg, cmaps ) )
        {
            auto it = cmaps -> signers . find ( alg );
            if ( it != cmaps -> signers . cend () )
                return it -> second;
        }

        throw JWAUnsupported (
            XP ( XLOC )
            << "signer"
            << " for algorithm '"
            << alg
            << "' is not supported"
            );
    }

    JWAVerifierRef JWARegistry :: getVerifier ( const String & alg ) const
    {
        makeMaps ();
        const Maps * cmaps = maps;

        if ( testJWSAlgorithm ( alg, cmaps ) )
        {
            auto it = cmaps -> verifiers . find ( alg );
            if ( it != cmaps -> verifiers . cend () )
                return it -> second;
        }

        throw JWAUnsupported (
            XP ( XLOC )
            << "verifier"
            << " for algorithm '"
            << alg
            << "' is not supported"
            );
    }

    void JWARegistry :: registerSigner ( const String & alg, const JWASignerRef & signer ) noexcept
    {
        makeMaps ();

        const Maps * cmaps = maps;
        auto it = cmaps -> sign_accept . find ( alg );
        if ( it != cmaps -> sign_accept . cend () )
            maps -> signers . emplace ( alg, signer );
    }

    void JWARegistry :: registerVerifier ( const String & alg, const JWAVerifierRef & verifier ) noexcept
    {
        makeMaps ();

        const Maps * cmaps = maps;
        auto it = cmaps -> verify_accept . find ( alg );
        if ( it != cmaps -> verify_accept . cend () )
            maps -> verifiers . emplace ( alg, verifier );
    }

    String JWARegistry :: mapCurveNameToNIST ( const String & crv ) const noexcept
    {
        makeMaps ();
        const Maps * cmaps = maps;

        String mapped;
        if ( testCurveName ( crv, cmaps ) )
        {
            auto it = cmaps -> crv_to_nist . find ( crv );
            if ( it != cmaps -> crv_to_nist . cend () )
                mapped = it -> second;
        }

        return mapped;
    }

    String JWARegistry :: mapCurveNameToSECG ( const String & crv ) const noexcept
    {
        makeMaps ();
        const Maps * cmaps = maps;

        String mapped;
        if ( testCurveName ( crv, cmaps ) )
        {
            auto it = cmaps -> crv_to_secg . find ( crv );
            if ( it != cmaps -> crv_to_secg . cend () )
                mapped = it -> second;
        }

        return mapped;
    }

    String JWARegistry :: mapCurveNameToANSI ( const String & crv ) const noexcept
    {
        makeMaps ();
        const Maps * cmaps = maps;

        String mapped;
        if ( testCurveName ( crv, cmaps ) )
        {
            auto it = cmaps -> crv_to_ansi . find ( crv );
            if ( it != cmaps -> crv_to_ansi . cend () )
                mapped = it -> second;
        }

        return mapped;
    }

    void JWARegistry :: doNothing () noexcept
    {
        includeJWA_none ( false );
        includeJWA_hmac ( false );
        includeJWA_rsa ( false );
        includeJWA_ecdsa ( false );
    }

    void JWARegistry :: makeMaps () const noexcept
    {
        if ( maps == nullptr )
        {
            try
            {
                Maps * tmp = new Maps ();
                maps = tmp;
            }
            catch ( ... )
            {
            }
        }
    }

    bool JWARegistry :: testJWSAlgorithm ( const String & alg, const Maps * cmaps )
    {
        if ( alg . isEmpty () )
            return false;
        if ( cmaps -> jws_alg_is_ascii && ! alg . isAscii () )
            return false;
        if ( alg . length () > cmaps -> jws_alg_max_length )
            return false;
        return true;
    }

    bool JWARegistry :: testCurveName ( const String & crv, const Maps * cmaps )
    {
        if ( crv . isEmpty () )
            return false;
        if ( cmaps -> crv_name_is_ascii && ! crv . isAscii () )
            return false;
        if ( crv . length () > cmaps -> crv_name_max_length )
            return false;
        return true;
    }

    JWARegistry :: JWARegistry () noexcept
    {
        makeMaps ();
    }

    JWARegistry :: ~ JWARegistry () noexcept
    {
        try
        {
            delete maps;
            maps = nullptr;
        }
        catch ( ... )
        {
        }
    }

    JWARegistry :: Maps :: Maps ()
        : jws_alg_max_length ( 0 )
        , jwk_typ_max_length ( 3 )
        , crv_name_max_length ( 0 )
        , jws_alg_is_ascii ( true )
        , jwk_typ_is_ascii ( true )
        , crv_name_is_ascii ( true )
    {
        size_t i;

        const char * sign_accept_algs [] =
        {
#if JWA_TESTING
            "none",
            "HS256", "HS384", "HS512",
#endif
            "RS256", "RS384", "RS512",
            "ES256", "ES384", "ES512",
            "PS256", "PS384", "PS512"
        };

        const char * verify_accept_algs [] =
        {
#if JWA_TESTING
            "none",
#endif
            "HS256", "HS384", "HS512",
            "RS256", "RS384", "RS512",
            "ES256", "ES384", "ES512",
            "PS256", "PS384", "PS512"
        };

        const char * oct_key_accept_algs [] =
        {
            "HS256", "HS384", "HS512"
        };

        const char * RSA_key_accept_algs [] =
        {
            "RS256", "RS384", "RS512"
        };

        const char * EC_key_accept_algs [] =
        {
            "ES256", "ES384", "ES512"
        };

        const char * OKP_key_accept_algs [] =
        {
            // TBD - examine more thoroughly
            "EdDSA"
        };

        struct { const char * nist, * secg; } NIST_SECG_curve_names [] =
        {
            { "B-163",  "sect163r2" },
            { "B-233",  "sect233r1" },
            { "B-283",  "sect283r1" },
            { "B-409",  "sect409r1" },
            { "B-571",  "sect571r1" },

            { "K-163",  "sect163k1" },
            { "K-233",  "sect233k1" },
            { "K-283",  "sect283k1" },
            { "K-409",  "sect409k1" },
            { "K-571",  "sect571k1" },

            { "P-192",  "secp192r1" },
            { "P-224",  "secp224r1" },
            { "P-256",  "secp256r1" }, // JOSE
            { "P-256K", "secp256k1" },
            { "P-384",  "secp384r1" }, // JOSE
            { "P-521",  "secp521r1" }  // JOSE
        };

        struct { const char * ansi, * secg; } ANSI_SECG_curve_names [] =
        {
            { "prime192v1", "secp192r1" },
            { "prime256v1", "secp256r1" }
        };

        for ( i = 0; i < sizeof sign_accept_algs / sizeof sign_accept_algs [ 0 ]; ++ i )
        {
            String str ( sign_accept_algs [ i ] );
            sign_accept . emplace ( str );
            if ( str . length () > jws_alg_max_length )
                jws_alg_max_length = str . length ();
            if ( ! str . isAscii () )
                jws_alg_is_ascii = false;
        }

        for ( i = 0; i < sizeof verify_accept_algs / sizeof verify_accept_algs [ 0 ]; ++ i )
        {
            String str ( verify_accept_algs [ i ] );
            verify_accept . emplace ( str );
            if ( str . length () > jws_alg_max_length )
                jws_alg_max_length = str . length ();
            if ( ! str . isAscii () )
                jws_alg_is_ascii = false;
        }

        std :: set < String > oct_set;
        for ( i = 0; i < sizeof oct_key_accept_algs / sizeof oct_key_accept_algs [ 0 ]; ++ i )
            oct_set . emplace ( String ( oct_key_accept_algs [ i ] ) );

        std :: set < String > RSA_set;
        for ( i = 0; i < sizeof RSA_key_accept_algs / sizeof RSA_key_accept_algs [ 0 ]; ++ i )
            RSA_set . emplace ( String ( RSA_key_accept_algs [ i ] ) );

        std :: set < String > EC_set;
        for ( i = 0; i < sizeof EC_key_accept_algs / sizeof EC_key_accept_algs [ 0 ]; ++ i )
            EC_set . emplace ( String ( EC_key_accept_algs [ i ] ) );

        std :: set < String > OKP_set;
        for ( i = 0; i < sizeof OKP_key_accept_algs / sizeof OKP_key_accept_algs [ 0 ]; ++ i )
            OKP_set . emplace ( String ( OKP_key_accept_algs [ i ] ) );

        // this is currently a hard-coded set of key types
        // they are ASCII with length 3
        key_accept . emplace ( "oct", oct_set );
        key_accept . emplace ( "RSA", RSA_set );
        key_accept . emplace ( "EC",  EC_set  );
        key_accept . emplace ( "OKP", OKP_set  );

        for ( i = 0; i < sizeof NIST_SECG_curve_names / sizeof NIST_SECG_curve_names [ 0 ]; ++ i )
        {
            String nist ( NIST_SECG_curve_names [ i ] . nist );
            String secg ( NIST_SECG_curve_names [ i ] . secg );

            crv_to_nist . emplace ( nist, nist );
            crv_to_nist . emplace ( secg, nist );
            crv_to_secg . emplace ( secg, secg );
            crv_to_secg . emplace ( nist, secg );

            if ( nist . length () > crv_name_max_length )
                crv_name_max_length = nist . length ();
            if ( secg . length () > crv_name_max_length )
                crv_name_max_length = secg . length ();
            if ( ! nist . isAscii () || ! secg . isAscii () )
                crv_name_is_ascii = false;
        }

        for ( i = 0; i < sizeof ANSI_SECG_curve_names / sizeof ANSI_SECG_curve_names [ 0 ]; ++ i )
        {
            String ansi ( ANSI_SECG_curve_names [ i ] . ansi );
            String secg ( ANSI_SECG_curve_names [ i ] . secg );

            crv_to_ansi . emplace ( ansi, ansi );
            crv_to_ansi . emplace ( secg, ansi );
            crv_to_secg . emplace ( ansi, secg );

            if ( ansi . length () > crv_name_max_length )
                crv_name_max_length = ansi . length ();
            if ( secg . length () > crv_name_max_length )
                crv_name_max_length = secg . length ();
            if ( ! ansi . isAscii () || ! secg . isAscii () )
                crv_name_is_ascii = false;

            auto nist = crv_to_nist . find ( secg );
            if ( nist == crv_to_nist . end () )
                crv_to_secg . emplace ( secg, secg );
            else
            {
                crv_to_nist . emplace ( ansi, nist -> second );
                crv_to_ansi . emplace ( nist -> second, ansi );
            }
        }
    }

    JWARegistry :: Maps :: ~ Maps ()
    {
    }
}
