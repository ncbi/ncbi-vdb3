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

#include "jwt-tool.hpp"

#include "cmdline.hpp"
#include "logging.hpp"

namespace ncbi
{
    static LocalLogger local_logger;
    Log log ( local_logger );

    void ParamBlock :: validate ( JWTMode mode )
    {
        if ( numDurationOpts > 1 )
                throw InvalidArgument (
                    XP ( XLOC, rc_param_err )
                    << "Multiple duration values"
                    );
            
        switch ( mode )
        {
        case decode:
            break;
        case sign:
        {
            // Params
            if ( inputParams . empty () )
                throw InvalidArgument (
                    XP ( XLOC, rc_param_err )
                    << "Missing input parameters"
                    );
            // Options
            if ( privKeyFilePaths . size () > 1 )
                throw InvalidArgument (
                    XP ( XLOC, rc_param_err )
                    << "Multiple private key paths"
                    );
            if ( privKeyFilePaths [ 0 ]  . isEmpty () )
                throw InvalidArgument (
                    XP ( XLOC, rc_param_err )
                    << "Required private signing key"
                    );
            if ( duration < 0 )
            {
                throw InvalidArgument (
                    XP ( XLOC, rc_param_err )
                    << "Required a duration of 0 or more (seconds)"
                    );
            }
            break;
        }
        case examine:
        {
            // Params
            if ( inputParams . empty () )
                throw InvalidArgument (
                    XP ( XLOC, rc_param_err )
                    << "Missing input parameters"
                    );
            
            // Options
            if ( pubKeyFilePaths . empty () )
                throw InvalidArgument (
                    XP ( XLOC, rc_param_err )
                    << "Required public key set"
                    );
            break;
        }
        case import_pem:
        {
            if ( inputParams . empty () )
                throw InvalidArgument (
                    XP ( XLOC, rc_param_err )
                    << "Missing pem files"
                    );            
            if ( numPwds > 1 )
                throw InvalidArgument (
                    XP ( XLOC, rc_param_err )
                    << "Too many pem file passwords"
                    );            
            if ( privPwd . isEmpty () )
                throw InvalidArgument (
                    XP ( XLOC, rc_param_err )
                    << "Missing pem file password"
                    );            
            if ( privKeyFilePaths . size () > 1 )
                throw InvalidArgument (
                    XP ( XLOC, rc_param_err )
                    << "Multiple private key paths"
                    );
            if ( privKeyFilePaths [ 0 ]  . isEmpty () )
                throw InvalidArgument (
                    XP ( XLOC, rc_param_err )
                    << "Required private key file path"
                    );
            break;
        }
        }
    }

    ParamBlock :: ParamBlock ()
        : duration ( 5 * 60 )
        , numDurationOpts ( 0 )
        , numPwds ( 0 )
    {
    }

    JWTTool :: JWTTool ( const ParamBlock & params, JWTMode mode )
        : inputParams ( params . inputParams )
        , pubKeyFilePaths ( params . pubKeyFilePaths )
        , privKeyFilePaths ( params . privKeyFilePaths )
        , privPwd ( params . privPwd )
        , duration ( params . duration )
        , jwtMode ( mode )
    {
    }
    
    JWTTool :: ~ JWTTool () noexcept
    {
    }

    void JWTTool :: run ()
    {
        // initialize and let any exceptions pass out
        init ();

        // we've been initilized
        try
        {
            exec ();
        }
        catch ( ... )
        {
            cleanup ();
            throw;
        }

        cleanup ();
    }

