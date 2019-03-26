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

    int     parse_args(int argc, char* argv[]); 
    int64_t parse_number( const char* s );
    float   parse_real( const char* s );

    const char* placement();
    const char* access();

			    // fundamental parameters
    int64_t pointer_size;		// number of bytes in a pointer
    int64_t bytes_per_line;	// working set cache line size (bytes)
    int64_t links_per_line;	// working set cache line size (links)
    int64_t bytes_per_page;	// working set page size (in bytes)
    int64_t lines_per_page;	// working set page size (in lines)
    int64_t links_per_page;	// working set page size (in links)
    int64_t bytes_per_chain;	// working set chain size (bytes)
    int64_t lines_per_chain;	// working set chain size (lines)
    int64_t links_per_chain;	// working set chain size (links)
    int64_t pages_per_chain;	// working set chain size (pages)
    int64_t bytes_per_thread;	// thread working set size (bytes)
    int64_t chains_per_thread;	// memory loading per thread
    int64_t num_threads;		// number of threads in the experiment
    int64_t bytes_per_test;	// test working set size (bytes)

    float   seconds;		// number of seconds per experiment
    int64_t iterations;		// number of iterations per experiment
    int64_t experiments;		// number of experiments per test

    enum { CSV, BOTH, HEADER, TABLE }
	output_mode;		// results output mode

    enum { RANDOM, STRIDED, STREAM }
	access_pattern;		// memory access pattern
    int64_t stride;

    enum { ADD, INTERLEAVE, LOCAL, GEN, MAP, XOR, }
	numa_placement;		// memory allocation mode
    int64_t offset_or_mask;
    char* placement_map;

				// maps threads and chains to numa domains
    int32_t* thread_domain;	// thread_domain[thread]
    int32_t** chain_domain;	// chain_domain[thread][chain]
    int32_t numa_max_domain;	// highest numa domain id
    int32_t num_numa_domains;	// number of numa domains

    char* gen_thread_domain;
    char* gen_pointer_domain;

    char** random_state;	// random state for each thread

    int strict;			// strictly adhere to user input, or fail

    const static int32_t DEFAULT_POINTER_SIZE      = sizeof(void*);
    const static int32_t DEFAULT_BYTES_PER_LINE    = 64;
    const static int32_t DEFAULT_LINKS_PER_LINE    = DEFAULT_BYTES_PER_LINE / DEFAULT_POINTER_SIZE;
    const static int32_t DEFAULT_BYTES_PER_PAGE    = 4096;
    const static int32_t DEFAULT_LINES_PER_PAGE    = DEFAULT_BYTES_PER_PAGE / DEFAULT_BYTES_PER_LINE;
    const static int32_t DEFAULT_LINKS_PER_PAGE    = DEFAULT_LINES_PER_PAGE * DEFAULT_LINKS_PER_LINE;
    const static int32_t DEFAULT_PAGES_PER_CHAIN   = 4096;
    const static int32_t DEFAULT_BYTES_PER_CHAIN   = DEFAULT_BYTES_PER_PAGE * DEFAULT_PAGES_PER_CHAIN;
    const static int32_t DEFAULT_LINES_PER_CHAIN   = DEFAULT_LINES_PER_PAGE * DEFAULT_PAGES_PER_CHAIN;
    const static int32_t DEFAULT_LINKS_PER_CHAIN   = DEFAULT_LINES_PER_CHAIN * DEFAULT_BYTES_PER_LINE / DEFAULT_POINTER_SIZE;
    const static int32_t DEFAULT_CHAINS_PER_THREAD = 1;
    const static int32_t DEFAULT_BYTES_PER_THREAD  = DEFAULT_BYTES_PER_CHAIN * DEFAULT_CHAINS_PER_THREAD;
    const static int32_t DEFAULT_THREADS           = 1;
    const static int32_t DEFAULT_BYTES_PER_TEST    = DEFAULT_BYTES_PER_THREAD * DEFAULT_THREADS;
    const static int32_t DEFAULT_SECONDS           = 1;
    const static int32_t DEFAULT_ITERATIONS        = 0;
    const static int32_t DEFAULT_EXPERIMENTS       = 1;

    const static int32_t DEFAULT_OUTPUT_MODE       = 1;

    void alloc_add();
    void alloc_interleave();
    void alloc_gen(const char* thd, const char* ptr);
    void alloc_local();
    void alloc_map();
    void alloc_xor();

    void print();

private:
};

#endif
