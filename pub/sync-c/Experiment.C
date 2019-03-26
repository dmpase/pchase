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
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Thread.h"
#include "Experiment.h"

Experiment::Experiment() :
    lock_ops_per_thread(0),
    lock_method        (PTHREAD_SPIN),
    alignment          (64), 
    experiments        (1), 
    iterations         (0),
    delay              (0),
    num_numa_domains   (1),
    num_threads        (1), 
    numa_max_domain    (0),
    numa_placement     (LOCAL),
    page_size          (4096), 
    output_mode        (TABLE),
    placement_map      (NULL),
    seconds            (10),
    thread_domain      (NULL)
{
}

Experiment::~Experiment()
{
}


int
Experiment::parse_args(int argc, char* argv[])
{
    int error = 0;

				// -L or --library {numa | sched}
    bool sched_found=false;
    for (int i=1; i < argc; i++) {
	if (strcmp(argv[i], "-L") == 0 || strcasecmp(argv[i], "--library") == 0) {
	    i++;
	    if (i == argc) { error = 1; break; }
	    if (strcasecmp(argv[i], "numa") == 0) {
		Thread::set_numa_method(Thread::LIBNUMA);
	    } else if (strcasecmp(argv[i], "sched") == 0) {
		sched_found = true;
		Thread::set_numa_method(Thread::LIBSCHED);
	    } else if (strcasecmp(argv[i], "none") == 0) {
		Thread::set_numa_method(Thread::NONE);
	    }
	}
    }

				// -d or --domains "d0,d1,...,dn"
    bool domains_found=false;
    for (int i=1; i < argc; i++) {
	if (strcmp(argv[i], "-d") == 0 || strcasecmp(argv[i], "--domains") == 0) {
	    i++;
	    if (i == argc) { error = 1; break; }
	    if (!sched_found) { 
		printf("\n");
		printf("The option --domains may only be used with --library sched.\n");
		printf("\n");
		error = 1; 
		break; 
	    }
	    if (domains_found) {
		printf("\n");
		printf("The option --domains may only be used once.\n");
		printf("\n");
		error = 1; 
		break; 
	    }
	    domains_found = true;
	    const char* domains = argv[i];
				// make a pass to find the largest domain number
	    int num_cores  = 1;
	    int max_domain = 0;
	    int cur_domain = 0;
	    for (int j=0; domains[j] != '\0'; j++) {
		if (domains[j] == ',') {
		    if (max_domain < cur_domain) {
			max_domain = cur_domain;
		    }
		    		// be careful of trailing commas
		    if (domains[j+1] != '\0') num_cores += 1;
		    cur_domain = 0;
		} else if ('0' <= domains[j] && domains[j] <= '9') {
		    cur_domain = 10 * cur_domain + domains[j] - '0';
		} else {
		    error = 1;
		    break;
		}
	    }
	    if (max_domain < cur_domain) {
		max_domain = cur_domain;
	    }

	    			// allocate the domain mask now
	    Thread::set_system_cores_domains(num_cores, max_domain+1);

				// set up the domain masks
	    cur_domain = 0;
	    int cur_core = 0;
	    for (int j=0; domains[j] != '\0'; j++) {
		if (domains[j] == ',') {
		    Thread::map_core_to_domain(cur_core, cur_domain);
		    cur_domain = 0;
		    cur_core += 1;
		} else if ('0' <= domains[j] && domains[j] <= '9') {
		    cur_domain = 10 * cur_domain + domains[j] - '0';
		} else {
		    error = 1;
		    break;
		}
	    }
	    Thread::map_core_to_domain(num_cores-1, cur_domain);
	}
    }

    if (sched_found && !domains_found) { 
	printf("\n");
	printf("The option --domains must be used whenever --library sched is used.\n");
	printf("\n");
	error = 1; 
    }

				// -L or --library {numa|sync}
				// -a or --align <number representing alignment boundary>
				// -d or --domains "d0,d1,...,dn"
				// -e or --experiments <number of experiments>
				// -i or --iterations <number of iterations>
				// -m or --method <sync method, e.g., lock, cmpxchg, atomic op, etc.>
				// -n or --numa <placement>
				// -o or --output {table|hdr|csv|both}
				// -p or --page <bytes per page>
				// -s or --seconds <number of seconds>
				// -t or --threads <number of threads>
    for (int i=1; i < argc; i++) {
	if (strcmp(argv[i], "-L") == 0 || strcasecmp(argv[i], "--library") == 0) {
	    i++;
	    if (i == argc) { error = 1; break; }
	    if (strcmp(argv[i], "numa") == 0) {
		;
	    } else if (strcmp(argv[i], "sched") == 0) {
		;
	    } else if (strcmp(argv[i], "none") == 0) {
		;
	    } else {
		error = 1;
		break;
	    }
	} else if (strcmp(argv[i], "-a") == 0 || strcasecmp(argv[i], "--align") == 0) {
	    i++;
	    if (i == argc) { error = 1; break; }
	    this->alignment = Experiment::parse_number(argv[i]);
	    if (this->alignment == 0) { error = 1; break; }
	} else if (strcmp(argv[i], "-d") == 0 || strcasecmp(argv[i], "--domains") == 0) {
	    i++;
	    if (i == argc) { error = 1; break; }
	    const char* domains = argv[i];
				// make a pass to find the largest domain number
	    for (int j=0; domains[j] != '\0'; j++) {
		if (domains[j] == ',') {
		    ;
		} else if ('0' <= domains[j] && domains[j] <= '9') {
		    ;
		} else {
		    error = 1;
		    break;
		}
	    }

				// set up the domain masks
	    for (int j=0; domains[j] != '\0'; j++) {
		if (domains[j] == ',') {
		    ;
		} else if ('0' <= domains[j] && domains[j] <= '9') {
		    ;
		} else {
		    error = 1;
		    break;
		}
	    }
	} else if (strcmp(argv[i], "-e") == 0 || strcasecmp(argv[i], "--experiments") == 0) {
	    i++;
	    if (i == argc) { error = 1; break; }
	    this->experiments = Experiment::parse_number(argv[i]);
	    if (this->experiments == 0) { error = 1; break; }
	} else if (strcmp(argv[i], "-h") == 0 || strcasecmp(argv[i], "--help") == 0) {
	    error = 1; 
	    break;
	} else if (strcmp(argv[i], "-i") == 0 || strcasecmp(argv[i], "--iterations") == 0) {
	    i++;
	    if (i == argc) { error = 1; break; }
	    this->iterations = Experiment::parse_number(argv[i]);
	    this->seconds = 0;
	    if (this->iterations == 0) { error = 1; break; }
	} else if (strcmp(argv[i], "-m") == 0 || strcasecmp(argv[i], "--method") == 0) {
	    i++;
	    if (i == argc) { error = 1; break; }
	    if (strcasecmp(argv[i], "spin") == 0) {
		this->lock_method = PTHREAD_SPIN;
	    } else if (strcasecmp(argv[i], "mutex") == 0) {
		this->lock_method = PTHREAD_MUTEX;
	    } else if (strcasecmp(argv[i], "cmpxspin") == 0) {
		this->lock_method = CMPXSPIN;
	    } else if (strcasecmp(argv[i], "cmpxchg32") == 0) {
		this->lock_method = CMPXCHG32;
	    } else if (strcasecmp(argv[i], "cmpxchg64") == 0) {
		this->lock_method = CMPXCHG64;
	    } else if (strcasecmp(argv[i], "fairspin") == 0) {
		this->lock_method = FAIRSPIN;
	    } else if (strcasecmp(argv[i], "atomic32") == 0) {
		this->lock_method = ATOMIC32;
	    } else if (strcasecmp(argv[i], "atomic64") == 0) {
		this->lock_method = ATOMIC64;
	    } else if (strcasecmp(argv[i], "reader") == 0) {
		this->lock_method = PTHREAD_READER;
	    } else if (strcasecmp(argv[i], "writer") == 0) {
		this->lock_method = PTHREAD_WRITER;
	    } else if (strcasecmp(argv[i], "barrier") == 0) {
		this->lock_method = PTHREAD_BARRIER;
	    } else {
		error = 1;
		break;
	    }
	} else if (strcmp(argv[i], "-n") == 0 || strcasecmp(argv[i], "--numa") == 0) {
	    i++;
	    if (i == argc) { error = 1; break; }
	    if (strcasecmp(argv[i], "gen") == 0) {
		this->numa_placement = GEN;
		i++;
		if (i == argc) { error = 1; break; }
		this->gen_thread_domain = argv[i];
	    } else if (strcasecmp(argv[i], "interleave") == 0) {
		this->numa_placement = INTERLEAVE;
	    } else if (strcasecmp(argv[i], "local") == 0) {
		this->numa_placement = LOCAL;
	    } else if (strcasecmp(argv[i], "map") == 0) {
		this->numa_placement = MAP;
		i++;
		if (i == argc) { error = 1; break; }
		this->placement_map = argv[i];

				// count the thread descriptors by counting ";" up to EOS
		int threads = 1;
		const char *p = this->placement_map;
		while (*p != '\0') {
		    if (*p == ';') threads += 1;
		    p++;
		}
		this->num_threads = threads;
	    } else {
		error = 1;
		break;
	    }
	} else if (strcmp(argv[i], "-o") == 0 || strcasecmp(argv[i], "--output") == 0) {
	    i++;
	    if (i == argc) { error = 1; break; }
	    if (strcasecmp(argv[i], "table") == 0) {
		this->output_mode = TABLE;
	    } else if (strcasecmp(argv[i], "csv") == 0) {
		this->output_mode = CSV;
	    } else if (strcasecmp(argv[i], "both") == 0) {
		this->output_mode = BOTH;
	    } else if (strcasecmp(argv[i], "hdr") == 0) {
		this->output_mode = HEADER;
	    } else if (strcasecmp(argv[i], "header") == 0) {
		this->output_mode = HEADER;
	    } else {
		error = 1;
		break;
	    }
	} else if (strcmp(argv[i], "-p") == 0 || strcasecmp(argv[i], "--page") == 0) {
	    i++;
	    if (i == argc) { error = 1; break; }
	    this->page_size = Experiment::parse_number(argv[i]);
	    if (this->page_size == 0) { error = 1; break; }
	} else if (strcmp(argv[i], "-s") == 0 || strcasecmp(argv[i], "--seconds") == 0) {
	    i++;
	    if (i == argc) { error = 1; break; }
	    this->seconds = Experiment::parse_real(argv[i]);
	    this->iterations = 0;
	    if (this->seconds == 0) { error = 1; break; }
	} else if (strcmp(argv[i], "-t") == 0 || strcasecmp(argv[i], "--threads") == 0) {
	    i++;
	    if (i == argc) { error = 1; break; }
	    this->num_threads = Experiment::parse_number(argv[i]);
	    if (this->num_threads == 0) { error = 1; break; }
	} else if (strcmp(argv[i], "-z") == 0 || strcasecmp(argv[i], "--delay") == 0) {
	    i++;
	    if (i == argc || argv[i][0] < '0' || '9' < argv[i][0]) { error = 1; break; }
	    this->delay = Experiment::parse_number(argv[i]);
	} else {
	    error = 1;
	    break;
	}
    }

				// -L or --library {numa|sync}
				// -a or --align <number representing alignment boundary>
				// -d or --domains "d0,d1,...,dn"
				// -e or --experiments <number of experiments>
				// -i or --iterations <number of iterations>
				// -m or --method <sync method, e.g., lock, cmpxchg, atomic op, etc.>
				// -n or --numa <placement>
				// -o or --output  {table|hdr|csv|both}
				// -s or --seconds <number of seconds>
				// -t or --threads <number of threads>
				// -z or --delay   <number of threads>

				// if we've hit an error, print a message and quit
    if (error) {
	for (int i=0; i < argc; i++) {
	    printf("%s ", argv[i]);
	}
	printf("usage: %s <options>\n", argv[0]);
	printf("where <options> are selected from the following:\n");
	printf("    [-h|--help]                    # this message\n");
	printf("    [-L|--library]     <library>   # select the library for memory and thread placement\n");
	printf("    [-d|--domains]     <domains>   # mapping of cores to domains\n");
	printf("    [-a|--align]       <number>    # alignment boundary for locks\n");
	printf("    [-e|--experiments] <number>    # experiments\n");
	printf("    [-i|--iterations]  <number>    # iterations per experiment\n");
	printf("    [-m|--method]      <method>    # synchronization method\n");
	printf("    [-n|--numa]        <placement> # numa placement\n");
	printf("    [-o|--output]      <format>    # output format\n");
	printf("    [-s|--seconds]     <number>    # run each experiment for <number> seconds\n");
	printf("    [-t|--threads]     <number>    # number of threads\n");
	printf("    [-z|--delay]       <number>    # number of CPU ticks between acquiring and releasing a lock\n");
	printf("\n");
	printf("<library> is selected from the following:\n");
	printf("    numa                           # \n");
	printf("    sched                          # \n");
	printf("    none                           # \n");
	printf("\n");
	printf("<domains> is a mapping of processor cores to NUMA domains in the format:\n");
	printf("\t\"D1,D2,...,Dn\"\n");
	printf("where each Di is zero or a small positive integer representing the NUMA domain in \n");
	printf("which the ith core resides. The benchmark will use only n cores (or SMT threads) even if\n");
	printf("the system supports more than are specified. Behavior is system dependent when more cores\n");
	printf("are specified than exist within the system.\n");
	printf("When --domain is specified, the benchmark uses the specified mapping to determin how it\n");
	printf("should allocate threads to cores in order to achieve the mapping of threads to domains\n");
	printf("specified by --numa. All cores and domains to be used must be included.\n");
	printf("When --domain is NOT specified, the benchmark attempts to determine the mapping on its \n");
	printf("own using libnuma or other means.\n");
	printf("\n");
	printf("<number> is an non-negative integer in decimal, octal or hexadecimal. Decimal numbers \n");
	printf("may have a suffix K, M, G or T appended to represent their respective powers of 2.\n");
	printf("Octal numbers are of the form 0dd...d where d is one of [0-7]. Hexadecimal numbers \n");
	printf("are of the form 0xdd...d where d is chosen from [0-9a-f].\n");
	printf("\n");
	printf("<method> is the specific method to use for synchronization. The available methods are:\n");
	printf("    spin                           # PTHREAD spin lock/unlock\n");
	printf("    mutex                          # PTHREAD mutex lock/unlock\n");
	printf("    reader                         # PTHREAD reader lock/unlock\n");
	printf("    writer                         # PTHREAD writer lock/unlock\n");
	printf("    cmpxspin                       # minimal spin lock/unlock built from a cmpxchg instruction\n");
	printf("    cmpxchg32                      # just the 4B cmpxchg instruction\n");
	printf("    cmpxchg64                      # just the 8B cmpxchg instruction\n");
	printf("    fairspin                       # fair spin lock/unlock built from a cmpxchg instruction\n");
	printf("    atomic32                       # 32-bit atomic increment \n");
	printf("    atomic64                       # 64-bit atomic increment \n");
	printf("\n");
	printf("<placement> is selected from the following:\n");
	printf("    interleave                     # not implemented\n");
	printf("    local                          # locks are allocated locally (default)\n");
	printf("    gen <thread>                   # locks are allocated according to <thread> and <lock>\n");
	printf("    map <map>                      # explicit mapping of threads, source and target to domains\n");
	printf("\n");
	printf("<thread> and <lock> are simple unary expressions used to compute the NUMA domain for the thread, \n");
	printf("source and target, respectively. All expressions follow the same simple patterns and similar rules apply.\n");
	printf("When <thread> is a number, whether decimal, octal or hexadecimal, the NUMA domain is the number specified. \n");
	printf("When <thread> is \"+d\", where d is a number, the NUMA domain is the domain of the thread index plus the number. \n");
	printf("When <thread> is \"-d\", where d is a number, the NUMA domain is the domain of the thread index minus the number. \n");
	printf("When <thread> is \"^d\", where d is a number, the NUMA domain is the domain of the thread index XORed with the number. \n");
	printf("When <lock> is the letter i, locks are interleaved across all NUMA domains.\n");
	printf("In all cases, the domain is computed using modulo arithmetic.\n");
	printf("\n"); 
	printf("<map> has the form \"t1;t2;...;tn\"\n");
	printf("where t[i] is the NUMA domain where the ith thread is run.\n");
	printf("(The values t[i] be zero or small positive integers.)\n");
	printf("\n");
	printf("Note: maps override the -t or --threads specification,\n");
	printf("NUMA domains are whole numbers in the range of 0..N, and\n");
	printf("thread or lock domains that exceed the maximum NUMA domain\n");
	printf("are wrapped around using a MOD function.\n");
	printf("\n");
	printf("To determine the number of NUMA domains currently available\n");
	printf("on your system, use a command such as \"numastat\".\n");
	printf("\n");
	printf("Final note: maps do not gracefully handle ill-formed map specifications.\n");
	printf("\n");
	printf("<format> is selected from the following:\n");
	printf("    hdr                            # csv header only\n");
	printf("    csv                            # results in csv format only\n");
	printf("    both                           # header and results in csv format\n");
	printf("    table                          # human-readable table of values\n");
	printf("\n");

	return 1;
    }

    this->numa_max_domain  = Thread::get_max_numa_domain();
    this->num_numa_domains = this->numa_max_domain + 1;


				// allocate the chain roots for all threads
				// and compute the chain locations
				// (the chains themselves are initialized by the threads)
    this->thread_domain = new int32_t [ this->num_threads ];
    assert(this->thread_domain != NULL);

				// map the memory to its NUMA domain
				// actual allocation will take place later
    switch (this->numa_placement) {
    case GEN :
	this->alloc_gen(this->gen_thread_domain);
	break;
    case INTERLEAVE :
	this->alloc_interleave();
	break;
    case LOCAL :
    default:
	this->alloc_local();
	break;
    case MAP :
	this->alloc_map();
	break;
    }

    this->random_state = new char*[this->num_threads];
    assert(this->random_state != NULL);
    for (int i=0; i < this->num_threads; i++) {
	const int state_size = 256;
	this->random_state[i] = new char[state_size];
	assert(this->random_state[i] != NULL);
	initstate((unsigned int) i, (char *) this->random_state[i], (size_t) state_size);
    }

    return 0;
}


