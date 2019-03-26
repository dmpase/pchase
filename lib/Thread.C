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
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#if defined(NUMA)
#include <numa.h>
#endif

#include "Thread.h"
#include "Queue.h"
#include "Lock.h"

Lock Thread::_global_lock;
int  Thread::count = 0;
#if defined(NUMA)
Thread::NumaMethod Thread::numa_method = Thread::LIBNUMA;
#elif defined(SCHED)
Thread::NumaMethod Thread::numa_method = Thread::LIBSCHED;
#else
Thread::NumaMethod Thread::numa_method = Thread::NONE;
#endif

Thread::Thread() 
: run_node_id(-1)
{
    Thread::global_lock();
	this->id = Thread::count;
	Thread::count += 1;
    Thread::global_unlock();
}

Thread::~Thread()
{
}

static void*
start_voidp(void* p)
{
    ((Thread*)p)->run();

    return NULL;
}

static int
start_int(void* p)
{
    return ((Thread*)p)->run();
}

int
Thread::start()
{
    return pthread_create(&this->thread, NULL, start_voidp, this); 
}

void
Thread::exit()
{
    pthread_exit(NULL);
}

int
Thread::wait()
{
    void *value_ptr=&this->value;
    pthread_join(this->thread, &value_ptr);

    return 0;
}

void
Thread::lock()
{
    this->object_lock.lock();
}

void
Thread::unlock()
{
    this->object_lock.unlock();
}

void
Thread::global_lock()
{
    Thread::_global_lock.lock();
}

void
Thread::global_unlock()
{
    Thread::_global_lock.unlock();
}


static int _cps = -1;			// cores per system
static int _dps = -1;			// domains per system
static cpu_set_t* domain_mask = NULL;	// one mask per domain, bits are set for cores in the domain
static int* core_to_domain_map;	        // one domain per core

                                        // THREAD_PIN_CPU allows you to compile WITH each thread being
                                        // pinned to an individual CPU (by asserting THREAD_PIN_CPU), 
                                        // or allowing each thread to float among the available CPUs
                                        // within a domain (by NOT asserting THREAD_PIN_CPU).

                                        // ON SOME SYSTEMS, particularly x86 systems that have enabled
                                        // more than one logical core per physical core, the performance 
                                        // is DIFFERENT between those two settings when the additional
                                        // logical cores are used. (In other words, when the number of 
                                        // threads exceeds the number of physical cores within the system.
#if defined(THREAD_PIN_CPU)
static Queue*   cpu_run_queue = NULL;	// one queue per domain, entries represent cores in the domain
#endif

#if defined(NUMA)
static bool mnat=false;
static int  mnav=0xFFFFFFFF;
int
Thread::my_numa_available()
{
    Thread::global_lock();
    if (!mnat) {
	mnat = true;
	mnav = numa_available();
	assert(0 <= mnav);
    }
    int local=mnav;
    Thread::global_unlock();

    return local;
}
#endif


