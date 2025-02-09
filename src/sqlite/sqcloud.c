//
//  sqcloud.c
//
//  Created by Marco Bambini on 08/02/21.
//

#include "lz4.h"
#include "sqcloud.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
//#include <sys/time.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include <Shlwapi.h>
#include <io.h>
#include <float.h>
//#include "pthread.h"
#else
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <inttypes.h>
#endif

#ifndef SQLITECLOUD_DISABLE_TLS

#ifdef SQLITECLOUD_USE_TLS_HEADER
#include "tls.h"
#else
#ifndef HEADER_TLS_H
#define TLS_WANT_POLLIN     -2
#define TLS_WANT_POLLOUT    -3
struct tls;
struct tls_config;
struct tls *tls_client(void);
struct tls_config *tls_config_new(void);
int tls_init(void);
int tls_configure(struct tls *_ctx, struct tls_config *_config);
int tls_connect_socket(struct tls *_ctx, int _s, const char *_servername);
int tls_close(struct tls *_ctx);
void tls_config_insecure_noverifycert(struct tls_config *config);
void tls_config_insecure_noverifyname(struct tls_config *config);
int tls_config_set_ca_file(struct tls_config *_config, const char *_ca_file);
int tls_config_set_cert_file(struct tls_config *_config,const char *_cert_file);
int tls_config_set_key_file(struct tls_config *_config, const char *_key_file);
ssize_t tls_read(struct tls *_ctx, void *_buf, size_t _buflen);
ssize_t tls_write(struct tls *_ctx, const void *_buf, size_t _buflen);
const char *tls_error(struct tls *_ctx);
const char *tls_config_error(struct tls_config *_config);
void tls_free(struct tls *_ctx);
#endif
#endif

#endif

// MARK: MACROS -
#ifdef _WIN32
#pragma warning (disable: 4005)
#pragma warning (disable: 4068)
#define readsocket(a,b,c)                   recv((a), (b), (c), 0L)
#define writesocket(a,b,c)                  send((a), (b), (c), 0L)
#else
#define readsocket                          read
#define writesocket                         write
#define closesocket(s)                      close(s)
#endif

#ifndef mem_alloc
#define mem_realloc                         realloc
#define mem_zeroalloc(_s)                   calloc(1,_s)
#define mem_alloc(_s)                       malloc(_s)
#define mem_free(_s)                        free(_s)
#define mem_string_dup(_s)                  strdup(_s)
#define mem_string_ndup(_s,_n)              strndup(_s,_n)
#endif
#ifndef MIN
#define MIN(a,b)                            (((a)<(b))?(a):(b))
#endif

#define MAX_SOCK_LIST                       6           // maximum number of socket descriptor to try to connect to
                                                        // this change is required to support IPv4/IPv6 connections
#define DEFAULT_TIMEOUT                     12          // default connection timeout in seconds

#define REPLY_OK                            "+2 OK"     // default OK reply
#define REPLY_OK_LEN                        5           // default OK reply string length

// https://levelup.gitconnected.com/8-ways-to-measure-execution-time-in-c-c-48634458d0f9
#define TIME_GET(_t1)                       struct timeval _t1; gettimeofday(&_t1, NULL)
#define TIME_VAL(_t1, _t2)                  ((double)(_t2.tv_sec - _t1.tv_sec) + (double)((_t2.tv_usec - _t1.tv_usec)*1e-6))

#define CMD_MINLEN                          2

#define CONNSTRING_KEYVALUE_SEPARATOR       '='
#define CONNSTRING_TOKEN_SEPARATOR          ';'

#define DEFAULT_CHUCK_NBUFFERS              20
#define DEFAULT_CHUNK_MINROWS               2000

#define ARRAY_STATIC_COUNT                  256
#define ARRAY_HEADER_BUFFER_SIZE            64

#ifndef TLS_DEFAULT_CA_FILE
#if CLI_WINDOWS
// windows
#elif __APPLE__
// macos
#define TLS_DEFAULT_CA_FILE                 "/etc/ssl/cert.pem"
#else
// linux
#define TLS_DEFAULT_CA_FILE                 "/etc/ssl/certs/ca-certificates.crt"
#endif
#endif

// MARK: - PROTOTYPES -

static SQCloudResult *internal_socket_read (SQCloudConnection *connection, bool mainfd);
static bool internal_socket_write (SQCloudConnection *connection, const char *buffer, size_t len, bool mainfd, bool compute_header);
static uint32_t internal_parse_number (char *buffer, uint32_t blen, uint32_t *cstart);
static SQCloudResult *internal_parse_buffer (SQCloudConnection *connection, char *buffer, uint32_t blen, uint32_t cstart, bool isstatic, bool externalbuffer);
static bool internal_connect (SQCloudConnection *connection, const char *hostname, int port, SQCloudConfig *config, bool mainfd);
static bool internal_set_error (SQCloudConnection *connection, int errcode, const char *format, ...);
static SQCloudResult *internal_array_exec (SQCloudConnection *connection, const char *r[], int64_t len[], uint32_t n, uint32_t count);

// MARK: -

struct SQCloudResult {
    SQCLOUD_RESULT_TYPE  tag;               // RESULT_OK, RESULT_ERROR, RESULT_STRING, RESULT_INTEGER, RESULT_FLOAT, RESULT_ROWSET, RESULT_NULL
    
    bool            ischunk;                // flag used to correctly access the union below
    union {
        struct {
            char        *buffer;            // buffer used by the user (it could be a ptr inside rawbuffer)
            char        *rawbuffer;         // ptr to the buffer to be freed
            uint32_t    balloc;             // buffer allocation size
        };
        struct {
            char        **buffers;          // array of buffers used by rowset sent in chunk
            bool        *bext;              // array of flags, if true the buffer must not be freed
            uint32_t    *blens;             // array of buffer len
            uint32_t    *nheads;            // array of header len
            uint32_t    bcount;             // number of buffers in the array
            uint32_t    bnum;               // number of pre-allocated buffers
            uint32_t    brows;              // number of pre-allocated rows
        };
    };
    
    // common
    uint32_t        blen;                   // total buffer length (also the sum of buffers)
    double          time;                   // full execution time (latency + server side time)
    bool            externalbuffer;         // true if the buffer is managed by the caller code
                                            // false if the buffer can be freed by the SQCloudResultFree func
    uint32_t        nheader;                // number of character in the first part of the header (which is usually skipped)
    
    // used in TYPE_ROWSET only
    uint32_t        version;                // rowset version
    uint32_t        nrows;                  // number of rows
    uint32_t        ncols;                  // number of columns
    uint32_t        ndata;                  // number of items stores in data
    char            **data;                 // data contained in the rowset
    char            **name;                 // column names
    char            **decltype;             // column declared types
    char            **dbname;               // column database names
    char            **tblname;              // column table names
    char            **origname;             // column origin names
    int             *notnull;               // column is not null
    int             *prikey;                // column is primary key
    int             *autoinc;               // column is auto increment
    uint32_t        *clen;                  // max len for each column (used to display result)
    uint32_t        maxlen;                 // max len for each row/column
    
    // vm related fields (reserved)
    uint32_t        n1;
    uint32_t        n2;
    uint32_t        n3;
    uint32_t        n4;
    uint32_t        n5;
} _SQCloudResult;

struct SQCloudConnection {
    int             fd;
    char            errmsg[1024];
    int             errcode;                // error code
    int             extcode;                // extended error code
    int             offcode;                // offset error code
    SQCloudResult   *_chunk;
    SQCloudConfig   *_config;
    bool            isblob;
    bool            config_to_free;
    
    // pub/sub
    char            *uuid;
    int             pubsubfd;
    SQCloudPubSubCB callback;
    void            *data;
    char            *hostname;
    int             port;
    pthread_t       tid;
    
    #ifndef SQLITECLOUD_DISABLE_TLS
    struct tls      *tls_context;
    struct tls      *tls_pubsub_context;
    #endif
} _SQCloudConnection;

struct SQCloudVM {
    SQCloudConnection   *connection;
    SQCloudResult       *result;
    int                 index;
    int                 type;
    bool                finalized;
    
    bool                isreadonly;
    int                 isexplain;
    int                 rowindex;
    int                 nparams;
    int                 ncolumns;
    
    int64_t             lastrowid;
    int64_t             changes;
    int64_t             totalchanges;
    
    char                *errmsg;
    int                 errcode;
    int                 xerrcode;
} _SQCloudVM;

struct SQCloudBlob {
    SQCloudConnection   *connection;
    int                 index;
    int64_t             bytes;
    int                 rc;
} _SQCloudBlob;

struct SQCloudBackup {
    SQCloudConnection   *connection;
    int                 index;
    int                 page_size;
    int                 page_total;
    int                 page_remaining;
    int                 counter;
    int                 size;
    void                *data;
} _SQCloudBackup;

static SQCloudResult SQCloudResultOK = {RESULT_OK, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0};
static SQCloudResult SQCloudResultNULL = {RESULT_NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0};

// MARK: - UTILS -

static uint32_t utf8_charbytes (const char *s, uint32_t i) {
    unsigned char c = (unsigned char)s[i];
    
    // determine bytes needed for character, based on RFC 3629
    if ((c > 0) && (c <= 127)) return 1;
    if ((c >= 194) && (c <= 223)) return 2;
    if ((c >= 224) && (c <= 239)) return 3;
    if ((c >= 240) && (c <= 244)) return 4;
    
    // means error
    return 0;
}

static uint32_t utf8_len (const char *s, uint32_t nbytes) {
    uint32_t pos = 0;
    uint32_t len = 0;
    
    while (pos < nbytes) {
        ++len;
        uint32_t n = utf8_charbytes(s, pos);
        if (n == 0) return 0; // means error
        pos += n;
    }
    
    return len;
}

#if 0
static char *extract_connection_token (const char *s, char *key, char buffer[256]) {
    char *target = strstr(s, key);
    if (!target) return NULL;
    
    // find out = separator
    char *p = target;
    while (p[0]) {
        if (p[0] == CONNSTRING_KEYVALUE_SEPARATOR) break;
        ++p;
    }
    
    // skip =
    ++p;
    
    // skip spaces (if any)
    while (p[0]) {
        if (!isspace(p[0])) break;
        ++p;
    }
    
    // copy value to buffer
    int len = 0;
    while (p[0] && len < 255) {
        if (isspace(p[0])) break;
        if (p[0] == CONNSTRING_TOKEN_SEPARATOR) break;
        buffer[len] = p[0];
        ++len;
        ++p;
    }
    
    // null-terminate returning value
    buffer[len] = 0;
    p = &buffer[0];
    
    return p;
}
#endif

// MARK: - PRIVATE -

static int socket_geterror (int fd) {
    int err;
    socklen_t errlen = sizeof(err);
    
    int sockerr = getsockopt(fd, SOL_SOCKET, SO_ERROR, (void *)&err, &errlen);
    if (sockerr < 0) return -1;
    
    return ((err == 0 || err == EINTR || err == EAGAIN || err == EINPROGRESS)) ? 0 : err;
}

static void *pubsub_thread (void *arg) {
    SQCloudConnection *connection = (SQCloudConnection *)arg;
    
    int fd = connection->pubsubfd;
    #ifndef SQLITECLOUD_DISABLE_TLS
    struct tls *tls = connection->tls_pubsub_context;
    #endif
    
    size_t blen = 2048;
    char *buffer = mem_alloc(blen);
    if (buffer == NULL) return NULL;
    
    char *original = buffer;
    uint32_t tread = 0;

    while (1) {
        fd_set set;
        FD_ZERO(&set);
        FD_SET(fd, &set);
        
        // wait for read event
        int rc = select(fd + 1, &set, NULL, NULL, NULL);
        if (rc <= 0) continue;
        
        //  read payload string
        #ifndef SQLITECLOUD_DISABLE_TLS
        ssize_t nread = (tls) ? tls_read(tls, buffer, blen) : readsocket(fd, buffer, blen);
        if ((tls) && (nread == TLS_WANT_POLLIN || nread == TLS_WANT_POLLOUT)) continue;
        #else
        ssize_t nread = readsocket(fd, buffer, blen);
        #endif
        
        if (nread < 0) {
            const char *msg = "";
            #ifndef SQLITECLOUD_DISABLE_TLS
            if (tls) msg = tls_error(tls);
            #endif
            
            internal_set_error(connection, INTERNAL_ERRCODE_NETWORK, "An error occurred while reading data: %s (%s).", strerror(errno), msg);
            if (connection->callback) connection->callback(connection, NULL, connection->data);
            break;
        }
        
        if (nread == 0) {
            internal_set_error(connection, INTERNAL_ERRCODE_SOCKCLOSED, "PubSub connection closed.");
            if (connection->callback) connection->callback(connection, NULL, connection->data);
            break;
        }
        
        tread += (uint32_t)nread;
        blen -= (uint32_t)nread;
        buffer += nread;
        
        uint32_t cstart = 0;
        uint32_t clen = internal_parse_number (&original[1], tread-1, &cstart);
        if (clen == 0) continue;
        
        // check if read is complete
        // clen is the lenght parsed in the buffer
        // cstart is the index of the first space
        // +1 because we skipped the first character in the internal_parse_number function
        if (clen + cstart + 1 != tread) {
            // check buffer allocation and continue reading
            if (clen + cstart > blen) {
                char *clone = mem_alloc(clen + cstart + 1);
                if (!clone) {
                    internal_set_error(connection, INTERNAL_ERRCODE_MEMORY, "Unable to allocate memory: %d.", clen + cstart + 1);
                    if (connection->callback) connection->callback(connection, NULL, connection->data);
                    break;
                }
                memcpy(clone, original, tread);
                buffer = original = clone;
                blen = (clen + cstart + 1) - tread;
                buffer += tread;
            }
            
            continue;
        }
        
        SQCloudResult *result = internal_parse_buffer(connection, original, tread, (clen) ? cstart : 0, false, false);
        if (result->tag == RESULT_STRING) result->tag = RESULT_JSON;
        if (!connection->callback) break;
        
        connection->callback(connection, result, connection->data);
        
        blen = 2048;
        buffer = mem_alloc(blen);
        if (!buffer) break;
        
        original = buffer;
        tread = 0;
    }
    
    if (buffer) mem_free(buffer);
    return NULL;
}

// MARK: -

static bool internal_init (void) {
    static bool inited = false;
    if (inited) return true;
    
    #ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
    #else
    // IGNORE SIGPIPE and SIGABORT
    struct sigaction act;
    act.sa_handler = SIG_IGN;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGPIPE, &act, (struct sigaction *)NULL);
    sigaction(SIGABRT, &act, (struct sigaction *)NULL);
    #endif
    
    inited = true;
    return true;
}

static bool internal_set_error (SQCloudConnection *connection, int errcode, const char *format, ...) {
    connection->errcode = errcode;
    
    va_list arg;
    va_start (arg, format);
    vsnprintf(connection->errmsg, sizeof(connection->errmsg), format, arg);
    va_end (arg);
    
    return false;
}

static void internal_clear_error (SQCloudConnection *connection) {
    connection->errcode = 0;
    connection->extcode = 0;
    connection->offcode = -1;   // If the most recent error does not reference a specific token in the input SQL, then the sqlite3_error_offset() function returns -1.
    connection->errmsg[0] = 0;
}

static bool internal_setup_tls (SQCloudConnection *connection, SQCloudConfig *config, bool mainfd) {
    #ifndef SQLITECLOUD_DISABLE_TLS
    if (config && config->insecure) return true;
    
    int rc = 0;
    
    if (tls_init() < 0) {
        return internal_set_error(connection, INTERNAL_ERRCODE_TLS, "Error while initializing TLS library.");
    }
    
    struct tls_config *tls_conf = tls_config_new();
    if (!tls_conf) {
        return internal_set_error(connection, INTERNAL_ERRCODE_TLS, "Error while initializing a new TLS configuration.");
    }
    
    if (config->no_verify_certificate) {
        tls_config_insecure_noverifycert(tls_conf);
        tls_config_insecure_noverifyname(tls_conf);
    }
    
    // loads a file containing the root certificates
    if (config && config->tls_root_certificate) {
        rc = tls_config_set_ca_file(tls_conf, config->tls_root_certificate);
        if (rc < 0) {internal_set_error(connection, INTERNAL_ERRCODE_TLS, "Error in tls_config_set_ca_file: %s.", tls_config_error(tls_conf));}
    #ifdef TLS_DEFAULT_CA_FILE
    } else {
        rc = tls_config_set_ca_file(tls_conf, TLS_DEFAULT_CA_FILE);
        if (rc < 0) {internal_set_error(connection, INTERNAL_ERRCODE_TLS, "Error in tls_config_set_ca_file: %s.", tls_config_error(tls_conf));}
    #endif
    }
    
    // loads a file containing the server certificate
    if (config && config->tls_certificate) {
        rc = tls_config_set_cert_file(tls_conf, config->tls_certificate);
        if (rc < 0) {internal_set_error(connection, INTERNAL_ERRCODE_TLS, "Error in tls_config_set_cert_file: %s.", tls_config_error(tls_conf));}
    }
    
    // loads a file containing the private key
    if (config && config->tls_certificate_key) {
        rc = tls_config_set_key_file(tls_conf, config->tls_certificate_key);
        if (rc < 0) {internal_set_error(connection, INTERNAL_ERRCODE_TLS, "Error in tls_config_set_key_file: %s.", tls_config_error(tls_conf));}
    }
    
    struct tls *tls_context = tls_client();
    if (!tls_context) {
        return internal_set_error(connection, INTERNAL_ERRCODE_TLS, "Error while initializing a new TLS client.");
    }
    
    // apply configuration to context
    rc = tls_configure(tls_context, tls_conf);
    if (rc < 0) {
        return internal_set_error(connection, INTERNAL_ERRCODE_TLS, "Error in tls_configure: %s.", tls_error(tls_context));
    }
    
    // save context
    if (mainfd) connection->tls_context = tls_context;
    else connection->tls_pubsub_context = tls_context;
    
    #endif
    return true;
}

