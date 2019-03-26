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
    access_pattern   (RANDOM),
    bytes_per_copy   (4096),
    bytes_per_pool   (1024*4096),
    bytes_per_test   (1024*4096), 
    copies_per_thread(0),
    copy_method      (MEMCPY),
    experiments      (1), 
    gen_source       (NULL),
    gen_target       (NULL),
    iterations       (0),
    num_numa_domains (1),
    num_threads      (1), 
    numa_max_domain  (0),
    numa_placement   (LOCAL),
    output_mode      (TABLE),
    placement_map    (NULL),
    seconds          (1),
    source_domain    (NULL),
    target_domain    (NULL), 
    thread_domain    (NULL)
{
}

Experiment::~Experiment()
{
}


int
Experiment::parse_args(int argc, char* argv[])
{
    int error = 0;

				// look for -L or --library
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

				// look for -d or --domains
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

    for (int i=1; i < argc; i++) {
	if (strcmp(argv[i], "-a") == 0 || strcasecmp(argv[i], "--access") == 0) {
	    i++;
	    if (i == argc) { error = 1; break; }
	    if (strcasecmp(argv[i], "random") == 0) {
		this->access_pattern = RANDOM;
	    } else if (strcasecmp(argv[i], "forward") == 0) {
		this->access_pattern = FORWARD;
	    } else if (strcasecmp(argv[i], "backward") == 0) {
		this->access_pattern = BACKWARD;
	    } else {
		error = 1;
		break;
	    }
	} else if (strcmp(argv[i], "-c") == 0 || strcasecmp(argv[i], "--copy") == 0) {
	    i++;
	    if (i == argc) { error = 1; break; }
	    this->bytes_per_copy = Experiment::parse_number(argv[i]);
	    if (this->bytes_per_copy == 0) { error = 1; break; }
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
	} else if (strcmp(argv[i], "-i") == 0 || strcasecmp(argv[i], "--iterations") == 0) {
	    i++;
	    if (i == argc) { error = 1; break; }
	    this->iterations = Experiment::parse_number(argv[i]);
	    this->seconds = 0;
	    if (this->iterations == 0) { error = 1; break; }
	} else if (strcmp(argv[i], "-m") == 0 || strcasecmp(argv[i], "--method") == 0) {
	    i++;
	    if (i == argc) { error = 1; break; }
	    if (strcasecmp(argv[i], "naive") == 0) {
		this->copy_method = NAIVE;
	    } else if (strcasecmp(argv[i], "memcpy") == 0) {
		this->copy_method = MEMCPY;
	    } else if (strcasecmp(argv[i], "memmove") == 0) {
		this->copy_method = MEMMOVE;
	    } else if (strcasecmp(argv[i], "bcopy") == 0) {
		this->copy_method = BCOPY;
	    } else if (strcasecmp(argv[i], "copy_tt") == 0) {
		this->copy_method = COPY_TT;
	    } else if (strcasecmp(argv[i], "copy_tn") == 0) {
		this->copy_method = COPY_TN;
	    } else if (strcasecmp(argv[i], "naivezero") == 0) {
		this->copy_method = NAIVEZERO;
	    } else if (strcasecmp(argv[i], "bzero") == 0) {
		this->copy_method = BZERO;
	    } else if (strcasecmp(argv[i], "memset") == 0) {
		this->copy_method = MEMSET;
	    } else if (strcasecmp(argv[i], "xtta") == 0) {
		this->copy_method = XTTA;
	    } else if (strcasecmp(argv[i], "xttu") == 0) {
		this->copy_method = XTTU;
	    } else if (strcasecmp(argv[i], "xtn") == 0) {
		this->copy_method = XTN;
	    } else if (strcasecmp(argv[i], "xnt") == 0) {
		this->copy_method = XNT;
	    } else if (strcasecmp(argv[i], "xnn") == 0) {
		this->copy_method = XNN;
	    } else if (strcasecmp(argv[i], "rd") == 0 || strcasecmp(argv[i], "read") == 0 || strcasecmp(argv[i], "memread") == 0) {
		this->copy_method = MEMREAD;
	    } else if (strcasecmp(argv[i], "wr") == 0 || strcasecmp(argv[i], "write") == 0 || strcasecmp(argv[i], "memwrite") == 0) {
		this->copy_method = MEMWRITE;
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
		this->gen_thread = argv[i];
		i++;
		if (i == argc) { error = 1; break; }
		this->gen_source = argv[i];
		i++;
		if (i == argc) { error = 1; break; }
		this->gen_target = argv[i];
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
	} else if (strcmp(argv[i], "-p") == 0 || strcasecmp(argv[i], "--pool") == 0) {
	    i++;
	    if (i == argc) { error = 1; break; }
	    this->bytes_per_pool = Experiment::parse_number(argv[i]);
	    if (this->bytes_per_pool == 0) { error = 1; break; }
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
	} else if (strcmp(argv[i], "-L") == 0 || strcasecmp(argv[i], "--library") == 0) {
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
	} else {
	    error = 1;
	    break;
	}
    }


				// if we've hit an error, print a message and quit
    if (error) {
	for (int i=0; i < argc; i++) {
	    printf("%s ", argv[i]);
	}
	printf("usage: %s <options>\n", argv[0]);
	printf("where <options> are selected from the following:\n");
	printf("    [-h|--help]                    # this message\n");
	printf("    [-a|--access]      <pattern>   # access pattern\n");
	printf("    [-c|--copy]        <number>    # bytes per copy\n");
	printf("    [-d|--domains]     <domains>   # mapping of cores to domains\n");
	printf("    [-e|--experiments] <number>    # experiments\n");
	printf("    [-i|--iterations]  <number>    # iterations per experiment\n");
	printf("    [-m|--method]      <method>    # copy method\n");
	printf("    [-n|--numa]        <placement> # numa placement\n");
	printf("    [-o|--output]      <format>    # output format\n");
	printf("    [-p|--pool]        <number>    # bytes per memory pool\n");
	printf("    [-s|--seconds]     <number>    # run each experiment for <number> seconds\n");
	printf("    [-t|--threads]     <number>    # number of threads (concurrency and contention)\n");
	printf("    [-L|--library]     <library>   # select the library for memory and thread placement\n");
	printf("\n");
	printf("<pattern> is RANDOM, FORWARD or BACKWARD.\n");
	printf("\n");
	printf("<number> is an non-negative integer in decimal, octal or hexadecimal. Decimal numbers \n");
	printf("may have a suffix K, M, G or T appended to represent their respective powers of 2.\n");
	printf("Octal numbers are of the form 0dd...d where d is one of [0-7]. Hexadecimal numbers \n");
	printf("are of the form 0xdd...d where d is [0-9a-f].\n");
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
	printf("<method> is the specific method to copy memory. The available methods are:\n");
	printf("    memcpy                         # the libc memcpy function\n");
	printf("    memmove                        # the libc memmove function\n");
	printf("    bcopy                          # the libc bcopy function\n");
	printf("    naivezero                      # simple b[i]='\\0' zero of the buffer\n");
	printf("    bzero                          # the libc bzero function\n");
	printf("    memset                         # the libc memset function\n");
	printf("    naive                          # byte-by-byte copy in C/C++\n");
	printf("    copy_tt                        # 64-bit temporal loads and stores\n");
	printf("    copy_tn                        # 64-bit temporal loads and non-temporal stores\n");
	printf("    xtta                           # aligned 128-bit temporal loads and stores\n");
	printf("    xttu                           # unaligned 128-bit temporal loads and stores\n");
	printf("    xtn                            # 128-bit temporal loads, non-temporal stores\n");
	printf("    xnt                            # 128-bit non-temporal loads, temporal stores\n");
	printf("    xnn                            # 128-bit non-temporal loads, non-temporal stores\n");
	printf("    rd                             # 128-bit non-temporal loads (only)\n");
	printf("    wr                             # 128-bit non-temporal stores (only)\n");
	printf("\n");
	printf("<format> is selected from the following:\n");
	printf("    hdr                            # csv header only\n");
	printf("    csv                            # results in csv format only\n");
	printf("    both                           # header and results in csv format\n");
	printf("    table                          # human-readable table of values\n");
	printf("\n");
	printf("<placement> is selected from the following:\n");
	printf("    interleave                     # not implemented\n");
	printf("    local                          # source and target are allocated locally (default)\n");
	printf("    gen <thd> <src> <tgt>          # source and target are allocated according to <src> and <tgt>\n");
	printf("    map <map>                      # explicit mapping of threads, source and target to domains\n");
	printf("\n");
	printf("<thd>, <src> and <tgt> are simple uniary expressions used to compute the NUMA domain for the thread, \n");
	printf("source and target, respectively. All expressions follow the same simple patterns and similar rules apply.\n");
	printf("When <thd> is a number, whether decimal, octal or hexadecimal, the NUMA domain is the number specified. \n");
	printf("When <thd> is \"+d\", where d is a number, the NUMA domain is the domain of the thread index plus the number. \n");
	printf("When <thd> is \"-d\", where d is a number, the NUMA domain is the domain of the thread index minus the number. \n");
	printf("When <thd> is \"^d\", where d is a number, the NUMA domain is the domain of the thread index XORed with the number. \n");
	printf("When <src> or <tgt> is the letter i, source or target is interleaved across all NUMA domains.\n");
	printf("In all cases, the domain is computed using modulo arithmetic.\n");
	printf("\n");
	printf("<map> has the form \"t1:s1,d1;t2:s2,d2;...;tn:sn,dn\"\n");
	printf("where t[i] is the NUMA domain where the ith thread is run,\n");
	printf("s[i] is the NUMA domain where the source in the ith thread is allocated, and\n");
	printf("d[i] is the NUMA domain where the target in the ith thread is allocated.\n");
	printf("(The values t[i], s[i] and d[i] must all be zero or small positive integers.)\n");
	printf("\n");
	printf("Note: maps override the -t or --threads specification,\n");
	printf("NUMA domains are whole numbers in the range of 0..N, and\n");
	printf("thread, source or target domains that exceed the maximum NUMA domain\n");
	printf("are wrapped around using a MOD function.\n");
	printf("\n");
	printf("To determine the number of NUMA domains currently available\n");
	printf("on your system, use a command such as \"numastat\".\n");
	printf("\n");
	printf("<library> is selected from the following:\n");
	printf("    numa                           # \n");
	printf("    sched                          # \n");
	printf("    none                           # \n");
	printf("\n");
	printf("Final note: maps do not gracefully handle ill-formed map specifications.\n");

	return 1;
    }

    this->numa_max_domain  = Thread::get_max_numa_domain();
    this->num_numa_domains = this->numa_max_domain + 1;


				// compute the copy size based on input and defaults.
    this->copies_per_thread = (this->bytes_per_pool + this->bytes_per_copy - 1) / this->bytes_per_copy;
    this->bytes_per_pool = this->copies_per_thread * this->bytes_per_copy;
    this->bytes_per_test = this->bytes_per_pool * this->num_threads;


				// allocate the chain roots for all threads
				// and compute the chain locations
				// (the chains themselves are initialized by the threads)
    this->thread_domain = new int32_t [ this->num_threads ];
    assert(this->thread_domain != NULL);
    this->source_domain = new int32_t [ this->num_threads ];
    assert(this->source_domain != NULL);
    this->target_domain = new int32_t [ this->num_threads ];
    assert(this->target_domain != NULL);

				// map the memory to its NUMA domain
				// actual allocation will take place later
    switch (this->numa_placement) {
    case GEN :
	this->alloc_gen(this->gen_thread, this->gen_source, this->gen_target);
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
Experiment::alloc_gen(const char* thd, const char* src, const char* tgt)
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

    p = src;
    char src_op = ' ';
    if (src[0] == '+') { src_op = '+'; p++; }
    if (src[0] == '-') { src_op = '-'; p++; }
    if (src[0] == '^') { src_op = '^'; p++; }
    if (src[0] == 'i') { src_op = 'i'; p++; }
    i = 0;
    while (*p != '\0') {
	buf[i] = *p;
	i++;
	p++;
    }
    buf[i] = '\0';
    int src_offset = Experiment::parse_number(buf);
    for (int i=0; i < this->num_threads; i++) {
	switch (src_op) {
	case ' ':
	    this->source_domain[i] = src_offset % this->num_numa_domains;
	    break;
	case '+':
	    this->source_domain[i] = (i + src_offset) % this->num_numa_domains;
	    break;
	case '-':
	    this->source_domain[i] = (((i - src_offset) % this->num_numa_domains)
		+ this->num_numa_domains) % this->num_numa_domains;
	    break;
	case '^':
	    this->source_domain[i] = (i ^ src_offset) % this->num_numa_domains;
	    break;
	case 'i':
	default:
	    this->source_domain[i] = this->thread_domain[i];
	    break;
	}
	assert(0 <= this->source_domain[i] && this->source_domain[i] < this->num_numa_domains);
    }

    p = tgt;
    char tgt_op = ' ';
    if (tgt[0] == '+') { tgt_op = '+'; p++; }
    if (tgt[0] == '-') { tgt_op = '-'; p++; }
    if (tgt[0] == '^') { tgt_op = '^'; p++; }
    if (tgt[0] == 'i') { tgt_op = 'i'; p++; }
    i = 0;
    while (*p != '\0') {
	buf[i] = *p;
	i++;
	p++;
    }
    buf[i] = '\0';
    int tgt_offset = Experiment::parse_number(buf);
    for (int i=0; i < this->num_threads; i++) {
	switch (tgt_op) {
	case ' ':
	    this->target_domain[i] = tgt_offset % this->num_numa_domains;
	    break;
	case '+':
	    this->target_domain[i] = (i + tgt_offset) % this->num_numa_domains;
	    break;
	case '-':
	    this->target_domain[i] = (((i - tgt_offset) % this->num_numa_domains)
		+ this->num_numa_domains) % this->num_numa_domains;
	    break;
	case '^':
	    this->target_domain[i] = (i ^ tgt_offset) % this->num_numa_domains;
	    break;
	case 'i':
	default:
	    this->target_domain[i] = this->thread_domain[i];
	    break;
	}
	assert(0 <= this->target_domain[i] && this->target_domain[i] < this->num_numa_domains);
    }
}

void
Experiment::alloc_interleave()
{
    for (int i=0; i < this->num_threads; i++) {
	this->thread_domain[i] = i % this->num_numa_domains;
	this->source_domain[i] = this->thread_domain[i];
	this->target_domain[i] = this->thread_domain[i];
    }
}

void
Experiment::alloc_local()
{
    for (int i=0; i < this->num_threads; i++) {
	this->thread_domain[i] = i % this->num_numa_domains;
	this->source_domain[i] = this->thread_domain[i];
	this->target_domain[i] = this->thread_domain[i];
    }
}

				// DOES NOT HANDLE ILL-FORMED SPECIFICATIONS
void
Experiment::alloc_map()
{
				// maps look like "t1:s1,d1;t2:s2,d2;...;tn:sn,dn"
				// where t[i] is the numa domain of the ith thread,
				// and s[i] and d[i] are the domains of the source and 
				// target of the ith thread, respectively.

    int threads = this->num_threads;
    const char *p = this->placement_map;
    int t=0;
    while (*p != '\0') {
    				// everything up to ":" is the thread domain
	int i = 0;
	char buf[64];
	while (*p != '\0') {
	    if (*p == ':') { p++; break; }
	    buf[i] = *p;
	    i++;
	    p++;
	}
	buf[i] = '\0';
	this->thread_domain[t] = Experiment::parse_number(buf);

    				// search for a ','
	i = 0;
	while (*p != '\0') {
	    if (*p == ',') { p++; break; }
	    buf[i] = *p;
	    i++;
	    p++;
	}
	buf[i] = '\0';
	this->source_domain[t] = Experiment::parse_number(buf);

    				// search for a ';' or '\0'
	i = 0;
	while (*p != '\0') {
	    if (*p == ';') { p++; break; }
	    buf[i] = *p;
	    i++;
	    p++;
	}
	buf[i] = '\0';
	this->target_domain[t] = Experiment::parse_number(buf);

	if (*p == '\0') break;
	if (*p == ';') p++;
	t++;
    }
}


void
Experiment::print()
{
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
}

const char*
Experiment::access()
{
    const char* result = NULL;

    if (this->access_pattern == RANDOM) {
	result = "random";
    } else if (this->access_pattern == FORWARD) {
	result = "forward";
    } else if (this->access_pattern == BACKWARD) {
	result = "backward";
    }

    return result;
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

    if (this->copy_method == NAIVE) {
	result = "naive";
    } else if (this->copy_method == MEMCPY) {
	result = "memcpy";
    } else if (this->copy_method == MEMMOVE) {
	result = "memmove";
    } else if (this->copy_method == BCOPY) {
	result = "bcopy";
    } else if (this->copy_method == COPY_TT) {
	result = "copy_tt";
    } else if (this->copy_method == COPY_TN) {
	result = "copy_tn";
    } else if (this->copy_method == NAIVEZERO) {
	result = "naivezero";
    } else if (this->copy_method == BZERO) {
	result = "bzero";
    } else if (this->copy_method == MEMSET) {
	result = "memset";
    } else if (this->copy_method == XTTA) {
	result = "xtta";
    } else if (this->copy_method == XTTU) {
	result = "xttu";
    } else if (this->copy_method == XTN) {
	result = "xtn";
    } else if (this->copy_method == XNT) {
	result = "xnt";
    } else if (this->copy_method == XNN) {
	result = "xnn";
    } else if (this->copy_method == MEMREAD) {
	result = "rd";
    } else if (this->copy_method == MEMWRITE) {
	result = "wr";
    }

    return result;
}

#if defined(ID)
const char* Experiment_C = "\0@ID " ID;
#endif
