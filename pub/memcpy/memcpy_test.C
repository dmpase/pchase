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
#include <stdlib.h>
#include <string.h>
#include "Memcpy.h"


int
main(int argc, char*argv[])
{
    unsigned char a[8191];
    unsigned char b[sizeof a];

    if (argc == 2 && strcmp(argv[1], "-memcpy") == 0) {
        set_memcpy(memcpy);
    } else if (argc == 2 && strcmp(argv[1], "-copy_tt") == 0) {
        set_memcpy(copy_tt);
    } else if (argc == 2 && strcmp(argv[1], "-copy_tn") == 0) {
        set_memcpy(copy_tn);
    } else if (argc == 2 && strcmp(argv[1], "-xtta") == 0) {
        set_memcpy(xtta);
    } else if (argc == 2 && strcmp(argv[1], "-xttu") == 0) {
        set_memcpy(xttu);
    } else if (argc == 2 && strcmp(argv[1], "-xtn") == 0) {
        set_memcpy(xtn);
    } else if (argc == 2 && strcmp(argv[1], "-xnt") == 0) {
        set_memcpy(xnt);
    } else if (argc == 2 && strcmp(argv[1], "-xnn") == 0) {
        set_memcpy(xnn);
    } else if (argc == 2 && strcmp(argv[1], "-naive") == 0) {
        set_memcpy(naive);
    } else {
        printf("usage: %s [ -memcpy | -copy_tt | -copy_tn | -xtta | -xttu | -xtn | -xnt | -xnn | -naive ]\n", argv[0]);
        return 0;
    }

    for (int i=0; i < sizeof a; i++) {
	a[i] = 0;
	b[i] = random() & 0xFF;
    }

    mb_memcpy(a, b, sizeof a);

    for (int i=0; i < sizeof a; i++) {
	if (a[i] != b[i]) {
	    printf("a[%d]=%2.2x != b[%d]=%2.2x\n", i, a[i]&0xff, i, b[i]&0xff);
	    return 0;
	}
    }

    printf("Successfully completed.\n");

    return 0;
}
