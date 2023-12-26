

CC_FLAGS = -Ofast -fopenmp -lpthread  -march=native -Wall
TARGET=my_stream.exe
TARGET_V2=my_stream_V2.exe
TARGET_OMP=my_stream_OMP.exe

############################################################
all: src/my_stream.c
	${CC} -o ${TARGET} src/my_stream.c ${CC_FLAGS} 

############################################################
v2: src/my_stream_utils.o src/my_stream_V2.o
	${CC}  src/my_stream_utils.o src/my_stream_V2.o -o ${TARGET_V2} ${CC_FLAGS} 

src/my_stream_V2.o: src/my_stream_V2.c
	${CC} -c ${CC_FLAGS}  src/my_stream_V2.c -o src/my_stream_V2.o

src/my_stream_utils.o: src/my_stream_utils.c src/my_stream_utils.h
	${CC}  -c  src/my_stream_utils.c -o src/my_stream_utils.o  ${CC_FLAGS}

############################################################
omp: src/my_stream_utils.o src/my_stream_OMP.o
	${CC}  src/my_stream_utils.o src/my_stream_OMP.o -o ${TARGET_OMP} ${CC_FLAGS} 

src/my_stream_OMP.o: src/my_stream_OMP.c
	${CC} -c ${CC_FLAGS}  src/my_stream_OMP.c -o src/my_stream_OMP.o

src/my_stream_utils.o: src/my_stream_utils.c src/my_stream_utils.h
	${CC}  -c  src/my_stream_utils.c -o src/my_stream_utils.o  ${CC_FLAGS}



clean:
	rm ${TARGET} ${TARGET_V2} ${TARGET_OMP} ${PWD}/src/*.o