static SQCLOUD_VALUE_TYPE internal_type (char *buffer) {
    // for VALUE_NULL values we don't return _ but the NULL value itself, so check for this special case
    // internal_parse_value is used both internally to set the value inside a Rowset (1)
    // and also from the public API SQCloudRowsetValue (2)
    // to fix this misbehaviour, (1) should return _ while (2) should return NULL
    // this is really just a convention so it is much more easier to just return NULL everytime
    if (!buffer) return VALUE_NULL;
    
    switch (buffer[0]) {
        case CMD_STRING:
        case CMD_ZEROSTRING: return VALUE_TEXT;
        case CMD_INT: return VALUE_INTEGER;
        case CMD_FLOAT: return VALUE_FLOAT;
        case CMD_NULL: return VALUE_NULL;
        case CMD_BLOB: return VALUE_BLOB;
    }
    return VALUE_NULL;
}

static bool internal_has_commandlen (int c) {
    return ((c == CMD_INT) || (c == CMD_FLOAT) || (c == CMD_NULL)) ? false : true;
}

static bool internal_canbe_zerolength (int c) {
    return ((c == CMD_BLOB) || (c == CMD_STRING));
}

static uint32_t internal_buffer_maxlen (SQCloudResult *result, char *value) {
    if (!value) return 2;
    
    // chunk case
    if (result->ischunk) {
        // lookup value index
        int32_t index = -1;
        char *buffer = NULL;
        uint32_t blen = 0;
        
        for (int32_t i=0; i < result->bcount; ++i) {
            buffer = result->buffers[i];
            blen = result->blens[i];
            if ((value >= buffer) && (value <= buffer + blen)) {
                index = i;
                break;
            }
        }
        
        // sanity check
        if (index == -1) return 0;
        
        return (uint32_t)(blen - (uint32_t)(value - buffer) + result->nheads[index]);
    }
    
    // default case
    return (uint32_t)(result->blen - (uint32_t)(value - result->rawbuffer) + result->nheader);
}

static uint32_t internal_parse_number_extended (char *buffer, uint32_t blen, uint32_t *cstart, uint32_t *extcode, int32_t *offcode) {
    uint32_t value = 0;
    uint32_t extvalue = 0;
    int32_t offvalue = -1;
    bool isext = false;
    bool isoff = false;
    
    for (uint32_t i=0; i<blen; ++i) {
        int c = buffer[i];
        
        // check for optional extended error code (ERRCODE[:EXTCODE:OFFCODE])
        if (c == ':') {
            if (isext == false) isext = true;
            else {isext = false; isoff = true; offvalue = 0;}
            continue;
        }
        
        // check for end of value
        if (c == ' ') {
            if (cstart) *cstart = i+1;
            if (extcode) *extcode = extvalue;
            if (offcode) *offcode = offvalue;
            return value;
        }
        
        // compute numeric value
        if (isext) extvalue = (extvalue * 10) + (buffer[i] - '0');
        else if (isoff) offvalue = (offvalue * 10) + (buffer[i] - '0');
        else value = (value * 10) + (buffer[i] - '0');
    }
    
    return 0;
}

static uint32_t internal_parse_number (char *buffer, uint32_t blen, uint32_t *cstart) {
    return internal_parse_number_extended(buffer, blen, cstart, NULL, NULL);
}

static int internal_parse_type (char *buffer) {
    if (!buffer) return 0;
    return buffer[0];
}

static char *internal_parse_value (char *buffer, uint32_t *len, uint32_t *cellsize) {
    if (*len <= 0) return NULL;
    
    // handle special NULL value case
    if (!buffer || buffer[0] == CMD_NULL) {
        *len = 0;
        if (cellsize) *cellsize = 2;
        return NULL;
    }
    
    // blen originally was hard coded to 24 because the max 64bit value is 20 characters long
    uint32_t cstart = 0;
    uint32_t blen = *len;   
    blen = internal_parse_number(&buffer[1], blen, &cstart);
    
    // handle decimal/float cases
    if ((buffer[0] == CMD_INT) || (buffer[0] == CMD_FLOAT)) {
        *len = cstart - 1;
        if (cellsize) *cellsize = cstart + 1;
        return &buffer[1];
    }

    // sanity check
    if (blen > *len) return NULL;
    
    *len = (buffer[0] == CMD_ZEROSTRING) ? blen - 1 : blen;
    if (cellsize) *cellsize = cstart + blen + 1;
    return &buffer[1+cstart];
}

static SQCloudResult *internal_run_command (SQCloudConnection *connection, const char *buffer, size_t blen, bool mainfd) {
    internal_clear_error(connection);
    
    if (!buffer || blen < CMD_MINLEN) return NULL;
    
    TIME_GET(tstart);
    if (!internal_socket_write(connection, buffer, blen, mainfd, true)) return NULL;
    SQCloudResult *result = internal_socket_read(connection, mainfd);
    TIME_GET(tend);
    if (result) result->time = TIME_VAL(tstart, tend);
    return result;
}

static bool internal_send_blob(SQCloudConnection *connection, void *buffer, uint32_t blen) {
    internal_clear_error(connection);
    
    // set connection to be BLOB
    connection->isblob = true;
    
    // check zero-size BLOB
    TIME_GET(tstart);
    bool rc = internal_socket_write(connection, (blen) ? buffer : NULL, blen, true, true);
    connection->isblob = false;
    if (!rc) return false;
    SQCloudResult *result = internal_socket_read(connection, true);
    TIME_GET(tend);
    if (result) result->time = TIME_VAL(tstart, tend);
    
    rc = (SQCloudResultType(result) == RESULT_OK);
    SQCloudResultFree(result);
    return rc;
}

static SQCloudResult *internal_setup_pubsub (SQCloudConnection *connection, const char *buffer, size_t blen) {
    // check if pubsub was already setup
    if (connection->pubsubfd != 0) return &SQCloudResultOK;
    
    if (!internal_setup_tls(connection, connection->_config, false)) return NULL;
    
    if (internal_connect(connection, connection->hostname, connection->port, connection->_config, false)) {
        SQCloudResult *result = internal_run_command(connection, buffer, blen, false);
        if (!SQCloudResultIsOK(result)) return result;
        pthread_create(&connection->tid, NULL, pubsub_thread, (void *)connection);
    } else {
        return NULL;
    }
    
    return &SQCloudResultOK;
}

static SQCloudResult *internal_reconnect (SQCloudConnection *connection, const char *buffer, size_t blen) {
    // DO RE-CONNECT HERE
    return NULL;
}

static int32_t internal_array_count (char *buffer, uint32_t blen) {
    // =LEN N VALUE1 VALUE2 ... VALUEN
    if (buffer[0] != CMD_ARRAY) return -1;
    
    do {
        ++buffer;
        --blen;
    }
    while (buffer[0] != ' ');
    ++buffer; --blen;
    
    uint32_t size = 0;
    return internal_parse_number(buffer, blen, &size);
}

static SQCloudResult *internal_parse_array (SQCloudConnection *connection, char *buffer, uint32_t blen, uint32_t bstart) {
    SQCloudResult *rowset = (SQCloudResult *)mem_zeroalloc(sizeof(SQCloudResult));
    if (!rowset) {
        internal_set_error(connection, INTERNAL_ERRCODE_MEMORY, "Unable to allocate memory for SQCloudResult: %d.", sizeof(SQCloudResult));
        return NULL;
    }
    
    rowset->tag = RESULT_ARRAY;
    rowset->rawbuffer = buffer;
    rowset->blen = blen;
    rowset->nheader = bstart;
    
    // =LEN N VALUE1 VALUE2 ... VALUEN
    uint32_t start1 = 0;
    uint32_t n = internal_parse_number(&buffer[bstart], blen-1, &start1);
    
    rowset->ndata = n;
    rowset->data = (char **) mem_alloc(rowset->ndata * sizeof(char *));
    if (!rowset->data) {
        internal_set_error(connection, INTERNAL_ERRCODE_MEMORY, "Unable to allocate memory for SQCloudResult: %d.", rowset->ndata * sizeof(char *));
        mem_free(rowset);
        return NULL;
    }
    
    // loop from i to n to parse each data
    buffer += bstart + start1;
    for (uint32_t i=0; i<n; ++i) {
        uint32_t cellsize = 0;
        uint32_t len = blen - start1;
        char *value = internal_parse_value(buffer, &len, &cellsize);
        rowset->data[i] = (value) ? buffer : NULL;
        buffer += cellsize;
        blen -= cellsize;
    }
    
    return rowset;
}

static SQCloudResult *internal_rowset_type (SQCloudConnection *connection, char *buffer, uint32_t blen, uint32_t bstart, SQCLOUD_RESULT_TYPE type) {
    SQCloudResult *rowset = (SQCloudResult *)mem_zeroalloc(sizeof(SQCloudResult));
    if (!rowset) {
        internal_set_error(connection, INTERNAL_ERRCODE_MEMORY, "Unable to allocate memory for SQCloudResult: %d.", sizeof(SQCloudResult));
        return NULL;
    }
    
    rowset->tag = type;
    rowset->buffer = &buffer[bstart];
    rowset->rawbuffer = buffer;
    rowset->blen = blen;
    rowset->balloc = blen;
    
    return rowset;
}

static char *internal_get_rowset_header (SQCloudResult *result, char **header, uint32_t col, uint32_t *len) {
    if (!result || result->tag != RESULT_ROWSET) return NULL;
    if (col >= result->ncols) return NULL;
    if (header == NULL) return NULL;
    
    char *buffer = (result->ischunk) ? result->buffers[0] : result->rawbuffer;
    *len = result->blen - (uint32_t)(header[col] - buffer);
    return internal_parse_value(header[col], len, NULL);
}

static int internal_get_rowset_header_int (SQCloudResult *result, int *header, uint32_t col) {
    if (!result || result->tag != RESULT_ROWSET) return -1;
    if (col >= result->ncols) return -1;
    if (header == NULL) return -1;
    return header[col];
}

static bool internal_parse_rowset_header (SQCloudResult *rowset, char **pbuffer, uint32_t *pblen, uint32_t ncols, uint32_t version) {
    if (version == ROWSET_TYPE_DATA_ONLY) return true;
    
    char *buffer = *pbuffer;
    uint32_t blen = *pblen;
    
    /*
    if (BITCHECK(flags, SQCLOUD_ROWSET_FLAG_METAVM)) {
        uint32_t cstart1 = 0, cstart2 = 0, cstart3 = 0, cstart4 = 0, cstart5 = 0;
        
        // bind parameter count
        buffer += 1; blen -= 1;
        uint32_t n1 = internal_parse_number(buffer, blen, &cstart1);
        
        // vm is readonly
        buffer += cstart1; blen -= cstart1;
        uint32_t n2 = internal_parse_number(buffer, blen, &cstart2);
        
        // column count
        buffer += cstart2; blen -= cstart2;
        uint32_t n3 = internal_parse_number(buffer, blen, &cstart3);
        
        // vm is explain
        buffer += cstart3; blen -= cstart3;
        uint32_t n4 = internal_parse_number(buffer, blen, &cstart4);
        
        // tail len
        buffer += cstart4; blen -= cstart4;
        uint32_t n5 = internal_parse_number(buffer, blen, &cstart5);
         
        buffer += cstart5; blen -= cstart5;
        
        rowset->n1 = n1;
        rowset->n2 = n2;
        rowset->n3 = n3;
        rowset->n4 = n4;
        rowset->n5 = n5;
    }
     */
    
    // header is guarantee to contain column names
    for (uint32_t i=0; i<ncols; ++i) {
        uint32_t cstart = 0;
        uint32_t len = internal_parse_number(&buffer[1], blen, &cstart);
        rowset->name[i] = buffer;
        buffer += cstart + len + 1;
        blen -= cstart + len + 1;
        if (rowset->clen[i] < len) rowset->clen[i] = len;
        if (rowset->maxlen < len) rowset->maxlen = len;
    }
    
    // check if additional metadata is contained
    if (version == ROWSET_TYPE_METADATA_v1) {
        rowset->decltype = (char **) mem_alloc(ncols * sizeof(char *));
        if (!rowset->decltype) return false;
        rowset->dbname = (char **) mem_alloc(ncols * sizeof(char *));
        if (!rowset->dbname) return false;
        rowset->tblname = (char **) mem_alloc(ncols * sizeof(char *));
        if (!rowset->tblname) return false;
        rowset->origname = (char **) mem_alloc(ncols * sizeof(char *));
        if (!rowset->origname) return false;
        rowset->notnull = (int *) mem_alloc(ncols * sizeof(int));
        if (!rowset->notnull) return false;
        rowset->prikey = (int *) mem_alloc(ncols * sizeof(int));
        if (!rowset->prikey) return false;
        rowset->autoinc = (int *) mem_alloc(ncols * sizeof(int));
        if (!rowset->autoinc) return false;
        
        // column declared types
        for (uint32_t i=0; i<ncols; ++i) {
            uint32_t cstart = 0;
            uint32_t len = internal_parse_number(&buffer[1], blen, &cstart);
            rowset->decltype[i] = buffer;
            buffer += cstart + len + 1;
            blen -= cstart + len + 1;
        }
        
        // column database names
        for (uint32_t i=0; i<ncols; ++i) {
            uint32_t cstart = 0;
            uint32_t len = internal_parse_number(&buffer[1], blen, &cstart);
            rowset->dbname[i] = buffer;
            buffer += cstart + len + 1;
            blen -= cstart + len + 1;
        }
        
        // column table names
        for (uint32_t i=0; i<ncols; ++i) {
            uint32_t cstart = 0;
            uint32_t len = internal_parse_number(&buffer[1], blen, &cstart);
            rowset->tblname[i] = buffer;
            buffer += cstart + len + 1;
            blen -= cstart + len + 1;
        }
        
        // column origin names
        for (uint32_t i=0; i<ncols; ++i) {
            uint32_t cstart = 0;
            uint32_t len = internal_parse_number(&buffer[1], blen, &cstart);
            rowset->origname[i] = buffer;
            buffer += cstart + len + 1;
            blen -= cstart + len + 1;
        }
        
        // column not null flag
        for (uint32_t i=0; i<ncols; ++i) {
            uint32_t cstart = 0;
            uint32_t value = internal_parse_number(&buffer[1], blen, &cstart);
            rowset->notnull[i] = (int)value;
            uint32_t len = 0;
            buffer += cstart + len + 1;
            blen -= cstart + len + 1;
        }
        
        // column primary key flag
        for (uint32_t i=0; i<ncols; ++i) {
            uint32_t cstart = 0;
            uint32_t value = internal_parse_number(&buffer[1], blen, &cstart);
            rowset->prikey[i] = (int)value;
            uint32_t len = 0;
            buffer += cstart + len + 1;
            blen -= cstart + len + 1;
        }
        
        // column autoincrement key flag
        for (uint32_t i=0; i<ncols; ++i) {
            uint32_t cstart = 0;
            uint32_t value = internal_parse_number(&buffer[1], blen, &cstart);
            rowset->autoinc[i] = (int)value;
            uint32_t len = 0;
            buffer += cstart + len + 1;
            blen -= cstart + len + 1;
        }
    }
    
    *pbuffer = buffer;
    *pblen = blen;
    
    return true;
}

static bool internal_parse_rowset_values (SQCloudResult *rowset, char **pbuffer, uint32_t *pblen, uint32_t index, uint32_t bound, uint32_t ncols, uint32_t version) {
    if (version == ROWSET_TYPE_HEADER_ONLY) return true;
    
    char *buffer = *pbuffer;
    uint32_t blen = *pblen;
    
    for (uint32_t i=index; i<bound; ++i) {
        uint32_t len = blen, cellsize;
        char *value = internal_parse_value(buffer, &len, &cellsize);
        rowset->data[i] = (value) ? buffer : NULL;
        buffer += cellsize;
        blen -= cellsize;
        ++rowset->ndata;
        if (rowset->clen[i % ncols] < len) rowset->clen[i % ncols] = len;
        if (rowset->maxlen < len) rowset->maxlen = len;
    }
    
    return true;
}

static SQCloudResult *internal_parse_rowset (SQCloudConnection *connection, char *buffer, uint32_t blen, uint32_t bstart,
                                             uint32_t nrows, uint32_t ncols, uint32_t version) {
    SQCloudResult *rowset = (SQCloudResult *)mem_zeroalloc(sizeof(SQCloudResult));
    if (!rowset) {
        internal_set_error(connection, INTERNAL_ERRCODE_MEMORY, "Unable to allocate memory for SQCloudResult: %d.", sizeof(SQCloudResult));
        return NULL;
    }
    
    rowset->tag = RESULT_ROWSET;
    rowset->buffer = buffer;
    rowset->rawbuffer = buffer;
    rowset->blen = blen;
    rowset->balloc = blen;
    rowset->nheader = bstart;
    rowset->version = version;
    
    rowset->nrows = nrows;
    rowset->ncols = ncols;
    rowset->data = (char **) mem_alloc(nrows * ncols * sizeof(char *));
    rowset->name = (char **) mem_alloc(ncols * sizeof(char *));
    rowset->clen = (uint32_t *) mem_zeroalloc(ncols * sizeof(uint32_t));
    if (!rowset->data || !rowset->name || !rowset->clen) goto abort_rowset;
    
    buffer += bstart;
    blen -= bstart;
    
    // parse rowset header
    if (!internal_parse_rowset_header(rowset, &buffer, &blen, ncols, version)) goto abort_rowset;
    
    // parse values (buffer and blen was updated in internal_parse_rowset_header)
    if (!internal_parse_rowset_values(rowset, &buffer, &blen, 0, nrows * ncols, ncols, version)) goto abort_rowset;
    
    return rowset;
    
abort_rowset:
    if (rowset->data) mem_free(rowset->data);
    if (rowset->name) mem_free(rowset->name);
    if (rowset->clen) mem_free(rowset->clen);
    if (rowset) mem_free(rowset);
    
    internal_set_error(connection, INTERNAL_ERRCODE_MEMORY, "Unable to allocate internal memory for SQCloudResult.");
    return NULL;
}