void
Thread::run_on_node(int run_node)
{
//    printf("enter (%d) Thread::run_on_node(id=%d)\n", thread_id(), run_node);
    assert(0 <= run_node);
    this->run_node_id = run_node;
    if (numa_method == Thread::LIBNUMA) {
#if defined(NUMA)
	int e0 = my_numa_available();
	assert(0 <= e0);
	int e1 = numa_run_on_node(run_node);
	assert(e1 == 0);
#else
	printf("\n");
	printf("The benchmark must be compiled with -DNUMA to use -L numa.\n");
	printf("\n");
	::exit(1);
#endif
    } else if (numa_method == Thread::LIBSCHED) {
#if defined(SCHED)
	assert(0 < _dps && 0 <= run_node && run_node < _dps);

#if defined(THREAD_PIN_CPU)
                                // select a CPU to run on from CPUs in the domain
                                // (once selected, the cpu goes back on the queue
                                // in case there are more threads than cores)
        int cpu = cpu_run_queue[run_node].pull_l();
        cpu_run_queue[run_node].push_l(cpu);
        cpu_set_t run_mask;
        CPU_ZERO(&run_mask);
        CPU_SET(cpu, &run_mask);

	int e0 = sched_setaffinity(0, sizeof run_mask, &run_mask);
#else
                                // pin the thread to a domain, but not to a single CPU
	int e0 = sched_setaffinity(0, sizeof domain_mask[run_node], &domain_mask[run_node]);
#endif

	if (e0 < 0) {
	    switch (errno) {
	    case EFAULT : printf("run_on_node: EFAULT\n"); break;
	    case ESRCH  : printf("run_on_node: ESRCH\n");  break;
	    case EPERM  : printf("run_on_node: EPERM\n");  break;
	    case EINVAL : printf("run_on_node: EINVAL\n"); break;
	    }
	}
	assert(e0 == 0);
#else
	printf("\n");
	printf("The benchmark must be compiled with -DSCHED to use -L sched.\n");
	printf("\n");
	::exit(1);
#endif
    } else {
    }
//    printf("leave (%d) Thread::run_on_node(id=%d)\n", thread_id(), run_node);
}


char*
Thread::alloc_on_node(int alloc_node_id, int bytes)
{
//    printf("enter (%d) Thread::alloc_on_node(id=%d, bytes=%d)\n", thread_id(), alloc_node_id, bytes);
    char* mem_buf = NULL;
    if (numa_method == Thread::LIBNUMA) {
#if defined(NUMA)
				// establish the node id where this thread's
				// memory will be allocated.
	int e0 = my_numa_available();
	assert(0 <= e0);

	nodemask_t alloc_mask;
	nodemask_zero(&alloc_mask);
	nodemask_set(&alloc_mask, alloc_node_id);
	numa_set_membind(&alloc_mask);

				// allocate the memory
	int e1 = sched_yield();
	assert(e1 == 0);
	mem_buf = new char[ bytes ];
	assert(mem_buf != NULL);

				// force map it to the domain
	memset(mem_buf, '\0', bytes);
#else
	printf("\n");
	printf("The benchmark must be compiled with -DNUMA to use -L numa.\n");
	printf("\n");
	::exit(1);
#endif
    } else if (numa_method == Thread::LIBSCHED) {
#if defined(SCHED)
				// save the old affinity mask
	cpu_set_t old_mask;
	int e0 = sched_getaffinity(0, sizeof old_mask, &old_mask);
	if (e0 < 0) {
	    switch (errno) {
	    case EFAULT : printf("alloc_on_node: EFAULT\n"); break;
	    case ESRCH  : printf("alloc_on_node: ESRCH\n");  break;
	    case EPERM  : printf("alloc_on_node: EPERM\n");  break;
	    case EINVAL : printf("alloc_on_node: EINVAL\n"); break;
	    }
	}
	assert(e0 == 0);

				// set up the new affinity mask
	assert(0 < _dps && 0 <= alloc_node_id && alloc_node_id < _dps);
	int e1 = sched_setaffinity(0, sizeof domain_mask[0], &domain_mask[alloc_node_id]);
	if (e1 < 0) {
	    switch (errno) {
	    case EFAULT : printf("alloc_on_node: EFAULT\n"); break;
	    case ESRCH  : printf("alloc_on_node: ESRCH\n");  break;
	    case EPERM  : printf("alloc_on_node: EPERM\n");  break;
	    case EINVAL : printf("alloc_on_node: EINVAL\n"); break;
	    }
	}
	assert(e1 == 0);

				// allocate the memory
	int e2 = sched_yield();
	assert(e2 == 0);
	mem_buf = new char[ bytes ];
	assert(mem_buf != NULL);

				// force map it to the domain
	memset(mem_buf, '\0', bytes);

				// restore the old affinity mask
	int e3 = sched_setaffinity(0, sizeof old_mask, &old_mask);
	if (e3 < 0) {
	    switch (errno) {
	    case EFAULT : printf("alloc_on_node: EFAULT\n"); break;
	    case ESRCH  : printf("alloc_on_node: ESRCH\n");  break;
	    case EPERM  : printf("alloc_on_node: EPERM\n");  break;
	    case EINVAL : printf("alloc_on_node: EINVAL\n"); break;
	    }
	}
	assert(e3 == 0);
	int e4 = sched_yield();
	assert(e4 == 0);
#else
	printf("\n");
	printf("The benchmark must be compiled with -DSCHED to use -L sched.\n");
	printf("\n");
	::exit(1);
#endif
    } else {
    }

//    printf("leave (%d) Thread::alloc_on_node(id=%d, links=%d)\n", thread_id(), alloc_node_id, bytes);
    return mem_buf;
}

