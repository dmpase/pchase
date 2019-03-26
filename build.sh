#!/bin/bash

###############################################################################
# Copyright (c) 2011, Douglas M. Pase                                         #
# All rights reserved.                                                        #
# Redistribution and use in source and binary forms, with or without          #
# modification, are permitted provided that the following conditions          #
# are met:                                                                    #
# o       Redistributions of source code must retain the above copyright      #
#         notice, this list of conditions and the following disclaimer.       #
# o       Redistributions in binary form must reproduce the above copyright   #
#         notice, this list of conditions and the following disclaimer in     #
#         the documentation and/or other materials provided with the          #
#         distribution.                                                       #
# o       Neither the name of the copyright holder nor the names of its       #
#         contributors may be used to endorse or promote products derived     #
#         from this software without specific prior written permission.       #
#                                                                             #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" #
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   #
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  #
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE   #
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR         #
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF        #
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS    #
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN     #
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)     #
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF      #
# THE POSSIBILITY OF SUCH DAMAGE.                                             #
###############################################################################


pushd `dirname $0`
export bin="`pwd`/bin"
export data="`pwd`/data"

pushd utils/sha256
make rmobj rmexe
make CXID=""
make rmobj
make 
make rmobj
popd

pushd lib
make BIT=32 rmobj rmlib
make BIT=32
make BIT=32 rmobj
make BIT=64 rmobj rmlib
make BIT=64
make BIT=64 rmobj
popd

pushd utils/cpuid
make BIT=32 rmobj rmexe
make BIT=32
make BIT=32 rmobj
make BIT=64 rmobj rmexe
make BIT=64
make BIT=64 rmobj
popd

pushd pub/getpid
make BIT=32 rmobj rmexe
make BIT=32
make BIT=32 rmobj
make BIT=64 rmobj rmexe
make BIT=64
make BIT=64 rmobj
# make EXE=getpid64_icc_sse3 rmobj rmexe 
# make CXX=icpc CXXFLAGS="-O3 -fast -xSSE3 -m64" EXE=getpid64_icc_sse3
# make EXE=getpid64_icc_sse4 rmobj rmexe 
# make CXX=icpc CXXFLAGS="-O3 -fast -xSSE4.2 -m64" EXE=getpid64_icc_sse4
make rmobj
popd

pushd pub/memcpy
make BIT=32 rmobj rmexe
make BIT=32
make BIT=32 rmobj
make BIT=64 rmobj rmexe
make BIT=64
make BIT=64 rmobj
# make EXE=memcpy64_icc_sse3 rmobj rmexe
# make CXX=icpc CXXFLAGS="-O3 -fast -xSSE3 -m64 -DSCHED -DNT_OUTONLY" EXE=memcpy64_icc_sse3
# make EXE=memcpy64_icc_sse4 rmobj rmexe
# make CXX=icpc CXXFLAGS="-O3 -fast -xSSE4.2 -m64 -DSCHED -DNT_OUTONLY" EXE=memcpy64_icc_sse4
make rmobj
popd

pushd pub/pchase
make BIT=32 rmobj rmexe
make BIT=32
make BIT=32 rmobj
make BIT=64 rmobj rmexe
make BIT=64
make BIT=64 rmobj
# make EXE=pchase64_icc_sse3 rmobj rmexe
# make CXX=icpc CXXFLAGS="-O3 -fast -xSSE3 -m64 -DSCHED -DMEM_CHK" EXE=pchase64_icc_sse3
# make EXE=pchase64_icc_sse4 rmobj rmexe
# make CXX=icpc CXXFLAGS="-O3 -fast -xSSE4.2 -m64 -DSCHED -DMEM_CHK" EXE=pchase64_icc_sse4
make rmobj
popd

# pushd pub/stream
# make rmobj rmexe
# make 
# make EXE=stream_gcc_omp_x86_64 rmexe
# make EXE=stream_gcc_omp_x86_64
# make EXE=stream_icc_omp_x86_64_sse3 rmobj rmexe
# make CC=icc CFLAGS="-O3 -fast -xSSE3 -openmp -m64" EXE=stream_icc_omp_x86_64_sse3
# make EXE=stream_icc_omp_x86_64_sse4 rmobj rmexe
# make CC=icc CFLAGS="-O3 -fast -xSSE4.2 -openmp -m64" EXE=stream_icc_omp_x86_64_sse4
# make rmobj
# cp stream_eko_omp_x86_64 ../../bin/stream_omp_x86_64
# popd

pushd pub/sync-c
make BIT=32 rmobj rmexe
make BIT=32
make BIT=32 rmobj
make BIT=64 rmobj rmexe
make BIT=64
make BIT=64 rmobj
# make EXE=sync-c64_icc_sse3 rmobj rmexe
# make CXX=icpc CXXFLAGS="-O3 -fast -xSSE3 -m64 -DSCHED" EXE=sync-c64_icc_sse3
# make EXE=sync-c64_icc_sse4 rmobj rmexe
# make CXX=icpc CXXFLAGS="-O3 -fast -xSSE4.2 -m64 -DSCHED" EXE=sync-c64_icc_sse4
popd

pushd pub/sync-l
make BIT=32 rmobj rmexe
make BIT=32
make BIT=32 rmobj
make BIT=64 rmobj rmexe
make BIT=64
make BIT=64 rmobj
# make EXE=sync-l64_icc_sse3 rmobj rmexe
# make CXX=icpc CXXFLAGS="-O3 -fast -xSSE3 -m64 -DSCHED" EXE=sync-c64_icc_sse3
# make EXE=sync-l64_icc_sse4 rmobj rmexe
# make CXX=icpc CXXFLAGS="-O3 -fast -xSSE4.2 -m64 -DSCHED" EXE=sync-c64_icc_sse4
popd

pushd pub/sync-u
make BIT=32 rmobj rmexe
make BIT=32
make BIT=32 rmobj
make BIT=64 rmobj rmexe
make BIT=64
make BIT=64 rmobj
# make EXE=sync-u64_icc_sse3 rmobj rmexe
# make CXX=icpc CXXFLAGS="-O3 -fast -xSSE3 -m64 -DSCHED" EXE=sync-u64_icc_sse3
# make EXE=sync-u64_icc_sse4 rmobj rmexe
# make CXX=icpc CXXFLAGS="-O3 -fast -xSSE4.2 -m64 -DSCHED" EXE=sync-u64_icc_sse4
popd

popd
