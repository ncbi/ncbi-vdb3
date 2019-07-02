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
#include <ncbi/jws.hpp>

#include <fstream>

namespace ncbi
{
    static
    String readTextFile ( const String & path )
    {
        // declare String for return
        String contents;
        
        // open the file using the nul-terminated path string
        std :: ifstream file;
        file . open ( path . toSTLString () );

        // handle exceptions or file-not-found
        if ( ! file . is_open () )
            throw RuntimeException (
                XP ( XLOC, rc_runtime_err )
                << "Failed to open file"
                );

        // measure the size of the file
        file . seekg ( 0, file . end );
        size_t size = file . tellg ();
        file . seekg ( 0, file . beg );
        
        // allocate a char[] buffer 
        char * buf = new char [ size ];

        try
        {
            // read file into buffer within try block
            file . read ( buf, size );
            
            file . close ();
            
            contents = String ( buf, size );
        }
        catch ( ... )
        {
            delete [] buf;
            throw;
        }
        
        delete [] buf;

        return contents;
    }

    static
    void writeTextFile ( const String & text, const String & path )
    {
        std :: cout << "writing to file" << std :: endl;
        
        std :: ofstream file;
        file . open ( path . toSTLString () );

        if ( ! file . is_open () )
            throw RuntimeException (
                XP ( XLOC, rc_runtime_err )
                << "Failed to open/create file"
                );

        try
        {
            file . write ( text . toSTLString () . c_str (), text . length () );

            file . close ();
        }
        catch ( ... )
        {
            throw;
        }

        return;

        
}

    void JWTTool :: loadPublicKeySet ( const String & path )
    {
        log . msg ( LOG_INFO )
            << "Attempting to load key sets from '"
            << path
            << '\''
            << endm
            ;

        // capture String with contents of file
        String contents = readTextFile ( path );
        
        log . msg ( LOG_INFO )
            << "Parsing keyset JSON"
            << endm
            ;

        JWKSetRef keySet = JWKMgr :: parseJWKSet ( contents );

        std :: cout << keySet -> readableJSON () << std :: endl; 
        
        // if pubKeys is null, just assign it
        if ( pubKeys == nullptr )
            pubKeys = keySet;
        
        // otherwise, we need to have a method to merge in the keys
        // with a single path, this won't happen - so wait until later
    }

    void JWTTool :: loadPrivateKey ( const String & path )
    {
        log . msg ( LOG_INFO )
            << "Reading  private key from '"
            << path
            << '\''
            << endm
            ;

        // capture String with contents of file
        String contents = readTextFile ( path );
        
        log . msg ( LOG_INFO )
            << "Decrypting private key '"
            << contents
            << '\''
            << endm
            ;

        std :: string pwd;
        std :: getline ( std :: cin, pwd );

        if ( pwd . empty () )
            throw RuntimeException (
                XP ( XLOC, rc_param_err )
                << "Missing input parameters"
                );

        JWKRef key = JWKMgr :: parsePEM ( contents, String ( pwd ), "sig", "RS256", "kid_1" );
        
        // if pubKeys is null, just assign it
        if ( privKey == nullptr )
            privKey = key;
    }

    void JWTTool :: importPrivPemFile ( const String & path )
    {
        log . msg ( LOG_INFO )
            << "Reading pem file from '"
            << path
            << '\''
            << endm
            ;

        String contents = readTextFile ( path );

        JWKRef privKey = JWKMgr :: parsePEM ( contents, privPwd, "sig", "RS256", "kid_1" );
        std :: cout << privKey -> readableJSON () << std :: endl;
        if ( ! privKey -> isPrivate () )
            throw RuntimeException (
                XP ( XLOC, rc_param_err )
                << "Pem file is not private"
                );
        if ( ! privKey -> isRSA () )
            throw RuntimeException (
                XP ( XLOC, rc_param_err )
                << "Pem file is not RSA"
                );

        // translate private key to public
        JWKRef pubKey = JWKMgr :: parseJWK ( * privKey );
        std :: cout << pubKey -> readableJSON () << std :: endl;
        if ( pubKey -> isPrivate () )
            throw RuntimeException (
                XP ( XLOC, rc_param_err )
                << "Public key extraction failed"
                );
        
        // save keys to text files
        if ( privKeyFilePaths . empty () )
            writeTextFile ( privKey -> readableJSON (), "tool/input/extPemPrivKey.txt" );
        else
            writeTextFile ( privKey -> readableJSON (), privKeyFilePaths [ 0 ] );
        
        if ( pubKeyFilePaths . empty () )
            writeTextFile ( pubKey -> readableJSON (), "tool/input/extPemPubKeys.txt" );
        else
            writeTextFile ( pubKey -> readableJSON (), pubKeyFilePaths [ 0 ] );

    }

    void JWTTool :: init ()
    {
        switch ( jwtMode )
        {
        case decode:
            break;
        case sign:
        {
            JWTMgr :: Policy jwtPolicy = JWTMgr :: getPolicy ();

            jwtPolicy . pre_serial_verify = false;
            // jwtPolicy . zero_dur_allowed = true;

            JWTMgr :: setPolicy ( jwtPolicy );
            
            log . msg ( LOG_INFO )
                << "Attempting to load "
                << 1
                << " pem file"
                << endm
                ;
            loadPrivateKey ( privKeyFilePaths [ 0 ] );
            
            log . msg ( LOG_INFO )
                << "Successfully loaded "
                << 1
                << " private key"
                << endm
                ;
            break;
        }
        case examine:
        {
            JWSMgr :: Policy jwsPolicy = JWSMgr :: getPolicy ();
            
            // allow ANY JOSE header
            jwsPolicy . kid_required = false;
            jwsPolicy . kid_gen = false;
            
            // unprotected JWS verification - turns off hardening
            jwsPolicy . require_simple_hdr = false;
            jwsPolicy . require_prestored_kid = false;
            jwsPolicy . require_prestored_key = false;
            
            JWSMgr :: setPolicy ( jwsPolicy );
            
            // load keysets in keyfilepaths into JWK obj
            log . msg ( LOG_INFO )
                << "Attempting to load "
                << pubKeyFilePaths . size ()
                << " keyset files"
                << endm
                ;
            
            for ( auto path : pubKeyFilePaths )
                loadPublicKeySet ( path );
            
            log . msg ( LOG_INFO )
                << "Successfully loaded "
                << pubKeys -> count ()
                << " keys"
                << endm
                ;
            break;
        }
        case import_pem:
        {
            for ( auto path : inputParams )
                importPrivPemFile ( path );
            break;
        }
        
        default:
            break;
        }
    }
}