int64_t
Experiment::parse_number( const char* s )
{
    int64_t result = 0;

    if (s != NULL) {
	if ('1' <= s[0] && s[0] <= '9') {				// decimal
	    int len = strlen( s );
	    for (int i=0; i < len; i++) {
		if ( '0' <= s[i] && s[i] <= '9' ) {
		    result = result * 10 + s[i] - '0';
		} else if (s[i] == 'k' || s[i] == 'K') {
		    result = result << 10;
		    break;
		} else if (s[i] == 'm' || s[i] == 'M') {
		    result = result << 20;
		    break;
		} else if (s[i] == 'g' || s[i] == 'G') {
		    result = result << 30;
		    break;
		} else if (s[i] == 't' || s[i] == 'T') {
		    result = result << 40;
		    break;
		} else {
		    break;
		}
	    }
	} else if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {	// hexadecimal
	    int len = strlen( s );
	    for (int i=2; i < len; i++) {
		if ( '0' <= s[i] && s[i] <= '9' ) {
		    result = result * 16 + s[i] - '0';
		} else if ( s[i] <= 'a' && s[i] <= 'f') {
		    result = result * 16 + 10 + s[i] - 'a';
		} else if ( s[i] <= 'A' && s[i] <= 'F') {
		    result = result * 16 + 10 + s[i] - 'A';
		} else {
		    break;
		}
	    }
	} else if (s[0] == '0' && !(s[1] == 'x' || s[1] == 'X')) {	// octal
	    int len = strlen( s );
	    for (int i=1; i < len; i++) {
		if ( '0' <= s[i] && s[i] <= '7' ) {
		    result = result * 8 + s[i] - '0';
		} else {
		    break;
		}
	    }
	}
    }

    return result;
}


