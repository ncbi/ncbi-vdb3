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

#include "cmdline.hpp"
#include "logging.hpp"

namespace ncbi
{
        struct ParamBlock
        {
            void validate ()
                {
                }

            ParamBlock ()
                {
                }

            ~ ParamBlock () {}
        };

        static
        void handle_params ( ParamBlock & params, int argc, char * argv [] )
        {
            Cmdline cmdline ( argc, argv );

            #if 0
            cmdline . addOption ( params . on_temp, & params . on_temp_count,
                                  "U", "upper", "deg-celsius", "set upper temperature threshold" );
            cmdline . addOption ( params . off_temp, & params . off_temp_count,
                                  "L", "lower", "deg-celsius", "set lower temperature threshold" );
            cmdline . addOption ( params . poll_interval, 0,
                                  "I", "poll-interval", "seconds", "time to wait between samples" );
            cmdline . addOption ( params . max_on_time, 0,
                                  "", "max-on-time", "seconds", "the maximum seconds to keep compressor on" );
            cmdline . addOption ( params . min_on_time, 0,
                                  "", "min-on-time", "seconds", "the minimum seconds to keep compressor on" );
            cmdline . addOption ( params . min_off_time, 0,
                                  "", "min-off-time", "seconds", "the minimum seconds to keep compressor off" );
            #endif
            // pre-parse to look for any configuration file path
            cmdline . parse ( true );

            // configure params from file

            // normal parse
            cmdline . parse ();

            params . validate ();
        }

        static
        int run ( int argc, char * argv [] )
        {
                    try
                    {
                        #if 0
                        // enable local logging to stderr
                        local_logger . init ( argv [ 0 ] );
                        #endif

                        // create params
                        ParamBlock params;
                        handle_params ( params, argc, argv );

                        #if 0
                        // run the task object
                        KeezerTask keezer ( params, gpio );
                        keezer . run ();

                        log . msg ( LOG_INFO )
                            << "exiting. "
                            << endm
                            ;
                        #endif
                    }
                    catch ( Exception & x )
                    {
                        log . msg ( LOG_ERR )
                            << "EXIT: exception - "
                            << x . what ()
                            << endm
                            ;
                        //return x . status;
                        return 99;
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
                    #if 0
                    catch ( ReturnCodes x )
                    {
                        log . msg ( LOG_NOTICE )
                            << "EXIT: due to exception"
                            << endm
                            ;
                        return x;
                    }
                    #endif
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
