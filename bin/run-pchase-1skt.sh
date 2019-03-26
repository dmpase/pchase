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

if [[ $bin == "" ]] ; then
    export bin="../bin"
fi

source $bin/include.sh

if [[ $pchase_bmk == "" ]] ; then
    if [[ $bit == 32 ]] ; then
        pchase_bmk="./pchase32"
    else
        pchase_bmk="./pchase64"
    fi
fi

if [[ $sec == "" ]] ; then
    if [[ $seconds == "" ]] ; then
	sec=1
    else
	sec=$seconds
    fi
fi

if [[ $locality == "" ]] ; then
    export locality="all"
fi

if [[ $tpr_min == "" ]] ; then
    export tpr_min=1
fi

if [[ $tpr_max == "" ]] ; then
    export tpr_max=1
fi

if [[ $access == "" ]] ; then
    export access="random"
fi

#
# max/min pool is in KiB
#

if [[ $minpool == "" ]] ; then
    let minpool=8
fi

if [[ $maxpool == "" ]] ; then
    let maxpool=256*1024
fi

old_rep=$rep
if [[ $rep == "" ]] ; then
    export rep="1"
fi

$pchase_bmk -L sched -d $domains -s 1.0 -o hdr
if [[ "$locality" == "local" || "$locality" == "both" || "$locality" == "all" ]] ; then 
    for (( t=$tpr_min ; $t <= $threads_per_domain && $t <= $tpr_max ; t+=1 )) ; do
        for refs in 1 ; do
            let max_pool_per_thread=$maxpool/$t
            for ((i=$minpool; $i <= $max_pool_per_thread; i=$i*2)) ; do
                let clen=$i
                if [[ $page_size == "" ]] ; then
                    ps=${clen}k
                else
                    ps=$page_size
                fi
                for ((j=0; $j < $rep ; j++)) ; do 
                    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                    source $bin/pause.sh
                done

                if [[ $grain == "32x" ]] ; then 
                    let clen=$i*33/32
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "16x" || $grain == "32x" ]] ; then 
                    let clen=$i*17/16
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "32x" ]] ; then 
                    let clen=$i*35/32
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "8x" || $grain == "16x" || $grain == "32x" ]] ; then 
                    let clen=$i*9/8
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "32x" ]] ; then 
                    let clen=$i*37/32
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "16x" || $grain == "32x" ]] ; then 
                    let clen=$i*19/16
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "32x" ]] ; then 
                    let clen=$i*39/32
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "4x" || $grain == "8x" || $grain == "16x" || $grain == "32x" ]] ; then 
                    let clen=$i*5/4
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "32x" ]] ; then 
                    let clen=$i*41/32
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "16x" || $grain == "32x" ]] ; then 
                    let clen=$i*21/16
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "32x" ]] ; then 
                    let clen=$i*43/32
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "8x" || $grain == "16x" || $grain == "32x" ]] ; then 
                    let clen=$i*11/8
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "32x" ]] ; then 
                    let clen=$i*45/32
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "16x" || $grain == "32x" ]] ; then 
                    let clen=$i*23/16
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "32x" ]] ; then 
                    let clen=$i*47/32
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "2x" || $grain == "4x" || $grain == "8x" || $grain == "16x" || $grain == "32x" ]] ; then 
                    let clen=$i*6/4
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi 

                if [[ $grain == "32x" ]] ; then 
                    let clen=$i*49/32
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "16x" || $grain == "32x" ]] ; then 
                    let clen=$i*25/16
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "32x" ]] ; then 
                    let clen=$i*51/32
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "8x" || $grain == "16x" || $grain == "32x" ]] ; then 
                    let clen=$i*13/8
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "32x" ]] ; then 
                    let clen=$i*53/32
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "16x" || $grain == "32x" ]] ; then 
                    let clen=$i*27/16
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "32x" ]] ; then 
                    let clen=$i*55/32
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "4x" || $grain == "8x" || $grain == "16x" || $grain == "32x" ]] ; then 
                    let clen=$i*7/4
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi 

                if [[ $grain == "32x" ]] ; then 
                    let clen=$i*57/32
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "16x" || $grain == "32x" ]] ; then 
                    let clen=$i*29/16
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "32x" ]] ; then 
                    let clen=$i*59/32
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "8x" || $grain == "16x" || $grain == "32x" ]] ; then 
                    let clen=$i*15/8
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "32x" ]] ; then 
                    let clen=$i*61/32
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "16x" || $grain == "32x" ]] ; then 
                    let clen=$i*31/16
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi

                if [[ $grain == "32x" ]] ; then 
                    let clen=$i*63/32
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 0 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done
                fi
            done
        done
    done