float
Experiment::parse_real( const char* s )
{
    float result = 0;
    bool decimal = false;
    float power = 1;

    int len = strlen( s );
    int i=0;
    for (; i < len; i++) {
	if ( '0' <= s[i] && s[i] <= '9' ) {
	    if (! decimal) {
		result = result * 10 + s[i] - '0';
	    } else {
		power = power / 10;
		result = result + (s[i] - '0') * power;
	    }
	} else if ( '.' == s[i] ) {
	    decimal = true;
	} else {
	    break;
	}
    }
    if ( s[i] == 'E' || s[i] == 'e' || s[i] == 'D' || s[i] == 'd' ) {
	i++;
	int exp = 0;
	bool exp_sign_pos=true;
	if ( s[i] == '-' ) {
	    exp_sign_pos = false;
	    i++;
	}
	for (; i < len; i++) {
	    if ( '0' <= s[i] && s[i] <= '9' ) {
		exp = exp * 10 + s[i] - '0';
	    } else {
		break;
	    }
	}
	if (!exp_sign_pos) {
	    exp = -exp;
	}
	result = result * pow(10, exp);
    }

    return result;
}

				//  d -- use d as the domain
				// +d -- add d to the thread domain
				// -d -- subtract d from the thread domain
				// ^d -- xor d and the thread domain
				//  i -- interleave the buffer pool
