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
#include "SpinBarrier.h"
#include "Timer.h"

#include "sync.h"


static inline double max( double v1, double v2 ) { if (v1 < v2) return v2; return v1; } 
static inline double min( double v1, double v2 ) { if (v2 < v1) return v2; return v1; }

static int64_t use_pthread_spin (double seconds, int64_t iterations, int64_t count, Run::lock_u list);
static int64_t use_pthread_mutex(double seconds, int64_t iterations, int64_t count, Run::lock_u list);
static int64_t use_cmpxchg_spin (double seconds, int64_t iterations, int64_t count, Run::lock_u list);
static int64_t use_cmpxchg32    (double seconds, int64_t iterations, int64_t count, Run::lock_u list);
static int64_t use_cmpxchg64    (double seconds, int64_t iterations, int64_t count, Run::lock_u list);
static int64_t use_fairspin     (double seconds, int64_t iterations, int64_t count, Run::lock_u list);
static int64_t use_atomic32     (double seconds, int64_t iterations, int64_t count, Run::lock_u list);
static int64_t use_atomic64     (double seconds, int64_t iterations, int64_t count, Run::lock_u list);
static int64_t use_reader       (double seconds, int64_t iterations, int64_t count, Run::lock_u list);
static int64_t use_writer       (double seconds, int64_t iterations, int64_t count, Run::lock_u list);
static int64_t (*run_benchmark) (double seconds, int64_t iterations, int64_t count, Run::lock_u list) = use_pthread_spin;

Lock     Run::global_mutex;
double   Run::_seconds       = 1E9;

Run::Run()
: exp(NULL), bp(NULL)
{
}

Run::~Run()
{
}

				// this->exp->thread_domain[ this->thread_id() ]
				// this->exp->locks_per_thread
				// this->exp->lock_method
				// this->exp->iterations
				// this->exp->experiments
				// this->exp->seconds
