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


#
# BIT  = { 32 | 64 }
#
ifndef BIT
BIT	= 64
endif

ifndef LIBFLAGS
ifeq ($(BIT), 64)
LIBFLAGS= -L$(LIB) -lmicrobench$(BIT) -lpthread
else
LIBFLAGS= -L$(LIB) -L/usr/lib -lmicrobench$(BIT) -lpthread
endif
endif

SRC	= Experiment.C Main.C Memcpy.C Output.C Run.C
HDR	= $(SRC:.C=.h)
OBJ	= $(SRC:.C=.o)
BIN     = ../../bin
LIB     = ../../lib
INC     = $(LIB)
EXE	= $(BIN)/memcpy$(BIT)
TOBJ	= memcpy_test.o Memcpy.o
TEXE	= memcpy_test

RM	= /bin/rm
MV	= /bin/mv
CI	= /usr/bin/ci
CO	= /usr/bin/co
CP	= /bin/cp
TAR	= /bin/tar

ifndef CXX
CXX	= /usr/bin/g++
endif
ifndef CXXFLAGS
CXXFLAGS= -O3 -m$(BIT) -I $(INC) -DNT_INOUT 
endif

ifndef CC
CC	= /usr/bin/gcc
endif 
ifndef CFLAGS
CFLAGS  = -O3 -m$(BIT) -I $(INC)
endif

ifndef LDFLAGS
ifeq ($(BIT),64)
LDFLAGS = -static 
else
LDFLAGS =
endif
endif

# CXXFLAGS= -O3 -m$(BIT) -D$(MODE) -DMEM_CHK -DLIBNUMA -opt-prefetch=3 -opt-prefetch=4 -msse4.1


CCV	= `( $(CC) -v 2>&1 ) | grep -i version`
CCID	= "\"`date +"%Y-%m-%d %T %Z"` `$(BIN)/sha256 -f $<` $(CCV) $(CC) $(CFLAGS) $<\""
CXV	= `( $(CXX) -v 2>&1 ) | grep -i version`
CXID	= "\"`date +"%Y-%m-%d %T %Z"` `$(BIN)/sha256 -f $<` $(CXV) $(CXX) $(CXXFLAGS) $<\""

.c.o:
	$(CC) -c $(CFLAGS) -DID=$(CCID) $<

.C.o:
	$(CXX) -c $(CXXFLAGS) -DID=$(CXID) $<


$(EXE):	$(OBJ)
	$(CXX) -o $(EXE) $(CXXFLAGS) $(OBJ) $(LIBFLAGS) $(LDFLAGS)

$(TEXE):	$(TOBJ)
	$(CXX) -o $(TEXE) $(CXXFLAGS) $(TOBJ) $(LIBFLAGS) $(LDFLAGS)

$(OBJ):	$(HDR)

rmexe:
	$(RM) -rf $(EXE) $(TEXE)

rmobj:
	$(RM) -rf $(OBJ) $(TOBJ)

ci:
	$(CI) -f $(SRC) $(HDR) Makefile

co:
	$(CO) -l $(SRC) $(HDR) Makefile