void
Experiment::alloc_gen(const char* thd)
{
    const char* p = thd;
    char thd_op = ' ';
    if (thd[0] == '+') { thd_op = '+'; p++; }
    if (thd[0] == '-') { thd_op = '-'; p++; }
    if (thd[0] == '^') { thd_op = '^'; p++; }
    if (thd[0] == 'i') { thd_op = 'i'; p++; }
    int i = 0;
    char buf[64];
    while (*p != '\0') {
	buf[i] = *p;
	i++;
	p++;
    }
    buf[i] = '\0';
    int thd_offset = Experiment::parse_number(buf);
    for (int i=0; i < this->num_threads; i++) {
	switch (thd_op) {
	case ' ':
	    this->thread_domain[i] = thd_offset % this->num_numa_domains;
	    break;
	case '+':
	    this->thread_domain[i] = (i + thd_offset) % this->num_numa_domains;
	    break;
	case '-':
	    this->thread_domain[i] = (((i - thd_offset) % this->num_numa_domains)
		+ this->num_numa_domains) % this->num_numa_domains;
	    break;
	case '^':
	    this->thread_domain[i] = (i ^ thd_offset) % this->num_numa_domains;
	    break;
	case 'i':
	default:
	    this->thread_domain[i] = i % this->num_numa_domains;
	    break;
	}
	assert(0 <= this->thread_domain[i] && this->thread_domain[i] < this->num_numa_domains);
    }
}