static SQCloudResult *internal_parse_rowset_chunck (SQCloudConnection *connection, char *buffer, uint32_t blen, uint32_t bstart, uint32_t idx,
                                                    uint32_t nrows, uint32_t ncols, uint32_t version) {
    SQCloudResult *rowset = connection->_chunk;
    bool first_chunk = false;
    
    // sanity check
    if (idx == 1 && connection->_chunk) {
        // something bad happened here because a first chunk is received while a saved one has not been fully processed
        // lets try to restart the whole process
        SQCloudResultFree(connection->_chunk);
        connection->_chunk = NULL;
        rowset = NULL;
    }
    
    if (!rowset) {
        // this should never happen
        if (idx != 1) return NULL;
        
        // allocate a new rowset
        rowset = (SQCloudResult *)mem_zeroalloc(sizeof(SQCloudResult));
        if (!rowset) {
            internal_set_error(connection, INTERNAL_ERRCODE_MEMORY, "Unable to allocate memory for SQCloudResult: %d.", sizeof(SQCloudResult));
            return NULL;
        }
        first_chunk = true;
        connection->_chunk = rowset;
    }
    
    if (first_chunk) {
        rowset->tag = RESULT_ROWSET;
        rowset->version = version;
        rowset->ischunk = true;
        
        rowset->buffers = (char **)mem_zeroalloc((sizeof(char *) * DEFAULT_CHUCK_NBUFFERS));
        if (!rowset->buffers) goto abort_rowset;
        
        rowset->bext = (bool *)mem_zeroalloc((sizeof(bool) * DEFAULT_CHUCK_NBUFFERS));
        if (!rowset->bext) goto abort_rowset;
        
        rowset->blens = (uint32_t *)mem_zeroalloc((sizeof(uint32_t) * DEFAULT_CHUCK_NBUFFERS));
        if (!rowset->blens) goto abort_rowset;
        
        rowset->nheads = (uint32_t *)mem_zeroalloc((sizeof(uint32_t) * DEFAULT_CHUCK_NBUFFERS));
        if (!rowset->nheads) goto abort_rowset;
        
        rowset->bnum = DEFAULT_CHUCK_NBUFFERS;
        rowset->buffers[0] = buffer;
        rowset->bext[0] = false;
        rowset->blens[0] = blen;
        rowset->nheads[0] = bstart;
        rowset->bcount = 1;
        
        rowset->brows = nrows + DEFAULT_CHUNK_MINROWS;
        rowset->nrows = nrows;
        rowset->ncols = ncols;
        rowset->data = (char **) mem_alloc(rowset->brows * ncols * sizeof(char *));
        rowset->name = (char **) mem_alloc(ncols * sizeof(char *));
        rowset->clen = (uint32_t *) mem_zeroalloc(ncols * sizeof(uint32_t));
        if (!rowset->data || !rowset->name || !rowset->clen) goto abort_rowset;
        
        buffer += bstart;
        
        // parse rowset header
        if (!internal_parse_rowset_header(rowset, &buffer, &blen, ncols, version)) goto abort_rowset;
    }
    
    // update total buffer size
    rowset->blen += blen;
    
    // check end-chunk condition
    if (idx == 0 && nrows == 0 && ncols == 0) {
        connection->_chunk = NULL;
        if (!rowset->externalbuffer) mem_free(buffer);
        return rowset;
    }
    
    // check if a resize is needed in the array of buffers
    if (rowset->bnum <= rowset->bcount + 1) {
        uint32_t n = rowset->bnum * 2;
        char **temp = (char **)mem_realloc(rowset->buffers, (sizeof(char *) * n));
        if (!temp) goto abort_rowset;
        rowset->buffers = temp;
        
        bool *temp1 = (bool*)mem_realloc(rowset->bext, (sizeof(bool) * n));
        if (!temp1) goto abort_rowset;
        rowset->bext = temp1;
        
        uint32_t *temp2 = (uint32_t*)mem_realloc(rowset->blens, (sizeof(uint32_t) * n));
        if (!temp2) goto abort_rowset;
        rowset->blens = temp2;
        
        uint32_t *temp3 = (uint32_t*)mem_realloc(rowset->nheads, (sizeof(uint32_t) * n));
        if (!temp3) goto abort_rowset;
        rowset->nheads = temp3;
        
        rowset->bnum = n;
    }
    
    // check if a resize is needed in the ptr data array
    if (rowset->brows <= rowset->nrows + nrows) {
        uint32_t n = rowset->brows * 2;
        char **temp = (char **)mem_realloc(rowset->data, n * ncols * (sizeof(char *)));
        if (!temp) goto abort_rowset;
        rowset->data = temp;
        rowset->brows = n;
    }
    
    // adjust internal fields
    if (!first_chunk) {
        rowset->buffers[rowset->bcount] = buffer;
        rowset->bext[rowset->bcount] = rowset->externalbuffer;
        rowset->blens[rowset->bcount] = blen;
        rowset->nheads[rowset->bcount] = bstart;
        rowset->nrows += nrows;
        buffer += bstart;
        
        // increment buffers counter
        ++rowset->bcount;
    }
    
    // parse values
    uint32_t index = rowset->ndata;
    uint32_t bound = rowset->ndata + (nrows * ncols);
    
    // parse values
    if (!internal_parse_rowset_values(rowset, &buffer, &blen, index, bound, ncols, version)) goto abort_rowset;
    
    // this check is for internal usage only
    if (connection->fd == 0) return rowset;
    
    #if 0
    // January 24th, 2024 -> ACK disabled for Rowset in chunks
    // send ACK
    // if (!internal_socket_write(connection, "OK", 2, true, true)) goto abort_rowset;
    #endif
        
    // read next chunk
    return internal_socket_read (connection, true);
    
abort_rowset:
    SQCloudResultFree(rowset);
    connection->_chunk = NULL;
    return NULL;
}

static SQCloudResult *internal_parse_buffer (SQCloudConnection *connection, char *buffer, uint32_t blen, uint32_t cstart, bool isstatic, bool externalbuffer) {
    if (blen <= 1) return false;
    
    bool buffer_canbe_freed = (!isstatic && !externalbuffer);
    
    // try to check if it is a OK reply: +2 OK
    if ((blen == REPLY_OK_LEN) && (strncmp(buffer, REPLY_OK, REPLY_OK_LEN) == 0)) {
        if (buffer_canbe_freed) mem_free(buffer);
        return &SQCloudResultOK;
    }
    
    // check for compressed reply before the parse step
    char *zdata = NULL;
    if (buffer[0] == CMD_COMPRESSED) {
        // %TLEN CLEN ULEN *0 NROWS NCOLS DATA
        uint32_t cstart1 = 0;
        uint32_t cstart2 = 0;
        uint32_t cstart3 = 0;
        uint32_t tlen = internal_parse_number(&buffer[1], blen-1, &cstart1);
        uint32_t clen = internal_parse_number(&buffer[cstart1 + 1], blen-(cstart1 + 1), &cstart2);
        uint32_t ulen = internal_parse_number(&buffer[cstart1 + cstart2 + 1], blen-(cstart1 + cstart2 + 1), &cstart3);
        
        // start of compressed buffer
        zdata = &buffer[tlen - clen + cstart1 + 1];
        
        // start of raw uncompressed header
        char *hstart = &buffer[cstart1 + cstart2 + cstart3 + 1];
        
        // try to allocate a buffer big enough to hold uncompressed data + raw header
        // 256 is an arbitrary memory cushion value
        uint32_t clonelen = ulen + (uint32_t)(hstart - buffer) + 256;
        char *clone = mem_alloc (clonelen);
        if (!clone) {
            internal_set_error(connection, INTERNAL_ERRCODE_MEMORY, "Unable to allocate memory to uncompress buffer: %d.", clonelen);
            if (buffer_canbe_freed) mem_free(buffer);
            return NULL;
        }
        
        // copy raw buffer
        memcpy(clone, hstart, zdata - hstart);
        
        // uncompress buffer and sanity check the result
        uint32_t rc = LZ4_decompress_safe(zdata, clone + (zdata - hstart), clen, ulen);
        if (rc <= 0 || rc != ulen) {
            internal_set_error(connection, INTERNAL_ERRCODE_GENERIC, "Unable to decompress buffer (err code: %d).", rc);
            if (buffer_canbe_freed) mem_free(buffer);
            return NULL;
        }
        
        // decompression is OK so replace buffer
        if (buffer_canbe_freed) mem_free(buffer);
        
        isstatic = false;
        buffer = clone;
        blen = clonelen;
        
        // at this point the buffer used in the SQCloudResult is a newly allocated one (clone)
        // so externalbuffer flag must be set to false
        externalbuffer = false;
    } else {
        // if buffer is static (stack based allocation) then it must be duplicated
        bool buffer_should_be_duplicated = (buffer[0] != CMD_ERROR);
        if (buffer_should_be_duplicated && isstatic) {
            char *clone = mem_alloc(blen);
            if (!clone) {
                internal_set_error(connection, INTERNAL_ERRCODE_MEMORY, "Unable to allocate memory: %d.", blen);
                if (buffer_canbe_freed) mem_free(buffer);
                return NULL;
            }
            memcpy(clone, buffer, blen);
            buffer = clone;
            isstatic = false;
        }
    }
    
    // re-compute flag
    buffer_canbe_freed = (!isstatic && !externalbuffer);
    
    // parse reply
    switch (buffer[0]) {
        case CMD_ZEROSTRING:
        case CMD_RECONNECT:
        case CMD_PUBSUB:
        case CMD_COMMAND:
        case CMD_STRING:
        case CMD_ARRAY:
        case CMD_BLOB:
        case CMD_JSON: {
            // +LEN string
            uint32_t cstart = 0;
            // parse explicit len
            uint32_t len = internal_parse_number(&buffer[1], blen-1, &cstart);
            SQCLOUD_RESULT_TYPE type = (buffer[0] == CMD_JSON) ? RESULT_JSON : RESULT_STRING;
            if (buffer[0] == CMD_ZEROSTRING) --len;
            else if (buffer[0] == CMD_COMMAND) return internal_run_command(connection, &buffer[cstart+1], len, true);
            else if (buffer[0] == CMD_PUBSUB) return internal_setup_pubsub(connection, &buffer[cstart+1], len);
            else if (buffer[0] == CMD_RECONNECT) return internal_reconnect(connection, &buffer[cstart+1], len);
            else if (buffer[0] == CMD_ARRAY) return internal_parse_array(connection, buffer, len, cstart+1);
            else if (buffer[0] == CMD_BLOB) type = RESULT_BLOB;
            SQCloudResult *res = internal_rowset_type(connection, buffer, len, cstart+1, type);
            if (res) res->externalbuffer = externalbuffer;
            return res;
        }
            
        case CMD_ERROR: {
            // -LEN ERRCODE[:EXTCODE:OFFCODE] ERRMSG
            uint32_t cstart = 0, cstart2 = 0;
            uint32_t len = internal_parse_number(&buffer[1], blen-1, &cstart);
            
            int32_t offcode = -1;
            uint32_t extcode = 0;
            uint32_t errcode = internal_parse_number_extended(&buffer[cstart + 1], blen-1, &cstart2, &extcode, &offcode);
            connection->errcode = (int)errcode;
            connection->extcode = (int)extcode;
            connection->offcode = (int)offcode;
            
            len -= cstart2;
            memcpy(connection->errmsg, &buffer[cstart + cstart2 + 1], MIN(len, sizeof(connection->errmsg)));
            connection->errmsg[len] = 0;
            
            // check free buffer
            if (buffer_canbe_freed) mem_free(buffer);
            return NULL;
        }
        
        case CMD_ROWSET:
        case CMD_ROWSET_CHUNK: {
            // CMD_ROWSET:          *LEN 0:VERSION ROWS COLS DATA
            // CMD_ROWSET_CHUNK:    /LEN IDX:VERSION ROWS COLS DATA
            uint32_t cstart1 = 0, cstart2 = 0, cstart3 = 0, cstart4 = 0;
            uint32_t version = 0;
            
            internal_parse_number(&buffer[1], blen-1, &cstart1); // parse len (already parsed in blen parameter)
            uint32_t idx = internal_parse_number_extended(&buffer[cstart1 + 1], blen-(cstart1+1), &cstart2, &version, NULL);
            uint32_t nrows = internal_parse_number(&buffer[cstart1 + cstart2 + 1], blen-(cstart1 + cstart2 + 1), &cstart3);
            uint32_t ncols = internal_parse_number(&buffer[cstart1 + cstart2 + + cstart3 + 1], blen-(cstart1 + cstart2 + + cstart3 + 1), &cstart4);
            
            // idx is always 0 if (buffer[0] == CMD_ROWSET)
            
            uint32_t bstart = cstart1 + cstart2 + cstart3 + cstart4 + 1;
            SQCloudResult *res = NULL;
            // the externalbuffer flag can change in case of compressed rowset when the end chunk is received
            if (connection->_chunk) connection->_chunk->externalbuffer = externalbuffer;
            if (buffer[0] == CMD_ROWSET) res = internal_parse_rowset(connection, buffer, blen, bstart, nrows, ncols, version);
            else res = internal_parse_rowset_chunck(connection, buffer, blen, bstart, idx, nrows, ncols, version);
            if (res) {
                res->externalbuffer = externalbuffer;
                if (res->ischunk && res->bcount == 1) res->bext[0] = externalbuffer;
            }
            
            // check free buffer
            if (!res && buffer_canbe_freed) mem_free(buffer);
            return res;
        }
        
        case CMD_NULL:
            if (buffer_canbe_freed) mem_free(buffer);
            return &SQCloudResultNULL;
            
        case CMD_INT:
        case CMD_FLOAT: {
            // NUMBER case
            internal_parse_value(buffer, &blen, NULL);
            SQCloudResult *res = internal_rowset_type(connection, buffer, blen, 1, (buffer[0] == CMD_INT) ? RESULT_INTEGER : RESULT_FLOAT);
            if (res) res->externalbuffer = externalbuffer;
            
            if (!res && buffer_canbe_freed) mem_free(buffer);
            return res;
        }
            
        case CMD_RAWJSON: {
            // SHOULD NEVER REACH THIS POINT
            // handle JSON here
            // a JSON parser must process raw buffer
            return &SQCloudResultNULL;
        }
    }
    
    if (buffer_canbe_freed) mem_free(buffer);
    return NULL;
}

static bool internal_socket_forward_read (SQCloudConnection *connection, bool (*forward_cb) (char *buffer, size_t blen, void *xdata, void *xdata2), void *xdata, void *xdata2) {
    char sbuffer[8129];
    uint32_t blen = sizeof(sbuffer);
    uint32_t cstart = 0;
    uint32_t tread = 0;
    uint32_t clen = 0;
    char type = 0;
    
    ssize_t nread = 0;
    char *buffer = sbuffer;
    char *original = buffer;
    int fd = connection->fd;
    #ifndef SQLITECLOUD_DISABLE_TLS
    struct tls *tls = connection->tls_context;
    #endif
    
    while (1) {
        // perform read operation
        #ifndef SQLITECLOUD_DISABLE_TLS
        nread = (tls) ? tls_read(tls, buffer, blen) : readsocket(fd, buffer, blen);
        if ((tls) && (nread == TLS_WANT_POLLIN || nread == TLS_WANT_POLLOUT)) continue;
        #else
        nread = readsocket(fd, buffer, blen);
        #endif
        if (nread == -1 && errno == EINTR) continue;
        
        // sanity check read
        if (nread <= 0) goto abort_read;
        
        // forward read to callback
        bool result = forward_cb(buffer, nread, xdata, xdata2);
        if (!result) goto abort_read;
        
        // read original type
        if (type == 0) type = buffer[0];
        
        if (type != CMD_ROWSET_CHUNK) {
            // update internal counter
            tread += (uint32_t)nread;
            
            // determine command length
            if (clen == 0) {
                clen = internal_parse_number (&original[1], tread-1, &cstart);
                
                // handle special cases
                if ((original[0] == CMD_INT) || (original[0] == CMD_FLOAT) || (original[0] == CMD_NULL)) clen = 0;
                else if (clen == 0) continue;
            }
            
            // check if read is complete
            if (clen + cstart + 1 == tread) break;
        } else {
            const char *end_of_chunk = "/6 0 0 0 ";
            size_t end_of_chunk_len = 9;
            
            if (nread >= end_of_chunk_len) {
                if (strncmp(buffer + nread - end_of_chunk_len, end_of_chunk, end_of_chunk_len) == 0) break;
            } else {
                // there is an extremely rare possibility that the end of chuck was split by the TCP driver
                // in that case we would have no way to determine the end of the rowset chunk
                ;
            }
        }
    }
    
    return true;
    
abort_read: {
        const char *msg = "";
        const char *format = (nread == 0) ? "Unexpected EOF found while reading data: %s (%s)." : "An error occurred while reading data: %s (%s).";
        #ifndef SQLITECLOUD_DISABLE_TLS
        if (tls) msg = tls_error(tls);
        #endif
        internal_set_error(connection, INTERNAL_ERRCODE_NETWORK, format, strerror(errno), msg);
    }
    return false;
}

static ssize_t internal_socket_read_nbytes (int fd, void *tlsp, char *buffer, ssize_t len) {
    ssize_t total_read = 0;
    
    while (1) {
        #ifndef SQLITECLOUD_DISABLE_TLS
        ssize_t nread = (tlsp) ? tls_read((struct tls *)tlsp, buffer + total_read, len - total_read) : readsocket(fd, buffer + total_read, len - total_read);
        if ((tlsp) && (nread == TLS_WANT_POLLIN || nread == TLS_WANT_POLLOUT)) continue;
        #else
        nread = readsocket(fd, buffer + total_read, len - total_read);
        #endif
        if (nread == -1 && errno == EINTR) continue;
        total_read += nread;
        
        if (nread <= 0) return nread;
        if (total_read == len) break;
    };
    
    return total_read;
}

