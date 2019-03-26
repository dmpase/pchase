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

#include "Run.h"

#include "Experiment.h"
#include "Memcpy.h"
#include "SpinBarrier.h"
#include "Timer.h"


static inline double max( double v1, double v2 ) { if (v1 < v2) return v2; return v1; } 
static inline double min( double v1, double v2 ) { if (v2 < v1) return v2; return v1; }

static int64_t use_naive       (double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target);
static int64_t use_memcpy      (double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target);
static int64_t use_memmove     (double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target);
static int64_t use_bcopy       (double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target);
static int64_t use_copy_tt     (double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target);
static int64_t use_copy_tn     (double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target);
static int64_t use_naive_zero  (double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target);
static int64_t use_bzero       (double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target);
static int64_t use_memset      (double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target);
static int64_t use_xtta        (double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target);
static int64_t use_xttu        (double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target);
static int64_t use_xtn         (double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target);
static int64_t use_xnt         (double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target);
static int64_t use_xnn         (double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target);
static int64_t use_memread     (double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target);
static int64_t use_memwrite    (double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target);
static int64_t (*run_benchmark)(double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target) = use_naive;


Lock     Run::global_mutex;
int64_t  Run::_ops_per_chain = 0;
double   Run::_seconds       = 1E9;


Run::Run()
: exp(NULL), bp(NULL)
{
}

Run::~Run()
{
}


int
Run::run()
{
				// establish the node id where this thread
				// will run. threads are mapped to nodes
				// by the set-up code for Experiment.
    int run_node_id = this->exp->thread_domain[this->thread_id()];
    this->run_on_node(run_node_id);

				// allocate the copy pointers locally
    char** source = (char **) this->alloc_on_node(run_node_id, this->exp->copies_per_thread * sizeof(char*));
    assert(source != NULL);
    char** target = (char **) this->alloc_on_node(run_node_id, this->exp->copies_per_thread * sizeof(char*));
    assert(target != NULL);

				// establish the node id where this thread's
				// memory will be allocated.
    int source_node_id = this->exp->source_domain[this->thread_id()];
    char* source_pool = this->alloc_on_node(source_node_id, this->exp->bytes_per_pool);
    assert(source_pool != NULL);
    int target_node_id = this->exp->target_domain[this->thread_id()];
    char* target_pool = this->alloc_on_node(target_node_id, this->exp->bytes_per_pool);
    assert(target_pool != NULL);

				// initialize the chains and 
				// select the function that
				// will execute the tests
    if (this->exp->access_pattern == Experiment::RANDOM) {
	random_mem_init( source, source_pool );
	random_mem_init( target, target_pool );
    } else if (this->exp->access_pattern == Experiment::FORWARD) {
	forward_mem_init( source, source_pool );
	forward_mem_init( target, target_pool );
    } else if (this->exp->access_pattern == Experiment::BACKWARD) {
	reverse_mem_init( source, source_pool );
	reverse_mem_init( target, target_pool );
    } else {
    }

    run_benchmark = use_naive;
    switch (this->exp->copy_method) {
    case Experiment::NAIVE :
	run_benchmark = use_naive;
	break;
    case Experiment::MEMCPY :
	run_benchmark = use_memcpy;
	break;
    case Experiment::BCOPY:
	run_benchmark = use_bcopy;
	break;
    case Experiment::COPY_TT:
	run_benchmark = use_copy_tt;
	break;
    case Experiment::COPY_TN:
	run_benchmark = use_copy_tn;
	break;
    case Experiment::NAIVEZERO:
	run_benchmark = use_naive_zero;
	break;
    case Experiment::BZERO:
	run_benchmark = use_bzero;
	break;
    case Experiment::MEMSET:
	run_benchmark = use_memset;
	break;
    case Experiment::XTTA:
	run_benchmark = use_xtta;
	break;
    case Experiment::XTTU:
	run_benchmark = use_xttu;
	break;
    case Experiment::XTN:
	run_benchmark = use_xtn;
	break;
    case Experiment::XNT:
	run_benchmark = use_xnt;
	break;
    case Experiment::XNN:
	run_benchmark = use_xnn;
	break;
    case Experiment::MEMREAD:
	run_benchmark = use_memread;
	break;
    case Experiment::MEMWRITE:
	run_benchmark = use_memwrite;
	break;
    }

				// barrier
    for (int e=0; e < this->exp->experiments; e++) {
	this->bp->barrier();

				// start timer
	double start = 0;
	if (this->thread_id() == 0) start = Timer::seconds();
	this->bp->barrier();

				// copy memory
	int64_t my_iters = run_benchmark(this->exp->seconds, this->exp->iterations, this->exp->copies_per_thread, this->exp->bytes_per_copy, source, target);

				// barrier
	this->bp->barrier();

				// stop timer
	double stop = 0;
	if (this->thread_id() == 0) stop = Timer::seconds();
	this->bp->barrier();

	if (0 <= e) {
	    if (this->thread_id() == 0) {
		double delta = stop - start;
		if (0 < delta) {
		    if ( this->exp->iterations/Run::_seconds < my_iters/delta ) {
			Run::_seconds = delta;
			this->exp->iterations = my_iters;
		    }
		}
	    }
	}
    }

    this->bp->barrier();

    return 0;
}



				// exclude 2 and mersienne primes, i.e.,
				// primes of the form 2**n - 1, e.g.,
				// 3, 7, 31, 127
