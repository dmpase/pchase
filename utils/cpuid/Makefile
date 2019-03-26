
ifndef BIT
BIT	= 64
endif
NAME	= cpuid
SRC	= main.c
OBJ	= $(SRC:.c=.o)
BIN     = ../../bin
LIB     = ../../lib
INC     = ../../lib
EXE	= $(BIN)/$(NAME)$(BIT)

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
CXXFLAGS= -O3 -m$(BIT)
endif

ifndef CC
CC	= /usr/bin/gcc
endif 
ifndef CFLAGS
CFLAGS  = -O3 -m$(BIT)
endif

ifndef LDFLAGS
ifeq ($(BIT),64)
LDFLAGS = -static -L $(LIB) -lmicrobench$(BIT)
else
LDFLAGS = -L /usr/lib -L $(LIB) -lmicrobench$(BIT)
endif
endif


CCV	= `( $(CC) -v 2>&1 ) | grep -i version`
CCID	= "\"`date +"%Y-%m-%d %T %Z"` `$(BIN)/sha256 -f $<` $(CXV) $(CXX) $(CXXFLAGS) $<\""
CXV	= `( $(CXX) -v 2>&1 ) | grep -i version`
CXID	= "\"`date +"%Y-%m-%d %T %Z"` `$(BIN)/sha256 -f $<` $(CXV) $(CXX) $(CXXFLAGS) $<\""

.c.o:
	$(CC) -c $(CFLAGS) -I $(INC) -DID=$(CCID) $<

.C.o:
	$(CXX) -c $(CXXFLAGS) -I $(INC) -DID=$(CXID) $<


$(EXE):	$(OBJ)
	$(CXX) -o $(EXE) $(CXXFLAGS) $(OBJ) $(LIBFLAGS) $(LDFLAGS)

$(OBJ):	$(HDR)

rmexe:
	$(RM) -rf $(EXE)

rmobj:
	$(RM) -rf $(OBJ)

ci:
	$(CI) -f $(SRC) $(HDR) Makefile

co:
	$(CO) -l $(SRC) $(HDR) Makefile

