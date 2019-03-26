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

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <assert.h>

#include "Allocate.h"

#include "Thread.h"
#include "sync.h"


				// domain < 0 implies interleaved allocation, but
				// interleaved is not implemented yet.
pthread_spinlock_t** 
Allocate::pt_spin (int count, uint64_t domain, int num_numa_domains, int threadcount, int align, int64_t& bytes)
{
// printf("enter Allocate::pt_spin: count=%d, domain=%lld, num_numa_domains=%d, threadcount=%d, align=%d\n", count, domain, num_numa_domains, threadcount, align);fflush(stdout);
				// check the input parameters
    assert(0 <  count);
    assert(0 <= num_numa_domains);
    assert(0 <  threadcount);
    assert(0 <  align);

				// figure out how big the locks must be in order
				// to be aligned on the boundary indicated
    int64_t size = ((sizeof(pthread_spinlock_t) + align - 1) / align) * align;

				// allocate the locks where they belong
    bytes = size * count;
    char* pool = Thread::alloc_on_node(domain, bytes);
    assert(pool != NULL);

				// allocate the lock pointers
    pthread_spinlock_t** result = new pthread_spinlock_t*[count];
    assert(result != NULL);

    				// map the lock pointers to the pool of locks
				// and initialize the locks
    for (int i=0; i < count; i++) {
	result[i] = (pthread_spinlock_t*)(pool + i*size);
	pthread_spin_init(result[i], PTHREAD_PROCESS_PRIVATE);
    }

    				// randomize the pointers
    Allocate::random_mem_list( (void**)result, count );

// printf("leave Allocate::pt_spin: bytes=%lld\n", bytes);fflush(stdout);
    return result;
}


pthread_mutex_t**
Allocate::pt_mutex(int count, uint64_t domain, int num_numa_domains, int threadcount, int align, int64_t& bytes)
{
    assert(0 <  count);
    assert(0 <= num_numa_domains);
    assert(0 <  threadcount);
    assert(0 <  align);

				// figure out how big the locks must be in order
				// to be aligned on the boundary indicated
    int64_t size = ((sizeof(pthread_mutex_t) + align - 1) / align) * align;

				// allocate the locks where they belong
    bytes = size * count;
    char* pool = Thread::alloc_on_node(domain, bytes);
    assert(pool != NULL);

				// allocate the lock pointers
    pthread_mutex_t** result = new pthread_mutex_t*[count];
    assert(result != NULL);

    				// map the lock pointers to the pool of locks
				// and initialize the locks
    for (int i=0; i < count; i++) {
	result[i] = (pthread_mutex_t*)(pool + i*size);
	pthread_mutex_init(result[i], NULL);
    }

    				// randomize the pointers
    Allocate::random_mem_list( (void**)result, count );

    return result;
}


pthread_rwlock_t**
Allocate::pt_rwlock(int count, uint64_t domain, int num_numa_domains, int threadcount, int align, int64_t& bytes)
{
    assert(0 <  count);
    assert(0 <= num_numa_domains);
    assert(0 <  threadcount);
    assert(0 <  align);

				// figure out how big the locks must be in order
				// to be aligned on the boundary indicated
    int64_t size = ((sizeof(pthread_rwlock_t) + align - 1) / align) * align;

				// allocate the locks where they belong
    bytes = size * count;
    char* pool = Thread::alloc_on_node(domain, bytes);
    assert(pool != NULL);

				// allocate the lock pointers
    pthread_rwlock_t** result = new pthread_rwlock_t*[count];
    assert(result != NULL);

    				// map the lock pointers to the pool of locks
				// and initialize the locks
    for (int i=0; i < count; i++) {
	result[i] = (pthread_rwlock_t*)(pool + i*size);
	pthread_rwlock_init(result[i], NULL);
    }

    				// randomize the pointers
    Allocate::random_mem_list( (void**)result, count );

    return result;
}


