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
#include "Chain.h"
#include "Experiment.h"
#include "SpinBarrier.h"

class Run: public Thread {
public:
    Run();
    ~Run();
    int run();
    void set( Experiment* e, SpinBarrier* sbp ) {exp=e; bp=sbp;}

    static int64_t ops_per_chain() { return _ops_per_chain; }
    static double  seconds()       { return _seconds; }

private:
    Experiment*  exp;			// experiment data used by all threads
    SpinBarrier* bp;			// spin barrier used by all threads

    void   mem_check( Chain *m );
    Chain* random_mem_init( Chain *m );
    Chain* forward_mem_init( Chain *m );
    Chain* reverse_mem_init( Chain *m );
    Chain* stream_mem_init( Chain *m );

    static Lock    global_mutex;		// global lock
    static int64_t _ops_per_chain;	// total number of operations per chain
    static double  _seconds;		// total number of seconds
};


#endif