static SQCloudResult *internal_socket_read (SQCloudConnection *connection, bool mainfd) {
    int fd = (mainfd) ? connection->fd : connection->pubsubfd;
    #ifndef SQLITECLOUD_DISABLE_TLS
    struct tls *tls = (mainfd) ? connection->tls_context : connection->tls_pubsub_context;
    #else
    void *tls = NULL;
    #endif
    
    ssize_t nread = 0;
    uint32_t clen = 0;
    uint32_t cstart = 0;
    char header[64];
    int header_index = 0;
    
    char *buffer = NULL;
    char static_buffer[4096];
    
    // read the buffer one character at a time until a space is encountered
    // see https://github.com/sqlitecloud/sdk/blob/master/PROTOCOL.md for more details about the protocol
    // after this loop we can know the buffer type and len
    while (1) {
        nread = internal_socket_read_nbytes(fd, tls, &header[header_index], 1);
        if (nread <= 0) goto abort_read;
        if (header[header_index] == ' ') break;
        ++header_index;
        
        // check for malformed header
        if (header_index >= sizeof(header)) {
            internal_set_error(connection, INTERNAL_ERRCODE_NETWORK, "Bad protocol reply from server: unable to find buffer size (type was %c).", header[0]);
            return NULL;
        }
    }
    
    // parse len (if any)
    int header_size = header_index + 1; // +1 because ++header_index; is after the break clause
    if (internal_has_commandlen(header[0])) {
        clen = internal_parse_number (&header[1], header_size-1, &cstart);
        
        // check special zero-length value
        if (clen == 0) {
            if (internal_canbe_zerolength(header[0])) {
                // it is perfectly legit to have a zero-bytes string or blob
                return internal_parse_buffer(connection, header, header_size, 0, true, false);
            } else {
                // we parsed a zero-length header but we command does not allow that value, so return an error
                internal_set_error(connection, INTERNAL_ERRCODE_NETWORK, "Bad protocol reply from server: the type %c cannot have a zero length buffer.", header[0]);
                return NULL;
            }
        }
    } else {
        // command does not have an explicit len so the header can be safely processed
        return internal_parse_buffer(connection, header, header_size, (clen) ? cstart : 0, true, false);
    }
    
    // header correctly parsed and len is greater than zero, check if allocate a buffer or use a static one
    // the static buffer optimization was added because of the +2 OK messages
    size_t blen = clen + header_size;
    buffer = (blen <= sizeof(static_buffer)) ? static_buffer : mem_alloc(blen);
    if (!buffer) {
        internal_set_error(connection, INTERNAL_ERRCODE_MEMORY, "Unable to allocate memory: %d.", blen);
        return NULL;
    }
    
    // copy header back to buffer
    memcpy(buffer, header, header_size);
    
    // read the remaing part of the command
    nread = internal_socket_read_nbytes(fd, tls, &buffer[header_size], clen);
    if (nread <= 0) goto abort_read;
    
    // command is complete so parse it
    return internal_parse_buffer(connection, buffer, (uint32_t)blen, (clen) ? cstart : 0, (buffer == static_buffer), false);
    
abort_read: {
        const char *msg = "";
        const char *format = (nread == 0) ? "Unexpected EOF found while reading data: %s (%s)." : "An error occurred while reading data: %s (%s).";
        #ifndef SQLITECLOUD_DISABLE_TLS
        if (tls) msg = tls_error(tls);
        #endif
        internal_set_error(connection, INTERNAL_ERRCODE_NETWORK, format, strerror(errno), msg);
    }
    
    if (buffer && buffer != static_buffer) mem_free(buffer);
    return NULL;
}

static bool internal_socket_raw_write (SQCloudConnection *connection, const char *buffer) {
    // this function is used only to debug possible security issues
    int fd = connection->fd;
    #ifndef SQLITECLOUD_DISABLE_TLS
    struct tls *tls = connection->tls_context;
    #endif
    
    size_t len = strlen(buffer);
    size_t written = 0;
    while (len > 0) {
        #ifndef SQLITECLOUD_DISABLE_TLS
        ssize_t nwrote = (tls) ? tls_write(tls, buffer, len) : writesocket(fd, buffer, len);
        if ((tls) && (nwrote == TLS_WANT_POLLIN || nwrote == TLS_WANT_POLLOUT)) continue;
        #else
        ssize_t nwrote = writesocket(fd, buffer, len);
        #endif
        
        if (nwrote < 0) {
            const char *msg = "";
            #ifndef SQLITECLOUD_DISABLE_TLS
            if (tls) msg = tls_error(tls);
            #endif
            return internal_set_error(connection, INTERNAL_ERRCODE_NETWORK, "An error occurred while writing data: %s (%s).", strerror(errno), msg);
        } else if (nwrote == 0) {
            return true;
        } else {
            written += nwrote;
            buffer += nwrote;
            len -= nwrote;
        }
    }
    
    return true;
}

static bool internal_socket_write (SQCloudConnection *connection, const char *buffer, size_t len, bool mainfd, bool compute_header) {
    int fd = (mainfd) ? connection->fd : connection->pubsubfd;
    #ifndef SQLITECLOUD_DISABLE_TLS
    struct tls *tls = (mainfd) ? connection->tls_context : connection->tls_pubsub_context;
    #endif
    
    
    // optimization tip
    // instead of writing twice to the socket (one for the header and one for the data)
    // try to pack everything inside the same buffer, which is generally faster
    // -12 is to reserve enough space for the header
    char blocal[4096];
    if (compute_header && !connection->isblob && (len < sizeof(blocal)-12)) {
        size_t len_local = snprintf(blocal, sizeof(blocal), "+%zu %s", len, buffer);
        if (len_local < sizeof(blocal)) {
            len = len_local;
            buffer = blocal;
            compute_header = false;
        }
    }
    
    // write header
    size_t written = 0;
    if (compute_header) {
        char header[32];
        char *p = header;
        int hlen = snprintf(header, sizeof(header), "%c%zu ", (connection->isblob) ? CMD_BLOB : CMD_STRING, len);
        int len1 = hlen;
        while (len1) {
            #ifndef SQLITECLOUD_DISABLE_TLS
            ssize_t nwrote = (tls) ? tls_write(tls, p, len1) : writesocket(fd, p, len1);
            if ((tls) && (nwrote == TLS_WANT_POLLIN || nwrote == TLS_WANT_POLLOUT)) continue;
            #else
            ssize_t nwrote = writesocket(fd, p, len1);
            #endif
            
            if ((nwrote < 0) || (nwrote == 0 && written != hlen)) {
                const char *msg = "";
                #ifndef SQLITECLOUD_DISABLE_TLS
                if (tls) msg = tls_error(tls);
                #endif
                return internal_set_error(connection, INTERNAL_ERRCODE_NETWORK, "An error occurred while writing header data: %s (%s).", strerror(errno), msg);
            } else {
                written += nwrote;
                p += nwrote;
                len1 -= nwrote;
            }
        }
    }
    
    // write buffer
    written = 0;
    while (len > 0) {
        #ifndef SQLITECLOUD_DISABLE_TLS
        ssize_t nwrote = (tls) ? tls_write(tls, buffer, len) : writesocket(fd, buffer, len);
        if ((tls) && (nwrote == TLS_WANT_POLLIN || nwrote == TLS_WANT_POLLOUT)) continue;
        #else
        ssize_t nwrote = writesocket(fd, buffer, len);
        #endif
        
        if (nwrote < 0) {
            const char *msg = "";
            #ifndef SQLITECLOUD_DISABLE_TLS
            if (tls) msg = tls_error(tls);
            #endif
            return internal_set_error(connection, INTERNAL_ERRCODE_NETWORK, "An error occurred while writing data: %s (%s).", strerror(errno), msg);
        } else if (nwrote == 0) {
            return true;
        } else {
            written += nwrote;
            buffer += nwrote;
            len -= nwrote;
        }
    }
    
    return true;
}

static void internal_socket_set_timeout (int sockfd, int timeout_secs) {
    #ifdef _WIN32
    DWORD timeout = timeout_secs * 1000;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof timeout);
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof timeout);
    #else
    struct timeval tv;
    tv.tv_sec = timeout_secs;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof tv);
    #endif
}

static bool internal_connect_apply_config (SQCloudConnection *connection, SQCloudConfig *config) {
    if (config->timeout) {
        internal_socket_set_timeout(connection->fd, config->timeout);
    }

    char buffer[2048];
    int len = 0;
    
    // non-linearizable option must be executed first
    if (config->non_linearizable) {
        len += snprintf(&buffer[len], sizeof(buffer) - len, "SET CLIENT KEY NONLINEARIZABLE TO 1;");
    }
    
    if (config->username && config->password && strlen(config->username) && strlen(config->password)) {
        char *command = config->password_hashed ? "HASH" : "PASSWORD";
        len += snprintf(&buffer[len], sizeof(buffer) - len, "AUTH USER %s %s %s;", config->username,  command, config->password);
    }
    
    if (config->api_key && strlen(config->api_key)) {
        len += snprintf(&buffer[len], sizeof(buffer) - len, "AUTH APIKEY %s;", config->api_key);
    }
    
    if (config->database && strlen(config->database)) {
        if (config->db_create && !config->db_memory) len += snprintf(&buffer[len], sizeof(buffer) - len, "CREATE DATABASE %s IF NOT EXISTS;", config->database);
        len += snprintf(&buffer[len], sizeof(buffer) - len, "USE DATABASE %s;", config->database);
    }
    
    if (config->compression) {
        len += snprintf(&buffer[len], sizeof(buffer) - len, "SET CLIENT KEY COMPRESSION TO 1;");
    }
    
    if (config->zero_text) {
        len += snprintf(&buffer[len], sizeof(buffer) - len, "SET CLIENT KEY ZEROTEXT TO 1;");
    }
    
    if (config->no_blob) {
        len += snprintf(&buffer[len], sizeof(buffer) - len, "SET CLIENT KEY NOBLOB TO 1;");
    }
    
    if (config->max_data > 0) {
        len += snprintf(&buffer[len], sizeof(buffer) - len, "SET CLIENT KEY MAXDATA TO %d;", config->max_data);
    }
    
    if (config->max_rows > 0) {
        len += snprintf(&buffer[len], sizeof(buffer) - len, "SET CLIENT KEY MAXROWS TO %d;", config->max_rows);
    }
    
    if (config->max_rowset > 0) {
        len += snprintf(&buffer[len], sizeof(buffer) - len, "SET CLIENT KEY MAXROWSET TO %d;", config->max_rowset);
    }
    
    if (config->callback) {
        len += config->callback(&buffer[len], sizeof(buffer) - len, config->data);
    }
    
    if (len > 0) {
        SQCloudResult *res = internal_run_command(connection, buffer, strlen(buffer), true);
        if (res != &SQCloudResultOK) return false;
    }
    
    return true;
}

static bool internal_connect (SQCloudConnection *connection, const char *hostname, int port, SQCloudConfig *config, bool mainfd) {
    // ipv4/ipv6 specific variables
    struct addrinfo hints, *addr_list = NULL, *addr;
    
    // ipv6 code from https://www.ibm.com/support/knowledgecenter/ssw_ibm_i_72/rzab6/xip6client.htm
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (config && config->family) {
        if (config->family == SQCLOUD_IPv6) hints.ai_family = AF_INET6;
        if (config->family == SQCLOUD_IPANY) hints.ai_family = AF_UNSPEC;
    }
    
    // get the address information for the server using getaddrinfo()
    char port_string[256];
    snprintf(port_string, sizeof(port_string), "%d", port);
    int rc = getaddrinfo(hostname, port_string, &hints, &addr_list);
    if (rc != 0 || addr_list == NULL) {
        return internal_set_error(connection, INTERNAL_ERRCODE_NETWORK, "Error while resolving getaddrinfo (host %s not found).", hostname);
    }
    
    // begin non-blocking connection loop
    int sock_index = 0;
    int sock_current = 0;
    int sock_list[MAX_SOCK_LIST] = {0};
    for (addr = addr_list; addr != NULL; addr = addr->ai_next, ++sock_index) {
        if (sock_index >= MAX_SOCK_LIST) break;
        if ((addr->ai_family != AF_INET) && (addr->ai_family != AF_INET6)) continue;
        
        sock_current = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (sock_current < 0) continue;
        
        // set socket options
        int len = 1;
        setsockopt(sock_current, SOL_SOCKET, SO_KEEPALIVE, (const char *) &len, sizeof(len));
        
        // disable Nagle algorithm because we want our writes to be sent ASAP
        // https://brooker.co.za/blog/2024/05/09/nagle.html
        len = 1;
        setsockopt(sock_current, IPPROTO_TCP, TCP_NODELAY, (const char *) &len, sizeof(len));
        
        #ifdef SO_NOSIGPIPE
        len = 1;
        setsockopt(sock_current, SOL_SOCKET, SO_NOSIGPIPE, (const char *) &len, sizeof(len));
        #endif
        
        // by default, an IPv6 socket created on Windows Vista and later only operates over the IPv6 protocol
        // in order to make an IPv6 socket into a dual-stack socket, the setsockopt function must be called
        if (addr->ai_family == AF_INET6) {
            #ifdef _WIN32
            DWORD ipv6only = 0;
            #else
            int   ipv6only = 0;
            #endif
            setsockopt(sock_current, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&ipv6only, sizeof(ipv6only));
        }
        
        // turn on non-blocking
        unsigned long ioctl_blocking = 1;    /* ~0; //TRUE; */
        ioctl(sock_current, FIONBIO, &ioctl_blocking);
        
        // initiate non-blocking connect ignoring return code
        rc = connect(sock_current, addr->ai_addr, addr->ai_addrlen);
        
        // add sock_current to internal list of trying to connect sockets
        sock_list[sock_index] = sock_current;
    }
    
    // free not more needed memory
    freeaddrinfo(addr_list);
    
    // calculate the connection timeout and reset timers
    // if timeout is <= 0 then it is set to SQCLOUD_DEFAULT_TIMEOUT for the connect phase
    int connect_timeout = (config && config->timeout > 0) ? config->timeout : SQCLOUD_DEFAULT_TIMEOUT;
    time_t start = time(NULL);
    time_t now = start;
    rc = 0;
    
    int sockfd = 0;
    fd_set write_fds;
    fd_set except_fds;
    struct timeval tv;
    
    while (rc == 0 && ((now - start) < connect_timeout)) {
        FD_ZERO(&write_fds);
        FD_ZERO(&except_fds);
        
        int nfds = 0;
        for (int i=0; i<MAX_SOCK_LIST; ++i) {
            if (sock_list[i]) {
                FD_SET(sock_list[i], &write_fds);
                FD_SET(sock_list[i], &except_fds);
                if (nfds < sock_list[i]) nfds = sock_list[i];
            }
        }
        
        tv.tv_sec = connect_timeout;
        tv.tv_usec = 0;
        rc = select(nfds + 1, NULL, &write_fds, &except_fds, &tv);
        
        if (rc == 0) break; // timeout
        else if (rc == -1) {
            if (errno == EINTR || errno == EAGAIN || errno == EINPROGRESS) continue;
            break; // handle error
        }
        
        // check for error first
        for (int i=0; i<MAX_SOCK_LIST; ++i) {
            if (sock_list[i] > 0) {
                if (FD_ISSET(sock_list[i], &except_fds)) {
                    closesocket(sock_list[i]);
                    sock_list[i] = 0;
                }
            }
        }
        
        // check which file descriptor is ready (need to check for socket error also)
        for (int i=0; i<MAX_SOCK_LIST; ++i) {
            if (sock_list[i] > 0) {
                if (FD_ISSET(sock_list[i], &write_fds)) {
                    int err = socket_geterror(sock_list[i]);
                    if (err > 0) {
                        closesocket(sock_list[i]);
                        sock_list[i] = 0;
                    } else {
                        sockfd = sock_list[i];
                        break;
                    }
                }
            }
        }
        // check if a valid descriptor has been found
        if (sockfd != 0) break;
        
        // no socket ready yet
        now = time(NULL);
        rc = 0;
    }
    
    // close still opened sockets
    for (int i=0; i<MAX_SOCK_LIST; ++i) {
        if ((sock_list[i] > 0) && (sock_list[i] != sockfd)) closesocket(sock_list[i]);
    }
    
    // bail if there was an error
    if (rc < 0) {
        return internal_set_error(connection, INTERNAL_ERRCODE_NETWORK, "An error occurred while trying to connect: %s.", strerror(errno));
    }
    
    // bail if there was a timeout
    if ((time(NULL) - start) >= connect_timeout) {
        return internal_set_error(connection, INTERNAL_ERRCODE_NETWORK, "Connection timeout while trying to connect (%d).", connect_timeout);
    }
    
    // turn off non-blocking
    int ioctl_blocking = 0;    /* ~0; //TRUE; */
    ioctl(sockfd, FIONBIO, &ioctl_blocking);
    
    // finalize connection
    if (mainfd) {
        connection->fd = sockfd;
        connection->port = port;
        connection->hostname = mem_string_dup(hostname);
        #ifndef SQLITECLOUD_DISABLE_TLS
        if (config && !config->insecure) {
            rc = tls_connect_socket(connection->tls_context, sockfd, hostname);
            if (rc < 0) printf("Error on tls_connect_socket: %s\n", tls_error(connection->tls_context));
        }
        #endif
    } else {
        connection->pubsubfd = sockfd;
        #ifndef SQLITECLOUD_DISABLE_TLS
        if (config && !config->insecure) {
            rc = tls_connect_socket(connection->tls_pubsub_context, sockfd, hostname);
            if (rc < 0) printf("Error on tls_connect_socket\n");
        }
        #endif
    }
    return true;
}

