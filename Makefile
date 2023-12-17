

CC_FLAGS = -Ofast -fopenmp -lpthread  -march=native -Wall
TARGET=my_stream.exe
TARGET_V2=my_stream_V2.exe

all:
	${CC} -o ${TARGET} src/my_stream.c ${CC_FLAGS} 

v2:
	${CC} -o ${TARGET_V2} src/my_stream_V2.c ${CC_FLAGS} 

clean:
	rm ${TARGET}
