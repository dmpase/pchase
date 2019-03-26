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

if [[ $memcpy_bmk == "" ]]
then
    if [[ $bit == 32 ]]
    then
        memcpy_bmk="./memcpy32"
    else
        memcpy_bmk="./memcpy64"
    fi
fi

if [[ $sockets == "" ]] 
then
    export sockets="both"
fi

old_rep=$rep
if [[ $rep == "" ]]
then
    export rep="10"
fi

if [[ ${#memcpy_list[*]} < 1 || ( ${#memcpy_list[*]} == 1 && ${memcpy_list[*]} == "" ) ]] ; then
    memcpy_list=( memcpy )
fi

if [[ ! -e $data/$base ]] ; then
    mkdir -p $data/$base
fi

for s in 8 ; do
    for m in ${memcpy_list[*]} ; do
        if [[ $sockets == "uni" || $sockets == "both" ]] ; then
            file="$data/$base/$host-$m-1s-${s}k-$dt.csv"
            date | tee $file
            method="$m" size="$s" ./run-memcpy-1skt.sh | tee -a $file
            $bin/wrapup.sh $memcpy_bmk $file
            if [[ $scp == "yes" ]] ; then echo /usr/bin/scp $file $outdir/$base ; /usr/bin/scp $file $outdir/$base ; fi
        fi

	if (( 1 < $dom_cnt )) ; then 
            if [[ $sockets == "multi" || $sockets == "both" ]] ; then
                file="$data/$base/$host-$m-${dom_cnt}s-${s}k-$dt.csv"
                date | tee $file
                method="$m" size="$s" ./run-memcpy-4skt.sh | tee -a $file
                $bin/wrapup.sh $memcpy_bmk $file
                if [[ $scp == "yes" ]] ; then echo /usr/bin/scp $file $outdir/$base ; /usr/bin/scp $file $outdir/$base ; fi
            fi

#	    file="$data/$base/$host-$m-ld-${s}k-$dt.csv"
#	    date | tee $file
#	    method="$m" size="$s" ./run-memcpy-load.sh | tee -a $file
#           $bin/wrapup.sh $memcpy_bmk $file
#	    if [[ $scp == "yes" ]] ; then echo /usr/bin/scp $file $outdir/$base ; /usr/bin/scp $file $outdir/$base ; fi
	fi
    done
done

rep=$old_rep

popd
