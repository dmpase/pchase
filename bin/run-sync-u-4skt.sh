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

if [[ $sync_u_bmk == "" ]]
then
    if [[ $bit == 32 ]]
    then
        export sync_u_bmk="./sync-u32"
    else
        export sync_u_bmk="./sync-u64"
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
    export locality="all"
fi

if [[ $tpr_min == "" ]] 
then
    export tpr_min=$dom_cnt
fi

if [[ $tpr_max == "" ]] 
then
    export tpr_max=$dom_cnt
fi

if [[ $method == "" ]]
then
    export method="spin"
fi

if [[ $minpool == "" ]]
then
    let minpool=128
fi

if [[ $maxpool == "" ]]
then
    let maxpool=8*1024*1024
fi

old_rep=$rep
if [[ $rep == "" ]]
then
    export rep="1"
fi


$sync_u_bmk -e 1 -i 1 -k 1 -L sched -d $domains -o hdr
				# local
if [[ "$locality" == "local" || "$locality" == "both" || "$locality" == "all" ]] ; then 
    if (( 1 < $dom_cnt )) ; then

        for (( t=$tpr_min ; $t <= $threads  && $t <= $tpr_max ; t+=$dom_cnt )) ; do
            let max_pool_per_thread=$maxpool/$t
            for ((i=$minpool; $i <= $max_pool_per_thread; i=$i*2)) ; do
                let size=$i
                for ((j=0; $j < $rep ; j++)) ; do 
                    $sync_u_bmk -e $exp -k $size -L sched -d $domains -o csv -m $method -n gen +0 +0 -t $t -s $sec
                    source $bin/pause.sh
                done

                if [[ $grain == "4x" ]] ; then 
                    let size=$i*5/4
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $sync_u_bmk -e $exp -k $size -L sched -d $domains -o csv -m $method -n gen +0 +0 -t $t -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "2x" || $grain == "4x" ]] ; then 
                    let size=$i*6/4
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $sync_u_bmk -e $exp -k $size -L sched -d $domains -o csv -m $method -n gen +0 +0 -t $t -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "4x" ]] ; then 
                    let size=$i*7/4
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $sync_u_bmk -e $exp -k $size -L sched -d $domains -o csv -m $method -n gen +0 +0 -t $t -s $sec
                        source $bin/pause.sh
                    done
                fi
            done
        done
    fi
fi

				# remote
if [[ "$locality" == "remote" || "$locality" == "both" ]] ; then 
    if (( 1 < $dom_cnt )) ; then

        for (( t=$tpr_min ; $t <= $threads && $t <= $tpr_max ; t+=$dom_cnt )) ; do
            let max_pool_per_thread=$maxpool/$t
            for ((i=$minpool; $i <= $max_pool_per_thread; i=$i*2)) ; do
                let size=$i
                for ((j=0; $j < $rep ; j++)) ; do 
                    $sync_u_bmk -e $exp -k $size -L sched -d $domains -o csv -m $method -n gen +0 +1 -t $t -s $sec
                    source $bin/pause.sh
                done

                if [[ $grain == "4x" ]] ; then 
                    let size=$i*5/4
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $sync_u_bmk -e $exp -k $size -L sched -d $domains -o csv -m $method -n gen +0 +1 -t $t -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "2x" || $grain == "4x" ]] ; then 
                    let size=$i*6/4
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $sync_u_bmk -e $exp -k $size -L sched -d $domains -o csv -m $method -n gen +0 +1 -t $t -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "4x" ]] ; then 
                    let size=$i*7/4
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $sync_u_bmk -e $exp -k $size -L sched -d $domains -o csv -m $method -n gen +0 +1 -t $t -s $sec
                        source $bin/pause.sh
                    done
                fi
            done
        done
    fi
fi

				# far
if [[ "$locality" == "far" || "$locality" == "all" ]] ; then 
    if (( 2 < $dom_cnt )) ; then

        for (( t=$tpr_min ; $t <= $threads && $t <= $tpr_max ; t+=$dom_cnt )) ; do
            let max_pool_per_thread=$maxpool/$t
            for ((i=$minpool; $i <= $max_pool_per_thread; i=$i*2)) ; do
                let size=$i
                for ((j=0; $j < $rep ; j++)) ; do 
                    $sync_u_bmk -e $exp -k $size -L sched -d $domains -o csv -m $method -n gen +0 +2 -t $t -s $sec
                    source $bin/pause.sh
                done

                if [[ $grain == "4x" ]] ; then 
                    let size=$i*5/4
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $sync_u_bmk -e $exp -k $size -L sched -d $domains -o csv -m $method -n gen +0 +2 -t $t -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "2x" || $grain == "4x" ]] ; then 
                    let size=$i*6/4
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $sync_u_bmk -e $exp -k $size -L sched -d $domains -o csv -m $method -n gen +0 +2 -t $t -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "4x" ]] ; then 
                    let size=$i*7/4
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $sync_u_bmk -e $exp -k $size -L sched -d $domains -o csv -m $method -n gen +0 +2 -t $t -s $sec
                        source $bin/pause.sh
                    done
                fi
            done
        done
    fi
fi

rep=$old_rep

popd
