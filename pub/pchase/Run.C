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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#include <assert.h>

#include "Run.h"

#include "Chain.h"
#include "Timer.h"
#include "SpinBarrier.h"
#include "x86.h"


static inline double max(double v1, double v2){if(v1 < v2) return v2; return v1;}
static inline double min(double v1, double v2){if(v2 < v1) return v2; return v1;}

static int64_t chase_pointers  (int64_t chains_per_thread, 
    int64_t iterations, double seconds, Chain** root, 
    int64_t bytes_per_line, int64_t bytes_per_chain, 
    int64_t stride);
static int64_t follow_streams  (int64_t chains_per_thread, 
    int64_t iterations, double seconds, Chain** root, 
    int64_t bytes_per_line, int64_t bytes_per_chain, 
    int64_t stride);
static int64_t (*run_benchmark)(int64_t chains_per_thread, 
    int64_t iterations, double seconds, Chain** root, 
    int64_t bytes_per_line, int64_t bytes_per_chain, 
    int64_t stride) = chase_pointers;

Lock    Run::global_mutex;
int64_t Run::_ops_per_chain = 0;
double  Run::_seconds       = 1E9;


Run::Run()
: exp(NULL), bp(NULL)
{
}

Run::~Run()
{
}


int
Run::run()
{ 
				// first allocate all memory for the chains,
				// making sure it is allocated within the 
				// intended numa domains
    Chain** chain_memory = new Chain* [ this->exp->chains_per_thread ];
    assert(chain_memory != NULL);
    Chain** root         = new Chain* [ this->exp->chains_per_thread ];
    assert(root != NULL);

				// establish the node id where this thread
				// will run. threads are mapped to nodes
				// by the set-up code for Experiment.
    int run_node_id = this->exp->thread_domain[this->thread_id()];
    this->run_on_node(run_node_id);

				// establish the node id where this thread's
				// memory will be allocated.
    for (int i=0; i < this->exp->chains_per_thread; i++) {
	int alloc_node_id = this->exp->chain_domain[this->thread_id()][i];
	int links_per_chain = this->exp->links_per_chain;
	chain_memory[i] = (Chain*) Thread::alloc_on_node(alloc_node_id, links_per_chain*sizeof(Chain));
	assert(chain_memory[i] != NULL);
    }

				// initialize the chains and 
				// select the function that
				// will execute the tests
    for (int i=0; i < this->exp->chains_per_thread; i++) {
	if (this->exp->access_pattern == Experiment::RANDOM) {
	    root[i] = random_mem_init( chain_memory[i] );
	    run_benchmark = chase_pointers;
	} else if (this->exp->access_pattern == Experiment::STRIDED) {
	    if (0 < this->exp->stride) {
		root[i] = forward_mem_init( chain_memory[i] );
	    } else {
		root[i] = reverse_mem_init( chain_memory[i] );
	    }
	    run_benchmark = chase_pointers;
	} else if (this->exp->access_pattern == Experiment::STREAM) {
	    root[i] = stream_mem_init( chain_memory[i] );
	    run_benchmark = follow_streams;
	}
    }

#if defined(USE_CLFLUSH)
                                // flush everything from the cache 
                                // so it cannot be in the cache in
                                // a dirty state.
    for (int i=0; i < this->exp->chains_per_thread; i++) {
	int links_per_chain = this->exp->links_per_chain;
        Chain* root = chain_memory[i];
        for (int j=0; j < links_per_chain; j++) {
            x86_clflush(root+j);
        }
    }
#endif

    int64_t local_best_iterations = 0;
    double  local_best_delta      = 1E9;
    for (int e=0; e < this->exp->experiments; e++) {
	this->bp->barrier();    // barrier 1

                                // NOTE: it may seem a bit odd to use:
                                //     start timer
                                //     barrier
                                //     chase pointers
                                //     barrier
                                //     stop timer
                                // 
                                // the code is structured in this way 
                                // to include an appropriate penalty 
                                // if two threads happen to be scheduled
                                // sequentially rather than in parallel.
                                // this helps systems that have more
                                // cores and greater memory bandwidth.
                                // if the barriers are removed, that
                                // penalty goes away.

				// start timer
	double start = Timer::seconds();
	this->bp->barrier();    // barrier 2
				// chase pointers
        int64_t iterations = run_benchmark(this->exp->chains_per_thread, 
            this->exp->iterations, this->exp->seconds, root, 
            this->exp->bytes_per_line, this->exp->bytes_per_chain, 
            this->exp->stride);
	this->bp->barrier();    // barrier 3
				// stop timer
	double stop = Timer::seconds();

        double delta = stop - start;
        if (0 < delta && local_best_iterations/local_best_delta < iterations/delta) {
            local_best_iterations = iterations;
            local_best_delta      = delta;
        }
    }

    Run::global_mutex.lock();
    if (this->exp->iterations/Run::_seconds < local_best_iterations/local_best_delta) {
        this->exp->iterations = local_best_iterations;
        Run::_seconds         = local_best_delta;
    }
    Run::global_mutex.unlock();

    this->bp->barrier();        // barrier 4

                                // DO NOT DELETE MEMORY!!! 
                                // I know this seems counterintuitive, and
                                // there's a great temptation to want to 
                                // clean up after ourselves, but it adds a
                                // significant amount to the benchmark
                                // execution time that doesn't add any benefit.
#if defined(UNDEFINED)
volatile double dstart = Timer::seconds();
    for (int i=0; i < this->exp->chains_per_thread; i++) {
	if (chain_memory[i] != NULL) delete [] chain_memory[i];
    }
    if (chain_memory != NULL) delete [] chain_memory;
volatile double dstop = Timer::seconds();
printf("memory delete time is %lf\n",dstop-dstart);
#endif

    return 0;
}