int 
Thread::get_max_numa_domain()
{
//    printf("enter Thread::get_max_numa_domain()\n");
    int max_node = 0;
    if (numa_method == Thread::LIBNUMA) {
#if defined(NUMA)
	int e0 = my_numa_available();
	assert(0 <= e0);

	max_node = numa_max_node();
#else
	printf("\n");
	printf("The benchmark must be compiled with -DNUMA to use -L numa.\n");
	printf("\n");
	::exit(1);
#endif
    } else if (numa_method == Thread::LIBSCHED) {
#if defined(SCHED)
	max_node = _dps - 1;
#else
	printf("\n");
	printf("The benchmark must be compiled with -DSCHED to use -L sched.\n");
	printf("\n");
	::exit(1);
#endif
    } else {
    }

    assert(0 <= max_node);

//    printf("leave Thread::get_max_numa_domain() = %d\n", max_node);
    return max_node;
}

int 
Thread::get_domain_id(int core)
{
//    printf("enter Thread::get_domain_id(core=%d)\n", core);
    assert(0 <= core && core < _cps);

    int domain_id = -1;
    if (numa_method == Thread::LIBNUMA) {
#if defined(NUMA)
	printf("\n");
	printf("Thread::get_domain_id is not defined for -L numa.\n");
	printf("\n");
	::exit(1);
#else
	printf("\n");
	printf("The benchmark must be compiled with -DNUMA to use -L numa.\n");
	printf("\n");
	::exit(1);
#endif
    } else if (numa_method == Thread::LIBSCHED) {
#if defined(SCHED)
	domain_id = core_to_domain_map[ core ];
#else
	printf("\n");
	printf("The benchmark must be compiled with -DSCHED to use -L sched.\n");
	printf("\n");
	::exit(1);
#endif
    } else {
    }
    assert(0 <= domain_id);

//    printf("leave Thread::get_domain_id(core=%d) domain_id=%d\n", core, domain_id);
    return domain_id;
}

int 
Thread::get_core_id(int domain, int index)
{
//    printf("enter Thread::get_core_id(domain=%d, index=%d)\n", domain, index);
    assert(0 <= index && index < _cps);
    assert(0 <= domain && domain < _dps);

    int core = -1;
    if (numa_method == Thread::LIBNUMA) {
#if defined(NUMA)
	printf("\n");
	printf("Thread::get_domain_id is not defined for -L numa.\n");
	printf("\n");
	::exit(1);
#else
	printf("\n");
	printf("The benchmark must be compiled with -DNUMA to use -L numa.\n");
	printf("\n");
	::exit(1);
#endif
    } else if (numa_method == Thread::LIBSCHED) {
#if defined(SCHED)
        for (int i=0; i < _cps; i++) {
            if (CPU_ISSET(index, &domain_mask[domain])) {
                if (index == 0) {
                    core = i;
                    break;
                }
                index -= 1;
            }
        }
#else
	printf("\n");
	printf("The benchmark must be compiled with -DSCHED to use -L sched.\n");
	printf("\n");
	::exit(1);
#endif
    } else {
    }
    assert(0 <= core);

//    printf("leave Thread::get_core_id(domain=%d, index=%d) core=%d\n", domain, index, core);
    return core;
}