int
Run::run()
{
// printf("Run::run(): enter\n");fflush(stdout);
				// establish the node id where this thread
				// will run. threads are mapped to nodes
				// by the set-up code for Experiment.
    int run_node_id = this->exp->thread_domain[this->thread_id()];
    this->run_on_node(run_node_id);

    run_benchmark = use_pthread_spin;
    switch (this->exp->lock_method) {
    case Experiment::PTHREAD_SPIN:
	run_benchmark = use_pthread_spin;
	break;
    case Experiment::PTHREAD_MUTEX:
	run_benchmark = use_pthread_mutex;
	break;
    case Experiment::CMPXSPIN:
	run_benchmark = use_cmpxchg_spin;
	break;
    case Experiment::CMPXCHG32:
	run_benchmark = use_cmpxchg32;
	break;
    case Experiment::CMPXCHG64:
	run_benchmark = use_cmpxchg64;
	break;
    case Experiment::FAIRSPIN:
	run_benchmark = use_fairspin;
	break;
    case Experiment::ATOMIC32:
	run_benchmark = use_atomic32;
	break;
    case Experiment::ATOMIC64:
	run_benchmark = use_atomic64;
	break;
    case Experiment::PTHREAD_READER:
	run_benchmark = use_reader;
	break;
    case Experiment::PTHREAD_WRITER:
	run_benchmark = use_writer;
	break;
    }

				// barrier
    for (int e=0; e < this->exp->experiments; e++) {
	this->bp->barrier();

				// start timer
	double start = 0;
	if (this->thread_id() == 0) start = Timer::seconds();
	this->bp->barrier();

				// exercise the locks
	int64_t my_iters = run_benchmark(this->exp->seconds, this->exp->iterations, this->exp->locks_per_thread, this->locks);

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

// printf("Run::run(): leave seconds=%lf iterations=%ld total=%ld time per op=%.2lf ns\n", 
// Run::_seconds, this->exp->iterations, this->exp->iterations*this->exp->locks_per_thread, 
// 1E9*Run::_seconds/(this->exp->iterations*this->exp->locks_per_thread));fflush(stdout);

    return 0;
}

static int64_t
use_pthread_spin (double seconds, int64_t iterations, int64_t count, Run::lock_u list)
{
#define unroll_factor 8
    pthread_spinlock_t** locks = list.pt_spin;
    if (seconds <= 0) {
	for (int i=0; i < iterations; i++) {
	    for (int j=0; j+(unroll_factor-1) < count; j+=unroll_factor) {
		pthread_spin_lock  (locks[j+0]);
		pthread_spin_unlock(locks[j+0]); 
		pthread_spin_lock  (locks[j+1]);
		pthread_spin_unlock(locks[j+1]); 
		pthread_spin_lock  (locks[j+2]);
		pthread_spin_unlock(locks[j+2]); 
		pthread_spin_lock  (locks[j+3]);
		pthread_spin_unlock(locks[j+3]); 
		pthread_spin_lock  (locks[j+4]);
		pthread_spin_unlock(locks[j+4]); 
		pthread_spin_lock  (locks[j+5]);
		pthread_spin_unlock(locks[j+5]); 
		pthread_spin_lock  (locks[j+6]);
		pthread_spin_unlock(locks[j+6]); 
		pthread_spin_lock  (locks[j+7]);
		pthread_spin_unlock(locks[j+7]);
	    }
	    int excess = (count/unroll_factor)*unroll_factor;
	    for (int j=excess; j < count; j++) {
		pthread_spin_lock  (locks[j]);
		pthread_spin_unlock(locks[j]);
/*
*/
	    }
	}
    } else {
	double start=Timer::seconds();
	for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
	    for (int j=0; j+(unroll_factor-1) < count; j+=unroll_factor) {
		pthread_spin_lock  (locks[j+0]);
		pthread_spin_unlock(locks[j+0]); 
		pthread_spin_lock  (locks[j+1]);
		pthread_spin_unlock(locks[j+1]); 
		pthread_spin_lock  (locks[j+2]);
		pthread_spin_unlock(locks[j+2]); 
		pthread_spin_lock  (locks[j+3]);
		pthread_spin_unlock(locks[j+3]); 
		pthread_spin_lock  (locks[j+4]);
		pthread_spin_unlock(locks[j+4]); 
		pthread_spin_lock  (locks[j+5]);
		pthread_spin_unlock(locks[j+5]); 
		pthread_spin_lock  (locks[j+6]);
		pthread_spin_unlock(locks[j+6]); 
		pthread_spin_lock  (locks[j+7]);
		pthread_spin_unlock(locks[j+7]);
	    }
	    int excess = (count/unroll_factor)*unroll_factor;
	    for (int j=excess; j < count; j++) {
		pthread_spin_lock  (locks[j]);
		pthread_spin_unlock(locks[j]);
/*
*/
	    }
	}
    }

    return iterations;
}

static int64_t
use_pthread_mutex(double seconds, int64_t iterations, int64_t count, Run::lock_u list)
{
#define unroll_factor 8
    pthread_mutex_t** locks = list.pt_mutex;
    if (seconds <= 0) {
	for (int i=0; i < iterations; i++) {
	    for (int j=0; j+(unroll_factor-1) < count; j+=unroll_factor) {
		pthread_mutex_lock  (locks[j+0]);
		pthread_mutex_unlock(locks[j+0]); 
		pthread_mutex_lock  (locks[j+1]);
		pthread_mutex_unlock(locks[j+1]); 
		pthread_mutex_lock  (locks[j+2]);
		pthread_mutex_unlock(locks[j+2]); 
		pthread_mutex_lock  (locks[j+3]);
		pthread_mutex_unlock(locks[j+3]); 
		pthread_mutex_lock  (locks[j+4]);
		pthread_mutex_unlock(locks[j+4]); 
		pthread_mutex_lock  (locks[j+5]);
		pthread_mutex_unlock(locks[j+5]); 
		pthread_mutex_lock  (locks[j+6]);
		pthread_mutex_unlock(locks[j+6]); 
		pthread_mutex_lock  (locks[j+7]);
		pthread_mutex_unlock(locks[j+7]);
	    }
	    int excess = (count/unroll_factor)*unroll_factor;
	    for (int j=excess; j < count; j++) {
		pthread_mutex_lock  (locks[j]);
		pthread_mutex_unlock(locks[j]);
/*
*/
	    }
	}
    } else {
	double start=Timer::seconds();
	for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
	    for (int j=0; j+(unroll_factor-1) < count; j+=unroll_factor) {
		pthread_mutex_lock  (locks[j+0]);
		pthread_mutex_unlock(locks[j+0]); 
		pthread_mutex_lock  (locks[j+1]);
		pthread_mutex_unlock(locks[j+1]); 
		pthread_mutex_lock  (locks[j+2]);
		pthread_mutex_unlock(locks[j+2]); 
		pthread_mutex_lock  (locks[j+3]);
		pthread_mutex_unlock(locks[j+3]); 
		pthread_mutex_lock  (locks[j+4]);
		pthread_mutex_unlock(locks[j+4]); 
		pthread_mutex_lock  (locks[j+5]);
		pthread_mutex_unlock(locks[j+5]); 
		pthread_mutex_lock  (locks[j+6]);
		pthread_mutex_unlock(locks[j+6]); 
		pthread_mutex_lock  (locks[j+7]);
		pthread_mutex_unlock(locks[j+7]);
	    }
	    int excess = (count/unroll_factor)*unroll_factor;
	    for (int j=excess; j < count; j++) {
		pthread_mutex_lock  (locks[j]);
		pthread_mutex_unlock(locks[j]);
/*
*/
	    }
	}
    }

    return iterations;
}

