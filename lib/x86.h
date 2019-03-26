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

#if !defined(__X86_H__)
#define __X86_H__

#include <stdint.h>


static inline uint8_t x86_xchg_u8(volatile uint8_t* u, uint8_t v)
{
    uint8_t w;

#if defined(__x86_64__) || defined(__i386__)
    __asm__ __volatile__(
        "mov %2, %%al;"		// move v to source register
        "lock xchg (%1), %%al;"	// exchange *u and source
        "mov %%al, %0;"		// store value from *u in w
        ""
        : "=r"(w)		// output symbols
        : "r"(u), "r"(v)	// input symbols
        : "%al","memory"	// registers trashed
    );
#else
#endif

	return w;
}

static inline uint16_t x86_xchg_u16(volatile uint16_t* u, uint16_t v)
{
    uint16_t w;

#if defined(__x86_64__) || defined(__i386__)
    __asm__ __volatile__(
        "mov %2, %%ax;"		// move v to source register
        "lock xchg (%1), %%ax;"	// exchange *u and source
        "mov %%ax, %0;"		// store value from *u in w
        ""
        : "=r"(w)		// output symbols
        : "r"(u), "r"(v)	// input symbols
        : "%ax","memory"	// registers trashed
    );
#else
#endif

    return w;
}

static inline uint32_t x86_xchg_u32(volatile uint32_t* u, uint32_t v)
{
    uint32_t w;

#if defined(__x86_64__) || defined(__i386__)
    __asm__ __volatile__(
        "mov %2, %%eax;"	// move v to source register
        "lock xchg (%1), %%eax;"// exchange *u and source
        "mov %%eax, %0;"	// store value from *u in w
        ""
        : "=r"(w)		// output symbols
        : "r"(u), "r"(v)	// input symbols
        : "%eax","memory"	// registers trashed
    );
#else
#endif

	return w;
}

static inline uint64_t x86_xchg_u64(volatile uint64_t* u, uint64_t v)
{
    uint64_t w = 0;

#if defined(__x86_64__) || defined(__i386__)
    __asm__ __volatile__(
        "mov %2, %%rax;"	// move v to source register
        "lock xchg (%1), %%rax;"// exchange *u and source
        "mov %%rax, %0;"	// store value from *u in w
        ""
        : "=r"(w)		// output symbols
        : "r"(u), "r"(v)	// input symbols
        : "%rax","memory"	// registers trashed
    );
#else
#endif

    return w;
}


static inline uint8_t x86_cmpxchg_u8(volatile uint8_t* u, uint8_t c, uint8_t n)
{
	uint8_t w;

#if defined(__x86_64__)
    __asm__ __volatile__(
        "mov %2, %%al;"		// move c to comparison register
        "lock cmpxchg %3,(%1);"	// compare if (*u == c) *u = n;
        "mov %%al, %0;"		// 
        ""
        : "=r"(w)		// output symbols
        : "r"(u),"r"(c),"r"(n)	// input symbols
        : "%al","memory","cc"	// registers trashed
    );
#elif defined(__i386__)
    printf("x86_cmpxchg_i64 must be compiled with x86_64\n");
    exit(-1);
#else
#endif

	return w;
}

static inline uint16_t x86_cmpxchg_u16(volatile uint16_t* u, uint16_t c, uint16_t n)
{
    uint16_t w;

#if defined(__x86_64__) || defined(__i386__)
    __asm__ __volatile__(
        "mov %2, %%ax;"		// move c to comparison register
        "lock cmpxchg %3,(%1);"	// compare if (*u == c) *u = n;
        "mov %%ax, %0;"		// 
        ""
        : "=r"(w)		// output symbols
        : "r"(u),"r"(c),"r"(n)	// input symbols
        : "%ax","memory","cc"	// registers trashed
    );
#else
#endif

    return w;
}

static inline uint32_t x86_cmpxchg_u32(volatile uint32_t* u, uint32_t c, uint32_t n)
{
	uint32_t w;

#if defined(__x86_64__) || defined(__i386__)
    __asm__ __volatile__(
        "mov %2, %%eax;"	// move c to comparison register
        "lock cmpxchg %3,(%1);"	// compare if (*u == c) *u = n;
        "mov %%eax, %0;"	// 
        ""
        : "=r"(w)		// output symbols
        : "r"(u),"r"(c),"r"(n)	// input symbols
        : "%eax","memory","cc"	// registers trashed
    );
#else
#endif

    return w;
}

static inline uint64_t x86_cmpxchg_u64(volatile uint64_t* u, uint64_t c, uint64_t n)
{
    uint64_t r;
#if defined(__x86_64__) || defined(__i386__)
    uint32_t ru,rl;
    uint32_t cu = (uint32_t)(c >> 32);
    uint32_t cl = (uint32_t)(c & 0x00000000FFFFFFFFUL);
    uint32_t nu = (uint32_t)(n >> 32);
    uint32_t nl = (uint32_t)(n & 0x00000000FFFFFFFFUL);

    __asm__ __volatile__(
        "mov %3, %%edx;"	// move c to comparison register
        "mov %4, %%eax;"
        "mov %5, %%ecx;"	// move n to transfer register
        "mov %6, %%ebx;"
        "lock cmpxchg8b (%2);"
        "mov %%edx, %0;"	// move result to ru:rl
        "mov %%eax, %1;"
        ""
        : "=r"(ru),"=r"(rl)			    // output symbols
        : "r"(u),"r"(cu),"r"(cl),"r"(nu),"r"(nl)    // input symbols
        : "%eax","%ebx","%ecx","%edx","cc","memory" // registers trashed
    );

    r = ((uint64_t) ru << 32) | (uint64_t) rl;
#else
#endif

    return r;
}


