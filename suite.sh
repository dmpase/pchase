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

if [[ "$locality" == "" ]] ; then
    export locality="both"
fi

if [[ `whoami` == root ]] ; then
#   seconds=0.5 rep=1   ./run.sh -getpid
    grain="32x" seconds=0.25 rep=10 ./run.sh -pchase
    seconds=0.5 rep=100 ./run.sh -stream
    seconds=0.5 rep=10  ./run.sh -memcpy copy_tn
#   seconds=0.5 rep=1   ./run.sh -sha sha1 sha1i
    seconds=0.5 rep=10  ./run.sh -sha sha1
    seconds=0.5 rep=1   ./run.sh -sync-u
#   seconds=0.5 rep=1   ./run.sh -sync-c
#   seconds=0.5 rep=1   ./run.sh -sync-l
else
    seconds=0.5 rep=1   ./run.sh -getpid
    seconds=0.5 rep=10  ./run.sh -memcpy copy_tn
    seconds=0.5 rep=1   ./run.sh -pchase
    seconds=0.5 rep=1   ./run.sh -sha sha1 sha1i
    seconds=0.5 rep=10  ./run.sh -sha sha1
    seconds=0.5 rep=100 ./run.sh -stream
    seconds=0.5 rep=1   ./run.sh -sync-u
#   seconds=0.5 rep=1   ./run.sh -sync-c
#   seconds=0.5 rep=1   ./run.sh -sync-l
fi