int dummy = 0;
void
Run::mem_check( Chain *m )
{
    if (m == NULL) dummy += 1;
}


				// exclude 2 and mersienne primes, i.e.,
				// primes of the form 2**n - 1, e.g.,
				// 3, 7, 31, 127
static const int prime_table[] = { 5, 11, 13, 17, 19, 23, 37, 41, 43, 47,
    53, 61, 71, 73, 79, 83, 89, 97, 101, 103, 109, 113, 131, 137, 139, 149,
    151, 157, 163, };
static const int prime_table_size = sizeof prime_table / sizeof prime_table[0];

Chain*
Run::random_mem_init( Chain *mem )
{
				// initialize pointers --
				// choose a page at random, then use
				// one pointer from each cache line
				// within the page.  all pages and
				// cache lines are chosen at random.
    Chain* root = NULL;
    Chain* prev = NULL;
    int link_within_line = 0;

				// we must set a lock because random()
				// is not thread safe
    Run::global_mutex.lock();
    setstate(this->exp->random_state[this->thread_id()]);
    int page_factor = prime_table[ random() % prime_table_size ];
    int page_offset = random() % this->exp->pages_per_chain;
    Run::global_mutex.unlock();

    assert(0 < this->exp->pages_per_chain);
    char** page = (char**) malloc(this->exp->pages_per_chain*sizeof(char*));
    assert(page != NULL);
    for (int i=0; i < this->exp->pages_per_chain; i++) {
        page[i] = (char*) mem + i * this->exp->bytes_per_page;
    }

				// loop through the copies
    for (int i=0; i < this->exp->pages_per_chain; i++) {
        int hack = (this->exp->pages_per_chain <= 1) ? 1 : this->exp->pages_per_chain - 1;
	int copy = (page_factor*i + page_offset) % hack;
	copy = (i+1+copy)%this->exp->pages_per_chain;
	char* tmp  = page[i];
	page[i]    = page[copy];
	page[copy] = tmp;
    }

    assert(0 < this->exp->lines_per_page);
    char** line = (char**) malloc(this->exp->lines_per_page*sizeof(char*));
    assert(line != NULL);
    assert(this->exp->lines_per_chain/this->exp->lines_per_page == this->exp->pages_per_chain);

    for (int i=0; i < this->exp->pages_per_chain; i++) {
        for (int j=0; j < this->exp->lines_per_page; j++) {
            line[j] = (char*) page[i] + j * this->exp->bytes_per_line;
        }

        int line_factor = page_factor+i;
        int line_offset = page_offset*i;

        for (int j=0; j < this->exp->lines_per_page; j++) {
            int hack = (this->exp->lines_per_page <= 1) ? 1 : this->exp->lines_per_page - 1;
            int copy = (line_factor*j + line_offset) % hack;
            copy = (j+1+copy)%this->exp->lines_per_page;

            char* tmp  = line[j];
            line[j]    = line[copy];
            line[copy] = tmp;
        }

        if (root == NULL) {
            root = prev = (Chain*) line[0];
        }

        for (int j=0; j < this->exp->lines_per_page; j++) {
            prev->next = (Chain*) line[j];
            prev = prev->next;
        }
        prev->next = NULL;
    }


#if defined(MEM_CHK)
    char* addr_min = (char*) mem;
    char* addr_max = (char*) mem + this->exp->bytes_per_chain;
    int64_t line_max = this->exp->lines_per_chain;
    int64_t total_lines = this->exp->pages_per_chain * this->exp->lines_per_page;
    bool line_in_use[total_lines];
    bzero(line_in_use, sizeof line_in_use);

    prev = root;
    for (int i=0; i < total_lines; i++) {
        int32_t line_no = ((char*) prev - (char*) mem) / this->exp->bytes_per_line;
        if (line_no < 0 || total_lines <= line_no) {
            fprintf(stderr, "Line index out of range @ line %d in %s (%s).\n", __LINE__, __FILE__, __func__);
        }

        if (line_in_use[line_no]) {
            fprintf(stderr, "Pointer %d used more than once @ line %d in %s (%s).\n", line_no, __LINE__, __FILE__, __func__);
        } else if ((char*)prev < addr_min || addr_max <= (char*)prev) {
            fprintf(stderr, "Pointer out of range @ line %d in %s (%s).\n", __LINE__, __FILE__, __func__);
        }

        if (0 <= line_no && line_no < total_lines) {
            line_in_use[line_no] = true;
        }
        prev = prev->next;
    }

    if (prev != NULL) {
        fprintf(stderr, "prev != NULL @ line %d in %s (%s).\n", __LINE__, __FILE__, __func__);
    }

    int64_t total_missed=0;
    for (int i=0; i < total_lines; i++) {
	if (! line_in_use[i]) {
            fprintf(stderr, "Line %d not used @ line %d in %s (%s).\n", i, __LINE__, __FILE__, __func__);
	    total_missed+=1;
	}
    }
    if (0 < total_missed) {
        fprintf(stderr, "Lines not used @ line %d in %s (%s): total missed is %ld.\n", __LINE__, __FILE__, __func__, total_missed);
    }
#endif

    int64_t local_ops_per_chain = this->exp->pages_per_chain * this->exp->lines_per_page;
    Run::global_mutex.lock();
    Run::_ops_per_chain = local_ops_per_chain;
    Run::global_mutex.unlock();

    return root;
}

Chain*
Run::forward_mem_init( Chain *mem )
{
    Chain* root = NULL;
    Chain* prev = NULL;
    int link_within_line = 0;
    int64_t local_ops_per_chain = 0;

    for (int i=0; i < this->exp->lines_per_chain; i += this->exp->stride) {
	int link = i * this->exp->links_per_line + link_within_line;
	if (root == NULL) {
//	    printf("root       = %d(%d)[0x%x].\n", page, line_within_page, mem+link);
	    prev = root = mem + link;
	    local_ops_per_chain += 1;
	} else {
//	    printf("0x%x = %d(%d)[0x%x].\n", prev, page, line_within_page, mem+link);
	    prev->next = mem + link;
	    prev = prev->next;
	    local_ops_per_chain += 1;
	}
    }

    Run::global_mutex.lock();
    Run::_ops_per_chain = local_ops_per_chain;
    Run::global_mutex.unlock();

    return root;
}

