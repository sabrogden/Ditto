//
//  sqcloud.h
//
//  Created by Marco Bambini on 08/02/21.
//

#ifndef __SQCLOUD_CLI__
#define __SQCLOUD_CLI__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SQCLOUD_SDK_VERSION         "0.9.9"
#define SQCLOUD_SDK_VERSION_NUM     0x000909
#define SQCLOUD_DEFAULT_PORT        8860
#define SQCLOUD_DEFAULT_TIMEOUT     12
#define SQCLOUD_DEFAULT_UPLOAD_SIZE 512*1024

#define SQCLOUD_IPv4                0
#define SQCLOUD_IPv6                1
#define SQCLOUD_IPANY               2

#ifndef BITCHECK
#define BITCHECK(byte,nbit)         ((byte) &   (1<<(nbit)))
#endif

// defined in https://github.com/sqlitecloud/sdk/blob/master/PROTOCOL.md
// was int const but apparently gcc doesn't like them in case statements
#define CMD_STRING                  '+'
#define CMD_ZEROSTRING              '!'
#define CMD_ERROR                   '-'
#define CMD_INT                     ':'
#define CMD_FLOAT                   ','
#define CMD_ROWSET                  '*'
#define CMD_ROWSET_CHUNK            '/'
#define CMD_JSON                    '#'
#define CMD_RAWJSON                 '{'
#define CMD_NULL                    '_'
#define CMD_BLOB                    '$'
#define CMD_COMPRESSED              '%'
#define CMD_PUBSUB                  '|'
#define CMD_COMMAND                 '^'
#define CMD_RECONNECT               '@'
#define CMD_ARRAY                   '='
#define CMD_ASYNC_STRING            '>'
#define CMD_ASYNC_ARRAY             '<'

typedef enum {
    ROWSET_TYPE_BASIC               = 1,
    ROWSET_TYPE_METADATA_v1         = 2,
    ROWSET_TYPE_HEADER_ONLY         = 3,
    ROWSET_TYPE_DATA_ONLY           = 4
} SQCLOUD_ROWSET_TYPE;

// MARK: -

// opaque datatypes
typedef struct SQCloudConnection            SQCloudConnection;
typedef struct SQCloudResult                SQCloudResult;
typedef struct SQCloudVM                    SQCloudVM;
typedef struct SQCloudBlob                  SQCloudBlob;
typedef struct SQCloudBackup                SQCloudBackup;
typedef void (*SQCloudPubSubCB)             (SQCloudConnection *connection, SQCloudResult *result, void *data);
typedef int (*config_cb)                    (char *buffer, int len, void *data);
typedef int64_t (*SQCloudBackupOnDataCB)    (SQCloudBackup *backup, const char *data, uint32_t len, int page_size, int page_counter);

// configuration struct to be passed to the connect function
typedef struct SQCloudConfigStruct {
    const char      *username;              // connection username
    const char      *password;              // connection password
    const char      *database;              // database to use during connection
    const char      *api_key;               // APIKEY
    int             timeout;                // connection timeout parameter
    int             family;                 // can be: SQCLOUD_IPv4, SQCLOUD_IPv6 or SQCLOUD_IPANY
    bool            compression;            // compression flag
    bool            zero_text;              // flag to tell the server to zero-terminate strings
    bool            password_hashed;        // private flag
    bool            non_linearizable;       // flag to request for immediate responses from the server node without waiting for linerizability guarantees
    bool            db_memory;              // flag to force the database to be in-memory
    bool            no_blob;                // flag to tell the server to not send BLOB columns
    bool            db_create;              // flag to force the creation of the database (if it does not exist)
    int             max_data;               // value to tell the server to not send columns with more than max_data bytes
    int             max_rows;               // value to control rowset chunks based on the number of rows
    int             max_rowset;             // value to control the maximum allowed size for a rowset
    #ifndef SQLITECLOUD_DISABLE_TLS
    const char      *tls_root_certificate;
    const char      *tls_certificate;
    const char      *tls_certificate_key;
    bool            insecure;               // flag to disable TLS
    bool            no_verify_certificate;  // flag to accept invalid TLS certificates
    #endif
    config_cb       callback;               // reserved callback for internal usage
    void            *data;                  // reserved callback data parameter
} SQCloudConfig;

// convenient struct to be used in SQCloudDownloadDatabase
typedef struct {
    void            *ptr;
    int             fd;
} SQCloudData;

