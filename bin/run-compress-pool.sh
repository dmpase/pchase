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

if [[ $bin == "" ]]
then
    export bin="../bin"
fi

source $bin/include.sh

if [[ $compress_bmk == "" ]]
then
    if [[ $bit == 32 ]]
    then
        compress_bmk="./compress32"
    else
        compress_bmk="./compress64"
    fi
fi

if [[ $sec == "" ]]
then
    if [[ $seconds == "" ]]
    then
	sec=1
    else
	sec=$seconds
    fi
fi

if [[ $size == "" ]]
then
    export size="8"
fi

if [[ $compress_method == "" ]]
then
    export compress_method="lz"
fi

if [[ $minpool == "" ]]
then
    let minpool=8
    export minpool
fi
if (( $minpool < $size )) ; then
    let minpool=$size
fi

if [[ $maxpool == "" ]]
then
    let maxpool=256*1024
    export maxpool
fi

old_rep=$rep
if [[ $rep == "" ]]
then
    export rep="10"
fi

$compress_bmk -e 1 -i 1 -L sched -d $domains -o hdr

#
# local memory, 1 and k threads
# 
let half=$threads/2
for t in 1 $half $threads
do
    let max_pool_per_thread=$maxpool/$t
    for ((i=$size; $i <= $max_pool_per_thread; i=$i*2))
    do
	let p=$i
	for (( j=0; $j < $rep ; j++ )) ; do
	    $compress_bmk -e $exp -b ${size}k -p ${p}k -L sched -d $domains -o csv -m $compress_method -n gen +0 +0 +0 -t $t -s $sec
            source $bin/pause.sh
	done

	if [[ $grain == "4x" ]] ; then 
	    let p=$i*5/4
	    let q=$i/$size
	    let r=$p/$size
	    if (( $q != $r )) ; then
		for ((j=0; $j < $rep ; j++)) ; do 
		    $compress_bmk -e $exp -b ${size}k -p ${p}k -L sched -d $domains -o csv -m $compress_method -n gen +0 +0 +0 -t $t -s $sec
                    source $bin/pause.sh
		done
	    fi
	fi

	if [[ $grain == "2x" || $grain == "4x" ]] ; then 
	    let p=$i*6/4
	    let q=$i*5/4/$size
	    let r=$p/$size
	    if (( $q != $r )) ; then
		for ((j=0; $j < $rep ; j++)) ; do 
		    $compress_bmk -e $exp -b ${size}k -p ${p}k -L sched -d $domains -o csv -m $compress_method -n gen +0 +0 +0 -t $t -s $sec
                    source $bin/pause.sh
		done
	    fi
	fi

	if [[ $grain == "4x" ]] ; then 
	    let p=$i*7/4
	    let q=$i*6/4/$size
	    let r=$p/$size
	    if (( $q != $r )) ; then
		for ((j=0; $j < $rep ; j++)) ; do 
		    $compress_bmk -e $exp -b ${size}k -p ${p}k -L sched -d $domains -o csv -m $compress_method -n gen +0 +0 +0 -t $t -s $sec
                    source $bin/pause.sh
		done
	    fi
	fi
    done
done

rep=$old_rep

popd
