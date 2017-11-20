/*
 *  SSL server demonstration program using fork() for handling multiple clients
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

 const char mbedtls_test_srv_key_local[] =
{
"-----BEGIN EC PRIVATE KEY-----\r\n"
"MHcCAQEEIB8EceOiaOYcRqhFNlLQz6AlCme5zBMWNhQDxYbwd6WWoAoGCCqGSM49\r\n"
"AwEHoUQDQgAEzfuze9LTuEJwGtwFm5zi37LAHuTll2czxtL0uGd6Mh7g8Quzdt4i\r\n"
"IkQGEmRBdSQK1/Izopoh0W3jZCGzCxekmA==\r\n"
"-----END EC PRIVATE KEY-----"
};

const char mbedtls_test_cas_pem_local[] = 
{
"-----BEGIN CERTIFICATE-----\r\n"
"MIIBmDCCATygAwIBAgIBATAMBggqhkjOPQQDAgUAMCkxDDAKBgNVBAMTA1BBTDEM\r\n"
"MAoGA1UEChMDYXJtMQswCQYDVQQGEwJJTDAeFw0xMzAxMDEwMDAwMDBaFw0yMDEy\r\n"
"MzEyMzU5NTlaMCkxDDAKBgNVBAMTA1BBTDEMMAoGA1UEChMDYXJtMQswCQYDVQQG\r\n"
"EwJJTDBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABKQmJimYQ2zCLL4KkON4RKkV\r\n"
"CwIm9hTaNlsPHRWxVnbsf1lrnWnZ2BYi8ZBjodO31zIxjce+j8ggWp36A3l6y36j\r\n"
"UzBRMA8GA1UdEwQIMAYBAf8CAQAwHQYDVR0OBBYEFP5TTPqaxy8Mcr+qR8UWkmd/\r\n"
"DgLvMB8GA1UdIwQYMBaAFP5TTPqaxy8Mcr+qR8UWkmd/DgLvMAwGCCqGSM49BAMC\r\n"
"BQADSAAwRQIhAJfOxTyUtdQ77JLtK5Y/3K4W0gJqpo31sVthlfxNFw6rAiBAeycR\r\n"
"g4f+YqJnfKiO1fPcYvk1kQtDgMkSMC5mZL/yyw==\r\n"
"-----END CERTIFICATE-----"
};

const char mbedtls_test_srv_crt_local[] = 
{
"-----BEGIN CERTIFICATE-----\r\n"
"MIIBmTCCAT2gAwIBAgIBATAMBggqhkjOPQQDAgUAMCkxDDAKBgNVBAMTA1BBTDEM\r\n"
"MAoGA1UEChMDYXJtMQswCQYDVQQGEwJJTDAeFw0xMzAxMDEwMDAwMDBaFw0yMDEy\r\n"
"MzEyMzU5NTlaMDAxEzARBgNVBAMTClBBTF9zZXJ2ZXIxDDAKBgNVBAoTA2FybTEL\r\n"
"MAkGA1UEBhMCSUwwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAATN+7N70tO4QnAa\r\n"
"3AWbnOLfssAe5OWXZzPG0vS4Z3oyHuDxC7N23iIiRAYSZEF1JArX8jOimiHRbeNk\r\n"
"IbMLF6SYo00wSzAJBgNVHRMEAjAAMB0GA1UdDgQWBBRnxPYtY+O2Hq2pq/H3eXDS\r\n"
"hr3neTAfBgNVHSMEGDAWgBT+U0z6mscvDHK/qkfFFpJnfw4C7zAMBggqhkjOPQQD\r\n"
"AgUAA0gAMEUCIH4S0uXk+q7hJ56vsOxZ2Ngc7koE5D5og4dj4R640eTOAiEAx7gD\r\n"
"2z83Qi4lFHFDjfxf2e71AFsTjZDKSAJZ/dmJTyg=\r\n"
"-----END CERTIFICATE-----"
};


#define PAL_TEST_PSK_IDENTITY "Client_identity"
#define PAL_TEST_PSK {0x12,0x34,0x45,0x67,0x89,0x10}


#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define mbedtls_fprintf    fprintf
#define mbedtls_printf     printf
#define mbedtls_time_t     time_t
#endif

#if !defined(MBEDTLS_BIGNUM_C) || !defined(MBEDTLS_CERTS_C) ||    \
    !defined(MBEDTLS_ENTROPY_C) || !defined(MBEDTLS_SSL_TLS_C) || \
    !defined(MBEDTLS_SSL_SRV_C) || !defined(MBEDTLS_NET_C) ||     \
    !defined(MBEDTLS_RSA_C) || !defined(MBEDTLS_CTR_DRBG_C) ||    \
    !defined(MBEDTLS_X509_CRT_PARSE_C) || !defined(MBEDTLS_TIMING_C) || \
    !defined(MBEDTLS_FS_IO) || !defined(MBEDTLS_PEM_PARSE_C)
int main( int argc, char *argv[] )
{
    ((void) argc);
    ((void) argv);

    mbedtls_printf("MBEDTLS_BIGNUM_C and/or MBEDTLS_CERTS_C and/or MBEDTLS_ENTROPY_C "
           "and/or MBEDTLS_SSL_TLS_C and/or MBEDTLS_SSL_SRV_C and/or "
           "MBEDTLS_NET_C and/or MBEDTLS_RSA_C and/or "
           "MBEDTLS_CTR_DRBG_C and/or MBEDTLS_X509_CRT_PARSE_C and/or "
           "MBEDTLS_TIMING_C and/or MBEDTLS_PEM_PARSE_C not defined.\n");
    return( 0 );
}
#elif defined(_WIN32)
int main( void )
{
    mbedtls_printf("_WIN32 defined. This application requires fork() and signals "
           "to work correctly.\n");
    return( 0 );
}
#else

#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509.h"
#include "mbedtls/ssl.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/timing.h"

#include <string.h>
#include <signal.h>

#if !defined(_MSC_VER) || defined(EFIX64) || defined(EFI32)
#include <unistd.h>
#endif

#define HTTP_RESPONSE \
    "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n" \
    "<h2>mbed TLS Test Server</h2>\r\n" \
    "<p>Successful connection using: %s</p>\r\n"

#define DEBUG_LEVEL 0

static void my_debug( void *ctx, int level,
                      const char *file, int line,
                      const char *str )
{
    ((void) level);

    mbedtls_fprintf( (FILE *) ctx, "%s:%04d: %s", file, line, str );
    fflush(  (FILE *) ctx  );
}

int main( void )
{
    int ret, len, cnt = 0, pid;

    mbedtls_net_context listen_fd, client_fd;
    unsigned char buf[1024];
    const char *pers = "ssl_fork_server";

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt srvcert;
    mbedtls_pk_context pkey;

    mbedtls_net_init( &listen_fd );
    mbedtls_net_init( &client_fd );
    mbedtls_ssl_init( &ssl );
    mbedtls_ssl_config_init( &conf );
    mbedtls_entropy_init( &entropy );
    mbedtls_pk_init( &pkey );
    mbedtls_x509_crt_init( &srvcert );
    mbedtls_ctr_drbg_init( &ctr_drbg );

    signal( SIGCHLD, SIG_IGN );

    const char* identity = PAL_TEST_PSK_IDENTITY;
    const char psk[]= PAL_TEST_PSK;

    /*
     * 0. Initial seeding of the RNG
     */
    mbedtls_printf( "\n  . Initial seeding of the random generator..." );
    fflush( stdout );

    if( ( ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,
                               (const unsigned char *) pers,
                               strlen( pers ) ) ) != 0 )
    {
        mbedtls_printf( " failed!  mbedtls_ctr_drbg_seed returned %d\n\n", ret );
        goto exit;
    }

    mbedtls_printf( " ok\n" );

    /*
     * 1. Load the certificates and private RSA key
     */
    mbedtls_printf( "  . Loading the server cert. and key..." );
    fflush( stdout );

    /*
     * This demonstration program uses embedded test certificates.
     * Instead, you may want to use mbedtls_x509_crt_parse_file() to read the
     * server and CA certificates, as well as mbedtls_pk_parse_keyfile().
     */
    ret = mbedtls_x509_crt_parse( &srvcert, (const unsigned char *) mbedtls_test_srv_crt_local,
                          sizeof(mbedtls_test_srv_crt_local) );
    if( ret != 0 )
    {
        mbedtls_printf( " failed!  mbedtls_x509_crt_parse returned %d\n\n", ret );
        goto exit;
    }

    ret = mbedtls_x509_crt_parse( &srvcert, (const unsigned char *) mbedtls_test_cas_pem_local,
                          sizeof(mbedtls_test_cas_pem_local) );
    if( ret != 0 )
    {
        mbedtls_printf( " failed!  mbedtls_x509_crt_parse returned %d\n\n", ret );
        goto exit;
    }

    ret =  mbedtls_pk_parse_key( &pkey, (const unsigned char *) mbedtls_test_srv_key_local,
                          sizeof(mbedtls_test_srv_key_local), NULL, 0 );
    if( ret != 0 )
    {
        mbedtls_printf( " failed!  mbedtls_pk_parse_key returned %d\n\n", ret );
        goto exit;
    }

    mbedtls_printf( " ok\n" );

    /*
     * 1b. Prepare SSL configuration
     */
    mbedtls_printf( "  . Configuring SSL..." );
    fflush( stdout );

    if( ( ret = mbedtls_ssl_config_defaults( &conf,
                    MBEDTLS_SSL_IS_SERVER,
                    MBEDTLS_SSL_TRANSPORT_STREAM,
                    MBEDTLS_SSL_PRESET_DEFAULT ) ) != 0 )
    {
        mbedtls_printf( " failed!  mbedtls_ssl_config_defaults returned %d\n\n", ret );
        goto exit;
    }

    mbedtls_ssl_conf_rng( &conf, mbedtls_ctr_drbg_random, &ctr_drbg );
    mbedtls_ssl_conf_dbg( &conf, my_debug, stdout );

