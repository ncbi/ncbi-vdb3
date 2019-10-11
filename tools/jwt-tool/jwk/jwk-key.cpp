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

    String JWK :: getUse () const
    {
        return props -> getValue ( "use" ) . toString ();
    }

    std :: vector < String > JWK :: getOperations () const
    {
        const JSONArray & ops = props -> getValue ( "key_ops" ) . toArray ();

        std :: vector < String > opsv;

        unsigned int i, count = ops . count ();
        for ( i = 0; i < count; ++ i )
            opsv . push_back ( ops [ i ] );

        return opsv;
    }

    String JWK :: getAlg () const
    {
        return props -> getValue ( "alg" ) . toString ();
    }

    String JWK :: getID () const
    {
        return props -> getValue ( "kid" ) . toString ();
    }

    
    JWKRef JWK :: toPublic () const
    {
        JSONObjectRef p = props -> cloneObject ();

        p -> deleteValue ( "d" );
        p -> deleteValue ( "dp" );
        p -> deleteValue ( "dq" );
        p -> deleteValue ( "p" );
        p -> deleteValue ( "q" );
        p -> deleteValue ( "qi" );

        JWKMgr :: validateJWK ( * p );
        
        return JWKRef ( new JWK ( p ) );
    }

    String JWK :: toJSON () const
    {
        return props -> toJSON ();
    }

    String JWK :: readableJSON ( unsigned int indent ) const
    {
        return props -> readableJSON ( indent );
    }

    JWK :: ~ JWK () noexcept
    {
        props -> invalidate ();
    }

    JWK :: JWK ( const JSONObjectRef & _props )
        : props ( _props )
    {
    }

}