static int64_t
use_cmpxchg_spin(double seconds, int64_t iterations, int64_t count, Run::lock_u list)
{
#define unroll_factor 8
    lock8_t** locks = list.lock8;
    if (seconds <= 0) {
	for (int i=0; i < iterations; i++) {
	    for (int j=0; j+(unroll_factor-1) < count; j+=unroll_factor) {
		spin_lock_u8_set(locks[j+0]);
		spin_lock_u8_clr(locks[j+0]);
		spin_lock_u8_set(locks[j+1]);
		spin_lock_u8_clr(locks[j+1]);
		spin_lock_u8_set(locks[j+2]);
		spin_lock_u8_clr(locks[j+2]);
		spin_lock_u8_set(locks[j+3]);
		spin_lock_u8_clr(locks[j+3]);
		spin_lock_u8_set(locks[j+4]);
		spin_lock_u8_clr(locks[j+4]);
		spin_lock_u8_set(locks[j+5]);
		spin_lock_u8_clr(locks[j+5]);
		spin_lock_u8_set(locks[j+6]);
		spin_lock_u8_clr(locks[j+6]);
		spin_lock_u8_set(locks[j+7]);
		spin_lock_u8_clr(locks[j+7]);
	    }
	    int excess = (count/unroll_factor)*unroll_factor;
	    for (int j=excess; j < count; j++) {
		spin_lock_u8_set(locks[j]);
		spin_lock_u8_clr(locks[j]);
/*
*/
	    }
	}
    } else {
	double start=Timer::seconds();
	for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
	    for (int j=0; j+(unroll_factor-1) < count; j+=unroll_factor) {
		spin_lock_u8_set(locks[j+0]);
		spin_lock_u8_clr(locks[j+0]);
		spin_lock_u8_set(locks[j+1]);
		spin_lock_u8_clr(locks[j+1]);
		spin_lock_u8_set(locks[j+2]);
		spin_lock_u8_clr(locks[j+2]);
		spin_lock_u8_set(locks[j+3]);
		spin_lock_u8_clr(locks[j+3]);
		spin_lock_u8_set(locks[j+4]);
		spin_lock_u8_clr(locks[j+4]);
		spin_lock_u8_set(locks[j+5]);
		spin_lock_u8_clr(locks[j+5]);
		spin_lock_u8_set(locks[j+6]);
		spin_lock_u8_clr(locks[j+6]);
		spin_lock_u8_set(locks[j+7]);
		spin_lock_u8_clr(locks[j+7]);
	    }
	    int excess = (count/unroll_factor)*unroll_factor;
	    for (int j=excess; j < count; j++) {
		spin_lock_u8_set(locks[j]);
		spin_lock_u8_clr(locks[j]);
/*
*/
	    }
	}
    }

    return iterations;
}