typedef enum {
    RESULT_OK,
    RESULT_ERROR,
    RESULT_STRING,
    RESULT_INTEGER,
    RESULT_FLOAT,
    RESULT_ROWSET,
    RESULT_ARRAY,
    RESULT_NULL,
    RESULT_JSON,
    RESULT_BLOB
} SQCLOUD_RESULT_TYPE;

typedef enum {
    VALUE_INTEGER = 1,
    VALUE_FLOAT = 2,
    VALUE_TEXT = 3,
    VALUE_BLOB = 4,
    VALUE_NULL = 5
} SQCLOUD_VALUE_TYPE;

typedef enum {
    ARRAY_TYPE_SQLITE_EXEC = 10,            // used in SQLITE_MODE only when a write statement is executed (instead of the OK reply)
    ARRAY_TYPE_DB_STATUS = 11,
    ARRAY_TYPE_METADATA = 12,
    
    ARRAY_TYPE_VM_STEP = 20,                // used in VM_STEP (when SQLITE_DONE is returned)
    ARRAY_TYPE_VM_COMPILE = 21,             // used in VM_PREPARE
    ARRAY_TYPE_VM_STEP_ONE = 22,            // unused in this version (will be used to step in a server-side rowset)
    ARRAY_TYPE_VM_SQL = 23,
    ARRAY_TYPE_VM_STATUS = 24,
    ARRAY_TYPE_VM_LIST = 25,
        
    ARRAY_TYPE_BACKUP_INIT = 40,            // used in BACKUP_INIT
    ARRAY_TYPE_BACKUP_STEP = 41,            // used in backupWrite (VFS)
    ARRAY_TYPE_BACKUP_END = 42,             // used in backupClose (VFS)
    
    ARRAY_TYPE_SQLITE_STATUS = 50           // used in sqlite_status
} SQCLOUD_ARRAY_TYPE;

typedef enum {
    INTERNAL_ERRCODE_NONE = 0,
    INTERNAL_ERRCODE_GENERIC = 100000,
    INTERNAL_ERRCODE_PUBSUB = 100001,
    INTERNAL_ERRCODE_TLS = 100002,
    INTERNAL_ERRCODE_URL = 100003,
    INTERNAL_ERRCODE_MEMORY = 100004,
    INTERNAL_ERRCODE_NETWORK = 100005,
    INTERNAL_ERRCODE_FORMAT = 100006,
    INTERNAL_ERRCODE_INDEX = 100007,
    INTERNAL_ERRCODE_SOCKCLOSED = 100008,
} SQCLOUD_INTERNAL_ERRCODE;

// from SQLiteCloud
typedef enum {
    CLOUD_ERRCODE_MEM = 10000,
    CLOUD_ERRCODE_NOTFOUND = 10001,
    CLOUD_ERRCODE_COMMAND = 10002,
    CLOUD_ERRCODE_INTERNAL = 10003,
    CLOUD_ERRCODE_AUTH = 10004,
    CLOUD_ERRCODE_GENERIC = 10005,
    CLOUD_ERRCODE_RAFT = 10006
} SQCLOUD_CLOUD_ERRCODE;

// MARK: - General -
SQCloudConnection *SQCloudConnect (const char *hostname, int port, SQCloudConfig *config);
SQCloudConnection *SQCloudConnectWithString (const char *s, SQCloudConfig *config);
SQCloudResult *SQCloudExec (SQCloudConnection *connection, const char *command);
SQCloudConfig *SQCloudGetConfig (SQCloudConnection *connection);
const char *SQCloudUUID (SQCloudConnection *connection);
void SQCloudDisconnect (SQCloudConnection *connection);

// MARK: - Pub/Sub -
void SQCloudSetPubSubCallback (SQCloudConnection *connection, SQCloudPubSubCB callback, void *data);
SQCloudResult *SQCloudSetPubSubOnly (SQCloudConnection *connection);

// MARK: - Error -
bool SQCloudIsError (SQCloudConnection *connection);
bool SQCloudIsSQLiteError (SQCloudConnection *connection);
int SQCloudErrorCode (SQCloudConnection *connection);
int SQCloudExtendedErrorCode (SQCloudConnection *connection);
int SQCloudErrorOffset (SQCloudConnection *connection);
const char *SQCloudErrorMsg (SQCloudConnection *connection);
void SQCloudErrorReset (SQCloudConnection *connection);
void SQCloudErrorSetCode (SQCloudConnection *connection, int errcode);
void SQCloudErrorSetMsg (SQCloudConnection *connection, const char *format, ...);