static inline int8_t x86_cmpxchg_i8(volatile int8_t* u, int8_t c, int8_t n)
{
    int8_t w;

#if defined(__x86_64__) || defined(__i386__)
    __asm__ __volatile__(
        "mov %2, %%al;"		// move c to comparison register
        "lock cmpxchg %3,(%1);"	// compare if (*u == c) *u = n;
        "mov %%al, %0;"		// 
        ""
        : "=r"(w)		// output symbols
        : "r"(u),"r"(c),"r"(n)	// input symbols
        : "%al","memory","cc"	// registers trashed
    );
#else
#endif

    return w;
}

static inline int16_t x86_cmpxchg_i16(volatile int16_t* u, int16_t c, int16_t n)
{
    int16_t w;

#if defined(__x86_64__) || defined(__i386__)
    __asm__ __volatile__(
        "mov %2, %%ax;"		// move c to comparison register
        "lock cmpxchg %3,(%1);"	// compare if (*u == c) *u = n;
        "mov %%ax, %0;"		// 
        ""
        : "=r"(w)		// output symbols
        : "r"(u),"r"(c),"r"(n)	// input symbols
        : "%ax","memory","cc"	// registers trashed
    );
#else
#endif

    return w;
}

static inline int32_t x86_cmpxchg_i32(volatile int32_t* u, int32_t c, int32_t n)
{
    int32_t w;

#if defined(__x86_64__) || defined(__i386__)
    __asm__ __volatile__(
        "mov %2, %%eax;"	// move c to comparison register
        "lock cmpxchg %3,(%1);"	// compare if (*u == c) *u = n;
        "mov %%eax, %0;"	// 
        ""
        : "=r"(w)		// output symbols
        : "r"(u),"r"(c),"r"(n)	// input symbols
        : "%eax","memory","cc"	// registers trashed
    );
#else
#endif

    return w;
}

static inline int64_t x86_cmpxchg_i64(volatile int64_t* u, int64_t c, int64_t n)
{
    int64_t r = 0;
#if defined(__x86_64__)
    int32_t ru,rl;
    int32_t cu = (int32_t)(c >> 32);
    int32_t cl = (int32_t)(c & 0x00000000FFFFFFFFUL);
    int32_t nu = (int32_t)(n >> 32);
    int32_t nl = (int32_t)(n & 0x00000000FFFFFFFFUL);

    __asm__ __volatile__(
        "mov %3, %%edx;"	// move c to comparison register
        "mov %4, %%eax;"
        "mov %5, %%ecx;"	// move n to transfer register
        "mov %6, %%ebx;"
        "lock cmpxchg8b (%2);"
        "mov %%edx, %0;"	// move result to ru:rl
        "mov %%eax, %1;"
        ""
        : "=r"(ru),"=r"(rl)			    // output symbols
        : "r"(u),"r"(cu),"r"(cl),"r"(nu),"r"(nl)    // input symbols
        : "%eax","%ebx","%ecx","%edx","cc","memory" // registers trashed
    );

    r = ((int64_t) ru << 32) | (int64_t) rl;
#else
    printf("x86_cmpxchg_i64 must be compiled with x86_64\n");
    exit(-1);
#endif

	return r;
}


static inline void x86_fence(void)
{
#if defined(__x86_64__) || defined(__i386__)
    __asm__ __volatile__(
        "mfence;"		// issue the memory fence
        ""
        : 			// output symbols
        : 			// input symbols
    );
#else
#endif
}

static inline void x86_lfence(void)
{
#if defined(__x86_64__) || defined(__i386__)
    __asm__ __volatile__(
        "lfence;"		// issue the load fence
        ""
        : 			// output symbols
        : 			// input symbols
    );
#else
#endif
}

static inline void x86_sfence(void)
{
#if defined(__x86_64__) || defined(__i386__)
    __asm__ __volatile__(
        "sfence;"		// issue the store fence
        ""
        : 			// output symbols
        : 			// input symbols
    );
#else
#endif
}

static inline void x86_mfence(void)
{
#if defined(__x86_64__) || defined(__i386__)
    __asm__ __volatile__(
        "mfence;"		// issue the memory fence
        ""
        : 			// output symbols
        : 			// input symbols
    );
#else
#endif
}

static inline uint64_t x86_rdtsc(void)
{
#if defined(__x86_64__) || defined(__i386__)
				// See pg. 406 of the AMD x86-64 Architecture
				// Programmer's Manual, Volume 2, System Programming
    unsigned int eax=0, edx=0;

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
#else
#endif
}

static inline void x86_clflush(volatile void *p)
{
#if defined(__x86_64__) || defined(__i386__)
    __asm__ __volatile__(
        "clflush (%0);"     	// flush the location contained in p
        "mfence;"		// issue the memory fence
        ""
        : 			// output symbols
        : "r"(p)		// input symbols
    );
#else
#endif
}


#endif