static int64_t
use_cmpxchg32(double seconds, int64_t iterations, int64_t count, Run::lock_u list)
{
#define unroll_factor 8
    uint32_t** locks = list.uint32;
    if (seconds <= 0) {
	for (int i=0; i < iterations; i++) {
	    for (int j=0; j+(unroll_factor-1) < count; j+=unroll_factor) {
		x86_cmpxchg_u32(locks[j+0], 0, 0);
		x86_cmpxchg_u32(locks[j+1], 0, 0);
		x86_cmpxchg_u32(locks[j+2], 0, 0);
		x86_cmpxchg_u32(locks[j+3], 0, 0);
		x86_cmpxchg_u32(locks[j+4], 0, 0);
		x86_cmpxchg_u32(locks[j+5], 0, 0);
		x86_cmpxchg_u32(locks[j+6], 0, 0);
		x86_cmpxchg_u32(locks[j+7], 0, 0);
	    }
	    int excess = (count/unroll_factor)*unroll_factor;
	    for (int j=excess; j < count; j++) {
		x86_cmpxchg_u32(locks[j], 0, 0);
/*
*/
	    }
	}
    } else {
	double start=Timer::seconds();
	for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
	    for (int j=0; j+(unroll_factor-1) < count; j+=unroll_factor) {
		x86_cmpxchg_u32(locks[j+0], 0, 0);
		x86_cmpxchg_u32(locks[j+1], 0, 0);
		x86_cmpxchg_u32(locks[j+2], 0, 0);
		x86_cmpxchg_u32(locks[j+3], 0, 0);
		x86_cmpxchg_u32(locks[j+4], 0, 0);
		x86_cmpxchg_u32(locks[j+5], 0, 0);
		x86_cmpxchg_u32(locks[j+6], 0, 0);
		x86_cmpxchg_u32(locks[j+7], 0, 0);
	    }
	    int excess = (count/unroll_factor)*unroll_factor;
	    for (int j=excess; j < count; j++) {
		x86_cmpxchg_u32(locks[j], 0, 0);
/*
*/
	    }
	}
    }

    return iterations;
}

static int64_t
use_cmpxchg64(double seconds, int64_t iterations, int64_t count, Run::lock_u list)
{
#define unroll_factor 8
    uint64_t** locks = list.uint64;
    if (seconds <= 0) {
	for (int i=0; i < iterations; i++) {
	    for (int j=0; j+(unroll_factor-1) < count; j+=unroll_factor) {
		x86_cmpxchg_u64(locks[j+0], 0, 0);
		x86_cmpxchg_u64(locks[j+1], 0, 0);
		x86_cmpxchg_u64(locks[j+2], 0, 0);
		x86_cmpxchg_u64(locks[j+3], 0, 0);
		x86_cmpxchg_u64(locks[j+4], 0, 0);
		x86_cmpxchg_u64(locks[j+5], 0, 0);
		x86_cmpxchg_u64(locks[j+6], 0, 0);
		x86_cmpxchg_u64(locks[j+7], 0, 0);
	    }
	    int excess = (count/unroll_factor)*unroll_factor;
	    for (int j=excess; j < count; j++) {
		x86_cmpxchg_u64(locks[j], 0, 0);
/*
*/
	    }
	}
    } else {
	double start=Timer::seconds();
	for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
	    for (int j=0; j+(unroll_factor-1) < count; j+=unroll_factor) {
		x86_cmpxchg_u64(locks[j+0], 0, 0);
		x86_cmpxchg_u64(locks[j+1], 0, 0);
		x86_cmpxchg_u64(locks[j+2], 0, 0);
		x86_cmpxchg_u64(locks[j+3], 0, 0);
		x86_cmpxchg_u64(locks[j+4], 0, 0);
		x86_cmpxchg_u64(locks[j+5], 0, 0);
		x86_cmpxchg_u64(locks[j+6], 0, 0);
		x86_cmpxchg_u64(locks[j+7], 0, 0);
	    }
	    int excess = (count/unroll_factor)*unroll_factor;
	    for (int j=excess; j < count; j++) {
		x86_cmpxchg_u64(locks[j], 0, 0);
/*
*/
	    }
	}
    }

    return iterations;
}