bool internal_rowset_compare(SQCloudResult *rs1, SQCloudResult *rs2) {
    if (rs1 == NULL && rs2 == NULL) return true;
    if (rs1 == NULL && rs2 != NULL) return false;
    if (rs1 != NULL && rs2 == NULL) return false;
    
    if (rs1->nrows != rs2 ->nrows) return false;
    if (rs1->ncols != rs2 ->ncols) return false;
    
    uint32_t nrows = rs1->nrows;
    uint32_t ncols = rs1->ncols;
    
    // check column names
    for (uint32_t i=0; i<ncols; ++i) {
        uint32_t len1 = internal_buffer_maxlen(rs1, rs1->name[i]);
        char *value1 = internal_parse_value(rs1->name[i], &len1, NULL);
        
        uint32_t len2 = internal_buffer_maxlen(rs2, rs2->name[i]);
        char *value2 = internal_parse_value(rs2->name[i], &len2, NULL);
        
        if (len1 != len2) return false;
        if (strncmp(value1, value2, len1) != 0) return false;
    }
    
    // check types
    for (uint32_t i=0; i<nrows; ++i) {
        for (uint32_t j=0; j<ncols; ++j) {
            SQCLOUD_VALUE_TYPE type1 = SQCloudRowsetValueType(rs1, i, j);
            SQCLOUD_VALUE_TYPE type2 = SQCloudRowsetValueType(rs2, i, j);
            if (type1 != type2) return false;
        }
    }
    
    // check values
    for (uint32_t i=0; i<nrows * ncols; ++i) {
        uint32_t len1 = internal_buffer_maxlen(rs1, rs1->data[i]);
        char *value1 = internal_parse_value(rs1->data[i], &len1, NULL);
        
        uint32_t len2 = internal_buffer_maxlen(rs2, rs2->data[i]);
        char *value2 = internal_parse_value(rs2->data[i], &len2, NULL);
        
        if (len1 != len2) return false;
        if (value1 == NULL && value2 == NULL) return true;
        if (value1 == NULL || value2 == NULL) return false;
        if (memcmp(value1, value2, len1) != 0) return false;
    }
    
    return true;
}

void internal_rowset_dump (SQCloudResult *result, uint32_t maxline, bool quiet) {
    uint32_t nrows = result->nrows;
    uint32_t ncols = result->ncols;
    
    // if user specify a maxline then do not print more than maxline characters for every column
    if (maxline > 0) {
        for (uint32_t i=0; i<ncols; ++i) {
            if (result->clen[i] > maxline) result->clen[i] = maxline;
        }
    }
    
    // print separator header
    for (uint32_t i=0; i<ncols; ++i) {
        for (uint32_t j=0; j<result->clen[i]+2; ++j) putchar('-');
        putchar('|');
    }
    printf("\n");
    
    // print column names
    for (uint32_t i=0; i<ncols; ++i) {
        uint32_t len = internal_buffer_maxlen(result, result->name[i]);
        uint32_t delta = 0;
        char *value = internal_parse_value(result->name[i], &len, NULL);
        
        // UTF-8 strings need special adjustments
        uint32_t utf8len = utf8_len(value, len);
        if (utf8len != len) delta = len - utf8len;
        printf(" %-*.*s |", result->clen[i] + delta, (maxline && len > maxline) ? maxline : len, value);
    }
    printf("\n");
    
    // print separator header
    for (uint32_t i=0; i<ncols; ++i) {
        for (uint32_t j=0; j<result->clen[i]+2; ++j) putchar('-');
        putchar('|');
    }
    printf("\n");
    
    #if 0
    // print types (just for debugging)
    printf("\n");
    for (uint32_t i=0; i<nrows; ++i) {
        for (uint32_t j=0; j<ncols; ++j) {
            SQCloudValueType type = SQCloudRowsetValueType(result, i, j);
            printf("%d ", type);
        }
        printf("\n");
    }
    printf("\n");
    #endif
    
    // print result
    for (uint32_t i=0; i<nrows * ncols; ++i) {
        uint32_t len = internal_buffer_maxlen(result, result->data[i]);
        uint32_t delta = 0;
        char *value = internal_parse_value(result->data[i], &len, NULL);

        // UTF-8 strings need special adjustments
        if (!value) {value = "NULL"; len = 4;}
        uint32_t utf8len = utf8_len(value, len);
        if (utf8len != len) delta = len - utf8len;
        printf(" %-*.*s |", (result->clen[i % ncols]) + delta, (maxline && len > maxline) ? maxline : len, value);
        
        bool newline = (((i+1) % ncols == 0) || (ncols == 1));
        if (newline) printf("\n");
    }
    
    // print footer
    for (uint32_t i=0; i<ncols; ++i) {
        for (uint32_t j=0; j<result->clen[i]+2; ++j) putchar('-');
        putchar('|');
    }
    printf("\n");
    
    printf("Rows: %d - Cols: %d - Bytes: %d", result->nrows, result->ncols, result->blen);
    if (!quiet) printf(" Time: %f secs", result->time);
    fflush( stdout );
}

bool internal_upload_database (SQCloudConnection *connection, const char *dbname, const char *key, bool isfiletransfer, uint64_t snapshotid, bool isinternaldb, void *xdata, int64_t dbsize, int (*xCallback)(void *xdata, void *buffer, uint32_t *blen, int64_t ntot, int64_t nprogress)) {
    // xCallback is mandatory
    if (!xCallback) return false;
    
    const char *keyarg = key ? "KEY " : "";
    const char *keyvalue = key ? key : "";
    
    // prepare command to execute
    char command[512]; 
    if (isfiletransfer) {
        char *internalarg = isinternaldb ? "INTERNAL" : "";
        snprintf(command, sizeof(command), "TRANSFER DATABASE '%s' %s%s SNAPSHOT %" PRIu64 " %s", dbname, keyarg, keyvalue, snapshotid, internalarg);
    } else {
        snprintf(command, sizeof(command), "UPLOAD DATABASE '%s' %s%s", dbname, keyarg, keyvalue);
    }
    
    // execute command on server side
    SQCloudResult *res = SQCloudExec(connection, command);
    bool isOK = (SQCloudResultType(res) == RESULT_OK);
    SQCloudResultFree(res);
    if (!isOK) return false;
    
    void *buffer = mem_alloc(SQCLOUD_DEFAULT_UPLOAD_SIZE);
    if (!buffer) return internal_set_error(connection, INTERNAL_ERRCODE_MEMORY, "Unable to allocate a buffer of size %d.", SQCLOUD_DEFAULT_UPLOAD_SIZE);
    
    uint32_t blen = 0;
    int64_t nprogress = 0;
    bool result = false;
    do {
        // execute callback to read buffer
        blen = SQCLOUD_DEFAULT_UPLOAD_SIZE;
        int rc = xCallback(xdata, buffer, &blen, dbsize, nprogress);
        if (rc != 0) {
            SQCloudResult *res = SQCloudExec(connection, "UPLOAD ABORT");
            SQCloudResultFree(res);
            goto cleanup;
        }
        
        // send BLOB
        if (internal_send_blob(connection, buffer, blen) == false) goto cleanup;
        
        // update progress
        nprogress += blen;
    } while (blen > 0);
    
    result = true;
    
cleanup:
    mem_free(buffer);
    return result;
}

// n is the total number of items in the array
// count is the total number of items contained in r and len
// instead of build a new text buffer +LEN TEXT
// is it easier to send +LEN in a buffer
// and TEXT in the next buffer
// that's the reason why count and n can be different
SQCloudResult *internal_array_exec (SQCloudConnection *connection, const char *r[], int64_t len[], uint32_t n, uint32_t count) {
    char header[512];
    char nitems[64];
    int64_t totsize = 0;
    
    internal_clear_error(connection);
    
    // compute total array size
    for (int i=0; i<count; ++i) totsize += len[i];
    
    // build header
    // =LEN N VALUE1 VALUE2 ... VALUEN
    
    TIME_GET(tstart);
    int nlen = snprintf(nitems, sizeof(nitems), "%d ", n);
    int hlen = snprintf(header, sizeof(header), "%c%lld %s", CMD_ARRAY, totsize+nlen, nitems);
    if (!internal_socket_write(connection, header, hlen, true, false)) return NULL;
    
    // send each individual array item
    for (int i=0; i<count; ++i) {
        if (!internal_socket_write(connection, r[i], (size_t)len[i], true, false)) return NULL;
    }
    
    // read reply
    SQCloudResult *result = internal_socket_read(connection, true);
    TIME_GET(tend);
    
    if (result) result->time = TIME_VAL(tstart, tend);
    return result;
    
abort:
    return NULL;
}

void internal_free_config (SQCloudConfig *config) {
    if (config->username) mem_free((void *)config->username);
    if (config->password) mem_free((void *)config->password);
    if (config->database) mem_free((void *)config->database);
    #ifndef SQLITECLOUD_DISABLE_TLS
    if (config->tls_root_certificate) mem_free((void *)config->tls_root_certificate);
    if (config->tls_certificate) mem_free((void *)config->tls_certificate);
    if (config->tls_certificate_key) mem_free((void *)config->tls_certificate_key);
    #endif
    mem_free(config);
}

// MARK: - URL -

static int char2hex (int c) {
    if (isdigit(c)) return (c - '0');
    c = toupper(c);
    if (c >='A' && c <='F') return (c - 'A' + 0x0A);
    return -1;
}

static int url_decode (char s[512]) {
    int i = 0;
    int j = 0;
    int len = (int)strlen(s);
    
    while (i < len) {
        int c = s[i];
        if (c == '%') {
            if (i + 2 >= len) return 0;
            c = (char2hex(s[i+1]) * 0x10) + char2hex(s[i+2]);
            if (c < 0) return 0;
            s[j] = c;
            i += 2;
        } else {
            s[j] = c;
        }
        ++i;
        ++j;
    }
    s[j] = 0;
    return j;
}

static int url_extract_username_password (const char *s, char b1[512], char b2[512]) {
    // user:pass@host.com:port/dbname?timeout=10&key2=value2&key3=value3
    
    // lookup username (if any)
    char *username = strchr(s, ':');
    if (!username) return 0;
    size_t len = username - s;
    if (len > 511) return -1;
    memcpy(b1, s, len);
    b1[len] = 0;
    if (url_decode(b1) <= 0) return 0;
    
    // lookup username (if any)
    char *password = strchr(s, '@');
    if (!password) return 0;
    len = password - username - 1;
    if (len > 511) return -1;
    memcpy(b2, username+1, len);
    b2[len] = 0;
    if (url_decode(b2) <= 0) return 0;
    
    return (int)(password - s) + 1;
}

static int url_extract_hostname_port (const char *s, char b1[512], char b2[512]) {
    // host.com:port/dbname?timeout=10&key2=value2&key3=value3
    
    // lookup hostname (if any)
    char *hostname = strchr(s, ':');
    if (!hostname) hostname = strchr(s, '/');
    if (!hostname) hostname = strchr(s, '?');
    if (!hostname) hostname = strchr(s, 0);
    if (!hostname) return -1;
    size_t len = hostname - s;
    if (len > 511) return -1;
    memcpy(b1, s, len);
    b1[len] = 0;
    if (url_decode(b1) <= 0) return 0;
    
    // lookup port (if any)
    char *port = strchr(s, ':');
    if (port) {
        char *p = port + 1;
        ++len;
        
        int i = 0;
        while (p[0]) {
            if ((p[0] == '/') || (p[0] == '?') || (p[0] == 0)) break;
            if (i+1 > 511) return -1;
            b2[i++] = p[0];
            ++len;
            ++p;
        }
        b2[len] = 0;
        if (url_decode(b2) <= 0) return 0;
    }
    
    // adjust returned len
    if (s[len] != 0) ++len;
    
    return (int)len;
}

static int url_extract_database (const char *s, char b1[512]) {
    // dbname?timeout=10&key2=value2&key3=value3
    
    // lookup database (if any)
    char *database = strchr(s, '?');
    if (database) {
        size_t len = database - s;
        if (len > 511) return -1;
        memcpy(b1, s, len);
        b1[len] = 0;
        if (url_decode(b1) <= 0) return 0;
        
        return (int)(len + 1);
    }
    
    // there is no ? separator character
    // that means that there should be
    // no key/value
    char *guard = strchr(s, '=');
    if (guard) return 0;
    
    // database name is the s string
    size_t len = strlen(s);
    if (len > 511) return -1;
    memcpy(b1, s, len);
    b1[len] = 0;
    if (url_decode(b1) <= 0) return 0;
    
    return (int)len;
}

static int url_extract_keyvalue (const char *s, char b1[512], char b2[512]) {
    // timeout=10&key2=value2&key3=value3
    
    // lookup key (if any)
    char *key = strchr(s, '=');
    if (!key) return 0;
    size_t len = key - s;
    if (len > 511) return -1;
    memcpy(b1, s, len);
    b1[len] = 0;
    if (url_decode(b1) <= 0) return 0;
    
    // lookup value (if any)
    char *value = strchr(s, '&');
    if (!value) value = strchr(s, 0);
    if (!value) return 0;
    len = value - key - 1;
    if (len > 511) return -1;
    memcpy(b2, key+1, len);
    b2[len] = 0;
    if (url_decode(b2) <= 0) return 0;
    
    return (int)(value - s) + 1;
}

// MARK: - RESERVED -

SQCloudResult *_reserved0 (SQCloudConnection *connection, const char *buffer, size_t blen, bool compute_header) {
    internal_clear_error(connection);
    
    if (!buffer || blen < CMD_MINLEN) return NULL;
    
    TIME_GET(tstart);
    if (!internal_socket_write(connection, buffer, blen, true, compute_header)) return NULL;
    SQCloudResult *result = internal_socket_read(connection, true);
    TIME_GET(tend);
    if (result) result->time = TIME_VAL(tstart, tend);
    return result;
}


bool _reserved1 (SQCloudConnection *connection, const char *command, size_t len, bool compute_header, bool (*forward_cb) (char *buffer, size_t blen, void *xdata, void *xdata2), void *xdata, void *xdata2) {
    if (!forward_cb) return false;
    if (!internal_socket_write(connection, command, len, true, compute_header)) return false;
    if (!internal_socket_forward_read(connection, forward_cb, xdata, xdata2)) return false;
    return true;
}

SQCloudResult *_reserved2 (SQCloudConnection *connection, const char *UUID) {
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "SET CLIENT KEY UUID TO %s;", UUID);
    return internal_run_command(connection, buffer, strlen(buffer), true);
}

SQCloudResult *_reserved3 (char *buffer, uint32_t blen, uint32_t cstart, SQCloudResult *chunk) {
    SQCloudConnection connection = {0};
    connection._chunk = chunk;
    SQCloudResult *res = internal_parse_buffer(&connection, buffer, blen, cstart, false, true);
    return res;
}

uint32_t _reserved4 (char *buffer, uint32_t blen, uint32_t *cstart) {
    return internal_parse_number(buffer, blen, cstart);
}

bool _reserved5 (SQCloudResult *res) {
    return res->ischunk;
}

bool _reserved6 (SQCloudConnection *connection, const char *buffer) {
    internal_clear_error(connection);
    return internal_socket_raw_write(connection, buffer);
}

bool _reserved8 (SQCloudConnection *connection, const char *dbname, const char *key, uint64_t snapshotid, bool isinternaldb, void *xdata, int64_t dbsize, int (*xCallback)(void *xdata, void *buffer, uint32_t *blen, int64_t ntot, int64_t nprogress)) {
    return internal_upload_database(connection, dbname, key, true, snapshotid, isinternaldb, xdata, dbsize, xCallback);
}

int _reserved9 (char *buffer) {
    return internal_parse_type(buffer);
}

char *_reserved10 (char *buffer, uint32_t *len, uint32_t *cellsize) {
    return internal_parse_value(buffer, len, cellsize);
}

char *_reserved11 (char *buffer, uint32_t blen, uint32_t index, uint32_t *len, uint32_t *cellsize, uint32_t *pos, int *type, SQCLOUD_INTERNAL_ERRCODE *err) {
    if (err) *err = INTERNAL_ERRCODE_NONE;
    
    // =LEN N VALUE1 VALUE2 ... VALUEN
    if (buffer[0] != CMD_ARRAY) {
        if (err) *err = INTERNAL_ERRCODE_FORMAT;
        return NULL;
    }
        
    char *p = buffer;
    do {
        ++buffer;
        --blen;
    }
    while (buffer[0] != ' ');
    ++buffer; --blen;
    
    uint32_t tcellsize = 0;
    uint32_t n = internal_parse_number(buffer, blen, &tcellsize);
    if (index >= n) {
        if (err) *err = INTERNAL_ERRCODE_INDEX;
        return NULL;
    }
        
    buffer += tcellsize;
    blen -= tcellsize;
    
    uint32_t tlen = blen;
    uint32_t csize = 0;
    for (int i=0; i<=index; ++i) {
        tcellsize = 0;
        char *value = internal_parse_value(buffer, &tlen, &tcellsize);
        
        if (i == index) {
            if (type && buffer) *type = buffer[0];
            if (len) *len = tlen;
            if (cellsize) *cellsize = tcellsize;
            if (pos) *pos = (uint32_t)(value - p);
            return value;
        }
        
        buffer += tcellsize;
        csize += tcellsize;
        tlen = blen - csize;
    }
    
    return NULL;
}

int32_t _reserved12 (char *buffer, uint32_t blen) {
    return internal_array_count(buffer, blen);
}

bool _reserved13 (SQCloudConnection *connection, const char *dbname, void *xdata,
                                      int (*xCallback)(void *xdata, const void *buffer, uint32_t blen, int64_t ntot, int64_t nprogress), uint64_t *raft_index, bool ifexists) {
    // xCallback is mandatory
    if (!xCallback) return false;
    
    // prepare command to execute
    char buffer[512];
    snprintf(buffer, sizeof(buffer), "DOWNLOAD DATABASE %s%s", dbname, (ifexists) ? " IF EXISTS" : "");
    
    // execute command on server side
    SQCloudResult *res = SQCloudExec(connection, buffer);
    
    // reply must be an Array value (otherwise it is an error)
    if (SQCloudResultType(res) != RESULT_ARRAY) return false;
    
    // res is an ARRAY (database size, number of pages, raft_index)
    int64_t db_size = SQCloudArrayInt64Value(res, 0);
    int64_t rindex = SQCloudArrayInt64Value(res, 2);
    SQCloudResultFree(res);
    
    // loop to download
    int64_t progress_size = 0;
    snprintf(buffer, sizeof(buffer), "DOWNLOAD STEP");
    
    while (progress_size < db_size) {
        res = SQCloudExec(connection, buffer);
        
        // reply must be a BLOB value (otherwise it is an error)
        if (SQCloudResultType(res) != RESULT_BLOB) return false;
        
        // res is BLOB, decode it
        const void *data = (const void *)SQCloudResultBuffer(res);
        uint32_t datalen = SQCloudResultLen(res);
        
        // execute callback (with progress_size updated)
        progress_size += datalen;
        int rc = xCallback(xdata, data, datalen, db_size, progress_size);
        SQCloudResultFree(res);
        
        // check exit condition
        if (datalen == 0) break;
        
        // check if download should be cancelled
        if (rc != 0) {
            snprintf(buffer, sizeof(buffer), "DOWNLOAD ABORT");
            SQCloudExec(connection, buffer);
            return false;
        }
    }
    
    if (raft_index) *raft_index = rindex;
    return true;
}