int 
Thread::get_cores_per_domain(int domain)
{
//    printf("enter Thread::get_cores_per_domain(domain=%d)\n", domain);
    assert(0 < _dps && 0 <= domain && domain < _dps);
    int cores=0;

    if (numa_method == Thread::LIBNUMA) {
#if defined(NUMA)
	unsigned long buffer[16];
        int e0 = numa_node_to_cpus(domain, buffer, 16);
	assert(0 <= e0);
#else
        printf("\n");
        printf("The benchmark must be compiled with -DNUMA to use -L numa.\n");
        printf("\n");
        ::exit(1);
#endif
    } else if (numa_method == Thread::LIBSCHED) {
#if defined(SCHED)
	for (int i=0; i < _cps; i++) {
	    if (CPU_ISSET(i, &domain_mask[domain])) {
		cores += 1;
	    }
	}
#else
        printf("\n");
        printf("The benchmark must be compiled with -DSCHED to use -L sched.\n");
        printf("\n");
        ::exit(1);
#endif
    } else {
    }
    assert(0 < cores);

//    printf("leave Thread::get_cores_per_domain() cores=%d\n", cores);
    return cores;
}

#if defined(UNDEFINED)
int 
Thread::get_domains_per_system()
{
//    printf("enter Thread::get_domains_per_system()\n");
    int domains = -1;
    if (numa_method == Thread::LIBNUMA) {
#if defined(NUMA)
	printf("\n");
	printf("Thread::get_domains_per_system is not defined for -L numa.\n");
	printf("\n");
	::exit(1);
#else
        printf("\n");
        printf("The benchmark must be compiled with -DNUMA to use -L numa.\n");
        printf("\n");
        ::exit(1);
#endif
    } else if (numa_method == Thread::LIBSCHED) {
#if defined(SCHED)
	domains = _dps;
#else
        printf("\n");
        printf("The benchmark must be compiled with -DSCHED to use -L sched.\n");
        printf("\n");
        ::exit(1);
#endif
    } else {
    }
    assert(0 < domains);

//    printf("leave Thread::get_domains_per_system() domains=%d\n", domains);
    return domains;
}
#endif

int 
Thread::get_cores_per_system()
{
//    printf("enter Thread::get_cores_per_system()\n");
    int cores = -1;
    if (numa_method == Thread::LIBNUMA) {
#if defined(NUMA)
	printf("\n");
	printf("Thread::get_cores_per_system is not defined for -L numa.\n");
	printf("\n");
	::exit(1);
#else
        printf("\n");
        printf("The benchmark must be compiled with -DNUMA to use -L numa.\n");
        printf("\n");
        ::exit(1);
#endif
    } else if (numa_method == Thread::LIBSCHED) {
#if defined(SCHED)
	if (_cps < 1) {
	    cpu_set_t old_mask;
	    int e0 = sched_getaffinity(0, sizeof old_mask, &old_mask);
	    if (e0 < 0) {
		switch (errno) {
		case EFAULT : printf("get_cores_per_system: EFAULT\n"); break;
		case ESRCH  : printf("get_cores_per_system: ESRCH\n");  break;
		case EPERM  : printf("get_cores_per_system: EPERM\n");  break;
		case EINVAL : printf("get_cores_per_system: EINVAL\n"); break;
		}
		assert(e0 == 0);
	    }

	    cpu_set_t new_mask;
	    for (int i=0; i < 8192; i++) {
		CPU_ZERO(&new_mask);
		CPU_SET(i, &new_mask);
		int e1 = sched_setaffinity(0, sizeof new_mask, &new_mask);
		if (e1 < 0) {
		    assert(errno == EINVAL);
		    _cps = i;
		}
	    }

	    int e2 = sched_setaffinity(0, sizeof old_mask, &old_mask);
	    if (e2 < 0) {
		switch (errno) {
		case EFAULT : printf("get_cores_per_system: EFAULT\n"); break;
		case ESRCH  : printf("get_cores_per_system: ESRCH\n");  break;
		case EPERM  : printf("get_cores_per_system: EPERM\n");  break;
		case EINVAL : printf("get_cores_per_system: EINVAL\n"); break;
		}
	    }
	    assert(e2 == 0);
	}

	cores = _cps;
#else
        printf("\n");
        printf("The benchmark must be compiled with -DSCHED to use -L sched.\n");
        printf("\n");
        ::exit(1);
#endif
    } else {
    }
    assert(0 < cores);

//    printf("leave Thread::get_cores_per_system() cores=%d\n", cores);
    return cores;
}

