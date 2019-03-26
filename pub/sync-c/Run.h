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

#if !defined(Run_h)
#define Run_h

#include <stdint.h>

#include "Thread.h"

#include "Lock.h"
#include "Experiment.h"
#include "SpinBarrier.h"

#include "sync.h"

class Run: public Thread {
public:
    union lock_u {
	pthread_spinlock_t** pt_spin;
	pthread_mutex_t**    pt_mutex;
	pthread_rwlock_t**   pt_rwlock;
	uint32_t**           uint32;
	uint64_t**           uint64;
	atomic32_t**         atomic32;
	atomic64_t**         atomic64;
	fair_lock_t**        fair_lock;
	lock8_t**            lock8;
	pthread_barrier_t**  pt_barrier;
    }; 

    Run();
    ~Run();
    int run();

    void set( Experiment* e, lock_u lck, SpinBarrier* sbp ) {exp=e; locks=lck; bp=sbp;}

    static double   seconds()       { return _seconds; }
    static int64_t  max_iters()     { return _max_iters; }
    static int64_t  min_iters()     { return _min_iters; }
    static int64_t  total_iters()   { return _total_iters; }

private:
    Experiment*  exp;			// experiment data used by all threads
    lock_u       locks;			// array of available locks
    SpinBarrier* bp;			// spin barrier used by all threads

    static Lock    global_mutex;	// global lock

    static double  _seconds;		// total number of seconds
    static int64_t _max_iters;
    static int64_t _min_iters;
    static int64_t _total_iters;

    static double  _exp_delta;
    static int64_t _exp_max_iters;
    static int64_t _exp_min_iters;
    static int64_t _exp_total_iters;
};


#endif