    static
    JWTMode handle_params ( ParamBlock & params, int argc, char * argv [] )
    {
        Cmdline cmdline ( argc, argv );
        
        cmdline . addMode ( "decode", "Extract and verify JWT claims" );
        cmdline . addMode ( "sign", "Sign a JSON claim set object" );
        cmdline . addMode ( "examine", "Examine a JWT without verification" );
        cmdline . addMode ( "import-pem", "Import a private pem file to extract and save private and public keys to files" );
        
        // to the cmdline parser, all params are optional
        // we will enforce their presence manually

        // sign
        cmdline . setCurrentMode ( "sign" );
        cmdline . startOptionalParams ();
        cmdline . addParam ( params . inputParams, 0, 256, "JSON", "JSON text to convert into a JWT" );
        cmdline . addListOption ( params . privKeyFilePaths, ',', 256,
                                  "", "priv-keys", "path-to-priv-key", "Private signing key; provide only 1" );
        cmdline . addOption ( params . duration, & params . numDurationOpts,
                              "", "duration", "duration in seconds" ,"amount of time JWT is valid" );

        // examine
        cmdline . setCurrentMode ( "examine" );
        cmdline . startOptionalParams ();
        cmdline . addParam ( params . inputParams, 0, 256, "token(s)", "optional list of tokens to process" );
        
        cmdline . addListOption ( params . pubKeyFilePaths, ',', 256,
                                  "", "key-sets", "path-to-JWKS", "provide one or more sets of public JWKs" );

        // import_pem
        cmdline . setCurrentMode ( "import-pem" );
        cmdline . startOptionalParams ();
        cmdline . addParam ( params . inputParams, 0, 256, "pem file(s)", "one or more pem files" );
        cmdline . addOption ( params . privPwd, & params . numPwds,
                              "", "pwd", "priv-pem-pwd" , "Private pem file password for decryption" );        
        cmdline . addListOption ( params . privKeyFilePaths, ',', 256,
                                  "", "priv-key", "path-to-priv-key-file",
                                  "Private key file; will create if it does not exist" );
        cmdline . addListOption ( params . pubKeyFilePaths, ',', 256,
                                  "", "pub-key", "path-to-pub-key", "Public key file; will create if it does not exist" );


        
        // pre-parse to look for any configuration file path
        cmdline . parse ( true );
        
        // configure params from file
        
        // normal parse
        cmdline . parse ();

        String ignore;
        JWTMode mode = static_cast <JWTMode> ( cmdline . getModeInfo ( ignore ) );
        params . validate ( mode );
        
        return mode;
    }

    static
    int run ( int argc, char * argv [] )
    {
        try
        {
            // enable local logging to stderr
            local_logger . init ( argv [ 0 ] );

            // create params
            ParamBlock params;
            JWTMode mode = handle_params ( params, argc, argv );

            // run the task object
            JWTTool jwt_tool ( params, mode );
            jwt_tool . run ();

            log . msg ( LOG_INFO )
                << "exiting. "
                << endm
                ;
        }
        catch ( Exception & x )
        {
            log . msg ( LOG_ERR )
                << "EXIT: exception - "
                << x . what ()
                << endm
                ;
            return x . status ();
        }
        catch ( std :: exception & x )
        {
            log . msg ( LOG_ERR )
                << "EXIT: exception - "
                << x . what ()
                << endm
                ;
            throw;
        }
        catch ( ReturnCodes x )
        {
            log . msg ( LOG_NOTICE )
                << "EXIT: due to exception"
                << endm
                ;
            return x;
        }
        catch ( ... )
        {
            log . msg ( LOG_ERR )
                << "EXIT: unknown exception"
                << endm
                ;
            throw;
        }
        
        return 0;
    }
}

extern "C"
{
    int main ( int argc, char * argv [] )
    {
        int status = 1;

        /* STATE:
             fd 0 is probably open for read
             fd 1 is probably open for write
             fd 2 is hopefully open for write
             launched as child process of some shell,
             or perhaps directly as a child of init
        */

        try
        {
            // run the tool within namespace
            status = ncbi :: run ( argc, argv );
        }
#if 0
        catch ( ReturnCodes x )
        {
            status = x;
        }
#endif
        catch ( ... )
        {
        }
        
        return status;
    }
}
