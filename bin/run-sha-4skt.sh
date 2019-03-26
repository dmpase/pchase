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

if [[ $sha_bmk == "" ]]
then
    if [[ $bit == 32 ]]
    then
        sha_bmk="./sha32"
    else
        sha_bmk="./sha64"
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

if [[ $locality == "" ]]
then
    export locality="local"
fi

if [[ $tpr_min == "" ]] 
then
    export tpr_min=$dom_cnt
fi

if [[ $tpr_max == "" ]] 
then
    export tpr_max=$threads
fi

if [[ $size == "" ]]
then
    export size="8"
fi

if [[ $hash_method == "" ]]
then
    export hash_method="sha1"
fi

if [[ $hash_sizes == "fixed" ]]
then
    export h="-f"
elif [[ $hash_sizes == "variable" ]]
then
    export h="-v"
else
    export h="-f"
fi

if [[ $copy_method == "" ]]
then
    export copy_method="memcpy"
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

$sha_bmk -e 1 -i 1 -L sched -d $domains -o hdr

if [[ "$locality" == "local" || "$locality" == "both" || "$locality" == "all" ]] ; then 
    if (( 1 < $dom_cnt )) ; then

        for (( t=$tpr_min ; $t <= $threads && $t <= $tpr_max ; t+=$dom_cnt )) ; do
            let pool=$maxpool/$t
            for (( i=0; $i < $rep ; i++ )) ; do
                $sha_bmk -e $exp -b ${size}k -p ${pool}k -L sched -d $domains -o csv -m $hash_method -c $copy_method -n gen +0 +0 +0 +0 +0 -t $t -s $sec $h
                source $bin/pause.sh
            done
        done
    fi
fi

if [[ "$locality" == "near" || "$locality" == "remote" || "$locality" == "both" || "$locality" == "all" ]] ; then 
    if (( 1 < $dom_cnt )) ; then

        for (( t=$tpr_min ; $t <= $threads && $t <= $tpr_max ; t+=$dom_cnt )) ; do
            let pool=$maxpool/$t
            for (( i=0; $i < $rep ; i++ )) ; do
                $sha_bmk -e $exp -b ${size}k -p ${pool}k -L sched -d $domains -o csv -m $hash_method -c $copy_method -n gen +0 +1 +1 +1 +1 -t $t -s $sec $h
                source $bin/pause.sh
            done
        done
    fi
fi

if [[ "$locality" == "far" || "$locality" == "all" ]] ; then 
    if (( 2 < $dom_cnt )) ; then

        for (( t=$tpr_min ; $t <= $threads && $t <= $tpr_max ; t+=$dom_cnt )) ; do
            let pool=$maxpool/$t
            for (( i=0; $i < $rep ; i++ )) ; do
                $sha_bmk -e $exp -b ${size}k -p ${pool}k -L sched -d $domains -o csv -m $hash_method -c $copy_method -n gen +0 +2 +2 +2 +2 -t $t -s $sec $h
                source $bin/pause.sh
            done
        done

    fi
fi

rep=$old_rep

popd
