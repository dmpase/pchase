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
Output::print( Experiment &e, int64_t ops, double secs, double ck_res )
{
    if (e.output_mode == Experiment::CSV) {
	Output::csv(e, ops, secs, ck_res);
    } else if (e.output_mode == Experiment::BOTH) {
	Output::header(e, ops, secs, ck_res);
	Output::csv(e, ops, secs, ck_res);
    } else if (e.output_mode == Experiment::HEADER) {
	Output::header(e, ops, secs, ck_res);
    } else {
	Output::table(e, ops, secs, ck_res);
    }
}


void
Output::header( Experiment &e, int64_t ops, double secs, double ck_res )
{
    printf("copy method,");
    printf("copy size (bytes),");
    printf("pool size (bytes),");
    printf("test size (bytes),");
    printf("copies per thread,");
    printf("number of threads,");
    printf("iterations,");
    printf("experiments,");
    printf("numa placement,");
    printf("numa domains,");
    printf("domain map,");
    printf("total copies,");
    printf("total bytes copied,");
    printf("elapsed time (seconds),");
    printf("elapsed time (timer ticks),");
    printf("clock resolution (ns),");
    printf("read bandwidth (MiB/s),");
    printf("write bandwidth (MiB/s)\n");

    fflush(stdout);
}


void
Output::csv( Experiment &e, int64_t ops, double secs, double ck_res )
{
    printf("%s," , e.method());
    printf("%ld,", e.bytes_per_copy);
    printf("%ld,", e.bytes_per_pool);
    printf("%ld,", e.bytes_per_test);
    printf("%ld,", e.copies_per_thread);
    printf("%d," , e.num_threads);
    printf("%ld,", e.iterations);
    printf("%ld,", e.experiments);
    printf("%s," , e.placement());
    printf("%d," , e.num_numa_domains);

    printf("\"");
    printf("%d:", e.thread_domain[0]);
    printf("%d,", e.source_domain[0]);
    printf("%d",  e.target_domain[0]);
    for (int i=1; i < e.num_threads; i++) {
	printf(";%d:", e.thread_domain[i]);
	printf("%d,",  e.source_domain[i]);
	printf("%d",   e.target_domain[i]);
    } 
    printf("\",");

    printf("%ld,", e.iterations * e.copies_per_thread * e.num_threads);
    printf("%ld,", e.iterations * e.copies_per_thread * e.num_threads * e.bytes_per_copy);
    printf("%.3f,", secs);
    printf("%.0f,", secs/ck_res);
    printf("%.2f,", ck_res * 1E9);

    switch (e.copy_method) {
    case Experiment::NAIVE     : case Experiment::MEMCPY    : 
    case Experiment::MEMMOVE   : case Experiment::BCOPY     : 
    case Experiment::COPY_TT   : case Experiment::COPY_TN   : 
    case Experiment::XTTA      : case Experiment::XTTU      : 
    case Experiment::XTN       : case Experiment::XNT       : 
    case Experiment::XNN       : case Experiment::MEMREAD   :
	printf("%.2f,", ((e.iterations * e.copies_per_thread * e.num_threads * e.bytes_per_copy)/secs)/(1024*1024));
	break;
    case Experiment::BZERO     : case Experiment::MEMSET    :
    case Experiment::NAIVEZERO : case Experiment::MEMWRITE  :
	printf("%.2f,", 0.0);
	break;
    default:
	printf("%s,", "broken");
	break;
    }

    switch (e.copy_method) {
    case Experiment::NAIVE     : case Experiment::MEMCPY    : 
    case Experiment::MEMMOVE   : case Experiment::BCOPY     : 
    case Experiment::COPY_TT   : case Experiment::COPY_TN   : 
    case Experiment::XTTA      : case Experiment::XTTU      : 
    case Experiment::XTN       : case Experiment::XNT       : 
    case Experiment::XNN       : case Experiment::MEMWRITE  :
    case Experiment::BZERO     : case Experiment::MEMSET    :
    case Experiment::NAIVEZERO :
	printf("%.2f\n", ((e.iterations * e.copies_per_thread * e.num_threads * e.bytes_per_copy)/secs)/(1024*1024));
	break;
    case Experiment::MEMREAD : 
	printf("%.2f\n", 0.0);
	break;
    default:
	printf("%s\n", "broken");
	break;
    }

    fflush(stdout);
}

