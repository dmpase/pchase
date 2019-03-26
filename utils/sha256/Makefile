
NAME	= sha256
SRC	= Main.C SHA.C 
HDR	= $(SRC:.C=.h)
OBJ	= $(SRC:.C=.o)
BIN     = ../../bin
LIB     = ../../lib
EXE	= $(BIN)/$(NAME)

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
ifeq ($(BIT),)
CXXFLAGS= -O3
else
CXXFLAGS= -O3 -m$(BIT)
endif
endif

ifndef CC
CC	= /usr/bin/gcc
endif 
ifndef CFLAGS
ifeq ($(BIT),)
CFLAGS  = -O3
else
CFLAGS  = -O3 -m$(BIT)
endif
endif

ifndef LDFLAGS
ifeq ($(BIT),64)
LDFLAGS = -static
else ifeq ($(BIT),32)
LDFLAGS = -L /usr/lib
endif
endif


CCV	= `( $(CC) -v 2>&1 ) | grep -i version`
CCID	= "\"`date +"%Y-%m-%d %T %Z"` `$(BIN)/sha256 -f $<` $(CXV) $(CXX) $(CXXFLAGS) $<\""
CXV	= `( $(CXX) -v 2>&1 ) | grep -i version`
CXID	= "\"`date +"%Y-%m-%d %T %Z"` `$(BIN)/sha256 -f $<` $(CXV) $(CXX) $(CXXFLAGS) $<\""

.c.o:
	$(CC) -c $(CFLAGS) -DID=$(CCID) $<

.C.o:
	$(CXX) -c $(CXXFLAGS) -DID=$(CXID) $<


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