Chain*
Run::reverse_mem_init( Chain *mem )
{
    Chain* root = NULL;
    Chain* prev = NULL;
    int link_within_line = 0;
    int64_t local_ops_per_chain = 0;

    int stride = -this->exp->stride;
    int last;
    for (int i=0; i < this->exp->lines_per_chain; i += stride) {
	last = i;
    }

    for (int i=last; 0 <= i; i -= stride) {
	int link = i * this->exp->links_per_line + link_within_line;
	if (root == NULL) {
//	    printf("root       = %d(%d)[0x%x].\n", page, line_within_page, mem+link);
	    prev = root = mem + link;
	    local_ops_per_chain += 1;
	} else {
//	    printf("0x%x = %d(%d)[0x%x].\n", prev, page, line_within_page, mem+link);
	    prev->next = mem + link;
	    prev = prev->next;
	    local_ops_per_chain += 1;
	}
    }

    Run::global_mutex.lock();
    Run::_ops_per_chain = local_ops_per_chain;
    Run::global_mutex.unlock();

    return root;
}

static int64_t dumb_ck = 0;
void
mem_chk( Chain *m )
{
    if (m == NULL) dumb_ck += 1;
}

static int64_t
chase_pointers(
    int64_t chains_per_thread,		// memory loading per thread
    int64_t iterations,			// number of iterations per experiment
    double seconds,                     // number of seconds to run
    Chain** root,			// root(s) of the chain(s) to follow
    int64_t bytes_per_line,		// ignored
    int64_t bytes_per_chain,		// ignored
    int64_t stride			// ignored
)
{
    assert( 0 < iterations || 0 < seconds );

    if (iterations <= 0) {
        volatile double start;
        volatile double stop ;
                                        // chase pointers
        switch (chains_per_thread) {
        default:
        case 1:
            for (start=stop=Timer::seconds(); (stop-start) < seconds; stop=Timer::seconds(),iterations++) {
                Chain* a = root[0];
                while (a != NULL) {
                    a = a->next;
                }
                mem_chk( a );
            }
            break;
        case 2:
            for (start=stop=Timer::seconds(); (stop-start) < seconds; stop=Timer::seconds(),iterations++) {
                Chain* a = root[0];
                Chain* b = root[1];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                }
                mem_chk( a );
                mem_chk( b );
            }
            break;
        case 3:
            for (start=stop=Timer::seconds(); (stop-start) < seconds; stop=Timer::seconds(),iterations++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
            }
            break;
        case 4:
            for (start=stop=Timer::seconds(); (stop-start) < seconds; stop=Timer::seconds(),iterations++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                Chain* d = root[3];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                    d = d->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
                mem_chk( d );
            }
            break;
        case 5:
            for (start=stop=Timer::seconds(); (stop-start) < seconds; stop=Timer::seconds(),iterations++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                Chain* d = root[3];
                Chain* e = root[4];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                    d = d->next;
                    e = e->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
                mem_chk( d );
                mem_chk( e );
            }
            break;
        case 6:
            for (start=stop=Timer::seconds(); (stop-start) < seconds; stop=Timer::seconds(),iterations++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                Chain* d = root[3];
                Chain* e = root[4];
                Chain* f = root[5];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                    d = d->next;
                    e = e->next;
                    f = f->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
                mem_chk( d );
                mem_chk( e );
                mem_chk( f );
            }
            break;
        case 7:
            for (start=stop=Timer::seconds(); (stop-start) < seconds; stop=Timer::seconds(),iterations++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                Chain* d = root[3];
                Chain* e = root[4];
                Chain* f = root[5];
                Chain* g = root[6];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                    d = d->next;
                    e = e->next;
                    f = f->next;
                    g = g->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
                mem_chk( d );
                mem_chk( e );
                mem_chk( f );
                mem_chk( g );
            }
            break;
        case 8:
            for (start=stop=Timer::seconds(); (stop-start) < seconds; stop=Timer::seconds(),iterations++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                Chain* d = root[3];
                Chain* e = root[4];
                Chain* f = root[5];
                Chain* g = root[6];
                Chain* h = root[7];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                    d = d->next;
                    e = e->next;
                    f = f->next;
                    g = g->next;
                    h = h->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
                mem_chk( d );
                mem_chk( e );
                mem_chk( f );
                mem_chk( g );
                mem_chk( h );
            }
            break;
        case 9:
            for (start=stop=Timer::seconds(); (stop-start) < seconds; stop=Timer::seconds(),iterations++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                Chain* d = root[3];
                Chain* e = root[4];
                Chain* f = root[5];
                Chain* g = root[6];
                Chain* h = root[7];
                Chain* j = root[8];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                    d = d->next;
                    e = e->next;
                    f = f->next;
                    g = g->next;
                    h = h->next;
                    j = j->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
                mem_chk( d );
                mem_chk( e );
                mem_chk( f );
                mem_chk( g );
                mem_chk( h );
                mem_chk( j );
            }
            break;
        case 10:
            for (start=stop=Timer::seconds(); (stop-start) < seconds; stop=Timer::seconds(),iterations++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                Chain* d = root[3];
                Chain* e = root[4];
                Chain* f = root[5];
                Chain* g = root[6];
                Chain* h = root[7];
                Chain* j = root[8];
                Chain* k = root[9];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                    d = d->next;
                    e = e->next;
                    f = f->next;
                    g = g->next;
                    h = h->next;
                    j = j->next;
                    k = k->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
                mem_chk( d );
                mem_chk( e );
                mem_chk( f );
                mem_chk( g );
                mem_chk( h );
                mem_chk( j );
                mem_chk( k );
            }
            break;
        case 11:
            for (start=stop=Timer::seconds(); (stop-start) < seconds; stop=Timer::seconds(),iterations++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                Chain* d = root[3];
                Chain* e = root[4];
                Chain* f = root[5];
                Chain* g = root[6];
                Chain* h = root[7];
                Chain* j = root[8];
                Chain* k = root[9];
                Chain* l = root[10];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                    d = d->next;
                    e = e->next;
                    f = f->next;
                    g = g->next;
                    h = h->next;
                    j = j->next;
                    k = k->next;
                    l = l->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
                mem_chk( d );
                mem_chk( e );
                mem_chk( f );
                mem_chk( g );
                mem_chk( h );
                mem_chk( j );
                mem_chk( k );
                mem_chk( l );
            }
            break;
        case 12:
            for (start=stop=Timer::seconds(); (stop-start) < seconds; stop=Timer::seconds(),iterations++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                Chain* d = root[3];
                Chain* e = root[4];
                Chain* f = root[5];
                Chain* g = root[6];
                Chain* h = root[7];
                Chain* j = root[8];
                Chain* k = root[9];
                Chain* l = root[10];
                Chain* m = root[11];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                    d = d->next;
                    e = e->next;
                    f = f->next;
                    g = g->next;
                    h = h->next;
                    j = j->next;
                    k = k->next;
                    l = l->next;
                    m = m->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
                mem_chk( d );
                mem_chk( e );
                mem_chk( f );
                mem_chk( g );
                mem_chk( h );
                mem_chk( j );
                mem_chk( k );
                mem_chk( l );
                mem_chk( m );
            }
            break;
        case 13:
            for (start=stop=Timer::seconds(); (stop-start) < seconds; stop=Timer::seconds(),iterations++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                Chain* d = root[3];
                Chain* e = root[4];
                Chain* f = root[5];
                Chain* g = root[6];
                Chain* h = root[7];
                Chain* j = root[8];
                Chain* k = root[9];
                Chain* l = root[10];
                Chain* m = root[11];
                Chain* n = root[12];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                    d = d->next;
                    e = e->next;
                    f = f->next;
                    g = g->next;
                    h = h->next;
                    j = j->next;
                    k = k->next;
                    l = l->next;
                    m = m->next;
                    n = n->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
                mem_chk( d );
                mem_chk( e );
                mem_chk( f );
                mem_chk( g );
                mem_chk( h );
                mem_chk( j );
                mem_chk( k );
                mem_chk( l );
                mem_chk( m );
                mem_chk( n );
            }
            break;
        case 14:
            for (start=stop=Timer::seconds(); (stop-start) < seconds; stop=Timer::seconds(),iterations++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                Chain* d = root[3];
                Chain* e = root[4];
                Chain* f = root[5];
                Chain* g = root[6];
                Chain* h = root[7];
                Chain* j = root[8];
                Chain* k = root[9];
                Chain* l = root[10];
                Chain* m = root[11];
                Chain* n = root[12];
                Chain* o = root[13];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                    d = d->next;
                    e = e->next;
                    f = f->next;
                    g = g->next;
                    h = h->next;
                    j = j->next;
                    k = k->next;
                    l = l->next;
                    m = m->next;
                    n = n->next;
                    o = o->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
                mem_chk( d );
                mem_chk( e );
                mem_chk( f );
                mem_chk( g );
                mem_chk( h );
                mem_chk( j );
                mem_chk( k );
                mem_chk( l );
                mem_chk( m );
                mem_chk( n );
                mem_chk( o );
            }
            break;
        case 15:
            for (start=stop=Timer::seconds(); (stop-start) < seconds; stop=Timer::seconds(),iterations++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                Chain* d = root[3];
                Chain* e = root[4];
                Chain* f = root[5];
                Chain* g = root[6];
                Chain* h = root[7];
                Chain* j = root[8];
                Chain* k = root[9];
                Chain* l = root[10];
                Chain* m = root[11];
                Chain* n = root[12];
                Chain* o = root[13];
                Chain* p = root[14];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                    d = d->next;
                    e = e->next;
                    f = f->next;
                    g = g->next;
                    h = h->next;
                    j = j->next;
                    k = k->next;
                    l = l->next;
                    m = m->next;
                    n = n->next;
                    o = o->next;
                    p = p->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
                mem_chk( d );
                mem_chk( e );
                mem_chk( f );
                mem_chk( g );
                mem_chk( h );
                mem_chk( j );
                mem_chk( k );
                mem_chk( l );
                mem_chk( m );
                mem_chk( n );
                mem_chk( o );
                mem_chk( p );
            }
            break;
        case 16:
            for (start=stop=Timer::seconds(); (stop-start) < seconds; stop=Timer::seconds(),iterations++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                Chain* d = root[3];
                Chain* e = root[4];
                Chain* f = root[5];
                Chain* g = root[6];
                Chain* h = root[7];
                Chain* j = root[8];
                Chain* k = root[9];
                Chain* l = root[10];
                Chain* m = root[11];
                Chain* n = root[12];
                Chain* o = root[13];
                Chain* p = root[14];
                Chain* q = root[15];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                    d = d->next;
                    e = e->next;
                    f = f->next;
                    g = g->next;
                    h = h->next;
                    j = j->next;
                    k = k->next;
                    l = l->next;
                    m = m->next;
                    n = n->next;
                    o = o->next;
                    p = p->next;
                    q = q->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
                mem_chk( d );
                mem_chk( e );
                mem_chk( f );
                mem_chk( g );
                mem_chk( h );
                mem_chk( j );
                mem_chk( k );
                mem_chk( l );
                mem_chk( m );
                mem_chk( n );
                mem_chk( o );
                mem_chk( p );
                mem_chk( q );
            }
        }
    } else {
                                        // chase pointers
        switch (chains_per_thread) {
        default:
        case 1:
            for (int64_t i=0; i < iterations; i++) {
                Chain* a = root[0];
                while (a != NULL) {
                    a = a->next;
                }
                mem_chk( a );
            }
            break;
        case 2:
            for (int64_t i=0; i < iterations; i++) {
                Chain* a = root[0];
                Chain* b = root[1];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                }
                mem_chk( a );
                mem_chk( b );
            }
            break;
        case 3:
            for (int64_t i=0; i < iterations; i++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
            }
            break;
        case 4:
            for (int64_t i=0; i < iterations; i++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                Chain* d = root[3];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                    d = d->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
                mem_chk( d );
            }
            break;
        case 5:
            for (int64_t i=0; i < iterations; i++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                Chain* d = root[3];
                Chain* e = root[4];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                    d = d->next;
                    e = e->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
                mem_chk( d );
                mem_chk( e );
            }
            break;
        case 6:
            for (int64_t i=0; i < iterations; i++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                Chain* d = root[3];
                Chain* e = root[4];
                Chain* f = root[5];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                    d = d->next;
                    e = e->next;
                    f = f->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
                mem_chk( d );
                mem_chk( e );
                mem_chk( f );
            }
            break;
        case 7:
            for (int64_t i=0; i < iterations; i++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                Chain* d = root[3];
                Chain* e = root[4];
                Chain* f = root[5];
                Chain* g = root[6];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                    d = d->next;
                    e = e->next;
                    f = f->next;
                    g = g->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
                mem_chk( d );
                mem_chk( e );
                mem_chk( f );
                mem_chk( g );
            }
            break;
        case 8:
            for (int64_t i=0; i < iterations; i++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                Chain* d = root[3];
                Chain* e = root[4];
                Chain* f = root[5];
                Chain* g = root[6];
                Chain* h = root[7];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                    d = d->next;
                    e = e->next;
                    f = f->next;
                    g = g->next;
                    h = h->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
                mem_chk( d );
                mem_chk( e );
                mem_chk( f );
                mem_chk( g );
                mem_chk( h );
            }
            break;
        case 9:
            for (int64_t i=0; i < iterations; i++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                Chain* d = root[3];
                Chain* e = root[4];
                Chain* f = root[5];
                Chain* g = root[6];
                Chain* h = root[7];
                Chain* j = root[8];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                    d = d->next;
                    e = e->next;
                    f = f->next;
                    g = g->next;
                    h = h->next;
                    j = j->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
                mem_chk( d );
                mem_chk( e );
                mem_chk( f );
                mem_chk( g );
                mem_chk( h );
                mem_chk( j );
            }
            break;
        case 10:
            for (int64_t i=0; i < iterations; i++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                Chain* d = root[3];
                Chain* e = root[4];
                Chain* f = root[5];
                Chain* g = root[6];
                Chain* h = root[7];
                Chain* j = root[8];
                Chain* k = root[9];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                    d = d->next;
                    e = e->next;
                    f = f->next;
                    g = g->next;
                    h = h->next;
                    j = j->next;
                    k = k->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
                mem_chk( d );
                mem_chk( e );
                mem_chk( f );
                mem_chk( g );
                mem_chk( h );
                mem_chk( j );
                mem_chk( k );
            }
            break;
        case 11:
            for (int64_t i=0; i < iterations; i++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                Chain* d = root[3];
                Chain* e = root[4];
                Chain* f = root[5];
                Chain* g = root[6];
                Chain* h = root[7];
                Chain* j = root[8];
                Chain* k = root[9];
                Chain* l = root[10];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                    d = d->next;
                    e = e->next;
                    f = f->next;
                    g = g->next;
                    h = h->next;
                    j = j->next;
                    k = k->next;
                    l = l->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
                mem_chk( d );
                mem_chk( e );
                mem_chk( f );
                mem_chk( g );
                mem_chk( h );
                mem_chk( j );
                mem_chk( k );
                mem_chk( l );
            }
            break;
        case 12:
            for (int64_t i=0; i < iterations; i++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                Chain* d = root[3];
                Chain* e = root[4];
                Chain* f = root[5];
                Chain* g = root[6];
                Chain* h = root[7];
                Chain* j = root[8];
                Chain* k = root[9];
                Chain* l = root[10];
                Chain* m = root[11];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                    d = d->next;
                    e = e->next;
                    f = f->next;
                    g = g->next;
                    h = h->next;
                    j = j->next;
                    k = k->next;
                    l = l->next;
                    m = m->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
                mem_chk( d );
                mem_chk( e );
                mem_chk( f );
                mem_chk( g );
                mem_chk( h );
                mem_chk( j );
                mem_chk( k );
                mem_chk( l );
                mem_chk( m );
            }
            break;
        case 13:
            for (int64_t i=0; i < iterations; i++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                Chain* d = root[3];
                Chain* e = root[4];
                Chain* f = root[5];
                Chain* g = root[6];
                Chain* h = root[7];
                Chain* j = root[8];
                Chain* k = root[9];
                Chain* l = root[10];
                Chain* m = root[11];
                Chain* n = root[12];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                    d = d->next;
                    e = e->next;
                    f = f->next;
                    g = g->next;
                    h = h->next;
                    j = j->next;
                    k = k->next;
                    l = l->next;
                    m = m->next;
                    n = n->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
                mem_chk( d );
                mem_chk( e );
                mem_chk( f );
                mem_chk( g );
                mem_chk( h );
                mem_chk( j );
                mem_chk( k );
                mem_chk( l );
                mem_chk( m );
                mem_chk( n );
            }
            break;
        case 14:
            for (int64_t i=0; i < iterations; i++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                Chain* d = root[3];
                Chain* e = root[4];
                Chain* f = root[5];
                Chain* g = root[6];
                Chain* h = root[7];
                Chain* j = root[8];
                Chain* k = root[9];
                Chain* l = root[10];
                Chain* m = root[11];
                Chain* n = root[12];
                Chain* o = root[13];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                    d = d->next;
                    e = e->next;
                    f = f->next;
                    g = g->next;
                    h = h->next;
                    j = j->next;
                    k = k->next;
                    l = l->next;
                    m = m->next;
                    n = n->next;
                    o = o->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
                mem_chk( d );
                mem_chk( e );
                mem_chk( f );
                mem_chk( g );
                mem_chk( h );
                mem_chk( j );
                mem_chk( k );
                mem_chk( l );
                mem_chk( m );
                mem_chk( n );
                mem_chk( o );
            }
            break;
        case 15:
            for (int64_t i=0; i < iterations; i++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                Chain* d = root[3];
                Chain* e = root[4];
                Chain* f = root[5];
                Chain* g = root[6];
                Chain* h = root[7];
                Chain* j = root[8];
                Chain* k = root[9];
                Chain* l = root[10];
                Chain* m = root[11];
                Chain* n = root[12];
                Chain* o = root[13];
                Chain* p = root[14];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                    d = d->next;
                    e = e->next;
                    f = f->next;
                    g = g->next;
                    h = h->next;
                    j = j->next;
                    k = k->next;
                    l = l->next;
                    m = m->next;
                    n = n->next;
                    o = o->next;
                    p = p->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
                mem_chk( d );
                mem_chk( e );
                mem_chk( f );
                mem_chk( g );
                mem_chk( h );
                mem_chk( j );
                mem_chk( k );
                mem_chk( l );
                mem_chk( m );
                mem_chk( n );
                mem_chk( o );
                mem_chk( p );
            }
            break;
        case 16:
            for (int64_t i=0; i < iterations; i++) {
                Chain* a = root[0];
                Chain* b = root[1];
                Chain* c = root[2];
                Chain* d = root[3];
                Chain* e = root[4];
                Chain* f = root[5];
                Chain* g = root[6];
                Chain* h = root[7];
                Chain* j = root[8];
                Chain* k = root[9];
                Chain* l = root[10];
                Chain* m = root[11];
                Chain* n = root[12];
                Chain* o = root[13];
                Chain* p = root[14];
                Chain* q = root[15];
                while (a != NULL) {
                    a = a->next;
                    b = b->next;
                    c = c->next;
                    d = d->next;
                    e = e->next;
                    f = f->next;
                    g = g->next;
                    h = h->next;
                    j = j->next;
                    k = k->next;
                    l = l->next;
                    m = m->next;
                    n = n->next;
                    o = o->next;
                    p = p->next;
                    q = q->next;
                }
                mem_chk( a );
                mem_chk( b );
                mem_chk( c );
                mem_chk( d );
                mem_chk( e );
                mem_chk( f );
                mem_chk( g );
                mem_chk( h );
                mem_chk( j );
                mem_chk( k );
                mem_chk( l );
                mem_chk( m );
                mem_chk( n );
                mem_chk( o );
                mem_chk( p );
                mem_chk( q );
            }
        }
    }

    return iterations;
}

				// NOT WRITTEN YET -- DMP
				// JUST A PLACE HOLDER!