void
Output::table( Experiment &e, int64_t ops, double secs, double ck_res )
{
    printf("copy method                = %s\n" , e.method());
    printf("copy size (bytes)          = %ld\n", e.bytes_per_copy);
    printf("pool size (bytes)          = %ld\n", e.bytes_per_pool);
    printf("test size (bytes)          = %ld\n", e.bytes_per_test);
    printf("copies per thread          = %ld\n", e.copies_per_thread);
    printf("number of threads          = %d\n" , e.num_threads);
    printf("iterations                 = %ld\n", e.iterations);
    printf("experiments                = %ld\n", e.experiments);
    printf("numa placement             = %s\n" , e.placement());
    printf("numa domains               = %d\n" , e.num_numa_domains);
    printf("domain map                 = ");
    printf("\"");
    printf("%d:", e.thread_domain[0]);
    printf("%d,", e.source_domain[0]);
    printf("%d",  e.target_domain[0]);
    for (int i=1; i < e.num_threads; i++) {
	printf(";%d:", e.thread_domain[i]);
	printf("%d,",  e.source_domain[i]);
	printf("%d",   e.target_domain[i]);
    } 
    printf("\"\n");

    printf("total copies               = %ld\n", e.iterations * e.copies_per_thread * e.num_threads);
    printf("total bytes copied         = %ld\n", e.iterations * e.copies_per_thread * e.num_threads * e.bytes_per_copy);
    printf("elapsed time (seconds)     = %.3f\n", secs);
    printf("elapsed time (timer ticks) = %.0f\n", secs/ck_res);
    printf("clock resolution (ns)      = %.2f\n", ck_res * 1E9);
    switch (e.copy_method) {
    case Experiment::NAIVE     : case Experiment::MEMCPY    : 
    case Experiment::MEMMOVE   : case Experiment::BCOPY     : 
    case Experiment::COPY_TT   : case Experiment::COPY_TN   : 
    case Experiment::XTTA      : case Experiment::XTTU      : 
    case Experiment::XTN       : case Experiment::XNT       : 
    case Experiment::XNN       : case Experiment::MEMREAD   :
	printf("read bandwidth (MiB/s)     = %.2f\n", ((e.iterations * e.copies_per_thread * e.num_threads * e.bytes_per_copy)/secs)/(1024*1024));
	break;
    case Experiment::BZERO     : case Experiment::MEMSET    :
    case Experiment::NAIVEZERO : case Experiment::MEMWRITE  :
	printf("read bandwidth (MiB/s)     = %.2f\n", 0.0);
	break;
    default:
	printf("read bandwidth (MiB/s)     = %s\n", "broken");
	break;
    }

    switch (e.copy_method) {
    case Experiment::NAIVE     : case Experiment::MEMCPY    : 
    case Experiment::MEMMOVE   : case Experiment::BCOPY     : 
    case Experiment::COPY_TT   : case Experiment::COPY_TN   : 
    case Experiment::XTTA      : case Experiment::XTTU      : 
    case Experiment::XTN       : case Experiment::XNT       : 
    case Experiment::XNN       : case Experiment::MEMWRITE  :
    case Experiment::BZERO     : case Experiment::MEMSET    :
    case Experiment::NAIVEZERO :
	printf("write bandwidth (MiB/s)    = %.2f\n", ((e.iterations * e.copies_per_thread * e.num_threads * e.bytes_per_copy)/secs)/(1024*1024));
	break;
    case Experiment::MEMREAD : 
	printf("write bandwidth (MiB/s)    = %.2f\n", 0.0);
	break;
    default:
	printf("write bandwidth (MiB/s)    = %s\n", "broken");
	break;
    }
    
    fflush(stdout);
}

#if defined(ID)
const char* Output_C = "\0@ID " ID;
#endif
