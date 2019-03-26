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


export dt=`date "+%Y%m%d%H%M"`

if [[ $outdir == "" ]] ; then
    export outdir="pased@dpase-dl:/home/pased/ub"
fi

if [[ $host == "" ]] ; then
    export host=`hostname | sed -e 's/[.].*$//'`
fi
if [[ $host == "" ]] ; then
    export host="host"
fi

export useall="yes"
benchmark_list=( )
export benchmark_list

export memcpy_useall="yes"
memcpy_list=( )
export memcpy_list

export sync_u_useall="yes"
sync_u_list=( )
export sync_u_list

export sync_c_useall="yes"
sync_c_list=( )
export sync_c_list

export sha_hash_useall="yes"
sha_hash_list=( )
export sha_hash_list

export compress_useall="yes"
compress_list=( )
export compress_list

args=( $* )
export args
for (( i=0 ; $i < ${#args[*]} ; i++ )) ; do
    if [[ ${args[$i]} == "-scp" ]] ; then
        export scp="yes"
        let i++ 
        if [[ ${args[$i]} == "" || ${args[$i]} == '-'* ]] ; then
            let i-- 
        else
            export outdir="${args[$i]}"
        fi
    elif [[ ${args[$i]} == "-locality" ]] ; then
        let i++ 
        export locality="${args[$i]}"
        if   [[ "$locality" =~ "[Ll][Oo][Cc][Aa][Ll]"     ]] ; then
            export locality="local"
        elif [[ "$locality" =~ "[Rr][Ee][Mm][Oo][Tt][Ee]" ]] ; then
            export locality="remote"
        elif [[ "$locality" =~ "[Bb][Oo][Tt][Hh]"         ]] ; then
            export locality="both"
        elif [[ "$locality" =~ "[Nn][Ee][Aa][Rr]"         ]] ; then
            export locality="near"
        elif [[ "$locality" =~ "[Ff][Aa][Rr]"             ]] ; then
            export locality="far"
        elif [[ "$locality" =~ "[Aa][Ll][Ll]"             ]] ; then
            export locality="all"
        else
            export locality=""
        fi
    elif [[ ${args[$i]} == "-grain" ]] ; then
        let i++ 
        if [[ ${args[$i]} == "1x" || ${args[$i]} == '2x' || ${args[$i]} == '4x' ]] ; then
            export grain="${args[$i]}"
        elif [[ ${args[$i]} == "1" ]] ; then
            export grain="1x"
        elif [[ ${args[$i]} == "2" ]] ; then
            export grain="2x"
        elif [[ ${args[$i]} == "4" ]] ; then
            export grain="4x"
        else
            echo "-grain needs '1x', '2x' or '4x'."
            exit 1
        fi
    elif [[ ${args[$i]} == "-getpid" ]] ; then
        benchmark_list=( ${benchmark_list[*]} getpid )
        export useall="no"
    elif [[ ${args[$i]} == "-memcpy" || ${args[$i]} == "-memcopy" ]] ; then
        benchmark_list=( ${benchmark_list[*]} memcpy )
        export useall="no"
        let i++
        for (( ; $i < ${#args[*]} ; i++ )) ; do
            if [[ ${args[$i]} == '-'* ]] ; then
                let i--
                break
            elif [[ ${args[$i]} == "all"       ]] ; then
                export memcpy_useall="yes"
            elif [[ ${args[$i]} == "bcopy"     ]] ; then
                export memcpy_useall="no"
                memcpy_list=( ${memcpy_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "memcpy"    ]] ; then
                export memcpy_useall="no"
                memcpy_list=( ${memcpy_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "memmove"   ]] ; then
                export memcpy_useall="no"
                memcpy_list=( ${memcpy_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "naive"     ]] ; then
                export memcpy_useall="no"
                memcpy_list=( ${memcpy_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "copy_tt"   ]] ; then
                export memcpy_useall="no"
                memcpy_list=( ${memcpy_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "copy_tn"   ]] ; then
                export memcpy_useall="no"
                memcpy_list=( ${memcpy_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "xtta"   ]] ; then
                export memcpy_useall="no"
                memcpy_list=( ${memcpy_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "xttu"   ]] ; then
                export memcpy_useall="no"
                memcpy_list=( ${memcpy_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "xtn"   ]] ; then
                export memcpy_useall="no"
                memcpy_list=( ${memcpy_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "xnt"   ]] ; then
                export memcpy_useall="no"
                memcpy_list=( ${memcpy_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "xnn"   ]] ; then
                export memcpy_useall="no"
                memcpy_list=( ${memcpy_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "rd" || ${args[$i]} == "read" ||${args[$i]} == "memread" ]] ; then
                export memcpy_useall="no"
                memcpy_list=( ${memcpy_list[*]} rd )
            elif [[ ${args[$i]} == "wr" || ${args[$i]} == "write" || ${args[$i]} == "memwrite" ]] ; then
                export memcpy_useall="no"
                memcpy_list=( ${memcpy_list[*]} wr )
            elif [[ ${args[$i]} == "bzero"     ]] ; then
                export memcpy_useall="no"
                memcpy_list=( ${memcpy_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "memset"    ]] ; then
                export memcpy_useall="no"
                memcpy_list=( ${memcpy_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "naivezero" ]] ; then
                export memcpy_useall="no"
                memcpy_list=( ${memcpy_list[*]} ${args[$i]} )
            else
                echo "Unknown memcpy test -- '${args[$i]}'"
                exit 1
            fi
        done
    elif [[ ${args[$i]} == "-pchase" ]] ; then
        benchmark_list=( ${benchmark_list[*]} pchase )
        export useall="no"
    elif [[ ${args[$i]} == "-stream" || ${args[$i]} == "-streams" ]] ; then
        benchmark_list=( ${benchmark_list[*]} stream )
        export useall="no"
    elif [[ ${args[$i]} == "-sync-u" || ${args[$i]} == "-syncu" || ${args[$i]} == "-sync_u" ]] ; then
        benchmark_list=( ${benchmark_list[*]} sync-u )
        export useall="no"
        let i++
        for (( ; $i < ${#args[*]} ; i++ )) ; do
            if [[ ${args[$i]} == '-'* ]] ; then
                let i--
                break
            elif [[ ${args[$i]} == "all"       ]] ; then
                export sync_u_useall="yes"
            elif [[ ${args[$i]} == "atomic32"  ]] ; then
                export sync_u_useall="no"
                sync_u_list=( ${sync_u_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "atomic64"  ]] ; then
                export sync_u_useall="no"
                sync_u_list=( ${sync_u_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "cmpxchg32" ]] ; then
                export sync_u_useall="no"
                sync_u_list=( ${sync_u_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "cmpxchg64" ]] ; then
                export sync_u_useall="no"
                sync_u_list=( ${sync_u_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "cmpxspin"  ]] ; then
                export sync_u_useall="no"
                sync_u_list=( ${sync_u_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "fairspin"  ]] ; then
                export sync_u_useall="no"
                sync_u_list=( ${sync_u_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "mutex"     ]] ; then
                export sync_u_useall="no"
                sync_u_list=( ${sync_u_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "spin"      ]] ; then
                export sync_u_useall="no"
                sync_u_list=( ${sync_u_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "reader"    ]] ; then
                export sync_u_useall="no"
                sync_u_list=( ${sync_u_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "writer"    ]] ; then
                export sync_u_useall="no"
                sync_u_list=( ${sync_u_list[*]} ${args[$i]} )
            else
                echo "Unknown sync-u test -- '${args[$i]}'"
                exit 1
            fi
        done
    elif [[ ${args[$i]} == "-sync-c" || ${args[$i]} == "-syncc" || ${args[$i]} == "-sync_c" ]] ; then
        benchmark_list=( ${benchmark_list[*]} sync-c )
        export useall="no"
        let i++
        for (( ; $i < ${#args[*]} ; i++ )) ; do
            if [[ ${args[$i]} == '-'* ]] ; then
                let i--
                break
            elif [[ ${args[$i]} == "all"       ]] ; then
                export sync_c_useall="yes"
            elif [[ ${args[$i]} == "atomic32"  ]] ; then
                export sync_c_useall="no"
                sync_c_list=( ${sync_c_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "atomic64"  ]] ; then
                export sync_c_useall="no"
                sync_c_list=( ${sync_c_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "cmpxchg32" ]] ; then
                export sync_c_useall="no"
                sync_c_list=( ${sync_c_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "cmpxchg64" ]] ; then
                export sync_c_useall="no"
                sync_c_list=( ${sync_c_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "cmpxspin"  ]] ; then
                export sync_c_useall="no"
                sync_c_list=( ${sync_c_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "fairspin"  ]] ; then
                export sync_c_useall="no"
                sync_c_list=( ${sync_c_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "mutex"     ]] ; then
                export sync_c_useall="no"
                sync_c_list=( ${sync_c_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "spin"      ]] ; then
                export sync_c_useall="no"
                sync_c_list=( ${sync_c_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "reader"    ]] ; then
                export sync_c_useall="no"
                sync_c_list=( ${sync_c_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "writer"    ]] ; then
                export sync_c_useall="no"
                sync_c_list=( ${sync_c_list[*]} ${args[$i]} )
            else
                echo "Unknown sync-c test -- '${args[$i]}'"
                exit 1
            fi
        done
    elif [[ ${args[$i]} == "-sync-l" || ${args[$i]} == "-syncl" || ${args[$i]} == "-sync_l" ]] ; then
        benchmark_list=( ${benchmark_list[*]} sync-l )
        export useall="no"
        let i++
        for (( ; $i < ${#args[*]} ; i++ )) ; do
            if [[ ${args[$i]} == '-'* ]] ; then
                let i--
                break
            elif [[ ${args[$i]} == "all"       ]] ; then
                export sync_l_useall="yes"
            elif [[ ${args[$i]} == "atomic32"  ]] ; then
                export sync_l_useall="no"
                sync_l_list=( ${sync_l_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "atomic64"  ]] ; then
                export sync_l_useall="no"
                sync_l_list=( ${sync_l_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "cmpxchg32" ]] ; then
                export sync_l_useall="no"
                sync_l_list=( ${sync_l_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "cmpxchg64" ]] ; then
                export sync_l_useall="no"
                sync_l_list=( ${sync_l_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "cmpxspin"  ]] ; then
                export sync_l_useall="no"
                sync_l_list=( ${sync_l_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "fairspin"  ]] ; then
                export sync_l_useall="no"
                sync_l_list=( ${sync_l_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "mutex"     ]] ; then
                export sync_l_useall="no"
                sync_l_list=( ${sync_l_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "spin"      ]] ; then
                export sync_l_useall="no"
                sync_l_list=( ${sync_l_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "reader"    ]] ; then
                export sync_l_useall="no"
                sync_l_list=( ${sync_l_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "writer"    ]] ; then
                export sync_l_useall="no"
                sync_l_list=( ${sync_l_list[*]} ${args[$i]} )
            else
                echo "Unknown sync-u test -- '${args[$i]}'"
                exit 1
            fi
        done
    elif [[ ${args[$i]} == "-sha" ]] ; then
        benchmark_list=( ${benchmark_list[*]} sha )
        export useall="no"
        let i++
        for (( ; $i < ${#args[*]} ; i++ )) ; do
            if [[ ${args[$i]} == '-'* ]] ; then
                let i--
                break
            elif [[ ${args[$i]} == "all"      ]] ; then
                export sha_hash_useall="yes"
            elif [[ ${args[$i]} == "sha1" || ${args[$i]} == "sha160" ]] ; then
                export sha_hash_useall="no"
                sha_hash_list=( ${sha_hash_list[*]} sha1 )
            elif [[ ${args[$i]} == "sha1i"  ]] ; then
                export sha_hash_useall="no"
                sha_hash_list=( ${sha_hash_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "sha256g"  ]] ; then
                export sha_hash_useall="no"
                sha_hash_list=( ${sha_hash_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "sha256s"  ]] ; then
                export sha_hash_useall="no"
                sha_hash_list=( ${sha_hash_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "sha256u"  ]] ; then
                export sha_hash_useall="no"
                sha_hash_list=( ${sha_hash_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "sha256v"  ]] ; then
                export sha_hash_useall="no"
                sha_hash_list=( ${sha_hash_list[*]} ${args[$i]} )
            elif [[ ${args[$i]} == "sha256x"  ]] ; then
                export sha_hash_useall="no"
                sha_hash_list=( ${sha_hash_list[*]} ${args[$i]} )
            else
                echo "Unknown sha test -- '${args[$i]}'"
                exit 1
            fi
        done
    elif [[ ${args[$i]} == "-compress" ]] ; then
        benchmark_list=( ${benchmark_list[*]} compress )
        export useall="no"
        let i++
        for (( ; $i < ${#args[*]} ; i++ )) ; do
            if [[ ${args[$i]} == '-'* ]] ; then
                let i--
                break
            elif [[ ${args[$i]} == "all" ]] ; then
                export compress_useall="yes"
            elif [[ ${args[$i]} == "lz" ]] ; then
                export compress_useall="no"
                compress_list=( ${compress_list[*]} ${args[$i]} )
            else
                echo "Unknown compress test -- '${args[$i]}'"
                exit 1
            fi
        done
    fi
done

if [[ $useall == "yes" ]] ; then
    benchmark_list=( )
    benchmark_list=( ${benchmark_list[*]} compress )
    benchmark_list=( ${benchmark_list[*]} getpid   )
    benchmark_list=( ${benchmark_list[*]} memcpy   )
    benchmark_list=( ${benchmark_list[*]} pchase   )
    benchmark_list=( ${benchmark_list[*]} sha      )
    benchmark_list=( ${benchmark_list[*]} stream   )
    benchmark_list=( ${benchmark_list[*]} sync-u   )
    benchmark_list=( ${benchmark_list[*]} sync-c   )
fi

if [[ $memcpy_useall == "yes" ]] ; then
    memcpy_list=( )
    memcpy_list=( ${memcpy_list[*]} memcpy  )
    memcpy_list=( ${memcpy_list[*]} copy_tt )
    memcpy_list=( ${memcpy_list[*]} copy_tn )
    memcpy_list=( ${memcpy_list[*]} naive   )
    memcpy_list=( ${memcpy_list[*]} xtn     )
    memcpy_list=( ${memcpy_list[*]} xnn     )
    memcpy_list=( ${memcpy_list[*]} rd      )
    memcpy_list=( ${memcpy_list[*]} wr      )
fi
export memcpy_list

if [[ $sync_u_useall == "yes" ]] ; then
    sync_u_list=( )
#   sync_u_list=( ${sync_u_list[*]} atomic32  )
#   sync_u_list=( ${sync_u_list[*]} atomic64  )
    sync_u_list=( ${sync_u_list[*]} cmpxchg32 )
    sync_u_list=( ${sync_u_list[*]} cmpxchg64 )
#   sync_u_list=( ${sync_u_list[*]} cmpxspin  )
#   sync_u_list=( ${sync_u_list[*]} fairspin  )
    sync_u_list=( ${sync_u_list[*]} mutex     )
    sync_u_list=( ${sync_u_list[*]} spin      )
    sync_u_list=( ${sync_u_list[*]} reader    )
    sync_u_list=( ${sync_u_list[*]} writer    )
fi

if [[ $sync_c_useall == "yes" ]] ; then
    sync_c_list=( )
#   sync_c_list=( ${sync_c_list[*]} atomic32  )
#   sync_c_list=( ${sync_c_list[*]} atomic64  )
    sync_c_list=( ${sync_c_list[*]} cmpxchg32 )
    sync_c_list=( ${sync_c_list[*]} cmpxchg64 )
#   sync_c_list=( ${sync_c_list[*]} cmpxspin  )
#   sync_c_list=( ${sync_c_list[*]} fairspin  )
    sync_c_list=( ${sync_c_list[*]} mutex     )
    sync_c_list=( ${sync_c_list[*]} spin      )
    sync_c_list=( ${sync_c_list[*]} reader    )
    sync_c_list=( ${sync_c_list[*]} writer    )
fi

if [[ $sync_l_useall == "yes" ]] ; then
    sync_l_list=( )
#   sync_l_list=( ${sync_l_list[*]} atomic32  )
#   sync_l_list=( ${sync_l_list[*]} atomic64  )
    sync_l_list=( ${sync_l_list[*]} cmpxchg32 )
    sync_l_list=( ${sync_l_list[*]} cmpxchg64 )
#   sync_l_list=( ${sync_l_list[*]} cmpxspin  )
#   sync_l_list=( ${sync_l_list[*]} fairspin  )
    sync_l_list=( ${sync_l_list[*]} mutex     )
    sync_l_list=( ${sync_l_list[*]} spin      )
    sync_l_list=( ${sync_l_list[*]} reader    )
    sync_l_list=( ${sync_l_list[*]} writer    )
fi

if [[ $sha_hash_useall == "yes" ]] ; then
    sha_hash_list=( )
    sha_hash_list=( ${sha_hash_list[*]} sha1    )
    sha_hash_list=( ${sha_hash_list[*]} sha256u )
    sha_hash_list=( ${sha_hash_list[*]} sha256x )
fi
export sha_hash_list

if [[ $comprpess_useall == "yes" ]]; then
    compress_list=( )
    compress_list=( ${compress_list[*]} lz )
fi
export compress_list

if [[ $bin == "" ]]
then
    export bin="../bin"
fi

source $bin/include.sh

if [[ ! -e $data ]] ; then
    mkdir -p $data
fi
if [[ ! -d $data ]] ; then
    echo "Can't create directory \"$data\"."
    exit
fi