static int64_t
use_fairspin(double seconds, int64_t iterations, int64_t count, Run::lock_u list)
{
#define unroll_factor 8
    fair_lock_t** locks = list.fair_lock;
    if (seconds <= 0) {
	for (int i=0; i < iterations; i++) {
	    for (int j=0; j+(unroll_factor-1) < count; j+=unroll_factor) {
		fair_lock_set(locks[j+0]);
		fair_lock_clr(locks[j+0]);
		fair_lock_set(locks[j+1]);
		fair_lock_clr(locks[j+1]);
		fair_lock_set(locks[j+2]);
		fair_lock_clr(locks[j+2]);
		fair_lock_set(locks[j+3]);
		fair_lock_clr(locks[j+3]);
		fair_lock_set(locks[j+4]);
		fair_lock_clr(locks[j+4]);
		fair_lock_set(locks[j+5]);
		fair_lock_clr(locks[j+5]);
		fair_lock_set(locks[j+6]);
		fair_lock_clr(locks[j+6]);
		fair_lock_set(locks[j+7]);
		fair_lock_clr(locks[j+7]);
	    }
	    int excess = (count/unroll_factor)*unroll_factor;
	    for (int j=excess; j < count; j++) {
		fair_lock_set(locks[j]);
		fair_lock_clr(locks[j]);
/*
*/
	    }
	}
    } else {
	double start=Timer::seconds();
	for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
	    for (int j=0; j+(unroll_factor-1) < count; j+=unroll_factor) {
		fair_lock_set(locks[j+0]);
		fair_lock_clr(locks[j+0]);
		fair_lock_set(locks[j+1]);
		fair_lock_clr(locks[j+1]);
		fair_lock_set(locks[j+2]);
		fair_lock_clr(locks[j+2]);
		fair_lock_set(locks[j+3]);
		fair_lock_clr(locks[j+3]);
		fair_lock_set(locks[j+4]);
		fair_lock_clr(locks[j+4]);
		fair_lock_set(locks[j+5]);
		fair_lock_clr(locks[j+5]);
		fair_lock_set(locks[j+6]);
		fair_lock_clr(locks[j+6]);
		fair_lock_set(locks[j+7]);
		fair_lock_clr(locks[j+7]);
	    }
	    int excess = (count/unroll_factor)*unroll_factor;
	    for (int j=excess; j < count; j++) {
		fair_lock_set(locks[j]);
		fair_lock_clr(locks[j]);
/*
*/
	    }
	}
    }

    return iterations;
}

static int64_t
use_atomic32(double seconds, int64_t iterations, int64_t count, Run::lock_u list)
{
#define unroll_factor 8
    atomic32_t** locks = list.atomic32;
    if (seconds <= 0) {
	for (int i=0; i < iterations; i++) {
	    for (int j=0; j+(unroll_factor-1) < count; j+=unroll_factor) {
		atomic_add_u32(locks[j+0], 1);
		atomic_add_u32(locks[j+1], 1);
		atomic_add_u32(locks[j+2], 1);
		atomic_add_u32(locks[j+3], 1);
		atomic_add_u32(locks[j+4], 1);
		atomic_add_u32(locks[j+5], 1);
		atomic_add_u32(locks[j+6], 1);
		atomic_add_u32(locks[j+7], 1);
	    }
	    int excess = (count/unroll_factor)*unroll_factor;
	    for (int j=excess; j < count; j++) {
		atomic_add_u32(locks[j], 1);
/*
*/
	    }
	}
    } else {
	double start=Timer::seconds();
	for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
	    for (int j=0; j+(unroll_factor-1) < count; j+=unroll_factor) {
		atomic_add_u32(locks[j+0], 1);
		atomic_add_u32(locks[j+1], 1);
		atomic_add_u32(locks[j+2], 1);
		atomic_add_u32(locks[j+3], 1);
		atomic_add_u32(locks[j+4], 1);
		atomic_add_u32(locks[j+5], 1);
		atomic_add_u32(locks[j+6], 1);
		atomic_add_u32(locks[j+7], 1);
	    }
	    int excess = (count/unroll_factor)*unroll_factor;
	    for (int j=excess; j < count; j++) {
		atomic_add_u32(locks[j], 1);
/*
*/
	    }
	}
    }

    return iterations;
}