Chain*
Run::stream_mem_init( Chain *mem )
{
// fprintf(stderr, "made it into stream_mem_init.\n");
// fprintf(stderr, "chains_per_thread = %ld\n", this->exp->chains_per_thread);
// fprintf(stderr, "iterations        = %ld\n", this->exp->iterations);
// fprintf(stderr, "bytes_per_chain   = %ld\n", this->exp->bytes_per_chain);
// fprintf(stderr, "stride            = %ld\n", this->exp->stride);
    int64_t local_ops_per_chain = 0;
    double* tmp = (double *) mem;
    int64_t refs_per_line  = this->exp->bytes_per_line  / sizeof(double);
    int64_t refs_per_chain = this->exp->bytes_per_chain / sizeof(double);
// fprintf(stderr, "refs_per_chain    = %ld\n", refs_per_chain);

    for (int64_t i=0; i < refs_per_chain; i += this->exp->stride*refs_per_line) {
	tmp[i] = 0;
	local_ops_per_chain += 1;
    }

    Run::global_mutex.lock();
    Run::_ops_per_chain = local_ops_per_chain;
    Run::global_mutex.unlock();

// fprintf(stderr, "made it out of stream_mem_init.\n");
    return mem;
}

static int64_t summ_ck = 0;
void
sum_chk( double t )
{
    if (t != 0) summ_ck += 1;
}

				// NOT WRITTEN YET -- DMP
				// JUST A PLACE HOLDER!