// MARK: - Result -
SQCLOUD_RESULT_TYPE SQCloudResultType (SQCloudResult *result);
uint32_t SQCloudResultLen (SQCloudResult *result);
char *SQCloudResultBuffer (SQCloudResult *result);
int32_t SQCloudResultInt32 (SQCloudResult *result);
int64_t SQCloudResultInt64 (SQCloudResult *result);
double SQCloudResultDouble (SQCloudResult *result);
float SQCloudResultFloat (SQCloudResult *result);
void SQCloudResultFree (SQCloudResult *result);
bool SQCloudResultIsOK (SQCloudResult *result);
bool SQCloudResultIsError (SQCloudResult *result);
void SQCloudResultDump (SQCloudConnection *connection, SQCloudResult *result);

// MARK: - Rowset -
SQCLOUD_VALUE_TYPE SQCloudRowsetValueType (SQCloudResult *result, uint32_t row, uint32_t col);
uint32_t SQCloudRowsetRowsMaxColumnLength (SQCloudResult *result, uint32_t col);
char *SQCloudRowsetColumnName (SQCloudResult *result, uint32_t col, uint32_t *len);
char *SQCloudRowsetColumnDeclType (SQCloudResult *result, uint32_t col, uint32_t *len);
char *SQCloudRowsetColumnDBName (SQCloudResult *result, uint32_t col, uint32_t *len);
char *SQCloudRowsetColumnTblName (SQCloudResult *result, uint32_t col, uint32_t *len);
char *SQCloudRowsetColumnOrigName (SQCloudResult *result, uint32_t col, uint32_t *len);
uint32_t SQCloudRowSetColumnNotNULL (SQCloudResult *result, uint32_t col);
uint32_t SQCloudRowSetColumnPrimaryKey (SQCloudResult *result, uint32_t col);
uint32_t SQCloudRowSetColumnAutoIncrement (SQCloudResult *result, uint32_t col);
uint32_t SQCloudRowsetRows (SQCloudResult *result);
uint32_t SQCloudRowsetCols (SQCloudResult *result);
uint32_t SQCloudRowsetMaxLen (SQCloudResult *result);
char *SQCloudRowsetValue (SQCloudResult *result, uint32_t row, uint32_t col, uint32_t *len);
uint32_t SQCloudRowsetValueLen (SQCloudResult *result, uint32_t row, uint32_t col);
int32_t SQCloudRowsetInt32Value (SQCloudResult *result, uint32_t row, uint32_t col);
int64_t SQCloudRowsetInt64Value (SQCloudResult *result, uint32_t row, uint32_t col);
float SQCloudRowsetFloatValue (SQCloudResult *result, uint32_t row, uint32_t col);
double SQCloudRowsetDoubleValue (SQCloudResult *result, uint32_t row, uint32_t col);
void SQCloudRowsetDump (SQCloudResult *result, uint32_t maxline, bool quiet);
bool SQCloudRowsetCompare (SQCloudResult *result1, SQCloudResult *result2);
bool SQCloudRowsetCanWrite (SQCloudResult *result);

// MARK: - Array -
SQCloudResult *SQCloudExecArray (SQCloudConnection *connection, const char *command, const char **values, uint32_t len[], SQCLOUD_VALUE_TYPE types[], uint32_t n);
SQCLOUD_VALUE_TYPE SQCloudArrayValueType (SQCloudResult *result, uint32_t index);
uint32_t SQCloudArrayCount (SQCloudResult *result);
char *SQCloudArrayValue (SQCloudResult *result, uint32_t index, uint32_t *len);
int32_t SQCloudArrayInt32Value (SQCloudResult *result, uint32_t index);
int64_t SQCloudArrayInt64Value (SQCloudResult *result, uint32_t index);
float SQCloudArrayFloatValue (SQCloudResult *result, uint32_t index);
double SQCloudArrayDoubleValue (SQCloudResult *result, uint32_t index);
void SQCloudArrayDump (SQCloudResult *result);

// MARK: - Upload/Download -
bool SQCloudDownloadDatabase (SQCloudConnection *connection, const char *dbname, void *xdata,
                              int (*xCallback)(void *xdata, const void *buffer, uint32_t blen, int64_t ntot, int64_t nprogress));
