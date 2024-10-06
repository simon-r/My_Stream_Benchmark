
CC_FLAGS ?= -Ofast -fopenmp -march=native -Wall  -mcmodel=large
LINK_FLAGS = -lpthread -lm

# get the OpenMP version
OPENMP_VERSION = $(shell ${CC} -fopenmp -dM -E - < /dev/null | grep -i openmp | cut -d' ' -f3)

ifeq ($(CC),icx)
    CC_FLAGS = -Ofast -march=native -qopenmp -Wall -mcmodel=large
	# get the OpenMP version
	OPENMP_VERSION = $(shell ${CC} -qopenmp -dM -E - < /dev/null | grep -i openmp | cut -d' ' -f3)
endif

# get the first word of the OpenMP version
OPENMP_VERSION_MAJOR = $(firstword $(subst ., ,$(OPENMP_VERSION)))

TARGET_mt_gm=my_stream_mt_gm.bin
TARGET_mt_lm=my_stream_mt_lm.bin
TARGET_OMP_V2=my_stream_OMP.bin
TARGET_MPI=my_stream_MPI.bin

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
CC_FLAGS += -DOPENMP_VERSION=${OPENMP_VERSION_MAJOR}

$(info OPENMP_VERSION: ${OPENMP_VERSION_MAJOR})

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


############################################################
install:
	@echo "Installing the executables in the bin directory"
	@mkdir -p ${INSTALL_DIR}
	@install -m 755 ${TARGET_mt_gm} ${INSTALL_DIR} --strip --verbose
	@install -m 755 ${TARGET_mt_lm} ${INSTALL_DIR} --strip --verbose
	@install -m 755 ${TARGET_OMP_V2} ${INSTALL_DIR} --strip --verbose
	@install -m 755 ${TARGET_MPI} ${INSTALL_DIR} --strip --verbose

uninstall:
	@echo "Uninstalling the executables from the bin directory"
	@rm -f ${INSTALL_DIR}/${TARGET_mt_gm} -v
	@rm -f ${INSTALL_DIR}/${TARGET_mt_lm} -v
	@rm -f ${INSTALL_DIR}/${TARGET_OMP_V2} -v
	@rm -f ${INSTALL_DIR}/${TARGET_MPI} -v
	
############################################################
clean:
	rm ${TARGET_mt_gm} ${TARGET_mt_lm} ${TARGET_MPI} ${PWD}/src/*.o ${TARGET_OMP_V2}
