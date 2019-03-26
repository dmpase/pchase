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

#if !defined(__SYNC_H__)
#define __SYNC_H__

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "x86.h"

#define SYNC_MAX_LOCK_SPIN		(1<<30)
#define SYNC_LOCK_SET			0xff
#define SYNC_LOCK_CLR			0x0

#define     spin_lock_u8_init	spin_lock_u8_clr

typedef volatile uint8_t lock8_t;

typedef union {
	struct {
		volatile uint16_t serving;
		volatile uint16_t next;
	} s;
	volatile uint32_t word;
} fair_lock_t;

typedef volatile int32_t atomic32_t;
typedef volatile int64_t atomic64_t;

// extern void     fair_lock_set (fair_lock_t* lock);
// extern void     fair_lock_clr (fair_lock_t* lock);
static inline void fair_lock_init(fair_lock_t* lock)
{
	lock->word = 0;
}

static inline void fair_lock_set (fair_lock_t* lock)
{
	fair_lock_t nlk, olk, rlk;

	rlk.word = 0;
	do {
		nlk = olk = rlk;
		nlk.s.next += 1;
	} while ((rlk.word=x86_cmpxchg_u32(&lock->word, olk.word, nlk.word)) != olk.word);

	while (lock->s.serving != olk.s.next) {
		continue;
	}
}

static inline void fair_lock_clr (fair_lock_t* lock)
{
	fair_lock_t nlk, olk, rlk;

	rlk.word = lock->word;
	do {
		nlk = olk = rlk;
		nlk.s.serving += 1;
		if (nlk.s.serving == nlk.s.next) {
			nlk.word = 0;
		}
	} while ((rlk.word=x86_cmpxchg_u32(&lock->word, olk.word, nlk.word)) != olk.word);
}



/*
 * PURPOSE: set (i.e., lock) an 8-bit lock. 
 * 
 * RETURN: TRUE if the lock has been cleanly set for the caller, or FALSE if 
 * the lock was owned by another thread and we were unable to set the lock 
 * for ourselves.
 */

static inline bool spin_lock_u8_set(lock8_t* lock)
{
	uint32_t i;
	uint8_t v = SYNC_LOCK_SET;
	for (i=0; i < SYNC_MAX_LOCK_SPIN && v != SYNC_LOCK_CLR; i++) {
		v = x86_cmpxchg_u8(lock, SYNC_LOCK_CLR, SYNC_LOCK_SET);
	}
	x86_sfence();

	return v == SYNC_LOCK_CLR;
}


/*
 * PURPOSE: clear (i.e., unlock) an 8-bit lock. 
 *
 * CAUTION: this routine will clear the lock, even if it was not set by the
 * calling thread.
 */

static inline void spin_lock_u8_clr(lock8_t* lock)
{
	*lock = SYNC_LOCK_CLR;
	x86_sfence();
}


static inline uint32_t atomic_add_u32(atomic32_t* word, int32_t value)
{
	int32_t v, n=*word;

	do {
		v = n;
	} while ((n=x86_cmpxchg_i32(word, v, v+value)) != v);

	x86_sfence();

	return v;
}


static inline uint32_t atomic_min_u32(atomic32_t* word, int32_t value)
{
	int32_t v, n=*word;

	do {
		v = n;
		if (v <= value) {
			break;
		}
	} while ((n=x86_cmpxchg_i32(word, v, value)) != v);

	x86_sfence();

	return v;
}


static inline uint32_t atomic_max_u32(atomic32_t* word, int32_t value)
{
	int32_t v, n=*word;

	do {
		v = n;
		if (value <= v) {
			break;
		}
	} while ((n=x86_cmpxchg_i32(word, v, value)) != v);

	x86_sfence();

	return v;
}


static inline uint64_t atomic_add_u64(atomic64_t* word, int64_t value)
{
	int64_t v, n=*word;

	do {
		v = n;
	} while ((n=x86_cmpxchg_i64(word, v, v+value)) != v);

	x86_sfence();

	return v;
}


static inline uint64_t atomic_min_u64(atomic64_t* word, int64_t value)
{
	int64_t v, n=*word;

	do {
		v = n;
		if (v <= value) {
			break;
		}
	} while ((n=x86_cmpxchg_i64(word, v, value)) != v);

	x86_sfence();

	return v;
}


static inline uint64_t atomic_max_u64(atomic64_t* word, int64_t value)
{
	int64_t v, n=*word;

	do {
		v = n;
		if (value <= v) {
			break;
		}
	} while ((n=x86_cmpxchg_i64(word, v, value)) != v);

	x86_sfence();

	return v;
}

#endif
