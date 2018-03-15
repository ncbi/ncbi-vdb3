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
* ===========================================================================
*
*/

// #include <klib/text.h>
// #include <klib/log.h> /* LOGERR */
// #include <klib/out.h> /* OUTMSG */
// #include <klib/refcount.h>
// #include <klib/rc.h>
// #include <klib/time.h>

// #include <kfs/directory.h>
// #include <kfs/file.h>

// #include <sysalloc.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>         /* atol (), need to remove */
#include <errno.h>
#include <unistd.h>

#include <arpa/tftp.h>      /* EACCESS */

/*********************************************************************
 * Some useful goo :D
 *********************************************************************/
#define ZS_MAX_CONN 5
#define ZS_MAX_TOUT 1000

/*********************************************************************
 * Hola, main and args are here :LOL:
 *********************************************************************/

#define ZS_STOP_TAG "-stop"
#define ZS_DAEM_TAG "-daemon"

bool         _StopSrv   = false;
bool         _Daemonize = false;
uint16_t     _Port      = 0;
int          _LowerFD   = 1024;     /* 0 - do not move */
const char * _Prog      = "zsrv";

void
uasge ()
{
    printf ( "\n" );
    printf ( "Usage:\n\n%s [%s] [%s] port\n", _Prog, ZS_STOP_TAG, ZS_DAEM_TAG ); 
    printf ( "\n" );
    printf ( "Where:\n" );
    printf ( "\n" );
    printf ( "  %s - flag to stop server on port\n", ZS_STOP_TAG );
    printf ( "  %s - flag to start server on port in daemon mode\n", ZS_DAEM_TAG );
    printf ( "\n" );
}   /* uasge () */

bool
parseArgs ( int argc, char ** argv )
{
    const char * pChr;
    int llp;

    pChr = NULL;
    llp = 0;

    pChr = strrchr ( * argv, '/' );
    if ( pChr == NULL ) {
        _Prog = * argv;
    }
    else {
        _Prog = pChr + 1;
    }

    for ( llp = 1; llp < argc; llp ++ ) {
        pChr = * ( argv + llp );

        if ( strcmp ( pChr, ZS_STOP_TAG ) == 0 ) {
            _StopSrv = true;
            continue;
        }

        if ( strcmp ( pChr, ZS_DAEM_TAG ) == 0 ) {
            _Daemonize = true;
            continue;
        }

        _Port = atol ( pChr );
        if ( _Port == 0 ) {
            printf ( "ERROR: Invalid port value \"%s\"\n", pChr );
            return false;
        }

        break;
    }

    if ( _Port == 0 ) {
        printf ( "ERROR: Port is not defined\n" );
        return false;
    }

    return true;
}   /* parseArgs () */

static int StopSrv ();
static int StartSrv ();

int
main ( int argc, char ** argv )
{
    if ( ! parseArgs ( argc, argv ) ) {
        uasge ();

        return 1;
    }

    return _StopSrv ?  StopSrv ():  StartSrv ();
}   /* main () */

/*********************************************************************
 * Vamos
 *********************************************************************/

int
StopSrv ()
{
    if ( _Daemonize ) {
        printf ( "Stopping server on port [%d] ignoring DAEMONIZE argument\n", _Port );
    }
    else {
        printf ( "Stopping server on port [%d]\n", _Port );
    }

    return 0;
}   /* StopSrv () */

int
_reportErrNo ( const char * ErrContext )
{
    const char * Message = NULL;

    if ( errno == 0 ) {
        return 0;
    }

    switch ( errno ) {
        case EACCESS:           Message = "EACCESS";            break;
        case EADDRINUSE:        Message = "EADDRINUSE";         break;
        case EADDRNOTAVAIL:     Message = "EADDRNOTAVAIL";      break;
        case EAFNOSUPPORT:      Message = "EAFNOSUPPORT";       break;
        case EAGAIN:            Message = "EAGAIN";             break;
        case EBADF:             Message = "EBADF";              break;
        case ECONNABORTED:      Message = "ECONNABORTED";       break;
        case EFAULT:            Message = "EFAULT";             break;
        case EINTR:             Message = "EINTR";              break;
        case EINVAL:            Message = "EINVAL";             break;
        case ELOOP:             Message = "ELOOP";              break;
        case EMFILE:            Message = "EMFILE";             break;
        case ENAMETOOLONG:      Message = "ENAMETOOLONG";       break;
        case ENFILE:            Message = "ENFILE";             break;
        case ENOBUFS:           Message = "ENOBUFS";            break;
        /* case ENOENT: duplicates EACCESS */
        case ENOMEM:            Message = "ENOMEM";             break;
        case ENOPROTOOPT:       Message = "ENOPROTOOPT";        break;
        case ENOTDIR:           Message = "ENOTDIR";            break;
        case ENOTSOCK:          Message = "ENOTSOCK";           break;
        case EOPNOTSUPP:        Message = "EOPNOTSUPP";         break;
        case EPERM:             Message = "EPERM";              break;
        case EPROTO:            Message = "EPROTO";             break;
        case EPROTONOSUPPORT:   Message = "EPROTONOSUPPORT";    break;
        case EROFS:             Message = "EROFS";              break;
        /* case EWOULDBLOCK: duplicates EAGAIN */

        default:                Message = "UNKNOWN";            break;
    }

    if ( ErrContext != NULL ) {
        printf ( "ERROR(%s): %s error was handled.", ErrContext, Message );
    }
    else {
        printf ( "ERROR: %s error was handled.", Message );
    }

    return errno;
}   /* _reportErrNo () */

