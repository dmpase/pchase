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

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "Main.h"

#include "Run.h"
#include "Timer.h"
#include "Output.h"
#include "Experiment.h"
#include "SpinBarrier.h"
#include "Allocate.h"


int verbose = 0;

int
main( int argc, char* argv[] )
{
    Timer::calibrate(10000);
    double clk_res = Timer::resolution();

    				// parse the args and find out what we're doing
    Experiment e;
    if (e.parse_args(argc, argv)) {
	return 0;
    }

				// allocate and initialize the memory
				// count == 1, lock domain == 0
    Run::lock_u locks;
    int64_t bytes = 0;
    switch (e.lock_method) { 
    case Experiment::PTHREAD_SPIN:
        locks.pt_spin   = Allocate::pt_spin     (1, 0, e.num_numa_domains, e.num_threads, e.alignment, bytes);
        assert(locks.pt_spin    != NULL);
        break;
    case Experiment::PTHREAD_MUTEX:
        locks.pt_mutex  = Allocate::pt_mutex    (1, 0, e.num_numa_domains, e.num_threads, e.alignment, bytes);
        assert(locks.pt_mutex   != NULL);
        break;
    case Experiment::PTHREAD_READER:
        locks.pt_rwlock = Allocate::pt_rwlock   (1, 0, e.num_numa_domains, e.num_threads, e.alignment, bytes);
        assert(locks.pt_rwlock  != NULL);
        break;
    case Experiment::PTHREAD_WRITER:
        locks.pt_rwlock = Allocate::pt_rwlock   (1, 0, e.num_numa_domains, e.num_threads, e.alignment, bytes);
        assert(locks.pt_rwlock  != NULL);
        break;
    case Experiment::CMPXSPIN:
        locks.lock8     = Allocate::lock8       (1, 0, e.num_numa_domains, e.num_threads, e.alignment, bytes);
        assert(locks.lock8      != NULL);
        break;
    case Experiment::CMPXCHG32:
        locks.uint32    = Allocate::uint32      (1, 0, e.num_numa_domains, e.num_threads, e.alignment, bytes);
        assert(locks.uint32     != NULL);
        break;
    case Experiment::CMPXCHG64:
        locks.uint64    = Allocate::uint64      (1, 0, e.num_numa_domains, e.num_threads, e.alignment, bytes);
        assert(locks.uint64     != NULL);
        break;
    case Experiment::FAIRSPIN:
        locks.fair_lock = Allocate::fair_lock   (1, 0, e.num_numa_domains, e.num_threads, e.alignment, bytes);
        assert(locks.fair_lock  != NULL);
        break;
    case Experiment::ATOMIC32:
        locks.atomic32  = Allocate::atomic32    (1, 0, e.num_numa_domains, e.num_threads, e.alignment, bytes);
        assert(locks.atomic32   != NULL);
        break;
    case Experiment::ATOMIC64:
        locks.atomic64  = Allocate::atomic64    (1, 0, e.num_numa_domains, e.num_threads, e.alignment, bytes);
        assert(locks.atomic64   != NULL);
        break;
    case Experiment::PTHREAD_BARRIER:
        locks.pt_barrier = Allocate::pt_barrier (1, 0, e.num_numa_domains, e.num_threads, e.alignment, bytes);
        assert(locks.pt_barrier != NULL);
        break;
    }

    SpinBarrier sb( e.num_threads );
    Run r[ e.num_threads ];
    for (int i=0; i < e.num_threads; i++) {
	r[i].set(&e, locks, &sb);
	r[i].start();
    }

    for (int i=0; i < e.num_threads; i++) {
	r[i].wait();
    }

    double  secs = Run::seconds();

    Output::print(e, secs, clk_res);

    return 0;
}

#if defined(ID)
const char* Main_C = "\0@ID " ID;
#endif