static int64_t
use_atomic64(double seconds, int64_t iterations, int64_t count, Run::lock_u list)
{
#define unroll_factor 8
    atomic64_t** locks = list.atomic64;
    if (seconds <= 0) {
	for (int i=0; i < iterations; i++) {
	    for (int j=0; j+(unroll_factor-1) < count; j+=unroll_factor) {
		atomic_add_u64(locks[j+0], 1);
		atomic_add_u64(locks[j+1], 1);
		atomic_add_u64(locks[j+2], 1);
		atomic_add_u64(locks[j+3], 1);
		atomic_add_u64(locks[j+4], 1);
		atomic_add_u64(locks[j+5], 1);
		atomic_add_u64(locks[j+6], 1);
		atomic_add_u64(locks[j+7], 1);
	    }
	    int excess = (count/unroll_factor)*unroll_factor;
	    for (int j=excess; j < count; j++) {
		atomic_add_u64(locks[j], 1);
/*
*/
	    }
	}
    } else {
	double start=Timer::seconds();
	for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
	    for (int j=0; j+(unroll_factor-1) < count; j+=unroll_factor) {
		atomic_add_u64(locks[j+0], 1);
		atomic_add_u64(locks[j+1], 1);
		atomic_add_u64(locks[j+2], 1);
		atomic_add_u64(locks[j+3], 1);
		atomic_add_u64(locks[j+4], 1);
		atomic_add_u64(locks[j+5], 1);
		atomic_add_u64(locks[j+6], 1);
		atomic_add_u64(locks[j+7], 1);
	    }
	    int excess = (count/unroll_factor)*unroll_factor;
	    for (int j=excess; j < count; j++) {
		atomic_add_u64(locks[j], 1);
/*
*/
	    }
	}
    }

    return iterations;
}

static int64_t
use_reader (double seconds, int64_t iterations, int64_t count, Run::lock_u list)
{
#define unroll_factor 8
    pthread_rwlock_t** locks = list.pt_rwlock;
    if (seconds <= 0) {
	for (int i=0; i < iterations; i++) {
	    for (int j=0; j+(unroll_factor-1) < count; j+=unroll_factor) {
		pthread_rwlock_rdlock(locks[j+0]);
		pthread_rwlock_unlock(locks[j+0]); 
		pthread_rwlock_rdlock(locks[j+1]);
		pthread_rwlock_unlock(locks[j+1]); 
		pthread_rwlock_rdlock(locks[j+2]);
		pthread_rwlock_unlock(locks[j+2]); 
		pthread_rwlock_rdlock(locks[j+3]);
		pthread_rwlock_unlock(locks[j+3]); 
		pthread_rwlock_rdlock(locks[j+4]);
		pthread_rwlock_unlock(locks[j+4]); 
		pthread_rwlock_rdlock(locks[j+5]);
		pthread_rwlock_unlock(locks[j+5]); 
		pthread_rwlock_rdlock(locks[j+6]);
		pthread_rwlock_unlock(locks[j+6]); 
		pthread_rwlock_rdlock(locks[j+7]);
		pthread_rwlock_unlock(locks[j+7]);
	    }
	    int excess = (count/unroll_factor)*unroll_factor;
	    for (int j=excess; j < count; j++) {
		pthread_rwlock_rdlock(locks[j]);
		pthread_rwlock_unlock(locks[j]);
/*
*/
	    }
	}
    } else {
	double start=Timer::seconds();
	for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
	    for (int j=0; j+(unroll_factor-1) < count; j+=unroll_factor) {
		pthread_rwlock_rdlock(locks[j+0]);
		pthread_rwlock_unlock(locks[j+0]); 
		pthread_rwlock_rdlock(locks[j+1]);
		pthread_rwlock_unlock(locks[j+1]); 
		pthread_rwlock_rdlock(locks[j+2]);
		pthread_rwlock_unlock(locks[j+2]); 
		pthread_rwlock_rdlock(locks[j+3]);
		pthread_rwlock_unlock(locks[j+3]); 
		pthread_rwlock_rdlock(locks[j+4]);
		pthread_rwlock_unlock(locks[j+4]); 
		pthread_rwlock_rdlock(locks[j+5]);
		pthread_rwlock_unlock(locks[j+5]); 
		pthread_rwlock_rdlock(locks[j+6]);
		pthread_rwlock_unlock(locks[j+6]); 
		pthread_rwlock_rdlock(locks[j+7]);
		pthread_rwlock_unlock(locks[j+7]);
	    }
	    int excess = (count/unroll_factor)*unroll_factor;
	    for (int j=excess; j < count; j++) {
		pthread_rwlock_rdlock(locks[j]);
		pthread_rwlock_unlock(locks[j]);
/*
*/
	    }
	}
    }

    return iterations;
}