void
Experiment::alloc_interleave()
{
    for (int i=0; i < this->num_threads; i++) {
	this->thread_domain[i] = i % this->num_numa_domains;
    }
}

void
Experiment::alloc_local()
{
    for (int i=0; i < this->num_threads; i++) {
	this->thread_domain[i] = i % this->num_numa_domains;
    }
}

				// DOES NOT HANDLE ILL-FORMED SPECIFICATIONS
void
Experiment::alloc_map()
{
				// maps look like "t1;t2;...;tn" where t[i] 
                                // is the numa domain of the ith thread.
    int threads = this->num_threads;
    const char *p = this->placement_map;
    int t=0;
    while (*p != '\0') {
    				// everything up to ";" is the thread domain
	int i = 0;
	char buf[64];
	while (*p != '\0') {
	    if (*p == ';') { p++; break; }
	    buf[i] = *p;
	    i++;
	    p++;
	}
	buf[i] = '\0';
	this->thread_domain[t] = Experiment::parse_number(buf);

	if (*p == '\0') break;
	if (*p == ';') p++;
	t++;
        assert(t <= threads);
    }
}


void
Experiment::print()
{
#if defined(UNDEFINED)
    printf("bytes_per_copy    = %ld\n", bytes_per_copy);
    printf("bytes_per_pool    = %ld\n", bytes_per_pool);
    printf("bytes_per_test    = %ld\n", bytes_per_test);
    printf("copies_per_thread = %ld\n", copies_per_thread);
    printf("num_threads       = %d\n" , num_threads);
    printf("iterations        = %ld\n", iterations);
    printf("experiments       = %ld\n", experiments);
    printf("seconds           = %.2f\n", seconds);
    printf("access_pattern    = %s\n" , access());
    printf("output_mode       = %d\n" , output_mode);
    printf("numa_max_domain   = %d\n" , numa_max_domain);
    printf("num_numa_domains  = %d\n" , num_numa_domains);

    printf("numa_placement    = ");
    printf("%d:", this->thread_domain[0]);
    printf("%d,", this->source_domain[0]);
    printf("%d",  this->target_domain[0]);
    for (int i=1; i < this->num_threads; i++) {
	printf(",%d:", this->thread_domain[i]);
	printf("%d,",  this->source_domain[i]);
	printf("%d",   this->target_domain[i]);
    }
    printf("\n");

    fflush(stdout);
#endif
}