uint32_t**
Allocate::uint32(int count, uint64_t domain, int num_numa_domains, int threadcount, int align, int64_t& bytes)
{
    assert(0 <  count);
    assert(0 <= num_numa_domains);
    assert(0 <  threadcount);
    assert(0 <  align);

				// figure out how big the locks must be in order
				// to be aligned on the boundary indicated
    int64_t size = ((sizeof(uint32_t) + align - 1) / align) * align;

				// allocate the locks where they belong
    bytes = size * count;
    char* pool = Thread::alloc_on_node(domain, bytes);
    assert(pool != NULL);

				// allocate the lock pointers
    uint32_t** result = new uint32_t*[count];
    assert(result != NULL);

    				// map the lock pointers to the pool of locks
				// and initialize the locks
    for (int i=0; i < count; i++) {
	result[i] = (uint32_t*)(pool + i*size);
	*result[i] = 0;
    }

    				// randomize the pointers
    Allocate::random_mem_list( (void**)result, count );

    return result;
}


uint64_t**
Allocate::uint64(int count, uint64_t domain, int num_numa_domains, int threadcount, int align, int64_t& bytes)
{
    assert(0 <  count);
    assert(0 <= num_numa_domains);
    assert(0 <  threadcount);
    assert(0 <  align);

				// figure out how big the locks must be in order
				// to be aligned on the boundary indicated
    int64_t size = ((sizeof(uint64_t) + align - 1) / align) * align;

				// allocate the locks where they belong
    bytes = size * count;
    char* pool = Thread::alloc_on_node(domain, bytes);
    assert(pool != NULL);

				// allocate the lock pointers
    uint64_t** result = new uint64_t*[count];
    assert(result != NULL);

    				// map the lock pointers to the pool of locks
				// and initialize the locks
    for (int i=0; i < count; i++) {
	result[i] = (uint64_t*)(pool + i*size);
	*result[i] = 0;
    }

    				// randomize the pointers
    Allocate::random_mem_list( (void**)result, count );

    return result;
}


atomic32_t**
Allocate::atomic32(int count, uint64_t domain, int num_numa_domains, int threadcount, int align, int64_t& bytes)
{
    assert(0 <  count);
    assert(0 <= num_numa_domains);
    assert(0 <  threadcount);
    assert(0 <  align);

				// figure out how big the locks must be in order
				// to be aligned on the boundary indicated
    int64_t size = ((sizeof(atomic32_t) + align - 1) / align) * align;

				// allocate the locks where they belong
    bytes = size * count;
    char* pool = Thread::alloc_on_node(domain, bytes);
    assert(pool != NULL);

				// allocate the lock pointers
    atomic32_t** result = new atomic32_t*[count];
    assert(result != NULL);

    				// map the lock pointers to the pool of locks
				// and initialize the locks
    for (int i=0; i < count; i++) {
	result[i] = (atomic32_t*)(pool + i*size);
	*result[i] = 0;
    }

    				// randomize the pointers
    Allocate::random_mem_list( (void**)result, count );

    return result;
}


atomic64_t**
Allocate::atomic64(int count, uint64_t domain, int num_numa_domains, int threadcount, int align, int64_t& bytes)
{
    assert(0 <  count);
    assert(0 <= num_numa_domains);
    assert(0 <  threadcount);
    assert(0 <  align);

				// figure out how big the locks must be in order
				// to be aligned on the boundary indicated
    int64_t size = ((sizeof(atomic64_t) + align - 1) / align) * align;

				// allocate the locks where they belong
    bytes = size * count;
    char* pool = Thread::alloc_on_node(domain, bytes);
    assert(pool != NULL);

				// allocate the lock pointers
    atomic64_t** result = new atomic64_t*[count];
    assert(result != NULL);

    				// map the lock pointers to the pool of locks
				// and initialize the locks
    for (int i=0; i < count; i++) {
	result[i] = (atomic64_t*)(pool + i*size);
	*result[i] = 0;
    }

    				// randomize the pointers
    Allocate::random_mem_list( (void**)result, count );

    return result;
}