static int64_t
use_writer (double seconds, int64_t iterations, int64_t count, Run::lock_u list)
{
#define unroll_factor 8
    pthread_rwlock_t** locks = list.pt_rwlock;
    if (seconds <= 0) {
	for (int i=0; i < iterations; i++) {
	    for (int j=0; j+(unroll_factor-1) < count; j+=unroll_factor) {
		pthread_rwlock_wrlock(locks[j+0]);
		pthread_rwlock_unlock(locks[j+0]); 
		pthread_rwlock_wrlock(locks[j+1]);
		pthread_rwlock_unlock(locks[j+1]); 
		pthread_rwlock_wrlock(locks[j+2]);
		pthread_rwlock_unlock(locks[j+2]); 
		pthread_rwlock_wrlock(locks[j+3]);
		pthread_rwlock_unlock(locks[j+3]); 
		pthread_rwlock_wrlock(locks[j+4]);
		pthread_rwlock_unlock(locks[j+4]); 
		pthread_rwlock_wrlock(locks[j+5]);
		pthread_rwlock_unlock(locks[j+5]); 
		pthread_rwlock_wrlock(locks[j+6]);
		pthread_rwlock_unlock(locks[j+6]); 
		pthread_rwlock_wrlock(locks[j+7]);
		pthread_rwlock_unlock(locks[j+7]);
	    }
	    int excess = (count/unroll_factor)*unroll_factor;
	    for (int j=excess; j < count; j++) {
		pthread_rwlock_wrlock(locks[j]);
		pthread_rwlock_unlock(locks[j]);
/*
*/
	    }
	}
    } else {
	double start=Timer::seconds();
	for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
	    for (int j=0; j+(unroll_factor-1) < count; j+=unroll_factor) {
		pthread_rwlock_wrlock(locks[j+0]);
		pthread_rwlock_unlock(locks[j+0]); 
		pthread_rwlock_wrlock(locks[j+1]);
		pthread_rwlock_unlock(locks[j+1]); 
		pthread_rwlock_wrlock(locks[j+2]);
		pthread_rwlock_unlock(locks[j+2]); 
		pthread_rwlock_wrlock(locks[j+3]);
		pthread_rwlock_unlock(locks[j+3]); 
		pthread_rwlock_wrlock(locks[j+4]);
		pthread_rwlock_unlock(locks[j+4]); 
		pthread_rwlock_wrlock(locks[j+5]);
		pthread_rwlock_unlock(locks[j+5]); 
		pthread_rwlock_wrlock(locks[j+6]);
		pthread_rwlock_unlock(locks[j+6]); 
		pthread_rwlock_wrlock(locks[j+7]);
		pthread_rwlock_unlock(locks[j+7]);
	    }
	    int excess = (count/unroll_factor)*unroll_factor;
	    for (int j=excess; j < count; j++) {
		pthread_rwlock_wrlock(locks[j]);
		pthread_rwlock_unlock(locks[j]);
/*
*/
	    }
	}
    }

    return iterations;
}

#if defined(ID)
const char* Run_C = "\0@ID " ID;
#endif
