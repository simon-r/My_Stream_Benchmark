
CC_FLAGS = -Ofast -fopenmp  -march=native -Wall
LINK_FLAGS = -lpthread
TARGET=my_stream_mt_gm.exe
TARGET_mt_lm=my_stream_mt_lm.exe
TARGET_OMP=my_stream_OMP.exe

all: mt mt_lm omp

############################################################
mt: src/my_stream_mt_gm.c
	${CC} -o ${TARGET} src/my_stream_mt_gm.c ${CC_FLAGS} ${LINK_FLAGS} 

############################################################
mt_lm: src/my_stream_utils.o src/my_stream_mt_lm.o
	${CC}  src/my_stream_utils.o src/my_stream_mt_lm.o -o ${TARGET_mt_lm} ${CC_FLAGS} ${LINK_FLAGS}

src/my_stream_mt_lm.o: src/my_stream_mt_lm.c
	${CC} -c ${CC_FLAGS}  src/my_stream_mt_lm.c -o src/my_stream_mt_lm.o

src/my_stream_utils.o: src/my_stream_utils.c src/my_stream_utils.h
	${CC}  -c  src/my_stream_utils.c -o src/my_stream_utils.o  ${CC_FLAGS}

############################################################
omp: src/my_stream_utils.o src/my_stream_OMP.o
	${CC}  src/my_stream_utils.o src/my_stream_OMP.o -o ${TARGET_OMP} ${CC_FLAGS} ${LINK_FLAGS} 

src/my_stream_OMP.o: src/my_stream_OMP.c
	${CC} -c ${CC_FLAGS}  src/my_stream_OMP.c -o src/my_stream_OMP.o

src/my_stream_utils.o: src/my_stream_utils.c src/my_stream_utils.h
	${CC}  -c  src/my_stream_utils.c -o src/my_stream_utils.o  ${CC_FLAGS}



clean:
	rm ${TARGET} ${TARGET_mt_lm} ${TARGET_OMP} ${PWD}/src/*.o
