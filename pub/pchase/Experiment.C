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
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Thread.h"
#include "Experiment.h"

Experiment::Experiment() :
    strict           (0),
    pointer_size     (DEFAULT_POINTER_SIZE),
    bytes_per_line   (DEFAULT_BYTES_PER_LINE),
    links_per_line   (DEFAULT_LINKS_PER_LINE),
    bytes_per_page   (DEFAULT_BYTES_PER_PAGE),
    lines_per_page   (DEFAULT_LINES_PER_PAGE),
    links_per_page   (DEFAULT_LINKS_PER_PAGE),
    bytes_per_chain  (DEFAULT_BYTES_PER_CHAIN),
    lines_per_chain  (DEFAULT_LINES_PER_CHAIN),
    links_per_chain  (DEFAULT_LINKS_PER_CHAIN),
    pages_per_chain  (DEFAULT_PAGES_PER_CHAIN),
    chains_per_thread(DEFAULT_CHAINS_PER_THREAD),
    bytes_per_thread (DEFAULT_BYTES_PER_THREAD),
    num_threads      (DEFAULT_THREADS), 
    bytes_per_test   (DEFAULT_BYTES_PER_TEST),
    seconds          (DEFAULT_SECONDS),
    iterations       (DEFAULT_ITERATIONS),
    experiments      (DEFAULT_EXPERIMENTS), 
    output_mode      (TABLE),
    access_pattern   (RANDOM),
    stride           (1),
    numa_placement   (LOCAL),
    offset_or_mask   (0),
    placement_map    (NULL),
    thread_domain    (NULL),
    chain_domain     (NULL),
    numa_max_domain  (0),
    num_numa_domains (1)
{
}