static const int prime_table[] = { 5, 11, 13, 17, 19, 23, 37, 41, 43, 47,
    53, 61, 71, 73, 79, 83, 89, 97, 101, 103, 109, 113, 131, 137, 139, 149,
    151, 157, 163, };
static const int prime_table_size = sizeof prime_table / sizeof prime_table[0];

void
Run::random_mem_init( char** source, char* source_pool )
{
    int copies_per_thread = this->exp->copies_per_thread;
    int bytes_per_copy    = this->exp->bytes_per_copy;
    int bytes_per_pool    = this->exp->bytes_per_pool;

#if defined(MEM_CHK)
    bool copy_in_use[copies_per_thread];
    memset(copy_in_use, '\0', copies_per_thread * sizeof(bool));
    int total_duplicated=0;
#endif

				// we must set a lock because random()
				// is not thread safe
    Run::global_mutex.lock();
    setstate(this->exp->random_state[this->thread_id()]);
    int copy_factor = prime_table[ random() % prime_table_size ];
    int copy_offset = random() % copies_per_thread;
    Run::global_mutex.unlock();

				// loop through the copies
    for (int i=0; i < copies_per_thread; i++) {
	int copy = (copy_factor * i + copy_offset) % copies_per_thread;
	source[i] = source_pool + copy * bytes_per_copy;
#if defined(MEM_CHK)
	if (copy_in_use[copy]) {
	    fprintf(stderr, "Ugh! @ line %d in %s: duplicated %d (%d).\n", __LINE__, __FILE__, i, copy);
	    total_duplicated++;
	}
	copy_in_use[copy] = true;
#endif
    }

#if defined(MEM_CHK)
    if (0 < total_duplicated) {
	    fprintf(stderr, "Ugh! @ line %d in %s: total duplicated is %d.\n", __LINE__, __FILE__, total_duplicated);
    }
    int total_missed=0;
    for (int i=0; i < copies_per_thread; i++) {
	if (! copy_in_use[i]) {
	    fprintf(stderr, "Ugh! @ line %d in %s: missed %d.\n", __LINE__, __FILE__, i);
	    total_missed+=1;
	}
    }
    if (0 < total_missed) {
	    fprintf(stderr, "Ugh! @ line %d in %s: total missed is %d.\n", __LINE__, __FILE__, total_missed);
    }
#endif
}

void
Run::forward_mem_init( char** source, char* source_pool )
{
    int copies_per_thread = this->exp->copies_per_thread;
    int bytes_per_copy    = this->exp->bytes_per_copy;
    int bytes_per_pool    = this->exp->bytes_per_pool;

    for (int i=0; i < copies_per_thread; i++) {
	source[i] = source_pool + i * bytes_per_copy;
    }
}

void
Run::reverse_mem_init( char** source, char* source_pool )
{
    int copies_per_thread = this->exp->copies_per_thread;
    int bytes_per_copy    = this->exp->bytes_per_copy;
    int bytes_per_pool    = this->exp->bytes_per_pool;

    for (int i=0; i < copies_per_thread; i++) {
	source[i] = source_pool + ((copies_per_thread-1) - i) * bytes_per_copy;
    }
}

