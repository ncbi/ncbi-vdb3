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
#include <cassert>

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

	void JWTTool :: loadPublicKey ( const JWKRef & key )
	{
		log . msg ( LOG_INFO )
		<< "Loading pub key to JWKS  '"
		<< endm
		;
		
		if ( pubKeys == nullptr )
			pubKeys = JWKMgr :: makeJWKSet ();
		
		pubKeys -> addKey ( key );
		log . msg ( LOG_INFO )
		<< "Added key to JWKS  '"
		<< endm
		;
	}
	
	void JWTTool :: loadPublicKey ( const String & path )
	{
		log . msg ( LOG_INFO )
		<< "Reading public key from '"
		<< path
		<< '\''
		<< endm
		;
		
		// capture String with contents of file
		String contents = readTextFile ( path );
		
		JWKRef key;
		if ( isPem )
			key = JWKMgr :: parsePEM ( contents, "sig", "RS256", "kid_1" );
		else
			key = JWKMgr :: parseJWK ( contents );
		
		if ( key -> isPrivate () )
			throw RuntimeException (
									XP ( XLOC, rc_param_err )
									<< "Key is not public"
									);
		if ( ! key -> isRSA () )
			throw RuntimeException (
									XP ( XLOC, rc_param_err )
									<< "Key is not RSA"
									);

		loadPublicKey ( key );
	}

	void JWTTool :: loadPrivateKey ( const JWKRef & key )
	{
		log . msg ( LOG_INFO )
		<< "Loading priv key to JWKS  '"
		<< endm
		;
		
		if ( privKeys == nullptr )
			privKeys = JWKMgr :: makeJWKSet ();
		
		privKeys -> addKey ( key );
	}
	
    void JWTTool :: loadPrivateKey ( const String & path )
    {
        log . msg ( LOG_INFO )
            << "Reading private key from '"
            << path
            << '\''
            << endm
            ;

        // capture String with contents of file
        String contents = readTextFile ( path );
		
		log . msg ( LOG_INFO )
		<< "Parsing JWK '"
		<< endm
		;
		
		JWKRef key = nullptr;
		if ( isPem )
			key = JWKMgr :: parsePEM ( contents, privPwd, "sig", "RS256", "kid_1" );
		else
			key = JWKMgr :: parseJWK ( contents );
		
        if ( ! key -> isPrivate () )
            throw RuntimeException (
                XP ( XLOC, rc_param_err )
                << "Pem file is not private"
                );
        if ( ! key -> isRSA () )
            throw RuntimeException (
                XP ( XLOC, rc_param_err )
                << "Pem file is not RSA"
                );
		
		loadPrivateKey ( key );
    }

	void JWTTool :: loadKeyorKeySet ( const String & path )
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
		
		JWKSetRef key_set = JWKMgr :: parseJWKorJWKSet ( contents );
		
		// otherwise, merge in the keys
		std :: vector <String> kids = key_set -> getKeyIDs ();
		
		for ( auto kid : kids )
		{
			JWKRef key = key_set -> getKey ( kid );
			if ( key -> isPrivate () )
				loadPrivateKey ( key );
			else
				loadPublicKey ( key );
		}
	}
	
    void JWTTool :: importPemFile ( const String & path )
    {
		loadPrivateKey ( path );
		
		assert ( privKeys -> count () == 1 ); //shouldnt have more than one private key from pem file
		
		log . msg ( LOG_INFO )
		<< "Attempting to translate a privat key to public '"
		<< path
		<< '\''
		<< endm
		;
		
        // translate private key to public
        JWKRef key = privKeys -> getKey ( 0 ) -> toPublic ();
		
		loadPublicKey ( key );
		
        // save keys to text files
        if ( privKeyFilePaths . empty () )
            writeTextFile ( privKeys -> readableJSON (), "tool/input/extPemPrivKey.txt" );
        else
            writeTextFile ( privKeys -> readableJSON (), privKeyFilePaths [ 0 ] );
        
        if ( pubKeyFilePaths . empty () )
            writeTextFile ( pubKeys -> readableJSON (), "tool/input/extPemPubKeys.txt" );
        else
            writeTextFile ( pubKeys -> readableJSON (), pubKeyFilePaths [ 0 ] );

    }

    void JWTTool :: init ()
    {
        switch ( jwtMode )
        {
        case decode:
        {
            // load keysets in keyfilepaths into JWK obj
            log . msg ( LOG_INFO )
                << "Attempting to load "
                << pubKeyFilePaths . size ()
                << " keyset files"
                << endm
                ;
            
            for ( auto path : pubKeyFilePaths )
                loadKeyorKeySet ( path );
            
            log . msg ( LOG_INFO )
                << "Successfully loaded "
                << pubKeys -> count ()
                << " keys"
                << endm
                ;
            break;
        }
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
            loadKeyorKeySet ( privKeyFilePaths [ 0 ] );
			
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
                loadKeyorKeySet ( path );
            
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
			{
                importPemFile ( path );
			}
            break;
        }
        
        default:
            break;
        }
    }
}