Experiment::~Experiment()
{
}

				// interface: 
				//
				// -l or --line             bytes per cache line (line size)
				// -p or --page             bytes per page  (page size)
				// -c or --chain            bytes per chain (used to compute pages per chain)
				// -r or --references       chains per thread (memory loading)
				// -t or --threads          number of threads (concurrency and contention)
				// -i or --iters            iterations
				// -e or --experiments      experiments
				// -a or --access           memory access pattern
				//         random           random access pattern
				//         forward <stride> exclusive OR and mask
				//         reverse <stride> addition and offset
				// -o or --output           output mode
				//         hdr              header only
				//         csv              csv only
				//         both             header + csv
				//         table            human-readable table of values
				// -n or --numa             numa placement
				//         local            local allocation of all chains
				//         xor <mask>       exclusive OR and mask
				//         add <offset>     addition and offset
				//         map <map>        explicit mapping of threads and chains to domains

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
		this->access_pattern = STRIDED;
		i++;
		if (i == argc) { error = 1; break; }
		this->stride = Experiment::parse_number(argv[i]);
		if (this->stride == 0) { error = 1; break; }
	    } else if (strcasecmp(argv[i], "reverse") == 0) {
		this->access_pattern = STRIDED;
		i++;
		if (i == argc) { error = 1; break; }
		this->stride = - Experiment::parse_number(argv[i]);
		if (this->stride == 0) { error = 1; break; }
	    } else if (strcasecmp(argv[i], "stream") == 0) {
		this->access_pattern = STREAM;
		i++;
		if (i == argc) { error = 1; break; }
		this->stride = Experiment::parse_number(argv[i]);
		if (this->stride == 0) { error = 1; break; }
	    } else {
		error = 1;
		break;
	    }
	} else if (strcmp(argv[i], "-c") == 0 || strcasecmp(argv[i], "--chain") == 0) {
	    i++;
	    if (i == argc) { error = 1; break; }
	    this->bytes_per_chain = Experiment::parse_number(argv[i]);
	    if (this->bytes_per_chain == 0) { error = 1; break; }
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
	} else if (strcmp(argv[i], "-l") == 0 || strcasecmp(argv[i], "--line") == 0) {
	    i++;
	    if (i == argc) { error = 1; break; }
	    this->bytes_per_line = Experiment::parse_number(argv[i]);
	    if (this->bytes_per_line == 0) { error = 1; break; }
	} else if (strcmp(argv[i], "-n") == 0 || strcasecmp(argv[i], "--numa") == 0) {
	    i++;
	    if (i == argc) { error = 1; break; }
	    if (strcasecmp(argv[i], "add") == 0) {
		this->numa_placement = ADD;
		i++;
		if (i == argc) { error = 1; break; }
		this->offset_or_mask = Experiment::parse_number(argv[i]);
	    } else if (strcasecmp(argv[i], "interleave") == 0) {
		this->numa_placement = INTERLEAVE;
	    } else if (strcasecmp(argv[i], "local") == 0) {
		this->numa_placement = LOCAL;
	    } else if (strcasecmp(argv[i], "gen") == 0) {
		this->numa_placement = GEN;
		i++;
		if (i == argc) { error = 1; break; }
		this->gen_thread_domain = argv[i];
		i++;
		if (i == argc) { error = 1; break; }
		this->gen_pointer_domain = argv[i];
	    } else if (strcasecmp(argv[i], "map") == 0) {
		this->numa_placement = MAP;
		i++;
		if (i == argc) { error = 1; break; }
		this->placement_map = argv[i];
	    } else if (strcasecmp(argv[i], "xor") == 0) {
		this->numa_placement = XOR;
		i++;
		if (i == argc) { error = 1; break; }
		this->offset_or_mask = Experiment::parse_number(argv[i]);
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
	    this->bytes_per_page = Experiment::parse_number(argv[i]);
	    if (this->bytes_per_page == 0) { error = 1; break; }
	} else if (strcmp(argv[i], "-r") == 0 || strcasecmp(argv[i], "--references") == 0) {
	    i++;
	    if (i == argc) { error = 1; break; }
	    this->chains_per_thread = Experiment::parse_number(argv[i]);
	    if (this->chains_per_thread == 0) { error = 1; break; }
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
	} else if (strcmp(argv[i], "-x") == 0 || strcasecmp(argv[i], "--strict") == 0) {
	    this->strict = 1;
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
	printf("    [-a|--access]      <pattern>   # memory access pattern\n");
	printf("    [-c|--chain]       <number>    # bytes per chain (used to compute pages per chain)\n");
	printf("    [-d|--domains]     <domains>   # mapping of cores to domains\n");
	printf("    [-e|--experiments] <number>    # experiments\n");
	printf("    [-i|--iterations]  <number>    # iterations per experiment\n");
	printf("    [-l|--line]        <number>    # bytes per cache line (cache line size)\n");
	printf("    [-n|--numa]        <placement> # numa placement\n");
	printf("    [-o|--output]      <format>    # output format\n");
	printf("    [-p|--page]        <number>    # bytes per page (page size)\n");
	printf("    [-r|--references]  <number>    # chains per thread (memory loading)\n");
	printf("    [-s|--seconds]     <number>    # run each experiment for <number> seconds\n");
	printf("    [-t|--threads]     <number>    # number of threads (concurrency and contention)\n");
	printf("    [-x|--strict]                  # fail rather than adjust options to sensible values\n");
	printf("    [-L|--library]     <library>   # select the library for memory and thread placement\n");
	printf("\n");
	printf("<number> is an integer in decimal, octal or hexadecimal. Decimal numbers may have\n");
	printf("a suffix K, M, G or T appended to represent their respective powers of 2.\n");
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
	printf("<pattern> is selected from the following:\n");
	printf("    random                         # all chains are accessed randomly\n");
	printf("    forward <stride>               # chains are in forward order with constant stride\n");
	printf("    reverse <stride>               # chains are in reverse order with constant stride\n");
	printf("    stream  <stride>               # references are calculated rather than read from memory\n");
	printf("\n");
	printf("Note: <stride> is always a small positive integer.\n");
	printf("\n");
	printf("<format> is selected from the following:\n");
	printf("    hdr                            # csv header only\n");
	printf("    csv                            # results in csv format only\n");
	printf("    both                           # header and results in csv format\n");
	printf("    table                          # human-readable table of values\n");
	printf("\n");
	printf("In every case but map, the threads are allocated to domains in a round-robin fashion.\n");
	printf("The placement of chains is determined by the specification of --numa <placement>.\n");
	printf("<placement> is selected from the following:\n");
	printf("    add <offset>                   # addition and offset\n");
	printf("    interleave                     # not implemented\n");
	printf("    local                          # all chains are allocated locally (the default)\n");
	printf("    gen <thread> <pointer>         # pointers are allocated according to <thread> and <pointer>\n");
	printf("    map <map>                      # explicit mapping of threads and chains to domains\n");
	printf("    xor <mask>                     # exclusive OR and mask\n");
	printf("\n");
	printf("<thread> and <pointer> are simple unary expressions used to compute the NUMA domain for the thread \n");
	printf("and pointers, respectively. All expressions follow the same simple patterns and similar rules apply.\n");
	printf("When <thread> is a number, whether decimal, octal or hexadecimal, the NUMA domain is the number specified. \n");
	printf("When <thread> is \"+d\", where d is a number, the NUMA domain is the domain of the thread index plus the number. \n");
	printf("When <thread> is \"-d\", where d is a number, the NUMA domain is the domain of the thread index minus the number. \n");
	printf("When <thread> is \"^d\", where d is a number, the NUMA domain is the domain of the thread index XORed with the number. \n");
	printf("The same rules apply to <pointer>, except that all pointer chains are allocated according to the one expression. \n");
	printf("When <pointer> is the letter i, pointers are interleaved across all NUMA domains (not yet implemented).\n");
	printf("In all cases, the domain is computed using modulo arithmetic.\n");
	printf("\n");
	printf("<mask> is a number represented in binary that is exclusive-ORed with the current domain to \n");
	printf("determine the new domain.\n");
	printf("\n");
	printf("<offset> is a number that is added, modulo the total number of domains, to determine the\n");
	printf("new domain.\n");
	printf("\n");
	printf("<map> has the form \"t1:c11,c12,...,c1m;t2:c21,...,c2m;...;tn:cn1,...,cnm\"\n");
	printf("where t[i] is the NUMA domain where the ith thread is run,\n");
	printf("and c[i][j] is the NUMA domain where the jth chain in the ith thread is allocated.\n");
	printf("(The values t[i] and c[i][j] must all be zero or small positive integers.)\n");
	printf("\n");
	printf("Note: for maps, each thread must have the same number of chains,\n");
	printf("maps override the -t or --threads specification,\n");
	printf("NUMA domains are whole numbers in the range of 0..N, and\n");
	printf("thread or chain domains that exceed the maximum NUMA domain\n");
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
	printf("Final note: strict is not yet fully implemented, and\n");
	printf("maps do not gracefully handle ill-formed map specifications.\n");

	return 1;
    }


				// STRICT -- fail if specifications are inconsistent

				// compute lines per page and lines per chain
				// based on input and defaults.
				// we round up page and chain sizes when needed.
    this->lines_per_page   = (this->bytes_per_page+this->bytes_per_line-1) / this->bytes_per_line;
    this->bytes_per_page   = this->bytes_per_line * this->lines_per_page;
    this->pages_per_chain  = (this->bytes_per_chain+this->bytes_per_page-1) / this->bytes_per_page;
    this->bytes_per_chain  = this->bytes_per_page * this->pages_per_chain;
    this->bytes_per_thread = this->bytes_per_chain * this->chains_per_thread;
    this->bytes_per_test   = this->bytes_per_thread * this->num_threads;
    this->links_per_line   = this->bytes_per_line / pointer_size;
    this->links_per_page   = this->lines_per_page * this->links_per_line;
    this->lines_per_chain  = this->lines_per_page * this->pages_per_chain;
    this->links_per_chain  = this->lines_per_chain * this->links_per_line;
                                // round links up to a full page
    this->links_per_chain  = ((this->links_per_chain + this->links_per_page - 1)
                                / this->links_per_page) * this->links_per_page;


				// allocate the chain roots for all threads
				// and compute the chain locations
				// (the chains themselves are initialized by the threads)
    switch (this->numa_placement) {
    case LOCAL :
    case GEN :
    case XOR :
    case ADD :
	this->thread_domain = new int32_t [ this->num_threads ];
	this->chain_domain  = new int32_t*[ this->num_threads ];
	this->random_state  = new char*   [ this->num_threads ];

	for (int i=0; i < this->num_threads; i++) {
	    this->chain_domain[i] = new int32_t [ this->chains_per_thread ];

	    const int state_size = 256;
	    this->random_state[i] = new char[state_size];
	    initstate((unsigned int) i, (char *) this->random_state[i], (size_t) state_size);
	}
	break;
    }


    this->numa_max_domain  = Thread::get_max_numa_domain();
    this->num_numa_domains = this->numa_max_domain + 1;


    switch (this->numa_placement) {
    case ADD :
	this->alloc_add();
	break;
    case INTERLEAVE :
	this->alloc_interleave();
	break;
    case GEN :
        this->alloc_gen(this->gen_thread_domain, this->gen_pointer_domain);
	break;
    case LOCAL :
    default:
	this->alloc_local();
	break;
    case MAP :
	this->alloc_map();
	break;
    case XOR :
	this->alloc_xor();
	break;
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
    for (int i=0; i < len; i++) {
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

    return result;
}

void
Experiment::alloc_add()
{
    for (int i=0; i < this->num_threads; i++) {
	this->thread_domain[i] = i % this->num_numa_domains;
	for (int j=0; j < this->chains_per_thread; j++) {
	    this->chain_domain[i][j] = (this->thread_domain[i] + this->offset_or_mask) % this->num_numa_domains;
	}
    }
}

void
Experiment::alloc_interleave()
{
    for (int i=0; i < this->num_threads; i++) {
	this->thread_domain[i] = i % this->num_numa_domains;
	for (int j=0; j < this->chains_per_thread; j++) {
	    this->chain_domain[i][j] = this->num_numa_domains;
	}
    }
}

void
Experiment::alloc_local()
{
    for (int i=0; i < this->num_threads; i++) {
	this->thread_domain[i] = i % this->num_numa_domains;
	for (int j=0; j < this->chains_per_thread; j++) {
	    this->chain_domain[i][j] = this->thread_domain[i];
	}
    }
}

				//  d -- use d as the domain
				// +d -- add d to the thread domain
				// -d -- subtract d from the thread domain
				// ^d -- xor d and the thread domain
				//  i -- interleave the buffer pool
void
Experiment::alloc_gen(const char* thd, const char* ptr)
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

    p = ptr;
    char ptr_op = ' ';
    if (ptr[0] == '+') { ptr_op = '+'; p++; }
    if (ptr[0] == '-') { ptr_op = '-'; p++; }
    if (ptr[0] == '^') { ptr_op = '^'; p++; }
    if (ptr[0] == 'i') { ptr_op = 'i'; p++; }
    i = 0;
    while (*p != '\0') {
	buf[i] = *p;
	i++;
	p++;
    }
    buf[i] = '\0';
    int ptr_offset = Experiment::parse_number(buf);
    for (int i=0; i < this->num_threads; i++) {
	for (int j=0; j < this->chains_per_thread; j++) {
            switch (ptr_op) {
            case ' ':
                this->chain_domain[i][j] = ptr_offset % this->num_numa_domains;
                break;
            case '+':
                this->chain_domain[i][j] = (i + ptr_offset) % this->num_numa_domains;
                break;
            case '-':
                this->chain_domain[i][j] = (((i - ptr_offset) % this->num_numa_domains)
                    + this->num_numa_domains) % this->num_numa_domains;
                break;
            case '^':
                this->chain_domain[i][j] = (i ^ ptr_offset) % this->num_numa_domains;
                break;
            case 'i':
            default:
                this->chain_domain[i][j] = this->thread_domain[i];
                break;
            }
            assert(0 <= this->chain_domain[i][j] && this->chain_domain[i][j] < this->num_numa_domains);
	}
    }
}

				// DOES NOT HANDLE ILL-FORMED SPECIFICATIONS
void
Experiment::alloc_map()
{
				// STRICT -- fail if specifications are inconsistent

				// maps look like "t1:c11,c12,...,c1m;t2:c21,...,c2m;...;tn:cn1,...,cnm"
				// where t[i] is the thread domain of the ith thread,
				// and c[i][j] is the chain domain of the jth chain in the ith thread

				// count the thread descriptors by counting ";" up to EOS
    int threads = 1;
    char *p = this->placement_map;
    while (*p != '\0') {
	if (*p == ';') threads += 1;
	p++;
    }
    int thread_domain[ threads ];

				// count the chain descriptors by counting "," up to ";" or EOS
    int chains = 1;
    p = this->placement_map;
    while (*p != '\0') {
	if (*p == ';') break;
	if (*p == ',') chains += 1;
	p++;
    }
    int chain_domain [ threads ][ chains ];

    int t=0, c=0;
    p = this->placement_map;
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
	thread_domain[t] = Experiment::parse_number(buf);

    				// search for one or several ','
	c = 0;
	while (*p != '\0' && *p != ';') {
	    if (chains <= c || threads <= t) {
				// error in the thread/chain specification
		fprintf(stderr, "Malformed map.\n");
		exit(1);
	    }
	    int i = 0;
	    while (*p != '\0' && *p != ';') {
		if (*p == ',') { p++; break; }
		buf[i] = *p;
		i++;
		p++;
	    }
	    buf[i] = '\0';
	    chain_domain[t][c] = Experiment::parse_number(buf);
	    c++;
	}

	if (*p == '\0') break;
	if (*p == ';') p++;
	t++;
    }


    this->num_threads = threads;
    this->chains_per_thread = chains;

    this->thread_domain = new int32_t [ this->num_threads ];
    this->chain_domain  = new int32_t*[ this->num_threads ];
    this->random_state  = new char*   [ this->num_threads ];

    for (int i=0; i < this->num_threads; i++) {
	const int state_size = 256;
	this->random_state[i] = new char[state_size];
	initstate((unsigned int) i, (char *) this->random_state[i], (size_t) state_size);

	this->chain_domain[i] = new int32_t [ this->chains_per_thread ];
	for (int j=0; j < this->chains_per_thread; j++) {
	    this->chain_domain[i][j] = chain_domain[i][j] % this->num_numa_domains;
	}
    }

    this->bytes_per_thread = this->bytes_per_chain * this->chains_per_thread;
    this->bytes_per_test   = this->bytes_per_thread * this->num_threads;
}

