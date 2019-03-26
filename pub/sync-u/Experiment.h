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

#if !defined(Experiment_h)
#define Experiment_h

#include <stdint.h>

class Experiment {
public:
    Experiment();
    ~Experiment();

    int   parse_args(int argc, char* argv[]); 
    int64_t parse_number( const char* s );
    float parse_real( const char* s );

    const char* access();

    void alloc_interleave();
    void alloc_local();
    void alloc_map();
    void alloc_gen(const char*, const char*);

    const char* placement();
    const char* method();
    void print();

			    // fundamental parameters
    int64_t locks_per_thread;
    enum { PTHREAD_SPIN, PTHREAD_MUTEX, CMPXSPIN, CMPXCHG32, CMPXCHG64, FAIRSPIN, ATOMIC32, ATOMIC64, PTHREAD_READER, PTHREAD_WRITER} lock_method;
    int64_t alignment;
    int64_t experiments;
    int64_t iterations;
    int32_t num_numa_domains;
    int32_t num_threads;
    int32_t numa_max_domain;
    int32_t page_size;
    enum { INTERLEAVE, LOCAL, MAP, GEN, } numa_placement;
    enum { CSV, BOTH, HEADER, TABLE } output_mode;
    int64_t lock_ops_per_thread;
    const char* placement_map;
    float seconds;
    int32_t* lock_domain;
    int32_t* thread_domain;
    char* gen_lock_domain;
    char* gen_thread_domain;

    char** random_state;	// random state for each thread

private:
};

#endif
