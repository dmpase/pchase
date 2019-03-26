
#
# BIT  = { 32 | 64 }
#
ifndef BIT
BIT	= 64
endif
LIB	= libmicrobench$(BIT).a
CPSRC	= Lock.C SpinBarrier.C Thread.C Timer.C Queue.C
CCSRC	= cpuid.c
HDR	= $(CPSRC:.C=.h) $(CCSRC:.c=.h)
OBJ	= $(CPSRC:.C=.o) $(CCSRC:.c=.o)
INC     = ../include
BIN     = ../bin

RM	= /bin/rm
MV	= /bin/mv
AR	= /usr/bin/ar
CI	= /usr/bin/ci
CO	= /usr/bin/co
CP	= /bin/cp
TAR	= /bin/tar


ifndef CXX
CXX	= /usr/bin/g++
endif
ifndef CXXFLAGS
CXXFLAGS= -O3 -m$(BIT) -DSCHED
endif

ifndef CC
CC	= /usr/bin/gcc
endif 
ifndef CFLAGS
CFLAGS  = -O3 -m$(BIT) -DSCHED
endif


CCV	= `( $(CC) -v 2>&1 ) | grep -i version`
CCID	= "\"`date +"%Y-%m-%d %T %Z"` `$(BIN)/sha256 -f $<` $(CXV) $(CXX) $(CXXFLAGS) $<\""
CXV	= `( $(CXX) -v 2>&1 ) | grep -i version`
CXID	= "\"`date +"%Y-%m-%d %T %Z"` `$(BIN)/sha256 -f $<` $(CXV) $(CXX) $(CXXFLAGS) $<\""

.c.o:
	$(CC) -c $(CFLAGS) -DID=$(CCID) $<

.C.o:
	$(CXX) -c $(CXXFLAGS) -DID=$(CXID) $<


$(LIB):	$(OBJ)
	$(AR) -cr $(LIB) $(OBJ) 

$(OBJ):	$(HDR)

rmlib:
	$(RM) -rf $(LIB)

rmobj:
	$(RM) -rf $(OBJ)

ci:
	$(CI) -f $(SRC) $(HDR) Makefile

co:
	$(CO) -l $(SRC) $(HDR) Makefile