fair_lock_t**
Allocate::fair_lock(int count, uint64_t domain, int num_numa_domains, int threadcount, int align, int64_t& bytes)
{
    assert(0 <  count);
    assert(0 <= num_numa_domains);
    assert(0 <  threadcount);
    assert(0 <  align);

				// figure out how big the locks must be in order
				// to be aligned on the boundary indicated
    int64_t size = ((sizeof(fair_lock_t) + align - 1) / align) * align;

				// allocate the locks where they belong
    bytes = size * count;
    char* pool = Thread::alloc_on_node(domain, bytes);
    assert(pool != NULL);

				// allocate the lock pointers
    fair_lock_t** result = new fair_lock_t*[count];
    assert(result != NULL);

    				// map the lock pointers to the pool of locks
				// and initialize the locks
    for (int i=0; i < count; i++) {
	result[i] = (fair_lock_t*)(pool + i*size);
	result[i]->word = 0;
    }

    				// randomize the pointers
    Allocate::random_mem_list( (void**)result, count );

    return result;
}


lock8_t**
Allocate::lock8(int count, uint64_t domain, int num_numa_domains, int threadcount, int align, int64_t& bytes)
{
    assert(0 <  count);
    assert(0 <= num_numa_domains);
    assert(0 <  threadcount);
    assert(0 <  align);

				// figure out how big the locks must be in order
				// to be aligned on the boundary indicated
    int64_t size = ((sizeof(lock8_t) + align - 1) / align) * align;

				// allocate the locks where they belong
    bytes = size * count;
    char* pool = Thread::alloc_on_node(domain, bytes);
    assert(pool != NULL);

				// allocate the lock pointers
    lock8_t** result = new lock8_t*[count];
    assert(result != NULL);

    				// map the lock pointers to the pool of locks
				// and initialize the locks
    for (int i=0; i < count; i++) {
	result[i] = (lock8_t*)(pool + i*size);
	*result[i] = 0;
    }

    				// randomize the pointers
    Allocate::random_mem_list( (void**)result, count );

    return result;
}


pthread_barrier_t** 
Allocate::pt_barrier(int count, uint64_t domain, int num_numa_domains, int threadcount, int align, int64_t& bytes)
{
// printf("enter Allocate::pt_spin: count=%d, domain=%lld, num_numa_domains=%d, threadcount=%d, align=%d\n", count, domain, num_numa_domains, threadcount, align);fflush(stdout);
				// check the input parameters
    assert(0 <  count);
    assert(0 <= num_numa_domains);
    assert(0 <  threadcount);
    assert(0 <  align);

				// figure out how big the locks must be in order
				// to be aligned on the boundary indicated
    int64_t size = ((sizeof(pthread_barrier_t) + align - 1) / align) * align;

				// allocate the locks where they belong
    bytes = size * count;
    char* pool = Thread::alloc_on_node(domain, bytes);
    assert(pool != NULL);

				// allocate the lock pointers
    pthread_barrier_t** result = new pthread_barrier_t*[count];
    assert(result != NULL);

    				// map the lock pointers to the pool of locks
				// and initialize the locks
    for (int i=0; i < count; i++) {
	result[i] = (pthread_barrier_t*)(pool + i*size);
        pthread_barrier_init( result[i], NULL, threadcount );
    }

    				// randomize the pointers
    Allocate::random_mem_list( (void**)result, count );

// printf("leave Allocate::pt_spin: bytes=%lld\n", bytes);fflush(stdout);
    return result;
}



				// exclude 2 and mersienne primes, i.e.,
				// primes of the form 2**n - 1, e.g.,
				// 3, 7, 31, 127
static const int prime_table[] = { 5, 11, 13, 17, 19, 23, 37, 41, 43, 47,
    53, 61, 71, 73, 79, 83, 89, 97, 101, 103, 109, 113, 131, 137, 139, 149,
    151, 157, 163, };
static const int prime_table_size = sizeof prime_table / sizeof prime_table[0];

void
Allocate::random_mem_list( void** list, int count )
{
    int factor = prime_table[ random() % prime_table_size ];
    int offset = random() % count;

				// loop through the copies
    for (int i=0; i < count; i++) {
	int copy = (factor * i + offset) % count;
	void* tmp = list[i];
	list[i] = list[copy];
	list[copy] = tmp;
    }
}

#if defined(ID)
const char* Allocate_C = "\0@ID " ID;
#endif