fi

if [[ "$locality" == "near" || "$locality" == "remote" || "$locality" == "both" || "$locality" == "all" ]] ; then 
    if (( 1 < $dom_cnt )) ; then

        for (( t=$tpr_min ; $t <= $threads_per_domain && $t <= $tpr_max ; t+=1 )) ; do
            for refs in 1 ; do
                let max_pool_per_thread=$maxpool/$t
                for ((i=$minpool; $i <= $max_pool_per_thread; i=$i*2)) ; do
                    let clen=$i
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*33/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*17/16
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*35/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "8x" || $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*9/8
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*37/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*19/16
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*39/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "4x" || $grain == "8x" || $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*5/4
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*41/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*21/16
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*43/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "8x" || $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*11/8
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*45/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*23/16
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*47/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "2x" || $grain == "4x" || $grain == "8x" || $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*6/4
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi 

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*49/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*25/16
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*51/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "8x" || $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*13/8
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*53/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*27/16
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*55/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "4x" || $grain == "8x" || $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*7/4
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi 

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*57/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*29/16
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*59/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "8x" || $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*15/8
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*61/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*31/16
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*63/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 1 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi
                done
            done
        done
    fi
fi

if [[ "$locality" == "far" || "$locality" == "all" ]] ; then 
    if (( 2 < $dom_cnt )) ; then

        for (( t=$tpr_min ; $t <= $threads_per_domain && $t <= $tpr_max ; t+=1 )) ; do
            for refs in 1
            do
                let max_pool_per_thread=$maxpool/$t
                for ((i=$minpool; $i <= $max_pool_per_thread; i=$i*2)) ; do
                    let clen=$i
                    if [[ $page_size == "" ]] ; then
                        ps=${clen}k
                    else
                        ps=$page_size
                    fi
                    for ((j=0; $j < $rep ; j++)) ; do 
                        $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
                        source $bin/pause.sh
                    done

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*33/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*17/16
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*35/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "8x" || $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*9/8
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*37/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*19/16
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*39/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "4x" || $grain == "8x" || $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*5/4
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*41/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*21/16
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*43/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "8x" || $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*11/8
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*45/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*23/16
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*47/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "2x" || $grain == "4x" || $grain == "8x" || $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*6/4
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi 

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*49/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*25/16
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*51/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "8x" || $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*13/8
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*53/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*27/16
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*55/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "4x" || $grain == "8x" || $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*7/4
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi 

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*57/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*29/16
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*59/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "8x" || $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*15/8
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*61/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "16x" || $grain == "32x" ]] ; then 
			let clen=$i*31/16
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi

		    if [[ $grain == "32x" ]] ; then 
			let clen=$i*63/32
			if [[ $page_size == "" ]] ; then
			    ps=${clen}k
			else
			    ps=$page_size
			fi
			for ((j=0; $j < $rep ; j++)) ; do 
			    $pchase_bmk -L sched -d $domains -p ${ps} -t $t -e $exp -r $refs -n gen 0 2 -a $access -c ${clen}k -s 1.0 -o csv -s $sec
			    source $bin/pause.sh
			done
		    fi
                done
            done
        done
    fi
fi

rep=$old_rep

popd