bool SQCloudUploadDatabase (SQCloudConnection *connection, const char *dbname, const char *key, void *xdata, int64_t dbsize, int (*xCallback)(void *xdata, void *buffer, uint32_t *blen, int64_t ntot, int64_t nprogress));

// MARK: - VM -
SQCloudVM *SQCloudVMCompile (SQCloudConnection *connection, const char *sql, int32_t len, const char **tail);
SQCLOUD_RESULT_TYPE SQCloudVMStep (SQCloudVM *vm);
SQCloudResult *SQCloudVMResult (SQCloudVM *vm);
bool SQCloudVMClose (SQCloudVM *vm);
const char *SQCloudVMErrorMsg (SQCloudVM *vm);
int SQCloudVMErrorCode (SQCloudVM *vm);
int SQCloudVMIndex (SQCloudVM *vm);
bool SQCloudVMIsReadOnly (SQCloudVM *vm);
int SQCloudVMIsExplain (SQCloudVM *vm);
bool SQCloudVMIsFinalized (SQCloudVM *vm);
int SQCloudVMBindParameterCount (SQCloudVM *vm);
int SQCloudVMBindParameterIndex (SQCloudVM *vm, const char *name);
const char *SQCloudVMBindParameterName (SQCloudVM *vm, int index);
int SQCloudVMColumnCount (SQCloudVM *vm);
bool SQCloudVMBindDouble (SQCloudVM *vm, int index, double value);
bool SQCloudVMBindInt (SQCloudVM *vm, int index, int value);
bool SQCloudVMBindInt64 (SQCloudVM *vm, int index, int64_t value);
bool SQCloudVMBindNull (SQCloudVM *vm, int index);
bool SQCloudVMBindText (SQCloudVM *vm, int index, const char *value, int32_t len);
bool SQCloudVMBindBlob (SQCloudVM *vm, int index, void *value, int32_t len);
bool SQCloudVMBindZeroBlob (SQCloudVM *vm, int index, int64_t len);
const void *SQCloudVMColumnBlob (SQCloudVM *vm, int index, uint32_t *len);
const char *SQCloudVMColumnText (SQCloudVM *vm, int index, uint32_t *len);
double SQCloudVMColumnDouble (SQCloudVM *vm, int index);
int SQCloudVMColumnInt32 (SQCloudVM *vm, int index);
int64_t SQCloudVMColumnInt64 (SQCloudVM *vm, int index);
int64_t SQCloudVMColumnLen (SQCloudVM *vm, int index);
SQCLOUD_VALUE_TYPE SQCloudVMColumnType (SQCloudVM *vm, int index);
int64_t SQCloudVMLastRowID (SQCloudVM *vm);
int64_t SQCloudVMChanges (SQCloudVM *vm);
int64_t SQCloudVMTotalChanges (SQCloudVM *vm);

// MARK: - BLOB -
SQCloudBlob *SQCloudBlobOpen (SQCloudConnection *connection, const char *dbname, const char *tablename, const char *colname, int64_t rowid, bool wrflag);
bool SQCloudBlobReOpen (SQCloudBlob *blob, int64_t rowid);
bool SQCloudBlobClose (SQCloudBlob *blob);
int SQCloudBlobBytes (SQCloudBlob *blob);
int SQCloudBlobRead (SQCloudBlob *blob, void *buffer, int blen, int offset);
int SQCloudBlobWrite (SQCloudBlob *blob, const void *buffer, int blen, int offset);

// MARK: - Backup -
SQCloudBackup *SQCloudBackupInit (SQCloudConnection *connection, const char *dest_name, const char *source_name);
int SQCloudBackupStep (SQCloudBackup *backup, int n, SQCloudBackupOnDataCB on_data);
bool SQCloudBackupFinish (SQCloudBackup *backup);
int SQCloudBackupPageRemaining (SQCloudBackup *backup);
int SQCloudBackupPageCount (SQCloudBackup *backup);
void *SQCloudBackupSetData (SQCloudBackup *backup, void *data);
void *SQCloudBackupData (SQCloudBackup *backup);
SQCloudConnection *SQCloudBackupConnection (SQCloudBackup *backup);

// MARK: - Reserved -


#ifdef __cplusplus
}
#endif

#endif