void 
Thread::set_system_cores_domains(int cores, int domains)
{
//    printf("enter Thread::set_system_cores_domains(cores=%d, domains=%d)\n", cores, domains);

    if (numa_method == Thread::LIBNUMA) {
#if defined(NUMA)
        printf("\n");
	printf("Thread::set_system_cores_domains is not defined for -L numa.\n");
        printf("\n");
        ::exit(1);
#else
        printf("\n");
        printf("The benchmark must be compiled with -DNUMA to use -L numa.\n");
        printf("\n");
        ::exit(1);
#endif
    } else if (numa_method == Thread::LIBSCHED) {
#if defined(SCHED)
	Thread::global_lock();

	assert(_cps < 0 && 0 < cores && cores < CPU_SETSIZE);
	_cps = cores;
	core_to_domain_map = new int[ cores ];
	bzero(core_to_domain_map, cores * sizeof core_to_domain_map[0]);

	assert(_dps < 0 && 0 < domains && domains < CPU_SETSIZE);
	_dps = domains;
        if (domain_mask == NULL) {
            domain_mask = new cpu_set_t[domains];
            assert(domain_mask != NULL);

#if defined(THREAD_PIN_CPU)
            cpu_run_queue = new Queue[domains];
            assert(cpu_run_queue != NULL);
#endif

            for (int i=0; i < domains; i++) {
                CPU_ZERO(&domain_mask[i]);
            }
        }

	Thread::global_unlock();
#else
        printf("\n");
        printf("The benchmark must be compiled with -DSCHED to use -L sched.\n");
        printf("\n");
        ::exit(1);
#endif
    } else {
    }

//    printf("leave Thread::set_system_cores_domains\n");
}

				// this function asserts that 'core' is a 
				// member of 'domain'
void 
Thread::map_core_to_domain(int core, int domain)
{
//    printf("enter Thread::map_core_to_domain(core=%d, domain=%d)\n", core, domain);

    if (numa_method == Thread::LIBNUMA) {
#if defined(NUMA)
	printf("\n");
	printf("Thread::map_core_to_domain is not defined for -L numa.\n");
	printf("\n");
	::exit(1);
#else
        printf("\n");
        printf("The benchmark must be compiled with -DNUMA to use -L numa.\n");
        printf("\n");
        ::exit(1);
#endif
    } else if (numa_method == Thread::LIBSCHED) {
#if defined(SCHED)
	Thread::global_lock();

	assert(0 <= core   && core   < _cps);
	core_to_domain_map[ core ] = domain;

	assert(0 <= domain && domain < _dps);
	CPU_SET(core, &domain_mask[domain]);
#if defined(THREAD_PIN_CPU)
        cpu_run_queue[domain].push_l(core);
#endif

	Thread::global_unlock();
#else
        printf("\n");
        printf("The benchmark must be compiled with -DSCHED to use -L sched.\n");
        printf("\n");
        ::exit(1);
#endif
    } else {
    }

//    printf("leave Thread::map_core_to_domain\n");
}

#if defined(ID)
const char* Thread_C = "\0@ID " ID;
#endif
