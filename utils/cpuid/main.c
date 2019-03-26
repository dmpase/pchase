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


 // http://www.intel.com/content/dam/doc/application-note/processor-identification-cpuid-instruction-note.pdf

#include <stdio.h>
#include <unistd.h>

#include "cpuid.h"

int
main(int ac, char* av[])
{
    hpc_cpuid_t c;

    get_cpuid(&c);
    print_cpuid(&c, printf);
    printf("CPU core frequency is %.2lf MHz\n", (double)(processor_clock_Hz()/1000000.));
    printf("System page size is %d bytes.\n", getpagesize());
}

#if defined(ID)
const char* main_c = "\0@ID " ID;
#endif
