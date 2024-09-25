
CC_FLAGS ?= -Ofast -fopenmp -march=native -Wall  -mcmodel=large
LINK_FLAGS = -lpthread -lm

ifeq ($(CC),icx)
    CC_FLAGS = -Ofast -march=native -qopenmp -Wall -mcmodel=large
endif

TARGET=my_stream_mt_gm.exe
TARGET_mt_lm=my_stream_mt_lm.exe
TARGET_OMP=my_stream_OMP.exe
TARGET_OMP_V2=my_stream_OMP_v2.exe
TARGET_OMP_LOC=my_stream_OMP_loc.exe
TARGET_MPI=my_stream_MPI.exe

export MPICH_CC=${CC}
export OMPI_CC=${CC}

MPICC=mpicc 

all: mt mt_lm omp omp_loc omp_v2 mpi

# set a string with the name of the used compiler
COMPILER = $(shell ${CC} --version | head -n 1)

# capture the system architecture
ARCHITECTURE = $(shell uname -m)

CC_FLAGS += -DCOMPILER="\"${COMPILER}\"" -DARCHITECTURE="\"${ARCHITECTURE}\""


# print the compiler version
$(info Compiler: ${COMPILER})

############################################################
mt: src/my_stream_utils.o src/my_stream_mt_gm.o
	${CC}  src/my_stream_utils.o src/my_stream_mt_gm.o -o ${TARGET} ${CC_FLAGS} ${LINK_FLAGS}

src/my_stream_mt_gm.o: src/my_stream_mt_gm.c
	${CC} -c src/my_stream_mt_gm.c -o src/my_stream_mt_gm.o ${CC_FLAGS}

############################################################
mt_lm: src/my_stream_utils.o src/my_stream_mt_lm.o
	${CC}  src/my_stream_utils.o src/my_stream_mt_lm.o -o ${TARGET_mt_lm} ${CC_FLAGS} ${LINK_FLAGS}

src/my_stream_mt_lm.o: src/my_stream_mt_lm.c
	${CC} -c src/my_stream_mt_lm.c -o src/my_stream_mt_lm.o ${CC_FLAGS}

############################################################
omp: src/my_stream_utils.o src/my_stream_OMP.o
	${CC}  src/my_stream_utils.o src/my_stream_OMP.o -o ${TARGET_OMP} ${CC_FLAGS} ${LINK_FLAGS} 

src/my_stream_OMP.o: src/my_stream_OMP.c
	${CC} -c src/my_stream_OMP.c -o src/my_stream_OMP.o ${CC_FLAGS} -D__GLOBAL_ALLOC__=1


omp_v2: src/my_stream_utils.o src/my_stream_OMP_v2.o
	${CC}  src/my_stream_utils.o src/my_stream_OMP_v2.o -o ${TARGET_OMP_V2} ${CC_FLAGS} ${LINK_FLAGS}

src/my_stream_OMP_v2.o: src/my_stream_OMP_v2.c
	${CC} -c src/my_stream_OMP_v2.c -o src/my_stream_OMP_v2.o ${CC_FLAGS}


############################################################
omp_loc: src/my_stream_utils.o src/my_stream_OMP_loc.o
	${CC}  src/my_stream_utils.o src/my_stream_OMP.o -o ${TARGET_OMP_LOC} ${CC_FLAGS} ${LINK_FLAGS} 

src/my_stream_OMP_loc.o: src/my_stream_OMP.c
	${CC} -c src/my_stream_OMP.c -o src/my_stream_OMP.o ${CC_FLAGS} -D__GLOBAL_ALLOC__=0




############################################################
mpi: src/my_stream_utils.o src/my_stream_MPI.o
	${MPICC} src/my_stream_utils.o src/my_stream_MPI.o -o ${TARGET_MPI} ${CC_FLAGS} ${LINK_FLAGS}

src/my_stream_MPI.o: src/my_stream_MPI.c
	${MPICC} -c src/my_stream_MPI.c -o src/my_stream_MPI.o ${CC_FLAGS}

############################################################
src/my_stream_utils.o: src/my_stream_utils.c src/my_stream_utils.h
	${CC}  -c src/my_stream_utils.c -o src/my_stream_utils.o  ${CC_FLAGS}


clean:
	rm ${TARGET} ${TARGET_mt_lm} ${TARGET_OMP} ${TARGET_MPI} ${PWD}/src/*.o ${TARGET_OMP_V2} ${TARGET_OMP_LOC}
