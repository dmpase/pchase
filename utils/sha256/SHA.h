#if !defined(SHA_H)
#define SHA_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>


static const uint32_t sha_bits_per_byte       = 8;
static const uint32_t sha256_bits_per_word    = 32;
static const uint32_t sha256_bits_per_block   = 512;
static const uint32_t sha256_bits_per_digest  = 256;
static const uint32_t sha256_bits_per_dword   = 64;
static const uint32_t sha256_msg_len_bits     = 64;

static const uint32_t sha256_bytes_per_word   = sha256_bits_per_word   / sha_bits_per_byte;
static const uint32_t sha256_bytes_per_block  = sha256_bits_per_block  / sha_bits_per_byte;
static const uint32_t sha256_words_per_block  = sha256_bits_per_block  / sha256_bits_per_word;
static const uint32_t sha256_bytes_per_digest = sha256_bits_per_digest / sha_bits_per_byte;
static const uint32_t sha256_words_per_digest = sha256_bits_per_digest / sha256_bits_per_word;

static const uint32_t sha256_bytes_per_dword  = sha256_bits_per_dword  / sha_bits_per_byte;
static const uint32_t sha256_dwords_per_block = sha256_bits_per_block  / sha256_bits_per_dword;

typedef struct {
    uint32_t W[64];
    uint32_t H[8];
    uint64_t bits;
} sha256_context_t;

typedef struct {
    uint8_t  bytes[sha256_bytes_per_digest];
    uint32_t words[sha256_words_per_digest];
} sha256_digest_t;

void sha256_compute(const char* mptr, uint64_t msg_len_bits, sha256_digest_t* digest);
void sha256_init(sha256_context_t* ctx);
void sha256_block(sha256_context_t* ctx, uint32_t M[16]);
void sha256_final(sha256_context_t* ctx, uint32_t M[16], uint64_t bits);
void sha256_digest(sha256_context_t* ctx, sha256_digest_t* digest);
void sha256_print_digest(sha256_digest_t* digest);
bool sha256_digest_equals(const sha256_digest_t* lhs, const sha256_digest_t* rhs);
void sha256_byte_to_word_copy(uint32_t* dst, const char* src, uint32_t size);
void sha256_word_to_byte_copy(uint8_t* dst, const uint32_t* src, uint32_t size);
void sha256_byte_to_dword_copy(uint64_t* dst, char* src, uint32_t size);

typedef struct profile_s {
    uint64_t start;
    uint64_t length;
} profile_t;
extern profile_t profile[128];

#endif