int
_makeListeningSocket ( int * RetSock )
{
    int Socket;
    int Opt;
    int RC;
    struct sockaddr_in SockAddr;

    Socket = -1;
    Opt = 1;
    RC = 0;
    memset ( & SockAddr, 0, sizeof ( struct sockaddr_in ) );

    if ( RetSock == 0 ) {
        return 1;
    }
    * RetSock = 1 * - 1;

        /*  yasss
         */
    errno = 0;

        /*  makin' socket.  Should we use FP_INET6 ?
         */
    Socket = socket ( PF_INET, SOCK_STREAM, 0 );
    if ( Socket == 1 * - 1 ) {
        return _reportErrNo ( "socket" );
    }

        /* setting options on socket */
    RC = setsockopt (
                    Socket,
                    SOL_SOCKET,
                    SO_REUSEADDR,
                    & Opt,
                    sizeof ( Opt )
                    );
    if ( RC == 1 * - 1 ) {
        return _reportErrNo ( "setsockopt" );
    }

        /* Assigning address to socket, to shame all the hackers
         */
    memset ( & SockAddr, 0, sizeof ( struct sockaddr_in ) );
    SockAddr . sin_family = AF_INET;
    SockAddr . sin_addr . s_addr = htonl ( INADDR_ANY );
    SockAddr . sin_port = htons ( _Port );
    RC = bind (
                Socket,
                ( struct sockaddr * ) & SockAddr,
                sizeof ( struct sockaddr_in )
                );
    if ( RC != 0 ) {
        return _reportErrNo ( "bind" );
    }

    RC = listen ( Socket, ZS_MAX_CONN );
    if ( RC == -1 ) {
        return _reportErrNo ( "listen" );
    }

    * RetSock = Socket;

    return 0;
}   /*  _makeListeningSocket () */

int
_acceptAndServe ( int Socket )
{
    int NewSocket, MovedSocket;
    struct sockaddr SocAddr;
    socklen_t SockLen;

    NewSocket = 1 * - 1;
    MovedSocket = 1 * - 1;
    memset ( & SocAddr, 0, sizeof ( struct sockaddr ) );
    SockLen = 0;

    NewSocket = accept ( Socket, & SocAddr, & SockLen );
    if ( NewSocket == 1 * - 1 ) {
        return _reportErrNo ( "accept" );
    }

        /* Checking for ... for sure :LOL:
         */
    if ( SocAddr . sa_family != AF_INET ) {

        if ( NewSocket != 1 * - 1 ) {
                /* JOJOBA ... here we should report something about
                 * "we can not accept invalid sa_family"
                 */

            close ( NewSocket );
        }

        return -1;
    }

        /*  Here we are moving Socket descriptor to other half of space
         */
    if ( _LowerFD == 0 ) {
        MovedSocket = NewSocket;
    }
    else {
        MovedSocket = fcntl ( NewSocket, F_DUPFD, _LowerFD );
        if ( MovedSocket == 1 * - 1 ) {
            MovedSocket = NewSocket;
        }
        else {
            close ( NewSocket );
        }
    }


    return 0;
}   /* _acceptAndServe () */

int
_listenAndServe ( int Socket )
{
    struct pollfd PollFd;
    int RC;

    memset ( & PollFd, 0, sizeof ( struct pollfd ) );
    RC = 0;

        /*  endless
         */
    while ( true ) {
        PollFd . fd = Socket;
        PollFd . events = POLLIN;
        PollFd . revents = 0;

            /*  Here we are pollin' 
             */
        printf ( "  [pollin]\n" );
        errno = 0;
        RC = poll ( & PollFd, 1, ZS_MAX_TOUT );
        if ( 0 < RC ) {
            RC = _acceptAndServe ( Socket );
            if ( RC != 0 ) {
                return RC;
            }
        }
        else {
            if ( RC == 0 ) {
                    /*  Poll timeout happened, nothing serious
                     */
                continue;
            }
            else {
                return _reportErrNo ( "poll" );
            }
        }
    }

    return 0;
}   /* _listenAndServe () */

int
_killListeningSocketAndShutdown ( int Socket )
{
    if ( Socket <= 0 ) {
            /* JOJOBA: say something */
        return 1;
    }

        /*  close socket ...
         */
    printf ( "Closing socket on port [%d]\n", _Port );
    close ( Socket );

        /*  not yet really :>
         */
    printf ( "Server stopped on port [%d]\n", _Port );

    return 0;
}   /* _killListeningSocketAndShutdown () */

int
StartSrv ()
{
    int Socket;
    int RC;

    Socket = -1;
    RC = 0;

    if ( _Daemonize ) {
        printf ( "Starting server as daemon on port [%d]\n", _Port );
    }
    else {
        printf ( "Starting server on port [%d]\n", _Port );
    }

    RC = _makeListeningSocket ( & Socket );
    if ( RC != 0 ) {
        return RC;
    }

    _listenAndServe ( Socket );

    _killListeningSocketAndShutdown ( Socket );

    return 0;
}   /* StartSrv () */

