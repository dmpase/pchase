/*******************************************************************************
 * Copyright (c) 2011, Douglas M. Pase                                         *
 * All rights reserved.                                                        *
 * Redistribution and use in source and binary forms, with or without          *
 * modification, are permitted provided that the following conditions          *
 * are met:                                                                    *
 * o       Redistributions of source code must retain the above copyright      *
 *         notice, this list of conditions and the following disclaimer.       *
 * o       Redistributions in binary form must reproduce the above copyright   *
 *         notice, this list of conditions and the following disclaimer in     *
 *         the documentation and/or other materials provided with the          *
 *         distribution.                                                       *
 * o       Neither the name of the copyright holder nor the names of its       *
 *         contributors may be used to endorse or promote products derived     *
 *         from this software without specific prior written permission.       *
 *                                                                             *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" *
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   *
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  *
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE   *
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR         *
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF        *
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS    *
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN     *
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)     *
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF      *
 * THE POSSIBILITY OF SUCH DAMAGE.                                             *
 *******************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <assert.h>

#include "Memcpy.h"


static void* (*memcpy_used)(void* dest, const void* src, size_t size);

void set_memcpy( void* (*mu)(void* dest, const void* src, size_t size) )
{
    memcpy_used = mu;
}

void* mb_memcpy(void* dest, const void* src, size_t size)
{
    return memcpy_used(dest, src, size);
}


typedef uint64_t copy_t;


void*
copy_tt(void* dest, const void* src, size_t bytes)
{
    char* target = (char*) dest;
    const char* source = (char*) src;

				// words here are 8 bytes (64-bits)
				// blocks are 8 words (64 bytes)
    if ((((int64_t)source) & 0x7) == (((int64_t)target) & 0x7)) {
				// addresses are word aligned to each other
	int k, len;
	copy_t* s = NULL;
	copy_t* t = NULL;

				// make the copy word aligned
	len = ((int64_t)source) & 0x7;
	for (k=0; k < len; k++) {
	    *target++ = *source++;
	}
	bytes -= len;

				// copy the blocks
	copy_t in0, in1, in2, in3, in4, in5, in6, in7;
	s = (copy_t*) source;
	t = (copy_t*) target;
	len = bytes/sizeof(copy_t);
	for (k=0; k < len; k+=8) {
	    *(t+0) = *(s + 0);
	    *(t+1) = *(s + 1);
	    *(t+2) = *(s + 2);
	    *(t+3) = *(s + 3);
	    *(t+4) = *(s + 4);
	    *(t+5) = *(s + 5);
	    *(t+6) = *(s + 6);
	    *(t+7) = *(s + 7);
	    s += 8;
	    t += 8;
	}
	bytes -= len*64;
	source = (char*) (s + len);
	target = (char*) (t + len);

				// copy trailing bytes
	len = bytes;
	for (k=0; k < len; k++) {
	    *target++ = *source++;
	}
	bytes -= len;
    } else {
				// addresses are NOT word aligned to each other
	memcpy(target, source, bytes);
    }

    return target;
}


#if defined(NT_OUTONLY) || defined(NT_INOUT)
static inline void
sse_movntiq(copy_t *mem, copy_t reg)
{
#if defined(__i386__)
    printf("sse_movntiq must be compiled for x86_64\n");
    exit(-1);
#elif defined(__x86_64__)
				// *mem = reg
    asm("movntiq    %1, %0;"
        : "=m" (*mem)
        : "r" (reg));
#endif
}

void*
copy_tn(void* dest, const void* src, size_t bytes)
{
    char* target = (char*) dest;
    const char* source = (char*) src;

				// words here are 8 bytes (64-bits)
				// blocks are 8 words (64 bytes)
    if ((((int64_t)source) & 0x7) == (((int64_t)target) & 0x7)) {
				// addresses are word aligned to each other
	int k, len;
	copy_t* s = NULL;
	copy_t* t = NULL;

				// make the copy word aligned
	len = ((int64_t)source) & 0x7;
	for (k=0; k < len; k++) {
	    *target++ = *source++;
	}
	bytes -= len;

				// copy the blocks
	copy_t in0, in1, in2, in3, in4, in5, in6, in7;
	s = (copy_t*) source;
	t = (copy_t*) target;
	len = bytes/sizeof(copy_t);
	for (k=0; k < len; k+=8) {
	    in0 = *(s + 0); sse_movntiq(t+0, in0);
	    in1 = *(s + 1); sse_movntiq(t+1, in1);
	    in2 = *(s + 2); sse_movntiq(t+2, in2);
	    in3 = *(s + 3); sse_movntiq(t+3, in3);
	    in4 = *(s + 4); sse_movntiq(t+4, in4);
	    in5 = *(s + 5); sse_movntiq(t+5, in5);
	    in6 = *(s + 6); sse_movntiq(t+6, in6);
	    in7 = *(s + 7); sse_movntiq(t+7, in7);
	    s += 8;
	    t += 8;
	}
	bytes -= len*64;
	source = (char*) (s + len);
	target = (char*) (t + len);

				// copy trailing bytes
	len = bytes;
	for (k=0; k < len; k++) {
	    *target++ = *source++;
	}
	bytes -= len;
    } else {
				// addresses are NOT word aligned to each other
	memcpy(target, source, bytes);
    }

    return target;
}
#else
void*
copy_tn(void* dest, const void* src, size_t bytes)
{
    printf("Compile with \"-DNT_OUTONLY\" or \"-DNT_INOUT\" to use copy_tn.\n");
    exit(0);

    return NULL;
}
#endif


#if defined(NT_INOUT) || (defined(NT_INONLY) && defined(NT_OUTONLY))
void*
xtta(void* dest, const void* src, size_t bytes)
{
#if defined(__i386__)
    printf("xtta must be compiled for x86_64\n");
    exit(-1);
#elif defined(__x86_64__)

    char* target = (char*) dest;
    const char* source = (char*) src;
    const uint8_t unroll_factor=8;
    const uint8_t word_size=sizeof(v4si_t);

				// words here are 16 bytes (128-bits)
				// blocks are 4 words (64 bytes)
    if ((((uint64_t)source) % word_size) == (((uint64_t)target) % word_size)) {
				// addresses are word aligned to each other
	int k, len;

				// make the copy word aligned
	len = ((uint64_t)source) % word_size;   // unaligned leading bytes
	for (k=0; k < len; k++) {
	    *target++ = *source++;
	}
	bytes -= len;

				// copy the blocks
	v4si_t* s = (v4si_t*) source;
	v4si_t* t = (v4si_t*) target;
        size_t words = bytes/word_size;         // number of 16-byte words
	for (k=0; k < words-(unroll_factor-1); k+=unroll_factor) {
            asm("movdqa     0(%0), %%xmm0;\n"
                "movdqa    %%xmm0, 0(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movdqa     16(%0), %%xmm0;\n"
                "movdqa    %%xmm0, 16(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movdqa     32(%0), %%xmm0;\n"
                "movdqa    %%xmm0, 32(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movdqa     48(%0), %%xmm0;\n"
                "movdqa    %%xmm0, 48(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movdqa     64(%0), %%xmm0;\n"
                "movdqa    %%xmm0, 64(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movdqa     80(%0), %%xmm0;\n"
                "movdqa    %%xmm0, 80(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movdqa     96(%0), %%xmm0;\n"
                "movdqa    %%xmm0, 96(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movdqa     112(%0), %%xmm0;\n"
                "movdqa    %%xmm0, 112(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

	    s += unroll_factor;
	    t += unroll_factor;
	}

				// copy trailing 128-bit words
	for ( ; k < words; k+=1) {
            asm("movdqa     0(%0), %%xmm0;\n"
                "movdqa    %%xmm0, 0(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

	    s += 1;
	    t += 1;
	}

	size_t copied = words*word_size;
	bytes -= copied;
	source = (char*) s;
	target = (char*) t;

				// copy trailing bytes
	for (k=0; k < bytes; k++) {
	    *target++ = *source++;
	}
    } else {
				// addresses are NOT word aligned to each other
	memcpy(target, source, bytes);
    }
#endif

    return dest;
}
#else
void*
xtta(void* dest, const void* src, size_t bytes)
{
    printf("Compile with \"-DNT_INOUT\" to use xtta.\n");
    exit(0);

    return NULL;
}
#endif

#if defined(NT_INOUT) || (defined(NT_INONLY) && defined(NT_OUTONLY))
void*
xttu(void* dest, const void* src, size_t bytes)
{
#if defined(__i386__)
    printf("xttu must be compiled for x86_64\n");
    exit(-1);
#elif defined(__x86_64__)

    char* target = (char*) dest;
    const char* source = (char*) src;
    const uint8_t unroll_factor=8;
    const uint8_t word_size=sizeof(v4si_t);

				// words here are 16 bytes (128-bits)
				// blocks are 4 words (64 bytes)
    if ((((uint64_t)source) % word_size) == (((uint64_t)target) % word_size)) {
				// addresses are word aligned to each other
	int k, len;

				// make the copy word aligned
	len = ((uint64_t)source) % word_size;   // unaligned leading bytes
	for (k=0; k < len; k++) {
	    *target++ = *source++;
	}
	bytes -= len;

				// copy the blocks
	v4si_t* s = (v4si_t*) source;
	v4si_t* t = (v4si_t*) target;
        size_t words = bytes/word_size;         // number of 16-byte words
	for (k=0; k < words-(unroll_factor-1); k+=unroll_factor) {
            asm("movdqu     0(%0), %%xmm0;\n"
                "movdqu    %%xmm0, 0(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movdqu     16(%0), %%xmm0;\n"
                "movdqu    %%xmm0, 16(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movdqu     32(%0), %%xmm0;\n"
                "movdqu    %%xmm0, 32(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movdqu     48(%0), %%xmm0;\n"
                "movdqu    %%xmm0, 48(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movdqu     64(%0), %%xmm0;\n"
                "movdqu    %%xmm0, 64(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movdqu     80(%0), %%xmm0;\n"
                "movdqu    %%xmm0, 80(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movdqu     96(%0), %%xmm0;\n"
                "movdqu    %%xmm0, 96(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movdqu     112(%0), %%xmm0;\n"
                "movdqu    %%xmm0, 112(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

	    s += unroll_factor;
	    t += unroll_factor;
	}

				// copy trailing 128-bit words
	for ( ; k < words; k+=1) {
            asm("movdqu     0(%0), %%xmm0;\n"
                "movdqu    %%xmm0, 0(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

	    s += 1;
	    t += 1;
	}

	size_t copied = words*word_size;
	bytes -= copied;
	source = (char*) s;
	target = (char*) t;

				// copy trailing bytes
	for (k=0; k < bytes; k++) {
	    *target++ = *source++;
	}
    } else {
				// addresses are NOT word aligned to each other
	memcpy(target, source, bytes);
    }
#endif

    return dest;
}
#else
void*
xttu(void* dest, const void* src, size_t bytes)
{
    printf("Compile with \"-DNT_INOUT\" to use xttu.\n");
    exit(0);

    return NULL;
}
#endif

#if defined(NT_INOUT) || (defined(NT_INONLY) && defined(NT_OUTONLY))
void*
xtn(void* dest, const void* src, size_t bytes)
{
#if defined(__i386__)
    printf("xtn must be compiled for x86_64\n");
    exit(-1);
#elif defined(__x86_64__)

    char* target = (char*) dest;
    const char* source = (char*) src;
    const uint8_t unroll_factor=8;
    const uint8_t word_size=sizeof(v4si_t);

				// words here are 16 bytes (128-bits)
				// blocks are 4 words (64 bytes)
    if ((((uint64_t)source) % word_size) == (((uint64_t)target) % word_size)) {
				// addresses are word aligned to each other
	int k, len;

				// make the copy word aligned
	len = ((uint64_t)source) % word_size;   // unaligned leading bytes
	for (k=0; k < len; k++) {
	    *target++ = *source++;
	}
	bytes -= len;

				// copy the blocks
	v4si_t* s = (v4si_t*) source;
	v4si_t* t = (v4si_t*) target;
        size_t words = bytes/word_size;         // number of 16-byte words
	for (k=0; k < words-(unroll_factor-1); k+=unroll_factor) {
            asm("movdqa     0(%0), %%xmm0;\n"
                "movntdq   %%xmm0, 0(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movdqa     16(%0), %%xmm0;\n"
                "movntdq   %%xmm0, 16(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movdqa     32(%0), %%xmm0;\n"
                "movntdq   %%xmm0, 32(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movdqa     48(%0), %%xmm0;\n"
                "movntdq   %%xmm0, 48(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movdqa     64(%0), %%xmm0;\n"
                "movntdq   %%xmm0, 64(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movdqa     80(%0), %%xmm0;\n"
                "movntdq   %%xmm0, 80(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movdqa     96(%0), %%xmm0;\n"
                "movntdq   %%xmm0, 96(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movdqa     112(%0), %%xmm0;\n"
                "movntdq   %%xmm0, 112(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

	    s += unroll_factor;
	    t += unroll_factor;
	}

				// copy trailing 128-bit words
	for ( ; k < words; k+=1) {
            asm("movdqa     0(%0), %%xmm0;\n"
                "movntdq   %%xmm0, 0(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

	    s += 1;
	    t += 1;
	}

	size_t copied = words*word_size;
	bytes -= copied;
	source = (char*) s;
	target = (char*) t;

				// copy trailing bytes
	for (k=0; k < bytes; k++) {
	    *target++ = *source++;
	}
    } else {
				// addresses are NOT word aligned to each other
	memcpy(target, source, bytes);
    }
#endif

    return dest;
}
#else
void*
xtn(void* dest, const void* src, size_t bytes)
{
    printf("Compile with \"-DNT_INOUT\" to use xtn.\n");
    exit(0);

    return NULL;
}
#endif


#if defined(NT_INOUT) || (defined(NT_INONLY) && defined(NT_OUTONLY))
void*
xnt(void* dest, const void* src, size_t bytes)
{
#if defined(__i386__)
    printf("xnt must be compiled for x86_64\n");
    exit(-1);
#elif defined(__x86_64__)

    char* target = (char*) dest;
    const char* source = (char*) src;
    const uint8_t unroll_factor=8;
    const uint8_t word_size=sizeof(v4si_t);

				// words here are 16 bytes (128-bits)
				// blocks are 4 words (64 bytes)
    if ((((uint64_t)source) % word_size) == (((uint64_t)target) % word_size)) {
				// addresses are word aligned to each other
	int k, len;

				// make the copy word aligned
	len = ((uint64_t)source) % word_size;   // unaligned leading bytes
	for (k=0; k < len; k++) {
	    *target++ = *source++;
	}
	bytes -= len;

				// copy the blocks
	v4si_t* s = (v4si_t*) source;
	v4si_t* t = (v4si_t*) target;
        size_t words = bytes/word_size;         // number of 16-byte words
	for (k=0; k < words-(unroll_factor-1); k+=unroll_factor) {
            asm("movntdqa   0(%0), %%xmm0;\n"
                "movdqa    %%xmm0, 0(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movntdqa   16(%0), %%xmm0;\n"
                "movdqa    %%xmm0, 16(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movntdqa   32(%0), %%xmm0;\n"
                "movdqa    %%xmm0, 32(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movntdqa   48(%0), %%xmm0;\n"
                "movdqa    %%xmm0, 48(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movntdqa   64(%0), %%xmm0;\n"
                "movdqa    %%xmm0, 64(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movntdqa   80(%0), %%xmm0;\n"
                "movdqa    %%xmm0, 80(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movntdqa   96(%0), %%xmm0;\n"
                "movdqa    %%xmm0, 96(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movntdqa   112(%0), %%xmm0;\n"
                "movdqa    %%xmm0, 112(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

	    s += unroll_factor;
	    t += unroll_factor;
	}

				// copy trailing 128-bit words
	for ( ; k < words; k+=1) {
            asm("movntdqa   0(%0), %%xmm0;\n"
                "movdqa    %%xmm0, 0(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

	    s += 1;
	    t += 1;
	}

	size_t copied = words*word_size;
	bytes -= copied;
	source = (char*) s;
	target = (char*) t;

				// copy trailing bytes
	for (k=0; k < bytes; k++) {
	    *target++ = *source++;
	}
    } else {
				// addresses are NOT word aligned to each other
	memcpy(target, source, bytes);
    }
#endif

    return dest;
}
#else
void*
xnt(void* dest, const void* src, size_t bytes)
{
    printf("Compile with \"-DNT_INOUT\" to use xnt.\n");
    exit(0);

    return NULL;
}
#endif


#if defined(NT_INOUT) || (defined(NT_INONLY) && defined(NT_OUTONLY))
void*
xnn(void* dest, const void* src, size_t bytes)
{
#if defined(__i386__)
    printf("xnn must be compiled for x86_64\n");
    exit(-1);
#elif defined(__x86_64__)

    char* target = (char*) dest;
    const char* source = (char*) src;
    const uint8_t unroll_factor=8;
    const uint8_t word_size=sizeof(v4si_t);

				// words here are 16 bytes (128-bits)
				// blocks are 4 words (64 bytes)
    if ((((uint64_t)source) % word_size) == (((uint64_t)target) % word_size)) {
				// addresses are word aligned to each other
	int k, len;

				// make the copy word aligned
	len = ((uint64_t)source) % word_size;   // unaligned leading bytes
	for (k=0; k < len; k++) {
	    *target++ = *source++;
	}
	bytes -= len;

				// copy the blocks
	v4si_t* s = (v4si_t*) source;
	v4si_t* t = (v4si_t*) target;
        size_t words = bytes/word_size;         // number of 16-byte words
	for (k=0; k < words-(unroll_factor-1); k+=unroll_factor) {
            asm("movntdqa   0(%0), %%xmm0;\n"
                "movntdq   %%xmm0, 0(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movntdqa   16(%0), %%xmm0;\n"
                "movntdq   %%xmm0, 16(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movntdqa   32(%0), %%xmm0;\n"
                "movntdq   %%xmm0, 32(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movntdqa   48(%0), %%xmm0;\n"
                "movntdq   %%xmm0, 48(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movntdqa   64(%0), %%xmm0;\n"
                "movntdq   %%xmm0, 64(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movntdqa   80(%0), %%xmm0;\n"
                "movntdq   %%xmm0, 80(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movntdqa   96(%0), %%xmm0;\n"
                "movntdq   %%xmm0, 96(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

            asm("movntdqa   112(%0), %%xmm0;\n"
                "movntdq   %%xmm0, 112(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

	    s += unroll_factor;
	    t += unroll_factor;
	}

				// copy trailing 128-bit words
	for ( ; k < words; k+=1) {
            asm("movntdqa   0(%0), %%xmm0;\n"
                "movntdq   %%xmm0, 0(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

	    s += 1;
	    t += 1;
	}

	size_t copied = words*word_size;
	bytes -= copied;
	source = (char*) s;
	target = (char*) t;

				// copy trailing bytes
	for (k=0; k < bytes; k++) {
	    *target++ = *source++;
	}
    } else {
				// addresses are NOT word aligned to each other
	memcpy(target, source, bytes);
    }
#endif

    return dest;
}
#else
void*
xnn(void* dest, const void* src, size_t bytes)
{
    printf("Compile with \"-DNT_INOUT\" to use xnn.\n");
    exit(0);

    return NULL;
}
#endif


#if defined(NT_INOUT) || (defined(NT_INONLY) && defined(NT_OUTONLY))
void*
memread(void* dest, const void* src, size_t bytes)
{
#if defined(__i386__)
    printf("memread must be compiled for x86_64\n");
    exit(-1);
#elif defined(__x86_64__)

    char* target = (char*) dest;
    const char* source = (char*) src;
    const uint8_t unroll_factor=8;
    const uint8_t word_size=sizeof(v4si_t);

				// words here are 16 bytes (128-bits)
				// blocks are 4 words (64 bytes)
    if ((((uint64_t)source) % word_size) == (((uint64_t)target) % word_size)) {
				// addresses are word aligned to each other
	int k, len;

				// make the read word aligned
	len = ((uint64_t)source) % word_size;   // unaligned leading bytes
	for (k=0; k < len; k++) {
	    register char char_reg = *source++;
	}
	bytes -= len;

				// read the blocks
	v4si_t* s = (v4si_t*) source;
	v4si_t* t = (v4si_t*) target;
        size_t words = bytes/word_size;         // number of 16-byte words
	for (k=0; k < words-(unroll_factor-1); k+=unroll_factor) {
            asm("movntdqa   0(%0), %%xmm0;\n\t"
		"movntdqa  16(%0), %%xmm0;\n\t"
		"movntdqa  32(%0), %%xmm0;\n\t"
		"movntdqa  48(%0), %%xmm0;\n\t"
		"movntdqa  64(%0), %%xmm0;\n\t"
		"movntdqa  80(%0), %%xmm0;\n\t"
		"movntdqa  96(%0), %%xmm0;\n\t"
		"movntdqa 112(%0), %%xmm0;\n\t"
                :
                : "r"(s)
                : "%xmm0"
                );

	    s += unroll_factor;
	    t += unroll_factor;
	}

				// read trailing 128-bit words
	for ( ; k < words; k+=1) {
            asm("movntdqa   0(%0), %%xmm0;\n"
                :
                : "r"(s)
                : "%xmm0"
                );

	    s += 1;
	    t += 1;
	}

	size_t copied = words*word_size;
	bytes -= copied;
	source = (char*) s;
	target = (char*) t;

				// copy trailing bytes
	for (k=0; k < bytes; k++) {
	    *target++ = *source++;
	}
    } else {
				// addresses are NOT word aligned to each other
	memcpy(target, source, bytes);
    }
#endif

    return dest;
}
#else
void*
memread(void* dest, const void* src, size_t bytes)
{
    printf("Compile with \"-DNT_INOUT\" to use memread.\n");
    exit(0);

    return NULL;
}
#endif


#if defined(NT_INOUT) || (defined(NT_INONLY) && defined(NT_OUTONLY))
void*
memwrite(void* dest, const void* src, size_t bytes)
{
#if defined(__i386__)
    printf("memwrite must be compiled for x86_64\n");
    exit(-1);
#elif defined(__x86_64__)

    char* target = (char*) dest;
    const char* source = (char*) src;
    const uint8_t unroll_factor=8;
    const uint8_t word_size=sizeof(v4si_t);

				// words here are 16 bytes (128-bits)
				// blocks are 4 words (64 bytes)
    if ((((uint64_t)source) % word_size) == (((uint64_t)target) % word_size)) {
				// addresses are word aligned to each other
	int k, len;

				// make the write word aligned
	len = ((uint64_t)source) % word_size;   // unaligned leading bytes
	for (k=0; k < len; k++) {
	    *target++ = *source++;
	}
	bytes -= len;

				// write the blocks
	v4si_t* s = (v4si_t*) source;
	v4si_t* t = (v4si_t*) target;
        size_t words = bytes/word_size;         // number of 16-byte words
	for (k=0; k < words-(unroll_factor-1); k+=unroll_factor) {
            asm(
                "movntdq   %%xmm0,   0(%0);\n\t"
                "movntdq   %%xmm0,  16(%0);\n\t"
                "movntdq   %%xmm0,  32(%0);\n\t"
                "movntdq   %%xmm0,  48(%0);\n\t"
                "movntdq   %%xmm0,  64(%0);\n\t"
                "movntdq   %%xmm0,  80(%0);\n\t"
                "movntdq   %%xmm0,  96(%0);\n\t"
                "movntdq   %%xmm0, 112(%0);\n\t"
                :
                : "r"(t)
                : "%xmm0"
                );

	    s += unroll_factor;
	    t += unroll_factor;
	}

				// copy trailing 128-bit words
	for ( ; k < words; k+=1) {
            asm("movntdqa   0(%0), %%xmm0;\n"
                "movntdq   %%xmm0, 0(%1);\n"
                :
                : "r"(s), "r"(t)
                : "%xmm0"
                );

	    s += 1;
	    t += 1;
	}

	size_t copied = words*word_size;
	bytes -= copied;
	source = (char*) s;
	target = (char*) t;

				// copy trailing bytes
	for (k=0; k < bytes; k++) {
	    *target++ = *source++;
	}
    } else {
				// addresses are NOT word aligned to each other
	memcpy(target, source, bytes);
    }
#endif

    return dest;
}
#else
void*
memwrite(void* dest, const void* src, size_t bytes)
{
    printf("Compile with \"-DNT_INOUT\" to use memwrite.\n");
    exit(0);

    return NULL;
}
#endif




void* 
naive(void* dest, const void* src, size_t n)
{
    char *s1 = (char*) dest;
    const char *s2 = (const char*) src;
    for(; 0<n; --n)*s1++ = *s2++;

    return dest;
}

#if defined(ID)
const char* Memcpy_C = "\0@ID " ID;
#endif