static int64_t
use_naive(double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target)
{
    if (seconds <= 0) {
	for (int i=0; i < iterations; i++) {
	    for (int j=0; j < copies_per_thread; j++) {
		char* s = source[j];
		char* t = target[j];
		for (int k=0; k < bytes_per_copy; k++) {
		    t[k] = s[k];
		}
	    }
	}
    } else {
	double start=Timer::seconds();
	for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
	    for (int j=0; j < copies_per_thread; j++) {
		char* s = source[j];
		char* t = target[j];
		for (int k=0; k < bytes_per_copy; k++) {
		    t[k] = s[k];
		}
	    }
	}
    }

    return iterations;
}

static int64_t
use_memcpy(double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target)
{
    if (seconds <= 0) {
	for (int i=0; i < iterations; i++) {
	    for (int j=0; j < copies_per_thread; j++) {
		char* s = source[j];
		char* t = target[j];
		memcpy(t, s, bytes_per_copy);
	    }
	}
    } else {
	double start=Timer::seconds();
	for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
	    for (int j=0; j < copies_per_thread; j++) {
		char* s = source[j];
		char* t = target[j];
		memcpy(t, s, bytes_per_copy);
	    }
	}
    }

    return iterations;
}

static int64_t
use_memmove(double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target)
{
    if (seconds <= 0) {
	for (int i=0; i < iterations; i++) {
	    for (int j=0; j < copies_per_thread; j++) {
		char* s = source[j];
		char* t = target[j];
		memmove(t, s, bytes_per_copy);
	    }
	}
    } else {
	double start=Timer::seconds();
	for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
	    for (int j=0; j < copies_per_thread; j++) {
		char* s = source[j];
		char* t = target[j];
		memmove(t, s, bytes_per_copy);
	    }
	}
    }

    return iterations;
}

static int64_t
use_bcopy(double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target)
{
    if (seconds <= 0) {
	for (int i=0; i < iterations; i++) {
	    for (int j=0; j < copies_per_thread; j++) {
		char* s = source[j];
		char* t = target[j];
		bcopy(s, t, bytes_per_copy);
	    }
	}
    } else {
	double start=Timer::seconds();
	for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
	    for (int j=0; j < copies_per_thread; j++) {
		char* s = source[j];
		char* t = target[j];
		bcopy(s, t, bytes_per_copy);
	    }
	}
    }

    return iterations;
}

static int64_t
use_copy_tt(double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target)
{
    if (seconds <= 0) {
	for (int i=0; i < iterations; i++) {
	    for (int j=0; j < copies_per_thread; j++) {
		copy_tt(target[j], source[j], bytes_per_copy);
	    }
	}
    } else {
	double start=Timer::seconds();
	for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
	    for (int j=0; j < copies_per_thread; j++) {
		copy_tt(target[j], source[j], bytes_per_copy);
	    }
	}
    }

    return iterations;
}

static int64_t
use_copy_tn(double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target)
{
    if (seconds <= 0) {
	for (int i=0; i < iterations; i++) {
	    for (int j=0; j < copies_per_thread; j++) {
		copy_tn(target[j], source[j], bytes_per_copy);
	    }
	}
    } else {
	double start=Timer::seconds();
	for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
	    for (int j=0; j < copies_per_thread; j++) {
		copy_tn(target[j], source[j], bytes_per_copy);
	    }
	}
    }

    return iterations;
}


static int64_t
use_naive_zero(double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target)
{
    if (seconds <= 0) {
	for (int i=0; i < iterations; i++) {
	    for (int j=0; j < copies_per_thread; j++) {
		char* t = target[j];
		for (int k=0; k < bytes_per_copy; k++) {
		    t[k] = '\0';
		}
	    }
	}
    } else {
	double start=Timer::seconds();
	for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
	    for (int j=0; j < copies_per_thread; j++) {
		char* t = target[j];
		for (int k=0; k < bytes_per_copy; k++) {
		    t[k] = '\0';
		}
	    }
	}
    }

    return iterations;
}

static int64_t
use_memset(double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target)
{
    if (seconds <= 0) {
	for (int i=0; i < iterations; i++) {
	    for (int j=0; j < copies_per_thread; j++) {
		char* t = target[j];
		memset(t, '\0', bytes_per_copy);
	    }
	}
    } else {
	double start=Timer::seconds();
	for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
	    for (int j=0; j < copies_per_thread; j++) {
		char* t = target[j];
		memset(t, '\0', bytes_per_copy);
	    }
	}
    }

    return iterations;
}

