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

#include <ncbi/jwk.hpp>

namespace ncbi
{
    bool JWK :: forSigning () const noexcept
    {
        try
        {
            const JSONArray & ops = props -> getValue ( "key_ops" ) . toArray ();
            unsigned int i, count = ops . count ();
            for ( i = 0; i < count; ++ i )
            {
                const String & op = ops [ i ];
                if ( op . compare ( "sign" ) == 0 )
                    return true;
            }
        }
        catch ( ... )
        {
        }

        try
        {
            if ( props -> getValue ( "use" ) . toString () . compare ( "sig" ) == 0 )
            {
                if ( isSymmetric () )
                    return true;

                return isPrivate ();
            }
        }
        catch ( ... )
        {
        }

        return isSymmetric ();
    }

    bool JWK :: forVerifying () const noexcept
    {
        try
        {
            const JSONArray & ops = props -> getValue ( "key_ops" ) . toArray ();
            unsigned int i, count = ops . count ();
            for ( i = 0; i < count; ++ i )
            {
                const String & op = ops [ i ];
                if ( op . compare ( "verify" ) == 0 )
                    return true;
            }
        }
        catch ( ... )
        {
        }

        try
        {
            if ( props -> getValue ( "use" ) . toString () . compare ( "sig" ) == 0 )
            {
                // symmetric keys are always dual use,
                // public or private asymmetric JWKs can be used for verifying
                return true;
            }
        }
        catch ( ... )
        {
        }

        return isSymmetric ();
    }

    bool JWK :: forEncryption () const noexcept
    {
        try
        {
            const JSONArray & ops = props -> getValue ( "key_ops" ) . toArray ();
            unsigned int i, count = ops . count ();
            for ( i = 0; i < count; ++ i )
            {
                const String & op = ops [ i ];
                if ( op . compare ( "encrypt" ) == 0 )
                    return true;
            }
        }
        catch ( ... )
        {
        }

        try
        {
            if ( props -> getValue ( "use" ) . toString () . compare ( "enc" ) == 0 )
            {
                // symmetric keys are always dual use,
                // public or private asymmetric JWKs can be used for encryption
                return true;
            }
        }
        catch ( ... )
        {
        }

        return false;
    }

    bool JWK :: forDecryption () const noexcept
    {
        try
        {
            const JSONArray & ops = props -> getValue ( "key_ops" ) . toArray ();
            unsigned int i, count = ops . count ();
            for ( i = 0; i < count; ++ i )
            {
                const String & op = ops [ i ];
                if ( op . compare ( "decrypt" ) == 0 )
                    return true;
            }
        }
        catch ( ... )
        {
        }

        try
        {
            if ( props -> getValue ( "use" ) . toString () . compare ( "enc" ) == 0 )
            {
                if ( isSymmetric () )
                    return true;

                return isPrivate ();
            }
        }
        catch ( ... )
        {
        }

        return false;
    }

    bool JWK :: isPrivate () const noexcept
    {
        if ( isSymmetric () )
            return false;

        if ( props -> exists ( "d" ) )
            return true;

        return false;
    }

    bool JWK :: isSymmetric () const noexcept
    {
        try
        {
            return getType () . compare ( "oct" ) == 0;
        }
        catch ( ... )
        {
        }
        return false;
    }

    bool JWK :: isRSA () const noexcept
    {
        try
        {
            return getType () . compare ( "RSA" ) == 0;
        }
        catch ( ... )
        {
        }
        return false;
    }

    bool JWK :: isEllipticCurve () const noexcept
    {
        try
        {
            return getType () . compare ( "EC" ) == 0;
        }
        catch ( ... )
        {
        }
        return false;
    }

    String JWK :: getType () const
    {
        return props -> getValue ( "kty" ) . toString ();
    }


}