void
Experiment::alloc_xor()
{
    for (int i=0; i < this->num_threads; i++) {
	this->thread_domain[i] = i % this->num_numa_domains;
	for (int j=0; j < this->chains_per_thread; j++) {
	    this->chain_domain[i][j] = (this->thread_domain[i] ^ this->offset_or_mask) % this->num_numa_domains;
	}
    }
}


void
Experiment::print()
{
    printf("strict            = %d\n", strict);
    printf("pointer_size      = %ld\n", pointer_size);
    printf("bytes_per_line    = %ld\n", bytes_per_line);
    printf("links_per_line    = %ld\n", links_per_line);
    printf("bytes_per_page    = %ld\n", bytes_per_page);
    printf("lines_per_page    = %ld\n", lines_per_page);
    printf("links_per_page    = %ld\n", links_per_page);
    printf("bytes_per_chain   = %ld\n", bytes_per_chain);
    printf("lines_per_chain   = %ld\n", lines_per_chain);
    printf("links_per_chain   = %ld\n", links_per_chain);
    printf("pages_per_chain   = %ld\n", pages_per_chain);
    printf("chains_per_thread = %ld\n", chains_per_thread);
    printf("bytes_per_thread  = %ld\n", bytes_per_thread);
    printf("num_threads       = %ld\n", num_threads);
    printf("bytes_per_test    = %ld\n", bytes_per_test);
    printf("iterations        = %ld\n", iterations);
    printf("experiments       = %ld\n", experiments);
    printf("access_pattern    = %d\n", access_pattern);
    printf("stride            = %ld\n", stride);
    printf("output_mode       = %d\n", output_mode);
    printf("numa_placement    = %d\n", numa_placement);
    printf("offset_or_mask    = %ld\n", offset_or_mask);
    printf("numa_max_domain   = %d\n", numa_max_domain);
    printf("num_numa_domains  = %d\n", num_numa_domains);

    for (int i=0; i < this->num_threads; i++) {
	printf("%d: ", this->thread_domain[i]);
	for (int j=0; j < this->chains_per_thread; j++) {
	    printf("%d,", this->chain_domain[i][j]);
	}
	printf("\n");
    }

    fflush(stdout);
}

const char*
Experiment::access()
{
    const char* result = NULL;

    if (this->access_pattern == RANDOM) {
	result = "random";
    } else if (this->access_pattern == STRIDED && 0 < this->stride) {
	result = "forward";
    } else if (this->access_pattern == STRIDED && this->stride < 0) {
	result = "reverse";
    } else if (this->access_pattern == STREAM) {
	result = "stream";
    }

    return result;
}

const char*
Experiment::placement()
{
    const char* result = NULL;

    if (this->numa_placement == LOCAL) {
	result = "local";
    } else if (this->numa_placement == GEN) {
	result = "gen";
    } else if (this->numa_placement == XOR) {
	result = "xor";
    } else if (this->numa_placement == ADD) {
	result = "add";
    } else if (this->numa_placement == MAP) {
	result = "map";
    }

    return result;
}

#if defined(ID)
const char* Experiment_C = "\0@ID " ID;
#endif