static int64_t
use_bzero(double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target)
{
    if (seconds <= 0) {
	for (int i=0; i < iterations; i++) {
	    for (int j=0; j < copies_per_thread; j++) {
		char* t = target[j];
		bzero(t, bytes_per_copy);
	    }
	}
    } else {
	double start=Timer::seconds();
	for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
	    for (int j=0; j < copies_per_thread; j++) {
		char* t = target[j];
		bzero(t, bytes_per_copy);
	    }
	}
    }

    return iterations;
}

static int64_t
use_xtta(double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target)
{
    if (seconds <= 0) {
	for (int i=0; i < iterations; i++) {
	    for (int j=0; j < copies_per_thread; j++) {
		xtta(target[j], source[j], bytes_per_copy);
	    }
	}
    } else {
	double start=Timer::seconds();
	for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
	    for (int j=0; j < copies_per_thread; j++) {
		xtta(target[j], source[j], bytes_per_copy);
	    }
	}
    }

    return iterations;
}

static int64_t
use_xttu(double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target)
{
    if (seconds <= 0) {
	for (int i=0; i < iterations; i++) {
	    for (int j=0; j < copies_per_thread; j++) {
		xttu(target[j], source[j], bytes_per_copy);
	    }
	}
    } else {
	double start=Timer::seconds();
	for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
	    for (int j=0; j < copies_per_thread; j++) {
		xttu(target[j], source[j], bytes_per_copy);
	    }
	}
    }

    return iterations;
}

static int64_t
use_xtn(double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target)
{
    if (seconds <= 0) {
	for (int i=0; i < iterations; i++) {
	    for (int j=0; j < copies_per_thread; j++) {
		xtn(target[j], source[j], bytes_per_copy);
	    }
	}
    } else {
	double start=Timer::seconds();
	for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
	    for (int j=0; j < copies_per_thread; j++) {
		xtn(target[j], source[j], bytes_per_copy);
	    }
	}
    }

    return iterations;
}

static int64_t
use_xnt(double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target)
{
    if (seconds <= 0) {
	for (int i=0; i < iterations; i++) {
	    for (int j=0; j < copies_per_thread; j++) {
		xnt(target[j], source[j], bytes_per_copy);
	    }
	}
    } else {
	double start=Timer::seconds();
	for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
	    for (int j=0; j < copies_per_thread; j++) {
		xnt(target[j], source[j], bytes_per_copy);
	    }
	}
    }

    return iterations;
}

static int64_t
use_xnn(double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target)
{
    if (seconds <= 0) {
	for (int i=0; i < iterations; i++) {
	    for (int j=0; j < copies_per_thread; j++) {
		xnn(target[j], source[j], bytes_per_copy);
	    }
	}
    } else {
	double start=Timer::seconds();
	for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
	    for (int j=0; j < copies_per_thread; j++) {
		xnn(target[j], source[j], bytes_per_copy);
	    }
	}
    }

    return iterations;
}

static int64_t
use_memread(double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target)
{
    if (seconds <= 0) {
	for (int i=0; i < iterations; i++) {
	    for (int j=0; j < copies_per_thread; j++) {
		memread(target[j], source[j], bytes_per_copy);
	    }
	}
    } else {
	double start=Timer::seconds();
	for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
	    for (int j=0; j < copies_per_thread; j++) {
		memread(target[j], source[j], bytes_per_copy);
	    }
	}
    }

    return iterations;
}

static int64_t
use_memwrite(double seconds, int64_t iterations, int64_t copies_per_thread, int64_t bytes_per_copy, char** source, char** target)
{
    if (seconds <= 0) {
	for (int i=0; i < iterations; i++) {
	    for (int j=0; j < copies_per_thread; j++) {
		memwrite(target[j], source[j], bytes_per_copy);
	    }
	}
    } else {
	double start=Timer::seconds();
	for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
	    for (int j=0; j < copies_per_thread; j++) {
		memwrite(target[j], source[j], bytes_per_copy);
	    }
	}
    }

    return iterations;
}

void 
intel_memcpy(char* b, const char* a, size_t n)
{
    char *s1 = b;
    const char *s2 = a;
    for(; 0<n; --n)*s1++ = *s2++;
}

#if defined(ID)
const char* Run_C = "\0@ID " ID;
#endif
