#ifndef ZCK_PRIVATE_H
#define ZCK_PRIVATE_H
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <regex.h>

#define BUF_SIZE 32768
/* Maximum string length for a compressed size_t */
#define MAX_COMP_SIZE (((sizeof(size_t) * 8) / 7) + 1)

#define zmalloc(x) calloc(1, x)

struct zckComp;

typedef int (*finit)(struct zckComp *comp);
typedef int (*fparam)(struct zckComp *comp, int option, void *value);
typedef int (*fcompend)(struct zckComp *comp, char **dst, size_t *dst_size,
                        int use_dict);
typedef int (*fcomp)(struct zckComp *comp, const char *src,
                     const size_t src_size, char **dst, size_t *dst_size,
                     int use_dict);
typedef int (*fdecomp)(struct zckComp *comp, const char *src,
                       const size_t src_size, char **dst, size_t dst_size,
                       int use_dict);
typedef int (*fcclose)(struct zckComp *comp);

typedef enum log_type log_type;

typedef struct zckHashType {
    int type;
    int digest_size;
} zckHashType;

typedef struct zckHash {
    zckHashType *type;
    void *ctx;
} zckHash;

/*typedef struct zckIndexItem zckIndexItem;*/
typedef void CURL;

typedef struct zckMP {
    int state;
    size_t length;
    char *buffer;
    size_t buffer_len;
} zckMP;

typedef struct zckDLPriv {
    CURL *curl_ctx;
    zckMP *mp;
    int parser_started;
    int is_chunk;
    size_t write_in_chunk;
    size_t dl_chunk_data;
    regex_t *dl_regex;
    regex_t *end_regex;
    regex_t *hdr_regex;
    zckIndexItem *tgt_check;
    zckHash *chunk_hash;
} zckDLPriv;

typedef struct zckComp {
    int started;

    uint8_t type;
    int level;

    void *cctx;
    void *dctx;
    void *cdict_ctx;
    void *ddict_ctx;
    void *dict;
    size_t dict_size;

    char *data;
    size_t data_size;

    finit init;
    fparam set_parameter;
    fcomp compress;
    fcompend end_chunk;
    fdecomp decompress;
    fcclose close;
} zckComp;

typedef struct zckCtx {
    int temp_fd;
    int fd;

    char *full_hash_digest;
    char *header_string;
    size_t header_size;
    char *index_string;
    size_t index_size;
    zckIndex index;
    zckIndexItem *work_index_item;
    zckHash work_index_hash;
    char *index_digest;
    zckHash full_hash;
    zckHash check_full_hash;
    zckComp comp;
    zckHashType hash_type;
    zckHashType chunk_hash_type;
} zckCtx;

const char *zck_hash_name_from_type(int hash_type);
int zck_get_tmp_fd();
int zck_validate_file(zckCtx *zck);
void zck_clear_work_index(zckCtx *zck);

/* hash/hash.h */
int zck_hash_setup(zckHashType *ht, int h);
int zck_hash_init(zckHash *hash, zckHashType *hash_type);
int zck_hash_update(zckHash *hash, const char *message, const size_t size);
char *zck_hash_finalize(zckHash *hash);
void zck_hash_close(zckHash *hash);
const char *zck_hash_get_printable(const char *digest, zckHashType *type);

/* index/index.c */
int zck_index_read(zckCtx *zck, char *data, size_t size);
int zck_index_finalize(zckCtx *zck);
int zck_index_new_chunk(zckIndex *index, char *digest, int digest_size,
                        size_t comp_size, size_t orig_size, int finished);
int zck_index_add_to_chunk(zckCtx *zck, char *data, size_t comp_size,
                        size_t orig_size);
int zck_index_finish_chunk(zckCtx *zck);
void zck_index_clean(zckIndex *index);
void zck_index_free(zckCtx *zck);
void zck_index_free_item(zckIndexItem **item);
int zck_write_index(zckCtx *zck);

/* io.c */
int zck_seek(int fd, off_t offset, int whence);
size_t zck_tell(int fd);
int zck_read(int fd, char *data, size_t length);
int zck_write(int fd, const char *data, size_t length);
int zck_write_comp_size(int fd, size_t val);
int zck_read_comp_size(int fd, size_t *val, size_t *length);
int zck_chunks_from_temp(zckCtx *zck);

/* header.c */
int zck_read_initial(zckCtx *zck, int src_fd);
int zck_read_index_hash(zckCtx *zck, int src_fd);
int zck_read_ct_is(zckCtx *zck, int src_fd);
int zck_read_index(zckCtx *zck, int src_fd);
int zck_header_create(zckCtx *zck);
int zck_read_header(zckCtx *zck, int src_fd);
int zck_write_header(zckCtx *zck);

/* dl/range.c */
char *zck_range_get_char(zckRangeItem **range, int max_ranges);
int zck_range_add(zckRange *info, zckIndexItem *idx, zckCtx *zck);

/* dl/multipart.c */
size_t zck_multipart_extract(zckDL *dl, char *b, size_t l);
size_t zck_multipart_get_boundary(zckDL *dl, char *b, size_t size);

/* dl/dl.c */
int zck_dl_write_range(zckDL *dl, const char *at, size_t length);
int zck_dl_range_chk_chunk(zckDL *dl, char *url, int is_chunk);

/* compint.c */
int zck_compint_from_int(char *compint, int val, size_t *length);
int zck_compint_from_size(char *compint, size_t val, size_t *length);
int zck_compint_to_int(int *val, const char *compint, size_t *length);
int zck_compint_to_size(size_t *val, const char *compint, size_t *length);



/* log.c */
void zck_log(log_type lt, const char *format, ...);
#endif