// MARK: - PUBLIC -

SQCloudConnection *SQCloudConnect (const char *hostname, int port, SQCloudConfig *config) {
    internal_init();
    
    SQCloudConnection *connection = mem_zeroalloc(sizeof(SQCloudConnection));
    if (!connection) return NULL;
    connection->_config = config;
    
    if (!internal_setup_tls(connection, config, true)) return connection;
    
    if (internal_connect(connection, hostname, port, config, true)) {
        if (config) internal_connect_apply_config(connection, config);
    }
    
    return connection;
}

SQCloudConnection *SQCloudConnectWithString (const char *s, SQCloudConfig *pconfig) {
    // URL STRING FORMAT
    // sqlitecloud://user:pass@host.com:port/dbname?timeout=10&key2=value2&key3=value3
    
    // sanity check
    const char domain[] = "sqlitecloud://";
    int n = sizeof(domain) - 1;
    if (strncmp(s, domain, n) != 0) return NULL;
    size_t slen = strlen(s);
    
    // config struct
    SQCloudConfig *config = (SQCloudConfig *)mem_zeroalloc(sizeof(SQCloudConfig));
    if (!config) return NULL;
    
    // default IPv4
    config->family = SQCLOUD_IPv4;
    
    // lookup for optional username/password
    char username[512];
    char password[512];
    int rc = url_extract_username_password(&s[n], username, password);
    if (rc == -1) goto abort_connect;
    
    if (rc) {
        config->username = mem_string_dup(username);
        config->password = mem_string_dup(password);
    }
    
    // lookup for mandatory hostname
    n += rc;
    if (n >= slen) goto abort_connect;
    
    char hostname[512];
    char port_s[512];
    rc = url_extract_hostname_port(&s[n], hostname, port_s);
    if (rc <= 0) goto abort_connect;
        
    int port = (int)strtol(port_s, NULL, 0);
    if (port <= 0) port = SQCLOUD_DEFAULT_PORT;
    
    // lookup for optional database
    n += rc;
    if (n < slen) {
        char database[512];
        rc = url_extract_database(&s[n], database);
        if (rc == -1) goto abort_connect;
        
        if (rc > 0) {
            config->database = mem_string_dup(database);
            config->db_memory = (strcasecmp(database, ":memory:") == 0 || strcasecmp(database, ":temp:") == 0);
        }
    }
    
    // lookup for optional key(s)/value(s)
    n += rc;
    char key[512];
    char value[512];
    while ((n < slen) && (rc = url_extract_keyvalue(&s[n], key, value)) > 0) {
        if (strcasecmp(key, "timeout") == 0) {
            int timeout = (int)strtol(value, NULL, 0);
            config->timeout = (timeout > 0) ? timeout : 0;
        }
        else if (strcasecmp(key, "compression") == 0) {
            int compression = (int)strtol(value, NULL, 0);
            config->compression = (compression > 0) ? true : false;
        }
        else if (strcasecmp(key, "zerotext") == 0) {
            int zero_text = (int)strtol(value, NULL, 0);
            config->zero_text = (zero_text > 0) ? true : false;
        }
        else if (strcasecmp(key, "memory") == 0) {
            int in_memory = (int)strtol(value, NULL, 0);
            if (in_memory) config->database = mem_string_dup(":memory:");
        }
        else if (strcasecmp(key, "create") == 0) {
            int db_create = (int)strtol(value, NULL, 0);
            if (db_create) config->db_create = (db_create > 0) ? true : false;
        }
        #ifndef SQLITECLOUD_DISABLE_TLS
        else if (strcasecmp(key, "insecure") == 0) {
            int insecure = (int)strtol(value, NULL, 0);
            config->insecure = (insecure > 0) ? true : false;
        }
        else if (strcasecmp(key, "no_verify_certificate") == 0) {
            int no_verify_certificate = (int)strtol(value, NULL, 0);
            config->no_verify_certificate = (no_verify_certificate > 0) ? true : false;
        }
        else if ((strcasecmp(key, "non_linearizable") == 0) || (strcasecmp(key, "nonlinearizable") == 0)) {
            int dvalue = (int)strtol(value, NULL, 0);
            config->non_linearizable = (dvalue > 0) ? true : false;
        }
        else if (strcasecmp(key, "root_certificate") == 0) {
            config->tls_root_certificate = mem_string_dup(value);
        }
        else if (strcasecmp(key, "client_certificate") == 0) {
            config->tls_certificate = mem_string_dup(value);
        }
        else if (strcasecmp(key, "client_certificate_key") == 0) {
            config->tls_certificate_key = mem_string_dup(value);
        }
        #endif
        else if (strcasecmp(key, "noblob") == 0) {
            int no_blob = (int)strtol(value, NULL, 0);
            config->no_blob = (no_blob > 0) ? true : false;
        }
        else if (strcasecmp(key, "maxdata") == 0) {
            int dvalue = (int)strtol(value, NULL, 0);
            if (dvalue >= 0) config->max_data = dvalue;
        }
        else if (strcasecmp(key, "maxrows") == 0) {
            int dvalue = (int)strtol(value, NULL, 0);
            if (dvalue >= 0) config->max_rows = dvalue;
        }
        else if (strcasecmp(key, "maxrowset") == 0) {
            int dvalue = (int)strtol(value, NULL, 0);
            if (dvalue >= 0) config->max_rowset = dvalue;
        }
        else if (strcasecmp(key, "apikey") == 0) {
            config->api_key = mem_string_dup(value);
        }
        n += rc;
    }
    
    // config parameter is used to force some configuration flags
    if (pconfig) {
        if (pconfig->timeout) config->timeout = pconfig->timeout;
        if (pconfig->compression) config->compression = pconfig->compression;
        if (pconfig->zero_text) config->zero_text = pconfig->zero_text;
        if (pconfig->non_linearizable) config->non_linearizable = pconfig->non_linearizable;
        if (pconfig->no_blob) config->no_blob = pconfig->no_blob;
        if (pconfig->db_create) config->db_create = pconfig->db_create;
        if (pconfig->max_data) config->max_data = pconfig->max_data;
        if (pconfig->max_rows) config->max_rows = pconfig->max_rows;
        if (pconfig->max_rowset) config->max_rowset = pconfig->max_rowset;
        if (pconfig->insecure) config->insecure = pconfig->insecure;
        if (pconfig->db_memory) {
            if (config->database) mem_free((void *)config->database);
            config->database = mem_string_dup(":memory:");
        }
        if (pconfig->api_key) {
            if (config->api_key) mem_free((void *)config->api_key);
            config->api_key = mem_string_dup(pconfig->api_key);
        }
    }
    
    SQCloudConnection *connection = SQCloudConnect(hostname, port, config);
    if (connection) connection->config_to_free = true;
    else goto abort_connect;
    
    return connection;
    
abort_connect:
    if (config) internal_free_config(config);
    return NULL;
}

SQCloudResult *SQCloudExec (SQCloudConnection *connection, const char *command) {
    return internal_run_command(connection, command, strlen(command), true);
}

SQCloudResult *SQCloudExecArray (SQCloudConnection *connection, const char *command, const char **values, uint32_t len[], SQCLOUD_VALUE_TYPE types[], uint32_t n) {
    if (!command) return NULL;
    if (n == 0) return SQCloudExec(connection, command);
    
    // compute the maximum number of required slots
    uint32_t ritems = n + 1; // add command
    uint32_t count = ritems * 2;
    SQCloudResult *result = NULL;
    
    // avoid dynamic memory allocation (if possible) with a 256 static array
    char s_head[ARRAY_STATIC_COUNT * ARRAY_HEADER_BUFFER_SIZE];
    const char *s_r[ARRAY_STATIC_COUNT];
    int64_t s_rlen[ARRAY_STATIC_COUNT];
    char *d_head = NULL;
    const char **d_r = NULL;
    int64_t *d_rlen = NULL;
    
    // initially set pointers to static buffers
    char *head = (char *)s_head;
    const char **r = s_r;
    int64_t *rlen = s_rlen;
    
    // check if dynamically allocated memory is required
    if (count >= ARRAY_STATIC_COUNT) {
        d_head = (char *)mem_alloc(count * ARRAY_HEADER_BUFFER_SIZE);
        d_r = (const char **)mem_alloc(count * sizeof(char *));
        d_rlen = (int64_t *)mem_alloc(count * sizeof(int64_t));
        if ((!d_head) || (!d_r) || (!d_rlen)) goto cleanup;
        
        head = d_head;
        r = d_r;
        rlen = d_rlen;
    }
    
    // 1st array item is the command
    size_t command_len = strlen(command) + 1; // +1 because string must be NULL terminated
    rlen[0] = snprintf(head, ARRAY_HEADER_BUFFER_SIZE, "%c%lu ", CMD_ZEROSTRING, command_len);
    rlen[1] = (int64_t)command_len;
    r[0] = head;
    r[1] = command;
   
    // update head ptr
    head += rlen[1] + 1;
    
    uint32_t index = 2;
    for (int i=0; i<n; ++i) {
        switch (types[i]) {
            case VALUE_INTEGER:
            case VALUE_FLOAT: {
                int c = (types[i] == VALUE_INTEGER) ? CMD_INT : CMD_FLOAT;
                rlen[index] = snprintf(head, ARRAY_HEADER_BUFFER_SIZE, "%c%s ", c, values[i]);
                r[index] = head;
                --count;
                ++index;
            } break;
                
            case VALUE_NULL: {
                rlen[index] = snprintf(head, ARRAY_HEADER_BUFFER_SIZE, "_ ");
                r[index] = head;
                --count;
                ++index;
            } break;
                
            case VALUE_TEXT:
            case VALUE_BLOB: {
                int c = (types[i] == VALUE_TEXT) ? CMD_ZEROSTRING : CMD_BLOB;
                uint32_t size = (types[i] == VALUE_TEXT) ? len[i]+1 : len[i]; // +1 because string must be NULL terminated
                rlen[index] = snprintf(head, ARRAY_HEADER_BUFFER_SIZE, "%c%u ", c, size);
                rlen[index+1] = (int64_t)size;
                r[index] = head;
                r[index+1] = values[i];
                index += 2;
            } break;
        }
        
        // update head ptr
        head += rlen[i+2] + 1;
    }

    result = internal_array_exec(connection, r, rlen, ritems, count);
    
cleanup:
    if (count >= ARRAY_STATIC_COUNT) {
        // free dynamically allocated memory
        if (d_head) mem_free(d_head);
        if (d_r) mem_free(d_r);
        if (d_rlen) mem_free(d_rlen);
    }
    
    return result;
}

void SQCloudDisconnect (SQCloudConnection *connection) {
    if (!connection) return;
    
    // free TLS
    #ifndef SQLITECLOUD_DISABLE_TLS
    if (connection->tls_context) {
        tls_close(connection->tls_context);
        tls_free(connection->tls_context);
    }
    
    if (connection->tls_pubsub_context) {
        tls_close(connection->tls_pubsub_context);
        tls_free(connection->tls_pubsub_context);
    }
    #endif
    
    // try to gracefully close connections
    if (connection->fd) {
        closesocket(connection->fd);
    }
    
    if (connection->pubsubfd) {
        closesocket(connection->pubsubfd);
    }
    
    // free memory
    if (connection->hostname) {
        mem_free(connection->hostname);
    }
    
    if (connection->uuid) {
        mem_free(connection->uuid);
    }
    
    if (connection->config_to_free) {
        internal_free_config(connection->_config);
    }
    
    mem_free(connection);
}

void SQCloudSetPubSubCallback (SQCloudConnection *connection, SQCloudPubSubCB callback, void *data) {
    connection->callback = callback;
    connection->data = data;
}

SQCloudResult *SQCloudSetPubSubOnly (SQCloudConnection *connection) {
    if (!connection->callback) {
        internal_set_error(connection, INTERNAL_ERRCODE_PUBSUB, "A PubSub callback must be set before executing a PUBSUB ONLY command.");
        return NULL;
    }
    
    const char *command = "PUBSUB ONLY";
    return internal_run_command(connection, command, strlen(command), true);
}

const char *SQCloudUUID (SQCloudConnection *connection) {
    if (!connection->uuid) {
        SQCloudResult *result = SQCloudExec(connection, "GET CLIENT KEY UUID");
        if (SQCloudResultType(result) == RESULT_STRING) connection->uuid = mem_string_ndup(SQCloudResultBuffer(result), SQCloudResultLen(result));
        SQCloudResultFree(result);
    }
    
    return (const char *)connection->uuid;
}

SQCloudConfig *SQCloudGetConfig (SQCloudConnection *connection) {
    return connection->_config;
}

// MARK: - ERROR -

bool SQCloudIsError (SQCloudConnection *connection) {
    return (!connection || connection->errcode);
}

bool SQCloudIsSQLiteError (SQCloudConnection *connection) {
    // https://www.sqlite.org/rescode.html
    return (connection && connection->errcode < 10000);
}

int SQCloudErrorCode (SQCloudConnection *connection) {
    return (connection) ? connection->errcode : INTERNAL_ERRCODE_GENERIC;
}

int SQCloudExtendedErrorCode (SQCloudConnection *connection) {
    return (connection) ? connection->extcode : 0;
}

int SQCloudErrorOffset (SQCloudConnection *connection) {
    return (connection) ? connection->offcode : -1;
}

const char *SQCloudErrorMsg (SQCloudConnection *connection) {
    return (connection) ? connection->errmsg : "Not enough memory to allocate a SQCloudConnection.";
}

void SQCloudErrorReset (SQCloudConnection *connection) {
    internal_clear_error(connection);
}

void SQCloudErrorSetCode (SQCloudConnection *connection, int errcode) {
    connection->errcode = errcode;
}

void SQCloudErrorSetMsg (SQCloudConnection *connection, const char *format, ...) {
    va_list arg;
    va_start (arg, format);
    vsnprintf(connection->errmsg, sizeof(connection->errmsg), format, arg);
    va_end (arg);
}

// MARK: - RESULT -

SQCLOUD_RESULT_TYPE SQCloudResultType (SQCloudResult *result) {
    return (result) ? result->tag : RESULT_ERROR;
}

bool SQCloudResultIsOK (SQCloudResult *result) {
    return (result == &SQCloudResultOK);
}

bool SQCloudResultIsError (SQCloudResult *result) {
    return (!result);
}

uint32_t SQCloudResultLen (SQCloudResult *result) {
    return (result) ? result->blen : 0;
}

char *SQCloudResultBuffer (SQCloudResult *result) {
    return (result) ? result->buffer : NULL;
}

int32_t SQCloudResultInt32 (SQCloudResult *result) {
    if ((!result) || (result->tag != RESULT_INTEGER)) return 0;
    
    char *buffer = result->buffer;
    buffer[result->blen] = 0;
    return (int32_t)strtol(buffer, NULL, 0);
}

int64_t SQCloudResultInt64 (SQCloudResult *result) {
    if ((!result) || (result->tag != RESULT_INTEGER)) return 0;
    
    char *buffer = result->buffer;
    buffer[result->blen] = 0;
    return (int64_t)strtoll(buffer, NULL, 0);
}

double SQCloudResultDouble (SQCloudResult *result) {
    if ((!result) || (result->tag != RESULT_FLOAT)) return 0.0;
    
    char *buffer = result->buffer;
    buffer[result->blen] = 0;
    return (double)strtod(buffer, NULL);
}

float SQCloudResultFloat (SQCloudResult *result) {
    if ((!result) || (result->tag != RESULT_FLOAT)) return 0.0;
    
    char *buffer = result->buffer;
    buffer[result->blen] = 0;
    return (float)strtof(buffer, NULL);
}

void SQCloudResultFree (SQCloudResult *result) {
    if (!result || (result == &SQCloudResultOK) || (result == &SQCloudResultNULL)) return;
    
    if (!result->ischunk && !result->externalbuffer) {
        mem_free(result->rawbuffer);
    }
    
    if (result->tag == RESULT_ROWSET) {
        mem_free(result->name);
        mem_free(result->data);
        mem_free(result->clen);
        if (result->decltype) mem_free(result->decltype);
        if (result->dbname) mem_free(result->dbname);
        if (result->tblname) mem_free(result->tblname);
        if (result->origname) mem_free(result->origname);
        
        if (result->ischunk) {
            // each buffer has its own externalbuffer flag, it depends on whether the original
            // buffer was external or not and whether it was reallocated (in case of compression) or not
            for (uint32_t i = 0; i<result->bcount; ++i) {
                if (result->buffers[i] && !result->bext[i]) mem_free(result->buffers[i]);
            }
            mem_free(result->buffers);
            mem_free(result->bext);
            mem_free(result->blens);
            mem_free(result->nheads);
        }
    }
    
    if (result->tag == RESULT_ARRAY) {
        mem_free(result->data);
    }
    
    mem_free(result);
}

