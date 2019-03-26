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

#include "Main.h"

#include "Run.h"
#include "Timer.h"
#include "Output.h"
#include "Experiment.h"
#include "SpinBarrier.h"

				// This program allocates and accesses
				// a number of blocks of memory, one or more
				// for each thread that executes.  Blocks
				// are divided into sub-blocks called
				// pages, and pages are divided into
				// sub-blocks called cache lines.
				// 
				// All pages are collected into a list.
				// Pages are selected for the list in
				// a particular order.   Each cache line
				// within the page is similarly gathered
				// into a list in a particular order.
				// In both cases the order may be random
				// or linear.
				//
				// A root pointer points to the first
				// cache line.  A pointer in the cache
				// line points to the next cache line,
				// which contains a pointer to the cache
				// line after that, and so on.  This
				// forms a pointer chain that touches all
				// cache lines within the first page,
				// then all cache lines within the second
				// page, and so on until all pages are
				// covered.  The last pointer contains
				// NULL, terminating the chain.
				//
				// Depending on compile-time options,
				// pointers may be 32-bit or 64-bit
				// pointers.

int verbose = 0;

int
main( int argc, char* argv[] )
{
    Timer::calibrate(10000);
    double clk_res = Timer::resolution();

    Experiment e;
    if (e.parse_args(argc, argv)) {
	return 0;
    }

    SpinBarrier sb( e.num_threads );
    Run r[ e.num_threads ];
    for (int i=0; i < e.num_threads; i++) {
	r[i].set(&e, &sb);
	r[i].start();
    }

    for (int i=0; i < e.num_threads; i++) {
	r[i].wait();
    }

    int64_t ops  = Run::ops_per_chain();
    double  secs = Run::seconds();

    Output::print(e, ops, secs, clk_res);

    return 0;
}

#if defined(ID)
const char* Main_C = "\0@ID " ID;
#endif