static int64_t 
follow_streams(
    int64_t chains_per_thread,		// memory loading per thread
    int64_t iterations,			// number of iterations per experiment
    double seconds,                     // or number of seconds to run
    Chain** root,			// root(s) of the chain(s) to follow
    int64_t bytes_per_line,		// ignored
    int64_t bytes_per_chain,		// ignored
    int64_t stride			// ignored
)
{
    int64_t refs_per_line  = bytes_per_line  / sizeof(double);
    int64_t refs_per_chain = bytes_per_chain / sizeof(double);

				// chase pointers
    switch (chains_per_thread) {
    default:
    case 1:
	for (int64_t i=0; i < iterations; i++) {
	    double t = 0;
	    double* a0 = (double *) root[0];
	    for (int64_t j=0; j < refs_per_chain; j+=stride*refs_per_line) {
		t += a0[j];
	    }
	    sum_chk( t );
	}
	break;
    case 2:
	for (int64_t i=0; i < iterations; i++) {
	    double t = 0;
	    double* a0 = (double *) root[0];
	    double* a1 = (double *) root[1];
	    for (int64_t j=0; j < refs_per_chain; j+=stride*refs_per_line) {
		t += a0[j] + a1[j];
	    }
	    sum_chk( t );
	}
	break;
    case 3:
	for (int64_t i=0; i < iterations; i++) {
	    double t = 0;
	    double* a0 = (double *) root[0];
	    double* a1 = (double *) root[1];
	    double* a2 = (double *) root[2];
	    for (int64_t j=0; j < refs_per_chain; j+=stride*refs_per_line) {
		t += a0[j] + a1[j] + a2[j];
	    }
	    sum_chk( t );
	}
	break;
    case 4:
	for (int64_t i=0; i < iterations; i++) {
	    double t = 0;
	    double* a0 = (double *) root[0];
	    double* a1 = (double *) root[1];
	    double* a2 = (double *) root[2];
	    double* a3 = (double *) root[3];
	    for (int64_t j=0; j < refs_per_chain; j+=stride*refs_per_line) {
		t += a0[j] + a1[j] + a2[j] + a3[j];
	    }
	    sum_chk( t );
	}
	break;
    case 5:
	for (int64_t i=0; i < iterations; i++) {
	    double t = 0;
	    double* a0 = (double *) root[0];
	    double* a1 = (double *) root[1];
	    double* a2 = (double *) root[2];
	    double* a3 = (double *) root[3];
	    double* a4 = (double *) root[4];
	    for (int64_t j=0; j < refs_per_chain; j+=stride*refs_per_line) {
		t += a0[j] + a1[j] + a2[j] + a3[j] + a4[j];
	    }
	    sum_chk( t );
	}
	break;
    case 6:
	for (int64_t i=0; i < iterations; i++) {
	    double t = 0;
	    double* a0 = (double *) root[0];
	    double* a1 = (double *) root[1];
	    double* a2 = (double *) root[2];
	    double* a3 = (double *) root[3];
	    double* a4 = (double *) root[4];
	    double* a5 = (double *) root[5];
	    for (int64_t j=0; j < refs_per_chain; j+=stride*refs_per_line) {
		t += a0[j] + a1[j] + a2[j] + a3[j] + a4[j] + a5[j];
	    }
	    sum_chk( t );
	}
	break;
    case 7:
	for (int64_t i=0; i < iterations; i++) {
	    double t = 0;
	    double* a0 = (double *) root[0];
	    double* a1 = (double *) root[1];
	    double* a2 = (double *) root[2];
	    double* a3 = (double *) root[3];
	    double* a4 = (double *) root[4];
	    double* a5 = (double *) root[5];
	    double* a6 = (double *) root[6];
	    for (int64_t j=0; j < refs_per_chain; j+=stride*refs_per_line) {
		t += a0[j] + a1[j] + a2[j] + a3[j] + a4[j] + a5[j] + a6[j];
	    }
	    sum_chk( t );
	}
	break;
    case 8:
	for (int64_t i=0; i < iterations; i++) {
	    double t = 0;
	    double* a0 = (double *) root[0];
	    double* a1 = (double *) root[1];
	    double* a2 = (double *) root[2];
	    double* a3 = (double *) root[3];
	    double* a4 = (double *) root[4];
	    double* a5 = (double *) root[5];
	    double* a6 = (double *) root[6];
	    double* a7 = (double *) root[7];
	    for (int64_t j=0; j < refs_per_chain; j+=stride*refs_per_line) {
		t += a0[j] + a1[j] + a2[j] + a3[j] + a4[j] + a5[j] + a6[j] + a7[j];
	    }
	    sum_chk( t );
	}
	break;
    case 9:
	for (int64_t i=0; i < iterations; i++) {
	    double t = 0;
	    double* a0 = (double *) root[0];
	    double* a1 = (double *) root[1];
	    double* a2 = (double *) root[2];
	    double* a3 = (double *) root[3];
	    double* a4 = (double *) root[4];
	    double* a5 = (double *) root[5];
	    double* a6 = (double *) root[6];
	    double* a7 = (double *) root[7];
	    double* a8 = (double *) root[8];
	    for (int64_t j=0; j < refs_per_chain; j+=stride*refs_per_line) {
		t += a0[j] + a1[j] + a2[j] + a3[j] + a4[j] + a5[j] + a6[j] + a7[j] +
		     a8[j];
	    }
	    sum_chk( t );
	}
	break;
    case 10:
	for (int64_t i=0; i < iterations; i++) {
	    double t = 0;
	    double* a0 = (double *) root[0];
	    double* a1 = (double *) root[1];
	    double* a2 = (double *) root[2];
	    double* a3 = (double *) root[3];
	    double* a4 = (double *) root[4];
	    double* a5 = (double *) root[5];
	    double* a6 = (double *) root[6];
	    double* a7 = (double *) root[7];
	    double* a8 = (double *) root[8];
	    double* a9 = (double *) root[9];
	    for (int64_t j=0; j < refs_per_chain; j+=stride*refs_per_line) {
		t += a0[j] + a1[j] + a2[j] + a3[j] + a4[j] + a5[j] + a6[j] + a7[j] +
		     a8[j] + a9[j];
	    }
	    sum_chk( t );
	}
	break;
    case 11:
	for (int64_t i=0; i < iterations; i++) {
	    double t = 0;
	    double* a0  = (double *) root[ 0];
	    double* a1  = (double *) root[ 1];
	    double* a2  = (double *) root[ 2];
	    double* a3  = (double *) root[ 3];
	    double* a4  = (double *) root[ 4];
	    double* a5  = (double *) root[ 5];
	    double* a6  = (double *) root[ 6];
	    double* a7  = (double *) root[ 7];
	    double* a8  = (double *) root[ 8];
	    double* a9  = (double *) root[ 9];
	    double* a10 = (double *) root[10];
	    for (int64_t j=0; j < refs_per_chain; j+=stride*refs_per_line) {
		t += a0[j] + a1[j] + a2 [j] + a3[j] + a4[j] + a5[j] + a6[j] + a7[j] +
		     a8[j] + a9[j] + a10[j];
	    }
	    sum_chk( t );
	}
	break;
    case 12:
	for (int64_t i=0; i < iterations; i++) {
	    double t = 0;
	    double* a0  = (double *) root[ 0];
	    double* a1  = (double *) root[ 1];
	    double* a2  = (double *) root[ 2];
	    double* a3  = (double *) root[ 3];
	    double* a4  = (double *) root[ 4];
	    double* a5  = (double *) root[ 5];
	    double* a6  = (double *) root[ 6];
	    double* a7  = (double *) root[ 7];
	    double* a8  = (double *) root[ 8];
	    double* a9  = (double *) root[ 9];
	    double* a10 = (double *) root[10];
	    double* a11 = (double *) root[11];
	    for (int64_t j=0; j < refs_per_chain; j+=stride*refs_per_line) {
		t += a0[j] + a1[j] + a2 [j] + a3 [j] + a4[j] + a5[j] + a6[j] + a7[j] +
		     a8[j] + a9[j] + a10[j] + a11[j];
	    }
	    sum_chk( t );
	}
	break;
    case 13:
	for (int64_t i=0; i < iterations; i++) {
	    double t = 0;
	    double* a0  = (double *) root[ 0];
	    double* a1  = (double *) root[ 1];
	    double* a2  = (double *) root[ 2];
	    double* a3  = (double *) root[ 3];
	    double* a4  = (double *) root[ 4];
	    double* a5  = (double *) root[ 5];
	    double* a6  = (double *) root[ 6];
	    double* a7  = (double *) root[ 7];
	    double* a8  = (double *) root[ 8];
	    double* a9  = (double *) root[ 9];
	    double* a10 = (double *) root[10];
	    double* a11 = (double *) root[11];
	    double* a12 = (double *) root[12];
	    for (int64_t j=0; j < refs_per_chain; j+=stride*refs_per_line) {
		t += a0[j] + a1[j] + a2 [j] + a3 [j] + a4 [j] + a5[j] + a6[j] + a7[j] +
		     a8[j] + a9[j] + a10[j] + a11[j] + a12[j];
	    }
	    sum_chk( t );
	}
	break;
    case 14:
	for (int64_t i=0; i < iterations; i++) {
	    double t = 0;
	    double* a0  = (double *) root[ 0];
	    double* a1  = (double *) root[ 1];
	    double* a2  = (double *) root[ 2];
	    double* a3  = (double *) root[ 3];
	    double* a4  = (double *) root[ 4];
	    double* a5  = (double *) root[ 5];
	    double* a6  = (double *) root[ 6];
	    double* a7  = (double *) root[ 7];
	    double* a8  = (double *) root[ 8];
	    double* a9  = (double *) root[ 9];
	    double* a10 = (double *) root[10];
	    double* a11 = (double *) root[11];
	    double* a12 = (double *) root[12];
	    double* a13 = (double *) root[13];
	    for (int64_t j=0; j < refs_per_chain; j+=stride*refs_per_line) {
		t += a0[j] + a1[j] + a2 [j] + a3 [j] + a4 [j] + a5 [j] + a6[j] + a7[j] +
		     a8[j] + a9[j] + a10[j] + a11[j] + a12[j] + a13[j];
	    }
	    sum_chk( t );
	}
	break;
    case 15:
	for (int64_t i=0; i < iterations; i++) {
	    double t = 0;
	    double* a0  = (double *) root[ 0];
	    double* a1  = (double *) root[ 1];
	    double* a2  = (double *) root[ 2];
	    double* a3  = (double *) root[ 3];
	    double* a4  = (double *) root[ 4];
	    double* a5  = (double *) root[ 5];
	    double* a6  = (double *) root[ 6];
	    double* a7  = (double *) root[ 7];
	    double* a8  = (double *) root[ 8];
	    double* a9  = (double *) root[ 9];
	    double* a10 = (double *) root[10];
	    double* a11 = (double *) root[11];
	    double* a12 = (double *) root[12];
	    double* a13 = (double *) root[13];
	    double* a14 = (double *) root[14];
	    for (int64_t j=0; j < refs_per_chain; j+=stride*refs_per_line) {
		t += a0[j] + a1[j] + a2 [j] + a3 [j] + a4 [j] + a5 [j] + a6 [j] + a7[j] +
		     a8[j] + a9[j] + a10[j] + a11[j] + a12[j] + a13[j] + a14[j];
	    }
	    sum_chk( t );
	}
	break;
    case 16:
	for (int64_t i=0; i < iterations; i++) {
	    double t = 0;
	    double* a0  = (double *) root[ 0];
	    double* a1  = (double *) root[ 1];
	    double* a2  = (double *) root[ 2];
	    double* a3  = (double *) root[ 3];
	    double* a4  = (double *) root[ 4];
	    double* a5  = (double *) root[ 5];
	    double* a6  = (double *) root[ 6];
	    double* a7  = (double *) root[ 7];
	    double* a8  = (double *) root[ 8];
	    double* a9  = (double *) root[ 9];
	    double* a10 = (double *) root[10];
	    double* a11 = (double *) root[11];
	    double* a12 = (double *) root[12];
	    double* a13 = (double *) root[13];
	    double* a14 = (double *) root[14];
	    double* a15 = (double *) root[15];
	    for (int64_t j=0; j < refs_per_chain; j+=stride*refs_per_line) {
		t += a0[j] + a1[j] + a2 [j] + a3 [j] + a4 [j] + a5 [j] + a6 [j] + a7 [j] +
		     a8[j] + a9[j] + a10[j] + a11[j] + a12[j] + a13[j] + a14[j] + a15[j];
	    }
	    sum_chk( t );
	}
	break;
    }

    return iterations;
}

#if defined(ID)
const char* Run_C = "\0@ID " ID;
#endif
