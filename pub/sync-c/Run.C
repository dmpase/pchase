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

static int64_t use_pthread_spin   (double seconds, int64_t iterations, Run::lock_u list, uint64_t delay);
static int64_t use_pthread_mutex  (double seconds, int64_t iterations, Run::lock_u list, uint64_t delay);
static int64_t use_cmpxchg_spin   (double seconds, int64_t iterations, Run::lock_u list, uint64_t delay);
static int64_t use_cmpxchg32      (double seconds, int64_t iterations, Run::lock_u list, uint64_t delay);
static int64_t use_cmpxchg64      (double seconds, int64_t iterations, Run::lock_u list, uint64_t delay);
static int64_t use_fairspin       (double seconds, int64_t iterations, Run::lock_u list, uint64_t delay);
static int64_t use_atomic32       (double seconds, int64_t iterations, Run::lock_u list, uint64_t delay);
static int64_t use_atomic64       (double seconds, int64_t iterations, Run::lock_u list, uint64_t delay);
static int64_t use_reader         (double seconds, int64_t iterations, Run::lock_u list, uint64_t delay);
static int64_t use_writer         (double seconds, int64_t iterations, Run::lock_u list, uint64_t delay);
static int64_t use_pthread_barrier(double seconds, int64_t iterations, Run::lock_u list, uint64_t delay);
static int64_t (*run_benchmark)   (double seconds, int64_t iterations, Run::lock_u list, uint64_t delay) = use_pthread_spin;

Lock    Run::global_mutex;
double  Run::_seconds         = 1E9;
int64_t Run::_max_iters       = 0;
int64_t Run::_min_iters       = (int64_t)1<<50;
int64_t Run::_total_iters     = 0;

double  Run::_exp_delta       = 1E9;
int64_t Run::_exp_max_iters   = 0;
int64_t Run::_exp_min_iters   = (int64_t)1<<50;
int64_t Run::_exp_total_iters = 0;

Run::Run()
: exp(NULL), bp(NULL)
{
}

Run::~Run()
{
}

				// this->exp->thread_domain[ this->thread_id() ]
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
    case Experiment::PTHREAD_BARRIER:
	run_benchmark = use_pthread_barrier;
	break;
    }

    for (int e=0; e < this->exp->experiments; e++) {
        Run::global_mutex.lock();
            Run::_exp_delta       = 0;
            Run::_exp_max_iters   = 0;
            Run::_exp_min_iters   = (int64_t)1<<50;
            Run::_exp_total_iters = 0;
        Run::global_mutex.unlock();

	this->bp->barrier();    // barrier 1

                                // NOTE: it may seem a bit odd to use:
                                //     start timer
                                //     barrier
                                //     run benchmark
                                //     barrier
                                //     stop timer
                                // 
                                // the code is structured in this way 
                                // to include an appropriate penalty 
                                // if two threads happen to be scheduled
                                // sequentially rather than in parallel.
                                // this helps systems that have more
                                // cores and greater memory bandwidth.
                                // if the barriers are removed, that
                                // penalty goes away.

				// start timer
	double start = Timer::seconds();
	this->bp->barrier();    // barrier 2
				// run benchmark
	int64_t iterations = run_benchmark(this->exp->seconds, this->exp->iterations, this->locks, this->exp->delay);
	this->bp->barrier();    // barrier 3
				// stop timer
	double stop = Timer::seconds();

        double delta = stop - start;
        Run::global_mutex.lock();
            if (0 < iterations && 0 < delta) {
                Run::_exp_delta       = (Run::_exp_delta     < delta     ) ? delta      : Run::_exp_delta    ;
                Run::_exp_max_iters   = (Run::_exp_max_iters < iterations) ? iterations : Run::_exp_max_iters;
                Run::_exp_min_iters   = (Run::_exp_min_iters < iterations) ? Run::_exp_min_iters : iterations;
                Run::_exp_total_iters += iterations;
            }
        Run::global_mutex.unlock();

	this->bp->barrier();    // barrier 3.5

        Run::global_mutex.lock();
            if (0 < Run::_exp_delta && 0 < Run::_exp_total_iters) {
                if (Run::_total_iters/Run::_seconds < Run::_exp_total_iters/Run::_exp_delta) {
                    Run::_seconds     = Run::_exp_delta;
                    Run::_max_iters   = Run::_exp_max_iters;
                    Run::_min_iters   = Run::_exp_min_iters;
                    Run::_total_iters = Run::_exp_total_iters;
                }
            }
        Run::global_mutex.unlock();
    }

    this->bp->barrier();        // barrier 4

