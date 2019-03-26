SRC =	stream_d_omp.c  \
	second_wall.c

OBJ =	$(SRC:.c=.o)
BIN     = ../../bin

EXE =	$(BIN)/stream_omp_x86_64
# EXE =	stream_icc_omp_x86_64

BIT = 64

LDFLAGS = -static
LIBFLAGS= -lm

# CC = /opt/intel/Compiler/11.1/073/bin/intel64/icc
# CFLAGS = -O3 -openmp
# CFLAGS = -O3 -openmp -xSSE4.2 -fast

ifndef CC
CC	= /usr/bin/gcc
endif 
ifndef CFLAGS
CFLAGS  = -O3 -fopenmp -m$(BIT)
endif

CCV	= `( $(CC) -v 2>&1 ) | grep -i version`
CCID	= "\"`date +"%Y-%m-%d %T %Z"` `$(BIN)/sha256 -f $<` $(CCV) $(CC) $(CFLAGS) $<\""
CXV	= `( $(CXX) -v 2>&1 ) | grep -i version`
CXID	= "\"`date +"%Y-%m-%d %T %Z"` `$(BIN)/sha256 -f $<` $(CXV) $(CXX) $(CXXFLAGS) $<\""

.c.o:
	$(CC) -c $(CFLAGS) -DID=$(CCID) $<

.C.o:
	$(CXX) -c $(CXXFLAGS) -DID=$(CXID) $<

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJ) $(LIBFLAGS) $(LDFLAGS)

rmobj:
	$(RM) $(OBJ)

rmexe:
	$(RM) $(EXE)