const char*
Experiment::access()
{
#if defined(UNDEFINED)
    const char* result = NULL;

    if (this->access_pattern == RANDOM) {
	result = "random";
    } else if (this->access_pattern == FORWARD) {
	result = "forward";
    } else if (this->access_pattern == BACKWARD) {
	result = "backward";
    }

    return result;
#else
    return "undefined";
#endif
}

const char*
Experiment::placement()
{
    const char* result = NULL;

    if (this->numa_placement == LOCAL) {
	result = "local";
    } else if (this->numa_placement == INTERLEAVE) {
	result = "interleave";
    } else if (this->numa_placement == GEN) {
	result = "gen";
    } else if (this->numa_placement == MAP) {
	result = "map";
    }

    return result;
}

const char*
Experiment::method()
{
    const char* result = NULL;

    if (this->lock_method == PTHREAD_SPIN) {
	result = "spin";
    } else if (this->lock_method == PTHREAD_MUTEX) {
	result = "mutex";
    } else if (this->lock_method == CMPXSPIN) {
	result = "cmpxspin";
    } else if (this->lock_method == CMPXCHG32) {
	result = "cmpxchg32";
    } else if (this->lock_method == CMPXCHG64) {
	result = "cmpxchg64";
    } else if (this->lock_method == FAIRSPIN) {
	result = "fairspin";
    } else if (this->lock_method == ATOMIC32) {
	result = "atomic32";
    } else if (this->lock_method == ATOMIC64) {
	result = "atomic64";
    } else if (this->lock_method == PTHREAD_READER) {
	result = "reader";
    } else if (this->lock_method == PTHREAD_WRITER) {
	result = "writer";
    } else if (this->lock_method == PTHREAD_BARRIER) {
	result = "barrier";
    }

    return result;
}

#if defined(ID)
const char* Experiment_C = "\0@ID " ID;
#endif