void SQCloudResultDump (SQCloudConnection *connection, SQCloudResult *result) {
    // res NULL means to read error message and error code from conn
    SQCLOUD_RESULT_TYPE type = SQCloudResultType(result);
    switch (type) {
        case RESULT_OK:
            printf("OK");
            break;
            
        case RESULT_ERROR:
            printf("ERROR: %s (%d)", SQCloudErrorMsg(connection), SQCloudErrorCode(connection));
            break;
            
        case RESULT_NULL:
            printf("NULL");
            break;
            
        case RESULT_STRING:
            (SQCloudResultLen(result)) ? printf("%.*s", SQCloudResultLen(result), SQCloudResultBuffer(result)) : printf("");
            break;
            
        case RESULT_JSON:
        case RESULT_INTEGER:
        case RESULT_FLOAT:
            printf("%.*s", SQCloudResultLen(result), SQCloudResultBuffer(result));
            break;
            
        case RESULT_ARRAY:
            SQCloudArrayDump(result);
            break;
            
        case RESULT_ROWSET:
            SQCloudRowsetDump(result, 0, false);
            break;
            
        case RESULT_BLOB:
            printf("BLOB data with len: %d", SQCloudResultLen(result));
            break;
    }
    
    printf("\n\n");
}

// MARK: - ROWSET -

// https://database.guide/2-sample-databases-sqlite/
// https://embeddedartistry.com/blog/2017/07/05/printf-a-limited-number-of-characters-from-a-string/
// https://stackoverflow.com/questions/1809399/how-to-format-strings-using-printf-to-get-equal-length-in-the-output

// SET DATABASE mediastore.sqlite
// SELECT * FROM Artist LIMIT 10;

static bool SQCloudRowsetSanityCheck (SQCloudResult *result, uint32_t row, uint32_t col) {
    if (!result || result->tag != RESULT_ROWSET) return false;
    if ((row >= result->nrows) || (col >= result->ncols)) return false;
    return true;
}

SQCLOUD_VALUE_TYPE SQCloudRowsetValueType (SQCloudResult *result, uint32_t row, uint32_t col) {
    if (!SQCloudRowsetSanityCheck(result, row, col)) return VALUE_NULL;
    return internal_type(result->data[row*result->ncols+col]);
}

uint32_t SQCloudRowsetRowsMaxColumnLength (SQCloudResult *result, uint32_t col) {
    return (result) ? result->clen[ col ] : 0;
}

char *SQCloudRowsetColumnName (SQCloudResult *result, uint32_t col, uint32_t *len) {
    return internal_get_rowset_header(result, result->name, col, len);
}

char *SQCloudRowsetColumnDeclType (SQCloudResult *result, uint32_t col, uint32_t *len) {
    return internal_get_rowset_header(result, result->decltype, col, len);
}

char *SQCloudRowsetColumnDBName (SQCloudResult *result, uint32_t col, uint32_t *len) {
    return internal_get_rowset_header(result, result->dbname, col, len);
}

char *SQCloudRowsetColumnTblName (SQCloudResult *result, uint32_t col, uint32_t *len){
    return internal_get_rowset_header(result, result->tblname, col, len);
}

char *SQCloudRowsetColumnOrigName (SQCloudResult *result, uint32_t col, uint32_t *len) {
    return internal_get_rowset_header(result, result->origname, col, len);
}

uint32_t SQCloudRowSetColumnNotNULL (SQCloudResult *result, uint32_t col) {
    return internal_get_rowset_header_int(result, result->notnull, col);
}

uint32_t SQCloudRowSetColumnPrimaryKey (SQCloudResult *result, uint32_t col) {
    return internal_get_rowset_header_int(result, result->prikey, col);
}

uint32_t SQCloudRowSetColumnAutoIncrement (SQCloudResult *result, uint32_t col) {
    return internal_get_rowset_header_int(result, result->autoinc, col);
}

bool SQCloudRowsetCanWrite (SQCloudResult *result) {
    // check if the rowset is not a JOIN (must have the same table)
    char *keytable = result->tblname[0];
    for (int i=1; i<result->ncols; ++i) {
        if (strcmp(keytable, result->tblname[i]) != 0) return false;
    }
    
    // check if contains at least a primary key
    for (int i=0; i<result->ncols; ++i) {
        if (result->prikey[i] == 1) return true;
    }
    
    return false;
}

uint32_t SQCloudRowsetRows (SQCloudResult *result) {
    if (!SQCloudRowsetSanityCheck(result, 0, 0)) return 0;
    return result->nrows;
}

uint32_t SQCloudRowsetCols (SQCloudResult *result) {
    if (!SQCloudRowsetSanityCheck(result, 0, 0)) return 0;
    return result->ncols;
}

uint32_t SQCloudRowsetMaxLen (SQCloudResult *result) {
    if (!SQCloudRowsetSanityCheck(result, 0, 0)) return 0;
    return result->maxlen;
}

char *SQCloudRowsetValue (SQCloudResult *result, uint32_t row, uint32_t col, uint32_t *len) {
    if (!SQCloudRowsetSanityCheck(result, row, col)) return NULL;
    
    // The *len var must contain the remaining length of the buffer pointed by
    // result->data[row*result->ncols+col]. The caller should not be aware of the
    // internal implementation of this buffer, so it must be set here.
    char *value = result->data[row*result->ncols+col];
    *len = internal_buffer_maxlen(result, value);
    return internal_parse_value(value, len, NULL);
}

uint32_t SQCloudRowsetValueLen (SQCloudResult *result, uint32_t row, uint32_t col) {
    uint32_t len = 0;
    SQCloudRowsetValue(result, row, col, &len);
    return len;
}

int32_t SQCloudRowsetInt32Value (SQCloudResult *result, uint32_t row, uint32_t col) {
    if (!SQCloudRowsetSanityCheck(result, row, col)) return 0;
    char *data = result->data[row*result->ncols+col];
    uint32_t len = internal_buffer_maxlen(result, data);
    char *value = internal_parse_value(data, &len, NULL);
    if (!value || len == 0) return 0;
    
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%.*s", len, value);
    return (int32_t)strtol(buffer, NULL, 0);
}

int64_t SQCloudRowsetInt64Value (SQCloudResult *result, uint32_t row, uint32_t col) {
    if (!SQCloudRowsetSanityCheck(result, row, col)) return 0;
    char *data = result->data[row*result->ncols+col];
    uint32_t len = internal_buffer_maxlen(result, data);
    char *value = internal_parse_value(data, &len, NULL);
    if (!value || len == 0) return 0;
    
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%.*s", len, value);
    return (int64_t)strtoll(buffer, NULL, 0);
}

float SQCloudRowsetFloatValue (SQCloudResult *result, uint32_t row, uint32_t col) {
    if (!SQCloudRowsetSanityCheck(result, row, col)) return 0.0;
    char *data = result->data[row*result->ncols+col];
    uint32_t len = internal_buffer_maxlen(result, data);
    char *value = internal_parse_value(data, &len, NULL);
    if (!value || len == 0) return 0.0;
    
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%.*s", len, value);
    return (float)strtof(buffer, NULL);
}

double SQCloudRowsetDoubleValue (SQCloudResult *result, uint32_t row, uint32_t col) {
    if (!SQCloudRowsetSanityCheck(result, row, col)) return 0.0;
    char *data = result->data[row*result->ncols+col];
    uint32_t len = internal_buffer_maxlen(result, data);
    char *value = internal_parse_value(data, &len, NULL);
    if (!value || len == 0) return 0.0;
    
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%.*s", len, value);
    return (double)strtod(buffer, NULL);
}

void SQCloudRowsetDump (SQCloudResult *result, uint32_t maxline, bool quiet) {
    internal_rowset_dump(result, maxline, quiet);
}

bool SQCloudRowsetCompare (SQCloudResult *result1, SQCloudResult *result2) {
    return internal_rowset_compare(result1, result2);
}

// MARK: - ARRAY -

static bool SQCloudArraySanityCheck (SQCloudResult *result, uint32_t index) {
    if (!result || result->tag != RESULT_ARRAY) return false;
    if (index >= result->ndata) return false;
    return true;
}

SQCLOUD_VALUE_TYPE SQCloudArrayValueType (SQCloudResult *result, uint32_t index) {
    if (!SQCloudArraySanityCheck(result, index)) return VALUE_NULL;
    return internal_type(result->data[index]);
}

uint32_t SQCloudArrayCount (SQCloudResult *result) {
    if (result->tag != RESULT_ARRAY) return 0;
    return result->ndata;
}

char *SQCloudArrayValue (SQCloudResult *result, uint32_t index, uint32_t *len) {
    if (!SQCloudArraySanityCheck(result, index)) return NULL;
    
    // The *len var must contain the remaining length of the buffer pointed by
    // result->data[index]. The caller should not be aware of the
    // internal implementation of this buffer, so it must be set here.
    char *value = result->data[index];
    *len = internal_buffer_maxlen(result, value);
    return internal_parse_value(value, len, NULL);
}

int32_t SQCloudArrayInt32Value (SQCloudResult *result, uint32_t index) {
    if (!SQCloudArraySanityCheck(result, index)) return 0;
    char *data = result->data[index];
    uint32_t len = internal_buffer_maxlen(result, data);
    char *value = internal_parse_value(data, &len, NULL);
    
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%.*s", len, value);
    return (int32_t)strtol(buffer, NULL, 0);
}

int64_t SQCloudArrayInt64Value (SQCloudResult *result, uint32_t index) {
    if (!SQCloudArraySanityCheck(result, index)) return 0;
    char *data = result->data[index];
    uint32_t len = internal_buffer_maxlen(result, data);
    char *value = internal_parse_value(data, &len, NULL);
    
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%.*s", len, value);
    return (int64_t)strtoll(buffer, NULL, 0);
}

float SQCloudArrayFloatValue (SQCloudResult *result, uint32_t index) {
    if (!SQCloudArraySanityCheck(result, index)) return 0.0;
    char *data = result->data[index];
    uint32_t len = internal_buffer_maxlen(result, data);
    char *value = internal_parse_value(data, &len, NULL);
    
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%.*s", len, value);
    return (float)strtof(buffer, NULL);
}

double SQCloudArrayDoubleValue (SQCloudResult *result, uint32_t index) {
    if (!SQCloudArraySanityCheck(result, index)) return 0.0;
    char *data = result->data[index];
    uint32_t len = internal_buffer_maxlen(result, data);
    char *value = internal_parse_value(data, &len, NULL);
    
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%.*s", len, value);
    return (double)strtod(buffer, NULL);
}

void SQCloudArrayDump (SQCloudResult *result) {
    if (result->tag != RESULT_ARRAY) return;
    
    for (uint32_t i=0; i<result->ndata; ++i) {
        uint32_t len;
        char *value = SQCloudArrayValue(result, i, &len);
        SQCLOUD_VALUE_TYPE type = SQCloudArrayValueType(result, i);
        if (!value) {value = "NULL"; len = 4;}
        if (type == VALUE_BLOB) printf("[%d] BLOB size %d\n", i, len);
        else printf("[%d] %.*s\n", i, len, value);
    }
}

// MARK: - UPLOAD/DOWNLOAD -

bool SQCloudDownloadDatabase (SQCloudConnection *connection, const char *dbname, void *xdata,
                              int (*xCallback)(void *xdata, const void *buffer, uint32_t blen, int64_t ntot, int64_t nprogress)) {
    return _reserved13(connection, dbname, xdata, xCallback, NULL, false);
}

bool SQCloudUploadDatabase (SQCloudConnection *connection, const char *dbname, const char *key, void *xdata, int64_t dbsize, int (*xCallback)(void *xdata, void *buffer, uint32_t *blen, int64_t ntot, int64_t nprogress)) {
    return internal_upload_database(connection, dbname, key, false, 0, false, xdata, dbsize, xCallback);
}

// MARK: - VM -

int32_t SQCloudRowsetResultDecode (SQCloudVM *vm, SQCloudResult *result) {
    // bind parameter count
    vm->nparams = result->n1;
    
    // vm is readonly
    vm->isreadonly = result->n2;
    
    // column count
    vm->ncolumns = result->n3;
    
    // vm is explain
    vm->isexplain = result->n4;
    
    // tail len
    return result->n5;
}

int32_t SQCloudArrayResultDecode (SQCloudVM *vm, SQCloudResult *result) {
    int type = SQCloudArrayInt32Value(result, 0);
    vm->type = type;

    if (type == ARRAY_TYPE_VM_COMPILE) {
        vm->index = (int)SQCloudArrayInt32Value(result, 1);
        vm->nparams = (int)SQCloudArrayInt32Value(result, 2);
        vm->isreadonly = (int)SQCloudArrayInt32Value(result, 3);
        vm->ncolumns = (int)SQCloudArrayInt32Value(result, 4);
        vm->isexplain = (int)SQCloudArrayInt32Value(result, 5);
        vm->finalized = (int)SQCloudArrayInt32Value(result, 7);

        // number of characters to skip to seek to the next statement
        int32_t nskip = SQCloudArrayInt32Value(result, 6);
        return nskip;
    }

    if ((type == ARRAY_TYPE_SQLITE_EXEC) || (type == ARRAY_TYPE_VM_STEP)) {
        vm->index = (int)SQCloudArrayInt32Value(result, 1);
        vm->lastrowid = SQCloudArrayInt64Value(result, 2);
        vm->changes = SQCloudArrayInt64Value(result, 3);
        vm->totalchanges = SQCloudArrayInt64Value(result, 4);
        vm->finalized = (int)SQCloudArrayInt32Value(result, 5);
        
        return 0;
    }
    
    /*
     remaning cases:
        ARRAY_TYPE_DB_STATUS
         
        ARRAY_TYPE_VM_STEP_ONE
        ARRAY_TYPE_VM_SQL
        ARRAY_TYPE_VM_STATUS
                  
        ARRAY_TYPE_BACKUP_INIT
        ARRAY_TYPE_BACKUP_STEP
        ARRAY_TYPE_BACKUP_END
     
     */
    
    return -1;
}

void SQCloudVMSetError (SQCloudVM *vm) {
    if (vm->errmsg) mem_free(vm->errmsg);
    if (vm->result) SQCloudResultFree(vm->result);
    vm->result = NULL;
    
    const char *errmsg = SQCloudErrorMsg(vm->connection);
    vm->errmsg = (errmsg) ? mem_string_dup(errmsg) : NULL;
    vm->errcode = SQCloudErrorCode(vm->connection);
    vm->xerrcode = SQCloudExtendedErrorCode(vm->connection);
}

void SQCloudVMSetResult (SQCloudVM *vm, SQCloudResult *result) {
    if (vm->result) SQCloudResultFree(vm->result);
    vm->result = result;
}

SQCloudResult *SQCloudVMResult (SQCloudVM *vm) {
    return vm->result;
}

SQCloudVM *SQCloudVMCompile (SQCloudConnection *connection, const char *sql, int32_t len, const char **tail) {
    if (len == -1) len = (int32_t)strlen(sql);
    
    const char *r[1] = {sql};
    uint32_t rlen[1] = {len};
    SQCLOUD_VALUE_TYPE types[1] = {VALUE_TEXT};
    
    SQCloudResult *result = SQCloudExecArray(connection, "VM COMPILE ?", r, rlen, types, 1);
    if (!result) return NULL;
    
    // result can be array or rowset
    SQCLOUD_RESULT_TYPE type = SQCloudResultType(result);
    
    if (type == RESULT_NULL) {
        SQCloudErrorReset(connection);
        return NULL;
    }
    
    // make sure result is an array
    if ((type != RESULT_ARRAY) && (type != RESULT_ROWSET)) {
        internal_set_error(connection, INTERNAL_ERRCODE_FORMAT, "Wrong result type received.");
        SQCloudResultFree(result);
        return NULL;
    }
    
    if ((type == RESULT_ARRAY) && (SQCloudArrayInt32Value(result, 0) != ARRAY_TYPE_VM_COMPILE)) {
        internal_set_error(connection, INTERNAL_ERRCODE_FORMAT, "Wrong array type received.");
        SQCloudResultFree(result);
        return NULL;
    }
    
    SQCloudVM *vm = (SQCloudVM *)mem_zeroalloc(sizeof(_SQCloudVM));
    if (!vm) {
        internal_set_error(connection, INTERNAL_ERRCODE_MEMORY, "Unable to allocate space for VM.");
        SQCloudResultFree(result);
        return NULL;
    }
    
    // decode array/rowset value
    int32_t nskip = (type == RESULT_ARRAY) ? SQCloudArrayResultDecode(vm, result) : SQCloudRowsetResultDecode(vm, result);
    
    // check for error
    if (nskip == -1) {
        mem_free(vm);
        internal_set_error(connection, INTERNAL_ERRCODE_MEMORY, "Unable to properly decode value from result.");
        SQCloudResultFree(result);
        return NULL;
    }
    
    // compute tail
    if (tail) {
        if (nskip == 0) nskip = (int32_t)strlen(sql);
        *tail = sql + nskip;
    }
    
    // setup resulting VM
    vm->connection = connection;
    vm->result = result;
    
    return vm;
}

bool SQCloudVMClose (SQCloudVM *vm) {
    bool rc = true;
    
    if (!vm->finalized) {
        char sql[512];
        snprintf(sql, sizeof(sql), "VM FINALIZE %d;", vm->index);
        
        SQCloudResult *result = SQCloudExec(vm->connection, sql);
        if (SQCloudResultType(result) == RESULT_ERROR) rc = false;
        SQCloudResultFree(result);
    }
    
    if (vm->result) SQCloudResultFree(vm->result);
    if (vm->errmsg) mem_free(vm->errmsg);
    mem_free(vm);
    
    return rc;
}