#if defined(MBEDTLS_SSL_RENEGOTIATION)
    mbedtls_ssl_conf_renegotiation( &conf, 1 );
#endif

    mbedtls_ssl_conf_ca_chain( &conf, srvcert.next, NULL );
    if( ( ret = mbedtls_ssl_conf_own_cert( &conf, &srvcert, &pkey ) ) != 0 )
    {
        mbedtls_printf( " failed!  mbedtls_ssl_conf_own_cert returned %d\n\n", ret );
        goto exit;
    }

    ret = mbedtls_ssl_conf_psk( &conf, psk, sizeof(psk),
                           (const unsigned char *) identity,
                           strlen( identity ) );

    mbedtls_printf( " ok\n" );

    /*
     * 2. Setup the listening TCP socket
     */
    mbedtls_printf( "  . Bind on https://localhost:5544/ ..." );
    fflush( stdout );

    if( ( ret = mbedtls_net_bind( &listen_fd, NULL, "5544", MBEDTLS_NET_PROTO_TCP ) ) != 0 )
    {
        mbedtls_printf( " failed!  mbedtls_net_bind returned %d\n\n", ret );
        goto exit;
    }

    mbedtls_printf( " ok\n" );

    while( 1 )
    {
        /*
         * 3. Wait until a client connects
         */
        mbedtls_net_init( &client_fd );
        mbedtls_ssl_init( &ssl );

        mbedtls_printf( "  . Waiting for a remote connection ...\n" );
        fflush( stdout );

        if( ( ret = mbedtls_net_accept( &listen_fd, &client_fd,
                                        NULL, 0, NULL ) ) != 0 )
        {
            mbedtls_printf( " failed!  mbedtls_net_accept returned %d\n\n", ret );
            goto exit;
        }

        /*
         * 3.5. Forking server thread
         */

        mbedtls_printf( "  . Forking to handle connection ..." );
        fflush( stdout );

        pid = fork();

        if( pid < 0 )
        {
            mbedtls_printf(" failed!  fork returned %d\n\n", pid );
            goto exit;
        }

        if( pid != 0 )
        {
            mbedtls_printf( " ok\n" );

            if( ( ret = mbedtls_ctr_drbg_reseed( &ctr_drbg,
                                         (const unsigned char *) "parent",
                                         6 ) ) != 0 )
            {
                mbedtls_printf( " failed!  mbedtls_ctr_drbg_reseed returned %d\n\n", ret );
                goto exit;
            }

            continue;
        }

        mbedtls_net_init( &listen_fd );

        pid = getpid();

        /*
         * 4. Setup stuff
         */
        mbedtls_printf( "pid %d: Setting up the SSL data.\n", pid );
        fflush( stdout );

        if( ( ret = mbedtls_ctr_drbg_reseed( &ctr_drbg,
                                     (const unsigned char *) "child",
                                     5 ) ) != 0 )
        {
            mbedtls_printf(
                    "pid %d: SSL setup failed!  mbedtls_ctr_drbg_reseed returned %d\n\n",
                    pid, ret );
            goto exit;
        }

        if( ( ret = mbedtls_ssl_setup( &ssl, &conf ) ) != 0 )
        {
            mbedtls_printf(
                    "pid %d: SSL setup failed!  mbedtls_ssl_setup returned %d\n\n",
                    pid, ret );
            goto exit;
        }

        mbedtls_ssl_set_bio( &ssl, &client_fd, mbedtls_net_send, mbedtls_net_recv, NULL );

        mbedtls_printf( "pid %d: SSL setup ok\n", pid );

        /*
         * 5. Handshake
         */
        mbedtls_printf( "pid %d: Performing the SSL/TLS handshake.\n", pid );
        fflush( stdout );

        while( ( ret = mbedtls_ssl_handshake( &ssl ) ) != 0 )
        {
            if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
            {
                mbedtls_printf(
                        "pid %d: SSL handshake failed!  mbedtls_ssl_handshake returned %d\n\n",
                        pid, ret );
                goto exit;
            }
        }

        mbedtls_printf( "pid %d: SSL handshake ok\n", pid );

        /*
         * 6. Read the HTTP Request
         */
        mbedtls_printf( "pid %d: Start reading from client.\n", pid );
        fflush( stdout );

        do
        {
            len = sizeof( buf ) - 1;
            memset( buf, 0, sizeof( buf ) );
            ret = mbedtls_ssl_read( &ssl, buf, len );

            if( ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE )
                continue;

            if( ret <= 0 )
            {
                switch( ret )
                {
                    case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
                        mbedtls_printf( "pid %d: connection was closed gracefully\n", pid );
                        break;

                    case MBEDTLS_ERR_NET_CONN_RESET:
                        mbedtls_printf( "pid %d: connection was reset by peer\n", pid );
                        break;

                    default:
                        mbedtls_printf( "pid %d: mbedtls_ssl_read returned %d\n", pid, ret );
                        break;
                }

                break;
            }

            len = ret;
            mbedtls_printf( "pid %d: %d bytes read\n\n%s", pid, len, (char *) buf );

            if( ret > 0 )
                break;
        }
        while( 1 );

        /*
         * 7. Write the 200 Response
         */
        mbedtls_printf( "pid %d: Start writing to client.\n", pid );
        fflush( stdout );

        len = sprintf( (char *) buf, HTTP_RESPONSE,
                mbedtls_ssl_get_ciphersuite( &ssl ) );

        while( cnt++ < 2 )
        {
            while( ( ret = mbedtls_ssl_write( &ssl, buf, len ) ) <= 0 )
            {
                if( ret == MBEDTLS_ERR_NET_CONN_RESET )
                {
                    mbedtls_printf(
                            "pid %d: Write failed!  peer closed the connection\n\n", pid );
                    goto exit;
                }

                if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
                {
                    mbedtls_printf(
                            "pid %d: Write failed!  mbedtls_ssl_write returned %d\n\n",
                            pid, ret );
                    goto exit;
                }
            }
            len = ret;
            mbedtls_printf( "pid %d: %d bytes written\n\n%s\n", pid, len, (char *) buf );

            mbedtls_net_usleep( 1000000 );
        }

        mbedtls_ssl_close_notify( &ssl );
        goto exit;
    }

exit:
    mbedtls_net_free( &client_fd );
    mbedtls_net_free( &listen_fd );

    mbedtls_x509_crt_free( &srvcert );
    mbedtls_pk_free( &pkey );
    mbedtls_ssl_free( &ssl );
    mbedtls_ssl_config_free( &conf );
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );

#if defined(_WIN32)
    mbedtls_printf( "  Press Enter to exit this program.\n" );
    fflush( stdout ); getchar();
#endif

    return( ret );
}
#endif /* MBEDTLS_BIGNUM_C && MBEDTLS_CERTS_C && MBEDTLS_ENTROPY_C &&
          MBEDTLS_SSL_TLS_C && MBEDTLS_SSL_SRV_C && MBEDTLS_NET_C &&
          MBEDTLS_RSA_C && MBEDTLS_CTR_DRBG_C && MBEDTLS_PEM_PARSE_C &&
          ! _WIN32 */
