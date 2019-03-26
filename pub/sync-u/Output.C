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

#include "Output.h"

#include "Experiment.h"


void
Output::print( Experiment &e, double secs, double ck_res, int64_t bytes )
{
    if (e.output_mode == Experiment::CSV) {
	Output::csv(e, secs, ck_res, bytes);
    } else if (e.output_mode == Experiment::BOTH) {
	Output::header();
	Output::csv(e, secs, ck_res, bytes);
    } else if (e.output_mode == Experiment::HEADER) {
	Output::header();
    } else {
	Output::table(e, secs, ck_res, bytes);
    }
}


void
Output::header()
{
    printf("op type,");
    printf("bytes allocated,");
    printf("ops per thread,");
    printf("number of threads,");
    printf("iterations,");
    printf("experiments,");
    printf("total ops,");
    printf("numa domains,");
    printf("placement,");
    printf("domain map,");
    printf("elapsed time (seconds),");
    printf("elapsed time (timer ticks),");
    printf("clock resolution (ns),");
    printf("op bandwidth (Mops/s),");
    printf("op latency (ns)");
    printf("\n");

    fflush(stdout);
}


void
Output::csv( Experiment &e, double secs, double ck_res, int64_t bytes )
{
    printf("%s," , e.method());
    printf("%ld,", bytes);
    printf("%ld,", e.locks_per_thread);
    printf("%d," , e.num_threads);
    printf("%ld,", e.iterations);
    printf("%ld,", e.experiments);
    printf("%ld,", e.locks_per_thread * e.num_threads * e.iterations);
    printf("%d," , e.num_numa_domains);
    printf("%s," , e.placement()); 
    printf("\"");
    printf("%d:", e.thread_domain[0]);
    printf("%d",  e.lock_domain[0]);
    for (int i=1; i < e.num_threads; i++) {
	printf(";%d:", e.thread_domain[i]);
	printf("%d",   e.lock_domain[i]);
    } 
    printf("\",");
    printf("%.3f,", secs);
    printf("%.0f,", secs/ck_res);
    printf("%.2f,", ck_res * 1E9);
    printf("%.2f,",  1E-6*(e.locks_per_thread * e.num_threads * e.iterations)/secs);
    printf("%.2f",   1E9*secs/(e.locks_per_thread * e.iterations));
    printf("\n");

    fflush(stdout);
}

void
Output::table( Experiment &e, double secs, double ck_res, int64_t bytes )
{
    printf("op type                ");
    printf("%s\n" , e.method());

    printf("bytes allocated        ");
    printf("%ld\n", bytes);

    printf("ops per thread         ");
    printf("%ld\n", e.locks_per_thread);

    printf("number of threads      ");
    printf("%d\n" , e.num_threads);

    printf("iterations             ");
    printf("%ld\n", e.iterations);

    printf("experiments            ");
    printf("%ld\n", e.experiments);

    printf("total ops              ");
    printf("%ld\n", e.locks_per_thread * e.num_threads * e.iterations);

    printf("numa domains           ");
    printf("%d\n" , e.num_numa_domains);

    printf("placement              ");
    printf("%s\n" , e.placement()); 

    printf("domain map             ");
    printf("\"");
    printf("%d:", e.thread_domain[0]);
    printf("%d",  e.lock_domain[0]);
    for (int i=1; i < e.num_threads; i++) {
	printf(";%d:", e.thread_domain[i]);
	printf("%d",   e.lock_domain[i]);
    } 
    printf("\"\n");

    printf("elapsed time           ");
    printf("%.3f seconds \n", secs);

    printf("elapsed time           ");
    printf("%.0f timer ticks\n", secs/ck_res);

    printf("clock resolution       ");
    printf("%.2f ns\n", ck_res * 1E9);

    printf("op bandwidth (Mops/s)  ");
    printf("%.2f Million ",  1E-6*(e.locks_per_thread * e.num_threads * e.iterations)/secs);
    switch(e.lock_method) {
    case Experiment::PTHREAD_SPIN:
	printf("PTHREAD spin locks ");
	break;
    case Experiment::PTHREAD_MUTEX:
	printf("PTHREAD mutexes ");
	break;
    case Experiment::CMPXSPIN:
	printf("cmpxchg spin locks ");
	break;
    case Experiment::CMPXCHG32:
	printf("cmpxchg32 instructions ");
	break;
    case Experiment::CMPXCHG64:
	printf("cmpxchg64 instructions ");
	break;
    case Experiment::FAIRSPIN:
	printf("fair spin locks ");
	break;
    case Experiment::ATOMIC32:
	printf("atomic32 operations ");
	break;
    case Experiment::ATOMIC64:
	printf("atomic64 operations ");
	break;
    case Experiment::PTHREAD_READER:
	printf("reader locks ");
	break;
    case Experiment::PTHREAD_WRITER:
	printf("writer locks ");
	break;
    }
    printf("per second\n");

    printf("op latency (ns)        ");
    printf("%.2f ns per ",  1E9*secs/(e.locks_per_thread * e.iterations));
    switch(e.lock_method) {
    case Experiment::PTHREAD_SPIN:
	printf("PTHREAD spin lock\n");
	break;
    case Experiment::PTHREAD_MUTEX:
	printf("PTHREAD mutex\n");
	break;
    case Experiment::CMPXSPIN:
	printf("cmpxchg spin lock\n");
	break;
    case Experiment::CMPXCHG32:
	printf("cmpxchg32 instruction\n");
	break;
    case Experiment::CMPXCHG64:
	printf("cmpxchg64 instruction\n");
	break;
    case Experiment::FAIRSPIN:
	printf("fair spin lock\n");
	break;
    case Experiment::ATOMIC32:
	printf("atomic32 operation\n");
	break;
    case Experiment::ATOMIC64:
	printf("atomic64 operation\n");
	break;
    case Experiment::PTHREAD_READER:
	printf("reader lock\n");
	break;
    case Experiment::PTHREAD_WRITER:
	printf("writer lock\n");
	break;
    }

    fflush(stdout);
}

#if defined(ID)
const char* Output_C = "\0@ID " ID;
#endif
