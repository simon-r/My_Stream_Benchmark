
CC_FLAGS ?= -Ofast -fopenmp -march=native -Wall  -mcmodel=large
LINK_FLAGS = -lpthread -lm

ifeq ($(CC),icx)
    CC_FLAGS = -Ofast -march=native -qopenmp -Wall -mcmodel=large
endif

TARGET_mt_gm=my_stream_mt_gm.exe
TARGET_mt_lm=my_stream_mt_lm.exe
TARGET_OMP_V2=my_stream_OMP.exe
TARGET_MPI=my_stream_MPI.exe

export MPICH_CC=${CC}
export OMPI_CC=${CC}

MPICC=mpicc 

.PHONY: all clean

all: mt_gm mt_lm omp mpi

# set a string with the name of the used compiler
COMPILER = $(shell ${CC} --version | head -n 1)

# capture the system architecture
ARCHITECTURE = $(shell uname -m)

CC_FLAGS += -DCOMPILER="\"${COMPILER}\"" -DARCHITECTURE="\"${ARCHITECTURE}\""


# print the compiler version
$(info Compiler: ${COMPILER})

############################################################
mt_gm: $(TARGET_mt_gm)

${TARGET_mt_gm}: src/my_stream_utils.o src/my_stream_mt_gm.o
	${CC}  src/my_stream_utils.o src/my_stream_mt_gm.o -o ${TARGET_mt_gm} ${CC_FLAGS} ${LINK_FLAGS}

src/my_stream_mt_gm.o: src/my_stream_mt_gm.c
	${CC} -c src/my_stream_mt_gm.c -o src/my_stream_mt_gm.o ${CC_FLAGS}

############################################################
mt_lm: $(TARGET_mt_lm)

$(TARGET_mt_lm): src/my_stream_utils.o src/my_stream_mt_lm.o
	${CC}  src/my_stream_utils.o src/my_stream_mt_lm.o -o ${TARGET_mt_lm} ${CC_FLAGS} ${LINK_FLAGS}

src/my_stream_mt_lm.o: src/my_stream_mt_lm.c
	${CC} -c src/my_stream_mt_lm.c -o src/my_stream_mt_lm.o ${CC_FLAGS}

############################################################
omp: $(TARGET_OMP_V2)

$(TARGET_OMP_V2): src/my_stream_utils.o src/my_stream_OMP.o
	${CC}  src/my_stream_utils.o src/my_stream_OMP.o -o ${TARGET_OMP_V2} ${CC_FLAGS} ${LINK_FLAGS}

src/my_stream_OMP.o: src/my_stream_OMP.c
	${CC} -c src/my_stream_OMP.c -o src/my_stream_OMP.o ${CC_FLAGS}

############################################################
mpi: $(TARGET_MPI)

$(TARGET_MPI): src/my_stream_utils.o src/my_stream_MPI.o
	${MPICC} src/my_stream_utils.o src/my_stream_MPI.o -o ${TARGET_MPI} ${CC_FLAGS} ${LINK_FLAGS}

src/my_stream_MPI.o: src/my_stream_MPI.c
	${MPICC} -c src/my_stream_MPI.c -o src/my_stream_MPI.o ${CC_FLAGS}

############################################################
src/my_stream_utils.o: src/my_stream_utils.c src/my_stream_utils.h
	${CC}  -c src/my_stream_utils.c -o src/my_stream_utils.o  ${CC_FLAGS}


clean:
	rm ${TARGET_mt_gm} ${TARGET_mt_lm} ${TARGET_MPI} ${PWD}/src/*.o ${TARGET_OMP_V2}
