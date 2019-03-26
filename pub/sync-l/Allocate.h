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

#if !defined(Allocate_h)
#define Allocate_h

#include <stdint.h>

#include "sync.h"


				// each routine returns a pointer to an array of pointers to the type of object.
				// the objects are suitably randomized, so walking the list accesses them randomly.
				// objects are allocated to the specified numa domain. interleaving is done by pages.
				// objects are aligned on boundaries according to "align", which should be a sensible
				// number, such as a small power of two, like 2**6 = 64. 
class Allocate {
public:
    static pthread_spinlock_t** pt_spin  (int count, uint64_t domain, int num_numa_domains, int pagesize, int align, int64_t& bytes);
    static pthread_mutex_t**    pt_mutex (int count, uint64_t domain, int num_numa_domains, int pagesize, int align, int64_t& bytes);
    static pthread_rwlock_t**   pt_rwlock(int count, uint64_t domain, int num_numa_domains, int pagesize, int align, int64_t& bytes);
    static fair_lock_t**        fair_lock(int count, uint64_t domain, int num_numa_domains, int pagesize, int align, int64_t& bytes);
    static atomic32_t**         atomic32 (int count, uint64_t domain, int num_numa_domains, int pagesize, int align, int64_t& bytes);
    static atomic64_t**         atomic64 (int count, uint64_t domain, int num_numa_domains, int pagesize, int align, int64_t& bytes);
    static uint32_t**           uint32   (int count, uint64_t domain, int num_numa_domains, int pagesize, int align, int64_t& bytes);
    static uint64_t**           uint64   (int count, uint64_t domain, int num_numa_domains, int pagesize, int align, int64_t& bytes);
    static lock8_t**            lock8    (int count, uint64_t domain, int num_numa_domains, int pagesize, int align, int64_t& bytes);

    static void                 random_mem_list( void** list, int count );

private:
};


#endif