// printf("Run::run(): leave seconds=%lf iterations=%ld total=%ld time per op=%.2lf ns\n", 
// Run::_seconds, this->exp->iterations, this->exp->iterations*this->exp->locks_per_thread, 
// 1E9*Run::_seconds/(this->exp->iterations*this->exp->locks_per_thread));fflush(stdout);

    return 0;
}

#define UNROLL_FACTOR 8

static int64_t
use_pthread_spin (double seconds, int64_t iterations, Run::lock_u list, uint64_t delay)
{
    pthread_spinlock_t** locks = list.pt_spin;
    if (delay <= 0) {
        if (seconds <= 0) {
            for (int i=0; i < iterations; i++) {
                pthread_spin_lock  (locks[0]);
                pthread_spin_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
            }
        } else {
            double start=Timer::seconds();
            for (iterations=0; (Timer::seconds()-start) < seconds; iterations+=UNROLL_FACTOR) {
                pthread_spin_lock  (locks[0]);
                pthread_spin_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_spin_lock  (locks[0]);
                pthread_spin_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_spin_lock  (locks[0]);
                pthread_spin_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_spin_lock  (locks[0]);
                pthread_spin_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_spin_lock  (locks[0]);
                pthread_spin_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_spin_lock  (locks[0]);
                pthread_spin_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_spin_lock  (locks[0]);
                pthread_spin_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_spin_lock  (locks[0]);
                pthread_spin_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
            }
        }
    } else {
        if (seconds <= 0) {
            for (int i=0; i < iterations; i++) {
                pthread_spin_lock  (locks[0]);
                uint64_t t0=Timer::ticks();
                while ((Timer::ticks()-t0) < delay)
                    ;
                pthread_spin_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
            }
        } else {
            double start=Timer::seconds();
            for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
                pthread_spin_lock  (locks[0]);
                uint64_t t0=Timer::ticks();
                while ((Timer::ticks()-t0) < delay)
                    ;
                pthread_spin_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
            }
        }
    }

    return iterations;
}

static int64_t
use_pthread_mutex(double seconds, int64_t iterations, Run::lock_u list, uint64_t delay)
{
    pthread_mutex_t** locks = list.pt_mutex;

    if (delay <= 0) {
        if (seconds <= 0) {
            for (int i=0; i < iterations; i++) {
                pthread_mutex_lock  (locks[0]);
                pthread_mutex_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
            }
        } else {
            double start=Timer::seconds();
            for (iterations=0; (Timer::seconds()-start) < seconds; iterations+=UNROLL_FACTOR) {
                pthread_mutex_lock  (locks[0]);
                pthread_mutex_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_mutex_lock  (locks[0]);
                pthread_mutex_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_mutex_lock  (locks[0]);
                pthread_mutex_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_mutex_lock  (locks[0]);
                pthread_mutex_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_mutex_lock  (locks[0]);
                pthread_mutex_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_mutex_lock  (locks[0]);
                pthread_mutex_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_mutex_lock  (locks[0]);
                pthread_mutex_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_mutex_lock  (locks[0]);
                pthread_mutex_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
            }
        }
    } else {
        if (seconds <= 0) {
            for (int i=0; i < iterations; i++) {
                pthread_mutex_lock  (locks[0]);
                uint64_t t0=Timer::ticks();
                while ((Timer::ticks()-t0) < delay)
                    ;
                pthread_mutex_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
            }
        } else {
            double start=Timer::seconds();
            for (iterations=0; (Timer::seconds()-start) < seconds; iterations++) {
                pthread_mutex_lock  (locks[0]);
                uint64_t t0=Timer::ticks();
                while ((Timer::ticks()-t0) < delay)
                    ;
                pthread_mutex_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
            }
        }
    }

    return iterations;
}

