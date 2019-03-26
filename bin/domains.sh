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


if [[ $domains == "" ]]
then
    if [[ -d /sys/devices/system/node/ ]] ; then
	pushd /sys/devices/system/node/
	nodelist=( `echo node[0-9]*/cpu[0-9]*` )
	popd

	let threads=1
	let dom_cnt=1
	for i in ${nodelist[*]} ; do
	    node=`echo $i | sed -e 's:node::' -e 's:/.*::'`
	    cpu=`echo $i | sed -e 's:node.*/cpu::'`
	    dl[$cpu]=$node
	    if (( $threads <= $cpu )) ; then
		let threads=$cpu+1
	    fi
	    if (( $dom_cnt <= $node )) ; then
		let dom_cnt=$node+1
	    fi
	done

	export threads
	export dom_cnt
	let threads_per_domain=$threads/$dom_cnt
	export threads_per_domain

	export domains=""
	for i in ${dl[*]} ; do
	    if [[ $domains == "" ]] ; then
		export domains="$i"
	    else 
		export domains="${domains},$i"
	    fi
	done
    elif [[ -d /sys/devices/system/cpu/ ]] ; then
	pushd /sys/devices/system/cpu/
	nodelist=( `echo cpu[0-9]*` )
	popd

	let threads=${#nodelist[*]}
	let dom_cnt=1
	let threads_per_domain=$threads
	export threads
	export dom_cnt
	export threads_per_domain

	export domains=""
	for ((i=0; i < $threads; i++)) ; do
	    if [[ $domains == "" ]] ; then
		export domains="$i"
	    else 
		export domains="${domains},0"
	    fi
	done
    else
	let threads=`cat /proc/cpuinfo | grep processor | wc -l`
	let dom_cnt=1
	let threads_per_domain=$threads
	export threads
	export dom_cnt
	export threads_per_domain

	export domains=""
	for ((i=0; i < $threads; i++)) ; do
	    if [[ $domains == "" ]] ; then
		export domains="$i"
	    else 
		export domains="${domains},0"
	    fi
	done
    fi
fi

