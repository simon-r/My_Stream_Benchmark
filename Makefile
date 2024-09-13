
CC_FLAGS ?= -Ofast -fopenmp -march=native -Wall
LINK_FLAGS = -lpthread -lm

ifeq ($(CC),icx)
    CC_FLAGS = -Ofast -march=native -qopenmp -Wall 
endif

TARGET=my_stream_mt_gm.exe
TARGET_mt_lm=my_stream_mt_lm.exe
TARGET_OMP=my_stream_OMP.exe
TARGET_OMP_LOC=my_stream_OMP_loc.exe
TARGET_MPI=my_stream_MPI.exe

export MPICH_CC=${CC}
export OMPI_CC=${CC}

MPICC=mpicc 

all: mt mt_lm omp omp_loc mpi

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
	rm ${TARGET} ${TARGET_mt_lm} ${TARGET_OMP} ${TARGET_MPI} ${PWD}/src/*.o
