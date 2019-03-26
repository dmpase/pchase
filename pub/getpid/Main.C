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
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/time.h>

#include "Main.h"

static void   calibrate(int n);
static double wall_seconds();
static double resolution();

static inline uint64_t
read_rtc()
{
				// See pg. 406 of the AMD x86-64 Architecture
				// Programmer's Manual, Volume 2, System Programming
    uint32_t eax=0, edx=0;

    __asm__ __volatile__(
	"rdtsc ;"
	"movl %%eax,%0;"
	"movl %%edx,%1;"
	""
    : "=r"(eax), "=r"(edx)
    :
    : "%eax", "%edx"
    );

    return ((int64_t) edx << 32) | (int64_t) eax;
}


static int32_t wall_ticks   = -1;
static int32_t rtc_ticks    = -1;
static double  wall_elapsed = -1;
static int64_t rtc_elapsed  = -1;
static double  time_factor  = -1;

#if !defined(RTC) && !defined(GTOD)
#define RTC
#endif

#if defined(RTC)

double
seconds()
{ 
    return (double) read_rtc() * time_factor;
}

void
calibrate(int n)
{
    wall_ticks = n;

    double wall_start,wall_finish,t;
    t = wall_seconds();
    while (t == (wall_start=wall_seconds())) {
	;
    }
    int64_t rtc_start = read_rtc();
    for (int i=0; i < wall_ticks; i++) {
	t = wall_seconds();
	while (t == (wall_finish=wall_seconds())) {
	    ;
	}
    }
    int64_t rtc_finish = read_rtc();

    wall_elapsed = wall_finish - wall_start;
    rtc_elapsed  = rtc_finish  - rtc_start;
    time_factor  = wall_elapsed / (double) rtc_elapsed;
}

static double
wall_seconds()
{ 
    struct timeval t;
    gettimeofday(&t, NULL);

    return (double) t.tv_sec + (double) t.tv_usec * 1E-6;
}

#else

double
seconds()
{ 
    struct timeval t;
    gettimeofday(&t, NULL);

    return (double) t.tv_sec + (double) t.tv_usec * 1E-6;
}

int64_t
ticks()
{ 
    struct timeval t;
    gettimeofday(&t, NULL);

    return 1000000 * (int64_t) t.tv_sec + (int64_t) t.tv_usec;
}

void
calibrate()
{
}

void
calibrate(int n)
{
}

#endif

static double
min( double v1, double v2 )
{
    if (v2 < v1) return v2;
    return v1;
}

double
resolution()
{ 
    double a,b,c=1E9;
    for (int i=0; i < 10; i++) {
	a = seconds();
	while (a == (b=seconds()))
	    ;
	a = seconds();
	while (a == (b=seconds()))
	    ;
	c = min(b - a, c);
    }

    return c;
}


int
main( int argc, char* argv[] )
{
    calibrate(10000);
    double clk_res = resolution();

    uint64_t count = 0;
    double limit = 1.0;
    double start = seconds();
    double finish;
    while (((finish=seconds())-start) < limit) {
	getpid();
	count += 1;
    }

    double x0 = finish-start;
    double x1 = count/(finish-start);
    double x2 = 1E9*(finish-start)/count;

#if defined(__i386__)
    printf("%llu mode switches in %.2f seconds, %.2f switches per second, %.2f nanoseconds per switch\n", 
	count, finish-start, count/(finish-start), 1000000000*(finish-start)/count);
#else
    printf("%lu mode switches in %.2lf seconds, %.2lf switches per second, %.2lf nanoseconds per switch\n", 
	count, finish-start, count/(finish-start), 1000000000*(finish-start)/count);
#endif

    return 0;
}

#if defined(ID)
const char* Main_C = "\0@ID " ID;
#endif