SQCLOUD_RESULT_TYPE SQCloudVMStep (SQCloudVM *vm) {
    // stepping into a VM that already contains a ROWSET means increasing its internal rowindex
    if (vm->result && SQCloudResultType(vm->result) == RESULT_ROWSET) {
        if (vm->rowindex + 1 == SQCloudRowsetRows(vm->result)) return RESULT_NULL;
        ++vm->rowindex;
        return RESULT_ROWSET;
    }
    
    char sql[512];
    snprintf(sql, sizeof(sql), "VM STEP %d;", vm->index);
    
    SQCloudResult *result = SQCloudExec(vm->connection, sql);
    SQCLOUD_RESULT_TYPE type = SQCloudResultType(result);
    
    if (type == RESULT_ROWSET) {
        SQCloudVMSetResult(vm, result);
        vm->finalized = true;
        vm->rowindex = 0;
        return RESULT_ROWSET;
    }
    
    if (type == RESULT_ARRAY) {
        SQCloudVMSetResult(vm, NULL);
        SQCloudArrayResultDecode(vm, result);
        SQCloudResultFree(result);
        return RESULT_OK;
    }
    
    if (type == RESULT_ERROR) {
        SQCloudVMSetError(vm);
        return RESULT_ERROR;
    }

    // should never reach this point
    return RESULT_ERROR;
}

int64_t SQCloudVMLastRowID (SQCloudVM *vm) {
    return vm->lastrowid;
}

int64_t SQCloudVMChanges (SQCloudVM *vm) {
    return vm->changes;
}

int64_t SQCloudVMTotalChanges (SQCloudVM *vm) {
    return vm->totalchanges;
}

const char *SQCloudVMErrorMsg (SQCloudVM *vm) {
    return vm->errmsg;
}

int SQCloudVMErrorCode (SQCloudVM *vm) {
    return vm->errcode;
}

int SQCloudVMIndex (SQCloudVM *vm) {
    return vm->index;
}

bool SQCloudVMIsFinalized (SQCloudVM *vm) {
    return vm->finalized;
}

bool SQCloudVMIsReadOnly (SQCloudVM *vm) {
    return vm->isreadonly;
}

int SQCloudVMIsExplain (SQCloudVM *vm) {
    return vm->isexplain;
}

int SQCloudVMBindParameterCount (SQCloudVM *vm) {
    return vm->nparams;
}

int SQCloudVMBindParameterIndex (SQCloudVM *vm, const char *name) {
    // VM PARAMETER <vmindex> INDEX <name>
    
    char sql[512];
    snprintf(sql, sizeof(sql), "VM PARAMETER %d INDEX ?;", vm->index);
        
    const char *r[1] = {name};
    uint32_t rlen[1] = {(uint32_t)strlen(name)};
    SQCLOUD_VALUE_TYPE types[1] = {VALUE_TEXT};
    
    SQCloudResult *result = SQCloudExecArray(vm->connection, sql, r, rlen, types, 1);
    if (SQCloudResultType(result) == RESULT_ERROR) {
        SQCloudVMSetError(vm);
        SQCloudResultFree(result);
        return 0;
    }
    
    int index = SQCloudResultInt32(result);
    SQCloudResultFree(result);
    return index;
}

const char *SQCloudVMBindParameterName (SQCloudVM *vm, int index) {
    // VM PARAMETER <vmindex> NAME <index>
    
    char sql[512];
    snprintf(sql, sizeof(sql), "VM PARAMETER %d NAME %d;", vm->index, index);
    
    SQCloudResult *result = SQCloudExec(vm->connection, sql);
    if (SQCloudResultType(result) == RESULT_ERROR) {
        SQCloudVMSetError(vm);
        SQCloudResultFree(result);
        return NULL;
    }
    
    const char *name = NULL;
    if (SQCloudResultBuffer(result) != NULL) {
        name = mem_string_dup(SQCloudResultBuffer(result));
    }
    
    SQCloudResultFree(result);
    return name;
}


int SQCloudVMColumnCount (SQCloudVM *vm) {
    return vm->ncolumns;
}

bool SQCloudVMBindDouble (SQCloudVM *vm, int index, double value) {
    // VM BIND <vmindex> TYPE <type> COLUMN <column> VALUE <value>
    char sql[512];
    snprintf(sql, sizeof(sql), "VM BIND %d TYPE DOUBLE COLUMN %d VALUE %f;", vm->index, index, value);
    
    SQCloudResult *result = SQCloudExec(vm->connection, sql);
    if (SQCloudResultType(result) == RESULT_ERROR) {
        SQCloudVMSetError(vm);
        return false;
    }
    
    SQCloudResultFree(result);
    return true;
}

bool SQCloudVMBindInt (SQCloudVM *vm, int index, int value) {
    // VM BIND <vmindex> TYPE <type> COLUMN <column> VALUE <value>
    char sql[512];
    snprintf(sql, sizeof(sql), "VM BIND %d TYPE INT COLUMN %d VALUE %d;", vm->index, index, value);
    
    SQCloudResult *result = SQCloudExec(vm->connection, sql);
    if (SQCloudResultType(result) == RESULT_ERROR) {
        SQCloudVMSetError(vm);
        return false;
    }
    
    SQCloudResultFree(result);
    return true;
}

bool SQCloudVMBindInt64 (SQCloudVM *vm, int index, int64_t value) {
    // VM BIND <vmindex> TYPE <type> COLUMN <column> VALUE <value>
    char sql[512];
    snprintf(sql, sizeof(sql), "VM BIND %d TYPE INT64 COLUMN %d VALUE %lld;", vm->index, index, value);
    
    SQCloudResult *result = SQCloudExec(vm->connection, sql);
    if (SQCloudResultType(result) == RESULT_ERROR) {
        SQCloudVMSetError(vm);
        return false;
    }
    
    SQCloudResultFree(result);
    return true;
}

bool SQCloudVMBindNull (SQCloudVM *vm, int index) {
    // VM BIND <vmindex> TYPE <type> COLUMN <column> VALUE <value>
    char sql[512];
    snprintf(sql, sizeof(sql), "VM BIND %d TYPE NULL COLUMN %d VALUE NULL;", vm->index, index);
    
    SQCloudResult *result = SQCloudExec(vm->connection, sql);
    if (SQCloudResultType(result) == RESULT_ERROR) {
        SQCloudVMSetError(vm);
        return false;
    }
    
    SQCloudResultFree(result);
    return true;
}

bool SQCloudVMBindText (SQCloudVM *vm, int index, const char *value, int32_t len) {
    // VM BIND <vmindex> TYPE <type> COLUMN <column> VALUE <value>
    char sql[512];
    snprintf(sql, sizeof(sql), "VM BIND %d TYPE TEXT COLUMN %d VALUE ?;", vm->index, index);
    
    if (len == -1) len = (int32_t)strlen(value);
    
    const char *r[1] = {value};
    uint32_t rlen[1] = {len};
    SQCLOUD_VALUE_TYPE types[1] = {VALUE_TEXT};
    
    SQCloudResult *result = SQCloudExecArray(vm->connection, sql, r, rlen, types, 1);
    if (SQCloudResultType(result) == RESULT_ERROR) {
        SQCloudVMSetError(vm);
        return false;
    }
    
    SQCloudResultFree(result);
    return true;
}

bool SQCloudVMBindBlob (SQCloudVM *vm, int index, void *value, int32_t len) {
    // VM BIND <vmindex> TYPE <type> COLUMN <column> VALUE <value>
    char sql[512];
    snprintf(sql, sizeof(sql), "VM BIND %d TYPE BLOB COLUMN %d VALUE ?;", vm->index, index);
    
    const char *r[1] = {value};
    uint32_t rlen[1] = {len};
    SQCLOUD_VALUE_TYPE types[1] = {VALUE_BLOB};
    
    SQCloudResult *result = SQCloudExecArray(vm->connection, sql, r, rlen, types, 1);
    if (SQCloudResultType(result) == RESULT_ERROR) {
        SQCloudVMSetError(vm);
        return false;
    }
    
    SQCloudResultFree(result);
    return true;
}

bool SQCloudVMBindZeroBlob (SQCloudVM *vm, int index, int64_t len) {
    // VM BIND <vmindex> TYPE <type> COLUMN <column> VALUE <value>
    char sql[512];
    snprintf(sql, sizeof(sql), "VM BIND %d TYPE ZEROBLOB COLUMN %d VALUE %lld;", vm->index, index, len);
    
    SQCloudResult *result = SQCloudExec(vm->connection, sql);
    if (SQCloudResultType(result) == RESULT_ERROR) {
        SQCloudVMSetError(vm);
        return false;
    }
    
    SQCloudResultFree(result);
    return true;
}

const void *SQCloudVMColumnBlob (SQCloudVM *vm, int index, uint32_t *len) {
    return (const void *)SQCloudRowsetValue(vm->result, vm->rowindex, index, len);
}

const char *SQCloudVMColumnText (SQCloudVM *vm, int index, uint32_t *len) {
    return (const char *)SQCloudRowsetValue(vm->result, vm->rowindex, index, len);
}

double SQCloudVMColumnDouble (SQCloudVM *vm, int index) {
    return SQCloudRowsetDoubleValue(vm->result, vm->rowindex, index);
}

int SQCloudVMColumnInt32 (SQCloudVM *vm, int index) {
    return (int)SQCloudRowsetInt32Value(vm->result, vm->rowindex, index);
}

int64_t SQCloudVMColumnInt64 (SQCloudVM *vm, int index) {
    return (int64_t)SQCloudRowsetInt64Value(vm->result, vm->rowindex, index);
}

int64_t SQCloudVMColumnLen (SQCloudVM *vm, int index) {
    return (int64_t)SQCloudRowsetValueLen(vm->result, vm->rowindex, index);
}

SQCLOUD_VALUE_TYPE SQCloudVMColumnType (SQCloudVM *vm, int index) {
    return SQCloudRowsetValueType(vm->result, vm->rowindex, index);
}

// MARK: - BLOB -

SQCloudBlob *SQCloudBlobOpen (SQCloudConnection *connection, const char *dbname, const char *tablename, const char *colname, int64_t rowid, bool wrflag) {
    // BLOB OPEN <database_name> TABLE <table_name> COLUMN <column_name> ROWID <rowid> RWFLAG <rwflag>
    char sql[512];
    snprintf(sql, sizeof(sql), "BLOB OPEN ? TABLE ? COLUMN ? ROWID %lld RWFLAG %d;", rowid, wrflag);
    
    if (!dbname) dbname = "main";
    const char *r[3] = {dbname, tablename, colname};
    uint32_t rlen[3] = {(uint32_t)strlen(dbname), (uint32_t)strlen(tablename), (uint32_t)strlen(colname)};
    SQCLOUD_VALUE_TYPE types[3] = {VALUE_TEXT, VALUE_TEXT, VALUE_TEXT};
    
    SQCloudResult *result = SQCloudExecArray(connection, sql, r, rlen, types, 3);
    if (SQCloudResultIsError(result)) {
        SQCloudResultFree(result);
        return NULL;
    }
    
    int index = (SQCloudResultType(result) == RESULT_INTEGER) ? SQCloudResultInt32(result) : -1;
    SQCloudResultFree(result);
    if (index == -1) return NULL; // errcode/errmsg should be already stored in connection
    
    SQCloudBlob *blob = (SQCloudBlob *)mem_zeroalloc(sizeof(_SQCloudBlob));
    if (!blob) {
        internal_set_error(connection, INTERNAL_ERRCODE_MEMORY, "Unable to allocate space for BLOB.");
        return NULL;
    }
    
    blob->connection = connection;
    blob->index = index;
    blob->bytes = -1;
    
    return blob;
}

bool SQCloudBlobReOpen (SQCloudBlob *blob, int64_t rowid) {
    if (blob->rc != 0) return false;
    
    // BLOB REOPEN <index> ROWID <rowid>
    char sql[512];
    snprintf(sql, sizeof(sql), "BLOB REOPEN %d ROWID %lld;", blob->index, rowid);
    
    SQCloudResult *result = SQCloudExec(blob->connection, sql);
    if (SQCloudResultType(result) == RESULT_ERROR) {
        blob->rc = SQCloudErrorCode(blob->connection);
    }
    SQCloudResultFree(result);
    
    // make sure to reset bytes counter
    blob->bytes = -1;
    return (blob->rc == 0);
}

bool SQCloudBlobClose (SQCloudBlob *blob) {
    if (!blob) return true;
    
    // BLOB CLOSE <index>
    char sql[512];
    snprintf(sql, sizeof(sql), "BLOB CLOSE %d;", blob->index);
    
    SQCloudResult *result = SQCloudExec(blob->connection, sql);
    bool rc = (SQCloudResultIsOK(result));
    
    SQCloudResultFree(result);
    return rc;
}

int SQCloudBlobBytes (SQCloudBlob *blob) {
    if (blob->rc != 0) return 0;
    if (blob->bytes != -1) return (int)blob->bytes;
    
    // BLOB BYTES <index>
    char sql[512];
    snprintf(sql, sizeof(sql), "BLOB BYTES %d;", blob->index);
    
    int rc = -1;
    SQCloudResult *result = SQCloudExec(blob->connection, sql);
    if (SQCloudResultType(result) == RESULT_INTEGER) {
        rc = (int)SQCloudResultInt32(result);
        blob->bytes = rc;
    }
    
    SQCloudResultFree(result);
    return rc;
}

int SQCloudBlobRead (SQCloudBlob *blob, void *zbuffer, int n, int offset) {
    if (blob->rc != 0) return -1;
    
    // BLOB READ <index> SIZE <size> OFFSET <offset>
    char sql[512];
    snprintf(sql, sizeof(sql), "BLOB READ %d SIZE %d OFFSET %d;", blob->index, n, offset);
    
    int rc = -1;
    SQCloudResult *result = SQCloudExec(blob->connection, sql);
    if (SQCloudResultType(result) == RESULT_BLOB) {
        char *buffer = SQCloudResultBuffer(result);
        int len = SQCloudResultLen(result);
        // len should be <= n
        memcpy(zbuffer, buffer, (len <= n) ? len : n);
        rc = len;
    }
    
    SQCloudResultFree(result);
    return rc;
}

int SQCloudBlobWrite (SQCloudBlob *blob, const void *buffer, int blen, int offset) {
    if (blob->rc != 0) return -1;
    
    // BLOB WRITE <index> OFFSET <offset> DATA <data>
    char sql[512];
    snprintf(sql, sizeof(sql), "BLOB WRITE %d OFFSET %d DATA ?;", blob->index, offset);
    
    const char *r[1] = {buffer};
    uint32_t rlen[1] = {blen};
    SQCLOUD_VALUE_TYPE types[1] = {VALUE_BLOB};
    
    int rc = 0;
    SQCloudResult *result = SQCloudExecArray(blob->connection, sql, r, rlen, types, 1);
    if (SQCloudResultType(result) == RESULT_ERROR) {
        rc = -1;
    }
    
    SQCloudResultFree(result);
    return (rc == 0);
}

// MARK: - BACKUP -

SQCloudBackup *SQCloudBackupInit (SQCloudConnection *connection, const char *dest_name, const char *source_name) {
    // BACKUP INIT [<dest_name>] [SOURCE <source_name>]
    if (!dest_name) dest_name = "main";
    if (!source_name) source_name = "main";
    
    char sql[512];
    snprintf(sql, sizeof(sql), "BACKUP INIT ? SOURCE ?");
    
    const char *r[2] = {dest_name, source_name};
    uint32_t rlen[2] = {(uint32_t)strlen(dest_name), (uint32_t)strlen(source_name)};
    SQCLOUD_VALUE_TYPE types[2] = {VALUE_TEXT, VALUE_TEXT};
    
    SQCloudBackup *backup = NULL;
    SQCloudResult *result = SQCloudExecArray(connection, sql, r, rlen, types, 2);
    if ((SQCloudResultType(result) == RESULT_ARRAY) && (SQCloudArrayInt32Value(result, 0) == ARRAY_TYPE_BACKUP_INIT)) {
        backup = (SQCloudBackup *)mem_zeroalloc(sizeof(_SQCloudBackup));
        backup->connection = connection;
        backup->page_size = (int)SQCloudArrayInt32Value(result, 2);
        backup->page_total = (int)SQCloudArrayInt32Value(result, 3);
    }
    
    SQCloudResultFree(result);
    return backup;
}

int SQCloudBackupStep (SQCloudBackup *backup, int n, SQCloudBackupOnDataCB on_data) {
    if (n <= 0) n = 0;
    
    // BACKUP STEP <index> PAGES <npages>
    char sql[512];
    snprintf(sql, sizeof(sql), "BACKUP STEP %d PAGES %d;", backup->index, n);
    
    int rc = -1;
    SQCloudResult *result = SQCloudExec(backup->connection, sql);
    if ((SQCloudResultType(result) == RESULT_ARRAY) && (SQCloudArrayInt32Value(result, 0) == ARRAY_TYPE_BACKUP_STEP)) {
        rc = (int)SQCloudArrayInt32Value(result, 2);
        backup->page_total = (int)SQCloudArrayInt32Value(result, 3);
        backup->page_remaining = (int)SQCloudArrayInt32Value(result, 4);
        backup->counter = (int)SQCloudArrayInt32Value(result, 5);
        
        // retrieve BLOB
        uint32_t blen = 0;
        char *buffer = SQCloudArrayValue(result, 6, &blen);
        if (on_data) on_data(backup, buffer, blen, backup->page_size, backup->counter);
    }
    
    SQCloudResultFree(result);
    return rc;
}

bool SQCloudBackupFinish (SQCloudBackup *backup) {
    bool rc = true;
    if (backup->connection) {
        // BACKUP FINISH <index>
        char sql[512];
        snprintf(sql, sizeof(sql), "BACKUP FINISH %d;", backup->index);
        
        SQCloudResult *result = SQCloudExec(backup->connection, sql);
        rc = (SQCloudResultIsOK(result));
        
        SQCloudResultFree(result);
    }
    mem_free(backup);
    return rc;
}

int SQCloudBackupPageRemaining (SQCloudBackup *backup) {
    // BACKUP REMAINING <index>
    return backup->page_remaining;
}

int SQCloudBackupPageCount (SQCloudBackup *backup) {
    // BACKUP PAGECOUNT <index>
    return backup->page_total;
}

void *SQCloudBackupSetData (SQCloudBackup *backup, void *data) {
    void *rc = backup->data;
    backup->data = data;
    return rc;
}

void *SQCloudBackupData (SQCloudBackup *backup) {
    return backup->data;
}

SQCloudConnection *SQCloudBackupConnection (SQCloudBackup *backup) {
    return backup->connection;
}
