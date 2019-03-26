#include <stdio.h>

#include "SHA.h"

static inline uint64_t
rd_tsc()
{
    uint32_t eax=0, edx=0;

    __asm__ __volatile__(
	"rdtsc ;"
	"movl %%eax,%0;"
	"movl %%edx,%1;"
	""
	: "=r"(eax), "=r"(edx)
	:
	: "%eax", "%edx"
    );

    return ((uint64_t) edx << 32) | (uint64_t) eax;
}


static uint32_t sha256_K[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

static uint32_t sha224_init_H[8] = {
	0xc1059ed8,	0x367dd507,	0x3070dd17,	0xf70e5939,	0xffc00b31,	0x68581151,	0x64f98fa7,	0xbefa4fa4,
};

static uint32_t sha256_init_H[8] = {
	0x6a09e667,	0xbb67ae85,	0x3c6ef372,	0xa54ff53a,	0x510e527f,	0x9b05688c,	0x1f83d9ab,	0x5be0cd19,
};

void
sha256_compute(const char* msg, uint64_t msg_len_bits, sha256_digest_t* digest)
{
    uint32_t M[sha256_words_per_block];

    uint64_t whole_blocks_per_message = msg_len_bits / sha256_bits_per_block;

    sha256_context_t ctx;
    sha256_init(&ctx);
    for (int i=0; i < whole_blocks_per_message; i++) {
	uint64_t byte_offset = i * sha256_bytes_per_block;
	sha256_byte_to_word_copy(M, msg + byte_offset, sizeof M);
	sha256_block(&ctx, M);
    }

    bzero(M, sizeof M);
    uint64_t byte_offset = whole_blocks_per_message * sha256_bytes_per_block;
    sha256_byte_to_word_copy(M, msg + byte_offset, (msg_len_bits % sha256_bits_per_block + sha_bits_per_byte - 1) / sha_bits_per_byte);
    sha256_final(&ctx, M, msg_len_bits);
    sha256_digest(&ctx, digest);
}


void
sha256_init(sha256_context_t* ctx)
{
    if (ctx != NULL) {
	memcpy(ctx->H, sha256_init_H, sizeof sha256_init_H);
	ctx->bits = 0;
    }
}

#define sha256_Ch(x, y, z)	(((x)&(y)) ^ ((~(x))&(z)))
#define sha256_Maj(x, y, z)	(((x)&(y)) ^ ((x)&(z)) ^ ((y)&(z)))
#define sha256_ROTL(n, x)	(((x) << (n)) | ((x) >> (32-(n))))
#define sha256_ROTR(n, x)	(((x) >> (n)) | ((x) << ((32)-(n))))
#define sha256_SHR(n, x)	((x) >> (n))
#define sha256_SIGMA_0(x)	(sha256_ROTR( 2,x) ^ sha256_ROTR(13,x) ^ sha256_ROTR(22,x))
#define sha256_SIGMA_1(x)	(sha256_ROTR( 6,x) ^ sha256_ROTR(11,x) ^ sha256_ROTR(25,x))
#define sha256_sigma_0(x)	(sha256_ROTR( 7,x) ^ sha256_ROTR(18,x) ^ sha256_SHR ( 3,x))
#define sha256_sigma_1(x)	(sha256_ROTR(17,x) ^ sha256_ROTR(19,x) ^ sha256_SHR (10,x))

void
sha256_block(sha256_context_t *ctx, uint32_t M[sha256_words_per_block])
{
    ctx->bits += 512;

				// Step 1. Prepare W[i]
    memcpy(ctx->W, M, 16*sizeof M[0]);

    for (int t=16; t < 64; t++) {
	ctx->W[t] = sha256_sigma_1(ctx->W[t-2]) + ctx->W[t-7] + sha256_sigma_0(ctx->W[t-15]) + ctx->W[t-16];
    }

				// Step 2. Initialize the working values
    uint32_t a = ctx->H[0];
    uint32_t b = ctx->H[1];
    uint32_t c = ctx->H[2];
    uint32_t d = ctx->H[3];
    uint32_t e = ctx->H[4];
    uint32_t f = ctx->H[5];
    uint32_t g = ctx->H[6];
    uint32_t h = ctx->H[7];

				// Step 3.
//    printf("          A        B        C        D        E        F        G        H       T1       T2\n");
    for (int t=0; t < 64; t++) {
	uint32_t T1 = h + sha256_SIGMA_1(e) + sha256_Ch(e,f,g) + sha256_K[t] + ctx->W[t];
	uint32_t T2 = sha256_SIGMA_0(a) + sha256_Maj(a,b,c);

	h = g;
	g = f;
	f = e;
	e = d;
	d = c;
	c = b;
	b = a;
	a = T1 + T2;
	e += T1;

//	if (t != 0 && t%5 == 0) printf("      ----A--------B--------C--------D--------E--------F--------G--------H-------T1-------T2---\n");
//	printf("t=%2d: %8.8x %8.8x %8.8x %8.8x %8.8x %8.8x %8.8x %8.8x %8.8x %8.8x\n", t, a, b, c, d, e, f, g, h, T1, T2);
    }

				// Step 4. Compute the ith intermediate hash value H[i]
    ctx->H[0] = a + ctx->H[0];
    ctx->H[1] = b + ctx->H[1];
    ctx->H[2] = c + ctx->H[2];
    ctx->H[3] = d + ctx->H[3];
    ctx->H[4] = e + ctx->H[4];
    ctx->H[5] = f + ctx->H[5];
    ctx->H[6] = g + ctx->H[6];
    ctx->H[7] = h + ctx->H[7];
}

void
sha256_final(sha256_context_t *ctx, uint32_t M[sha256_words_per_block], uint64_t bits)
{
    uint64_t gbits;		// global bits, or total bits in the message
    uint64_t lbits;		// local bits, or bits within this block
    if (bits <= sha256_bits_per_block) {
				// "bits" reflects the number of bits used in this block only
	lbits = bits;
	gbits = ctx->bits + bits;
    } else if (ctx->bits < bits && bits <= ctx->bits + sha256_bits_per_block) {
				// "bits" reflects the total number of bits used in all blocks
	lbits = bits & (sha256_bits_per_block - 1);
	gbits = bits;
    } else {
				// "bits" is in error
    }

    if (lbits == 0) {
    				// no data, just padding
	M[0] = 0x80000000U;
	M[sha256_words_per_block-2] = (gbits >> 32);
	M[sha256_words_per_block-1] = (gbits >>  0);
	sha256_block(ctx, M);
    } else if (lbits < sha256_bits_per_block - sha256_msg_len_bits) {
    				// data and padding fit in a single block
	int one_bit_word_idx = lbits / sha256_bits_per_word;
	int one_bit_bit_idx  = sha256_bits_per_word - 1 - (lbits % sha256_bits_per_word);
	M[one_bit_word_idx] |= 1U << one_bit_bit_idx;
	M[sha256_words_per_block-2] = (gbits >> 32);
	M[sha256_words_per_block-1] = (gbits >>  0);
	sha256_block(ctx, M);
    } else if (lbits < sha256_bits_per_block) {
    				// data and padding require two blocks
				// final marker fits in 1st block, bit count fits in 2nd
	int one_bit_word_idx = lbits / sha256_bits_per_word;
	int one_bit_bit_idx  = sha256_bits_per_word - 1 - (lbits % sha256_bits_per_word);
	M[one_bit_word_idx] |= 1U << one_bit_bit_idx;
	sha256_block(ctx, M);

	uint32_t final[sha256_words_per_block];
	bzero(final, sizeof final);
	final[sha256_words_per_block-2] = (gbits >> 32);
	final[sha256_words_per_block-1] = (gbits >>  0);
	sha256_block(ctx, final);
    } else if (lbits == sha256_bits_per_block) {
    				// data and padding require two blocks, and the trailing 
				// 10...0<bit count> fits entirely in the second block
	sha256_block(ctx, M);

	uint32_t final[sha256_words_per_block];
	bzero(final, sizeof final);
	final[0] = 0x80000000U;
	final[sha256_words_per_block-2] = (gbits >> 32);
	final[sha256_words_per_block-1] = (gbits >>  0);
	sha256_block(ctx, final);
    } else {
				// "lbits" is in error
    }
}

void
sha256_digest(sha256_context_t *ctx, sha256_digest_t* digest)
{
    for (int i=0; i < sha256_words_per_digest; i++) {
	digest->words[i] = ctx->H[i];
    }
    sha256_word_to_byte_copy(digest->bytes, digest->words, sizeof digest->words);
}

void
sha256_print_digest(sha256_digest_t* digest)
{
#if !defined(UNDEFINED)
    for (int i=0; i < sha256_words_per_digest; i++) {
	printf("%8.8x", digest->words[i]);
    }
    printf("\n");
#else
    for (int i=0; i < sha256_bytes_per_digest; i++) {
	printf("%2.2x", digest->bytes[i]);
    }
    printf("\n");
#endif
}

bool 
sha256_digest_equals(const sha256_digest_t* lhs, const sha256_digest_t* rhs)
{
    for (int i=0; i < sha256_words_per_digest; i++) {
	if (lhs->words[i] != rhs->words[i]) {
	    return false;
	}
    }

    return true;
}

void
sha256_byte_to_word_copy(uint32_t* dst, const char* src, uint32_t size)
{
#if defined(__x86_64__) || defined(__i386__)
				// little endian copy
    uint32_t words_to_copy = size / sha256_bytes_per_word;
    for (int i=0; i < words_to_copy; i++) {
	int src_idx = i * sha256_bytes_per_word;
	dst[i] = src[src_idx+0] << 24 | src[src_idx+1] << 16 |
		 src[src_idx+2] <<  8 | src[src_idx+3];
    }
    uint32_t residual = size % sha256_bytes_per_word;
    switch (residual) {
    case 1:
	dst[words_to_copy] = src[words_to_copy*sha256_bytes_per_word+0]<<24;
	break;
    case 2:
	dst[words_to_copy] = src[words_to_copy*sha256_bytes_per_word+0]<<24 |
			     src[words_to_copy*sha256_bytes_per_word+1]<<16;
	break;
    case 3:
	dst[words_to_copy] = src[words_to_copy*sha256_bytes_per_word+0]<<24 |
			     src[words_to_copy*sha256_bytes_per_word+1]<<16 |
			     src[words_to_copy*sha256_bytes_per_word+2]<< 8;
	break;
    }
#else
				// big endian copy
#error It should be simple to do, but we need a big-endian copy here.
#endif
} 

void
sha256_word_to_byte_copy(uint8_t* dst, const uint32_t* src, uint32_t size)
{
#if defined(__x86_64__) || defined(__i386__)
				// little endian copy
    uint32_t words_to_copy = size / sha256_bytes_per_word;
    for (int i=0; i < words_to_copy; i++) {
	int dst_idx = i * sha256_bytes_per_word;
	dst[dst_idx+0] = (src[i] >> 24) & 0xFF;
	dst[dst_idx+1] = (src[i] >> 16) & 0xFF;
	dst[dst_idx+2] = (src[i] >>  8) & 0xFF;
	dst[dst_idx+3] = (src[i] >>  0) & 0xFF;
    }
    uint32_t residual = size % sha256_bytes_per_word;
    int dst_idx = words_to_copy * sha256_bytes_per_word;
    switch (residual) {
    case 1:
	dst[dst_idx+0] = (src[words_to_copy]>>24)&0xFF;
	break;
    case 2:
	dst[dst_idx+0] = (src[words_to_copy]>>24)&0xFF;
	dst[dst_idx+1] = (src[words_to_copy]>>16)&0xFF;
	break;
    case 3:
	dst[dst_idx+0] = (src[words_to_copy]>>24)&0xFF;
	dst[dst_idx+1] = (src[words_to_copy]>>16)&0xFF;
	dst[dst_idx+2] = (src[words_to_copy]>> 8)&0xFF;
	break;
    }
#else
				// big endian copy
#error It should be simple to do, but we need a big-endian copy here.
#endif
} 


#if defined(UNDEFINED)
void
sha256_byte_to_dword_copy(uint64_t* dst, char* src, uint32_t size)
{
#if defined(__x86_64__) || defined(__i386__)
				// little endian copy
    uint32_t dwords_to_copy = size / sha256_bytes_per_dword;
    for (int i=0; i < dwords_to_copy; i++) {
	int src_idx = i * sha256_bytes_per_dword;
	dst[i] = src[src_idx+0] << 56 | 
		 src[src_idx+1] << 48 | 
		 src[src_idx+2] << 40 | 
		 src[src_idx+3] << 32 | 
		 src[src_idx+4] << 24 | 
		 src[src_idx+5] << 16 |
		 src[src_idx+6] <<  8 | 
		 src[src_idx+7];
    }
    uint32_t residual = size % sha256_bytes_per_dword;
    switch (residual) {
    case 1:
	dst[dwords_to_copy] = src[dwords_to_copy*sha256_bytes_per_dword+0]<<56;
	break;
    case 2:
	dst[dwords_to_copy] = src[dwords_to_copy*sha256_bytes_per_dword+0]<<56 |
			      src[dwords_to_copy*sha256_bytes_per_dword+1]<<48;
	break;
    case 3:
	dst[dwords_to_copy] = src[dwords_to_copy*sha256_bytes_per_dword+0]<<56 |
			      src[dwords_to_copy*sha256_bytes_per_dword+1]<<48 |
			      src[dwords_to_copy*sha256_bytes_per_dword+2]<<40;
	break;
    case 4:
	dst[dwords_to_copy] = src[dwords_to_copy*sha256_bytes_per_dword+0]<<56 |
			      src[dwords_to_copy*sha256_bytes_per_dword+1]<<48 |
			      src[dwords_to_copy*sha256_bytes_per_dword+1]<<40 |
			      src[dwords_to_copy*sha256_bytes_per_dword+2]<<32;
	break;
    case 5:
	dst[dwords_to_copy] = src[dwords_to_copy*sha256_bytes_per_dword+0]<<56 |
			      src[dwords_to_copy*sha256_bytes_per_dword+1]<<48 |
			      src[dwords_to_copy*sha256_bytes_per_dword+1]<<40 |
			      src[dwords_to_copy*sha256_bytes_per_dword+1]<<32 |
			      src[dwords_to_copy*sha256_bytes_per_dword+2]<<24;
	break;
    case 6:
	dst[dwords_to_copy] = src[dwords_to_copy*sha256_bytes_per_dword+0]<<56 |
			      src[dwords_to_copy*sha256_bytes_per_dword+1]<<48 |
			      src[dwords_to_copy*sha256_bytes_per_dword+1]<<40 |
			      src[dwords_to_copy*sha256_bytes_per_dword+1]<<32 |
			      src[dwords_to_copy*sha256_bytes_per_dword+1]<<24 |
			      src[dwords_to_copy*sha256_bytes_per_dword+2]<<16;
	break;
    case 7:
	dst[dwords_to_copy] = src[dwords_to_copy*sha256_bytes_per_dword+0]<<56 |
			      src[dwords_to_copy*sha256_bytes_per_dword+1]<<48 |
			      src[dwords_to_copy*sha256_bytes_per_dword+1]<<40 |
			      src[dwords_to_copy*sha256_bytes_per_dword+1]<<32 |
			      src[dwords_to_copy*sha256_bytes_per_dword+1]<<24 |
			      src[dwords_to_copy*sha256_bytes_per_dword+1]<<16 |
			      src[dwords_to_copy*sha256_bytes_per_dword+2]<< 8;
	break;
    }
#else
				// big endian copy
#error It should be simple to do, but we need a big-endian copy here.
#endif
} 
#endif

const char* SHA_C = "\0@ID " ID;
