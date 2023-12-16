

CC_FLAGS = -Ofast 

all:
	${CC} -o my_stream src/my_stream.c ${CC_FLAGS} -fopenmp 