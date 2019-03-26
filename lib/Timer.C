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
#include <sys/time.h>

#include "Timer.h"


static int64_t read_rtc();
static void    calibrate_rtc(int n);
static double  wall_seconds();

static int     wall_ticks   = -1;
static int     rtc_ticks    = -1;
static double  wall_elapsed = -1;
static int64_t rtc_elapsed  = -1;
// static double  time_factor  = -1;
double Timer::time_factor  = -1;

#if !defined(RTC) && !defined(GTOD)
#define RTC
#endif

#if defined(RTC)

#if defined(UNDEFINED)
double
Timer::seconds()
{ 
    return (double) read_rtc() * time_factor;
}
#endif

int64_t
Timer::ticks()
{ 
				// See pg. 406 of the AMD x86-64 Architecture
				// Programmer's Manual, Volume 2, System Programming
    unsigned int eax=0, edx=0;

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

#if defined(UNDEFINED)
static int64_t
read_rtc()
{
				// See pg. 406 of the AMD x86-64 Architecture
				// Programmer's Manual, Volume 2, System Programming
    unsigned int eax=0, edx=0;

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
#endif

void
Timer::calibrate()
{
    Timer::calibrate(1000);
}

void
Timer::calibrate(int n)
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
Timer::seconds()
{ 
    struct timeval t;
    gettimeofday(&t, NULL);

    return (double) t.tv_sec + (double) t.tv_usec * 1E-6;
}

int64_t
Timer::ticks()
{ 
    struct timeval t;
    gettimeofday(&t, NULL);

    return 1000000 * (int64_t) t.tv_sec + (int64_t) t.tv_usec;
}

void
Timer::calibrate()
{
}

void
Timer::calibrate(int n)
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
Timer::resolution()
{ 
    double a,b,c=1E9;
    for (int i=0; i < 10; i++) {
	a = Timer::seconds();
	while (a == (b=Timer::seconds()))
	    ;
	a = Timer::seconds();
	while (a == (b=Timer::seconds()))
	    ;
	c = min(b - a, c);
    }

    return c;
}

#if defined(ID)
const char* Timer_C = "\0@ID " ID;
#endif