static int64_t
use_cmpxchg_spin(double seconds, int64_t iterations, Run::lock_u list, uint64_t delay)
{
    lock8_t** locks = list.lock8;
    if (delay <= 0) {
        if (seconds <= 0) {
            for (int i=0; i < iterations; i++) {
                spin_lock_u8_set(locks[0]);
                spin_lock_u8_clr(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
            }
        } else {
            double start=Timer::seconds();
            for (iterations=0; (Timer::seconds()-start) < seconds; iterations+=UNROLL_FACTOR) {
                spin_lock_u8_set(locks[0]);
                spin_lock_u8_clr(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                spin_lock_u8_set(locks[0]);
                spin_lock_u8_clr(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                spin_lock_u8_set(locks[0]);
                spin_lock_u8_clr(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                spin_lock_u8_set(locks[0]);
                spin_lock_u8_clr(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                spin_lock_u8_set(locks[0]);
                spin_lock_u8_clr(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                spin_lock_u8_set(locks[0]);
                spin_lock_u8_clr(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                spin_lock_u8_set(locks[0]);
                spin_lock_u8_clr(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                spin_lock_u8_set(locks[0]);
                spin_lock_u8_clr(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
            }
        }
    } else {
        if (seconds <= 0) {
            for (int i=0; i < iterations; i++) {
                spin_lock_u8_set(locks[0]);
                uint64_t t0=Timer::ticks();
                while ((Timer::ticks()-t0) < delay)
                    ;
                spin_lock_u8_clr(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
            }
        } else {
            double start=Timer::seconds();
            for (iterations=0; (Timer::seconds()-start) < seconds; iterations+=1) {
                spin_lock_u8_set(locks[0]);
                uint64_t t0=Timer::ticks();
                while ((Timer::ticks()-t0) < delay)
                    ;
                spin_lock_u8_clr(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
            }
        }
    }

    return iterations;
}

static int64_t
use_cmpxchg32(double seconds, int64_t iterations, Run::lock_u list, uint64_t delay)
{
    uint32_t** locks = list.uint32;
    if (delay <= 0) {
        if (seconds <= 0) {
            for (int i=0; i < iterations; i++) {
                x86_cmpxchg_u32(locks[0], 0, 0);
            }
        } else {
            double start=Timer::seconds();
            for (iterations=0; (Timer::seconds()-start) < seconds; iterations+=UNROLL_FACTOR) {
                x86_cmpxchg_u32(locks[0], 0, 0);
                x86_cmpxchg_u32(locks[0], 0, 0);
                x86_cmpxchg_u32(locks[0], 0, 0);
                x86_cmpxchg_u32(locks[0], 0, 0);
                x86_cmpxchg_u32(locks[0], 0, 0);
                x86_cmpxchg_u32(locks[0], 0, 0);
                x86_cmpxchg_u32(locks[0], 0, 0);
                x86_cmpxchg_u32(locks[0], 0, 0);
            }
        }
    } else {
        if (seconds <= 0) {
            for (int i=0; i < iterations; i++) {
                x86_cmpxchg_u32(locks[0], 0, 0);
                uint64_t t0=Timer::ticks();
                while ((Timer::ticks()-t0) < delay)
                    ;
            }
        } else {
            double start=Timer::seconds();
            for (iterations=0; (Timer::seconds()-start) < seconds; iterations+=1) {
                x86_cmpxchg_u32(locks[0], 0, 0);
                uint64_t t0=Timer::ticks();
                while ((Timer::ticks()-t0) < delay)
                    ;
            }
        }
    }

    return iterations;
}

static int64_t
use_cmpxchg64(double seconds, int64_t iterations, Run::lock_u list, uint64_t delay)
{
    uint64_t** locks = list.uint64;
    if (delay <= 0) {
        if (seconds <= 0) {
            for (int i=0; i < iterations; i++) {
                x86_cmpxchg_u64(locks[0], 0, 0);
            }
        } else {
            double start=Timer::seconds();
            for (iterations=0; (Timer::seconds()-start) < seconds; iterations+=UNROLL_FACTOR) {
                x86_cmpxchg_u64(locks[0], 0, 0);
                x86_cmpxchg_u64(locks[0], 0, 0);
                x86_cmpxchg_u64(locks[0], 0, 0);
                x86_cmpxchg_u64(locks[0], 0, 0);
                x86_cmpxchg_u64(locks[0], 0, 0);
                x86_cmpxchg_u64(locks[0], 0, 0);
                x86_cmpxchg_u64(locks[0], 0, 0);
                x86_cmpxchg_u64(locks[0], 0, 0);
            }
        }
    } else {
        if (seconds <= 0) {
            for (int i=0; i < iterations; i++) {
                x86_cmpxchg_u64(locks[0], 0, 0);
                uint64_t t0=Timer::ticks();
                while ((Timer::ticks()-t0) < delay)
                    ;
            }
        } else {
            double start=Timer::seconds();
            for (iterations=0; (Timer::seconds()-start) < seconds; iterations+=1) {
                x86_cmpxchg_u64(locks[0], 0, 0);
                uint64_t t0=Timer::ticks();
                while ((Timer::ticks()-t0) < delay)
                    ;
            }
        }
    }

    return iterations;
}

static int64_t
use_fairspin(double seconds, int64_t iterations, Run::lock_u list, uint64_t delay)
{
    fair_lock_t** locks = list.fair_lock;
    if (delay <= 0) {
        if (seconds <= 0) {
            for (int i=0; i < iterations; i++) {
                fair_lock_set(locks[0]);
                fair_lock_clr(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
            }
        } else {
            double start=Timer::seconds();
            for (iterations=0; (Timer::seconds()-start) < seconds; iterations+=UNROLL_FACTOR) {
                fair_lock_set(locks[0]);
                fair_lock_clr(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                fair_lock_set(locks[0]);
                fair_lock_clr(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                fair_lock_set(locks[0]);
                fair_lock_clr(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                fair_lock_set(locks[0]);
                fair_lock_clr(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                fair_lock_set(locks[0]);
                fair_lock_clr(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                fair_lock_set(locks[0]);
                fair_lock_clr(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                fair_lock_set(locks[0]);
                fair_lock_clr(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                fair_lock_set(locks[0]);
                fair_lock_clr(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
            }
        }
    } else {
        if (seconds <= 0) {
            for (int i=0; i < iterations; i++) {
                fair_lock_set(locks[0]);
                uint64_t t0=Timer::ticks();
                while ((Timer::ticks()-t0) < delay)
                    ;
                fair_lock_clr(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
            }
        } else {
            double start=Timer::seconds();
            for (iterations=0; (Timer::seconds()-start) < seconds; iterations+=1) {
                fair_lock_set(locks[0]);
                uint64_t t0=Timer::ticks();
                while ((Timer::ticks()-t0) < delay)
                    ;
                fair_lock_clr(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
            }
        }
    }

    return iterations;
}

static int64_t
use_atomic32(double seconds, int64_t iterations, Run::lock_u list, uint64_t delay)
{
    atomic32_t** locks = list.atomic32;
    if (delay <= 0) {
        if (seconds <= 0) {
            for (int i=0; i < iterations; i++) {
                atomic_add_u32(locks[0], 1);
            }
        } else {
            double start=Timer::seconds();
            for (iterations=0; (Timer::seconds()-start) < seconds; iterations+=UNROLL_FACTOR) {
                atomic_add_u32(locks[0], 1);
                atomic_add_u32(locks[0], 1);
                atomic_add_u32(locks[0], 1);
                atomic_add_u32(locks[0], 1);
                atomic_add_u32(locks[0], 1);
                atomic_add_u32(locks[0], 1);
                atomic_add_u32(locks[0], 1);
                atomic_add_u32(locks[0], 1);
            }
        }
    } else {
        if (seconds <= 0) {
            for (int i=0; i < iterations; i++) {
                atomic_add_u32(locks[0], 1);
                uint64_t t0=Timer::ticks();
                while ((Timer::ticks()-t0) < delay)
                    ;
            }
        } else {
            double start=Timer::seconds();
            for (iterations=0; (Timer::seconds()-start) < seconds; iterations+=1) {
                atomic_add_u32(locks[0], 1);
                uint64_t t0=Timer::ticks();
                while ((Timer::ticks()-t0) < delay)
                    ;
            }
        }
    }

    return iterations;
}

static int64_t
use_atomic64(double seconds, int64_t iterations, Run::lock_u list, uint64_t delay)
{
    atomic64_t** locks = list.atomic64;
    if (delay <= 0) {
        if (seconds <= 0) {
            for (int i=0; i < iterations; i++) {
                atomic_add_u64(locks[0], 1);
            }
        } else {
            double start=Timer::seconds();
            for (iterations=0; (Timer::seconds()-start) < seconds; iterations+=UNROLL_FACTOR) {
                atomic_add_u64(locks[0], 1);
                atomic_add_u64(locks[0], 1);
                atomic_add_u64(locks[0], 1);
                atomic_add_u64(locks[0], 1);
                atomic_add_u64(locks[0], 1);
                atomic_add_u64(locks[0], 1);
                atomic_add_u64(locks[0], 1);
                atomic_add_u64(locks[0], 1);
            }
        }
    } else {
        if (seconds <= 0) {
            for (int i=0; i < iterations; i++) {
                atomic_add_u64(locks[0], 1);
                uint64_t t0=Timer::ticks();
                while ((Timer::ticks()-t0) < delay)
                    ;
            }
        } else {
            double start=Timer::seconds();
            for (iterations=0; (Timer::seconds()-start) < seconds; iterations+=1) {
                atomic_add_u64(locks[0], 1);
                uint64_t t0=Timer::ticks();
                while ((Timer::ticks()-t0) < delay)
                    ;
            }
        }
    }

    return iterations;
}

static int64_t
use_reader (double seconds, int64_t iterations, Run::lock_u list, uint64_t delay)
{
    pthread_rwlock_t** locks = list.pt_rwlock;
    if (delay <= 0) {
        if (seconds <= 0) {
            for (int i=0; i < iterations; i++) {
                pthread_rwlock_rdlock(locks[0]);
                pthread_rwlock_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
            }
        } else {
            double start=Timer::seconds();
            for (iterations=0; (Timer::seconds()-start) < seconds; iterations+=UNROLL_FACTOR) {
                pthread_rwlock_rdlock(locks[0]);
                pthread_rwlock_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_rwlock_rdlock(locks[0]);
                pthread_rwlock_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_rwlock_rdlock(locks[0]);
                pthread_rwlock_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_rwlock_rdlock(locks[0]);
                pthread_rwlock_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_rwlock_rdlock(locks[0]);
                pthread_rwlock_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_rwlock_rdlock(locks[0]);
                pthread_rwlock_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_rwlock_rdlock(locks[0]);
                pthread_rwlock_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_rwlock_rdlock(locks[0]);
                pthread_rwlock_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
            }
        }
    } else {
        if (seconds <= 0) {
            for (int i=0; i < iterations; i++) {
                pthread_rwlock_rdlock(locks[0]);
                uint64_t t0=Timer::ticks();
                while ((Timer::ticks()-t0) < delay)
                    ;
                pthread_rwlock_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
            }
        } else {
            double start=Timer::seconds();
            for (iterations=0; (Timer::seconds()-start) < seconds; iterations+=1) {
                pthread_rwlock_rdlock(locks[0]);
                uint64_t t0=Timer::ticks();
                while ((Timer::ticks()-t0) < delay)
                    ;
                pthread_rwlock_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
            }
        }
    }

    return iterations;
}

static int64_t
use_writer (double seconds, int64_t iterations, Run::lock_u list, uint64_t delay)
{
    pthread_rwlock_t** locks = list.pt_rwlock;
    if (delay <= 0) {
        if (seconds <= 0) {
            for (int i=0; i < iterations; i++) {
                pthread_rwlock_wrlock(locks[0]);
                pthread_rwlock_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
            }
        } else {
            double start=Timer::seconds();
            for (iterations=0; (Timer::seconds()-start) < seconds; iterations+=UNROLL_FACTOR) {
                pthread_rwlock_wrlock(locks[0]);
                pthread_rwlock_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_rwlock_wrlock(locks[0]);
                pthread_rwlock_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_rwlock_wrlock(locks[0]);
                pthread_rwlock_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_rwlock_wrlock(locks[0]);
                pthread_rwlock_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_rwlock_wrlock(locks[0]);
                pthread_rwlock_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_rwlock_wrlock(locks[0]);
                pthread_rwlock_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_rwlock_wrlock(locks[0]);
                pthread_rwlock_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
                pthread_rwlock_wrlock(locks[0]);
                pthread_rwlock_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
            }
        }
    } else {
        if (seconds <= 0) {
            for (int i=0; i < iterations; i++) {
                pthread_rwlock_wrlock(locks[0]);
                uint64_t t0=Timer::ticks();
                while ((Timer::ticks()-t0) < delay)
                    ;
                pthread_rwlock_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
            }
        } else {
            double start=Timer::seconds();
            for (iterations=0; (Timer::seconds()-start) < seconds; iterations+=1) {
                pthread_rwlock_wrlock(locks[0]);
                uint64_t t0=Timer::ticks();
                while ((Timer::ticks()-t0) < delay)
                    ;
                pthread_rwlock_unlock(locks[0]);
#if !defined(NO_CLFLUSH)
		x86_clflush(locks[0]);
#endif
            }
        }
    }

    return iterations;
}

static int64_t
use_pthread_barrier(double seconds, int64_t iterations, Run::lock_u list, uint64_t delay)
{
    pthread_barrier_t** barrier = list.pt_barrier;
    if (delay <= 0) {
        for (int i=0; i < iterations; i+=UNROLL_FACTOR) {
            pthread_barrier_wait(barrier[0]);
            pthread_barrier_wait(barrier[0]);
            pthread_barrier_wait(barrier[0]);
            pthread_barrier_wait(barrier[0]);
            pthread_barrier_wait(barrier[0]);
            pthread_barrier_wait(barrier[0]);
            pthread_barrier_wait(barrier[0]);
            pthread_barrier_wait(barrier[0]);
        }
    } else {
        for (int i=0; i < iterations; i++) {
            pthread_barrier_wait(barrier[0]);
            uint64_t t0=Timer::ticks();
            while ((Timer::ticks()-t0) < delay)
                ;
        }
    }

    return iterations;
}

#if defined(ID)
const char* Run_C = "\0@ID " ID;
#endif
