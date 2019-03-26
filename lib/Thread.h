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

#if !defined(Thread_h)
#define Thread_h

// #include <sched.h>
#include <stdint.h>
#include <pthread.h>

#include "Lock.h"

class Thread {
public:
    Thread();			// creates the data structure
    ~Thread();			// deletes the data structure

    virtual int run() = 0;

    int start();		// creates the thread and begins executing
    int wait();			// waits until the thread terminates
    int thread_count() { return count; }
    int thread_id() { return id; }

    int    get_run_node_id()  { return run_node_id; }
    void   run_on_node(int run_node_id);
    static char* alloc_on_node(int alloc_node_id, int bytes);

    static int get_max_numa_domain();
    static int get_domain_id(int core);
    static int get_core_id(int domain, int index);
    static int get_cores_per_domain(int core);
    static int get_cores_per_system();
    static int get_domains_per_system();

    static void set_system_cores_domains(int cores, int domains);
    static void map_core_to_domain(int core, int domain);

    enum NumaMethod { LIBNUMA, LIBSCHED, NONE, };
    static NumaMethod set_numa_method(NumaMethod m) {
    			numa_method = (LIBNUMA <= m && m <= NONE) ? m : numa_method; 
			return numa_method;
		    }

    static void exit();

protected:
    void lock();
    void unlock();
    static void global_lock();
    static void global_unlock();

private:
    static Lock _global_lock;	// thread global lock
    Lock object_lock;		// thread lock (one per thread)

    pthread_t thread;		// pthread thread object
    int run_node_id;		// numa domain where thread resides

    static int count;		// number of threads created
    int id;			// thread id
    static NumaMethod numa_method;	// use libnuma, or sched, or nothing

    static int my_numa_available();

    unsigned long long value;
};

#endif
