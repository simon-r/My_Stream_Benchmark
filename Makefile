

CC_FLAGS = -Ofast -fopenmp -lpthread  -march=native
TARGET=my_stream.exe

all:
	${CC} -o ${TARGET} src/my_stream.c ${CC_FLAGS} 

clean:
	rm ${TARGET}
