
CC=gcc
CFLAGS=-std=gnu99 -Wall -fsanitize=address,undefined
#LDFLAGS=-fsanitize=address,undefined
#LDLIBS=-lpthread -lm
NAME=main

.PHONY: clean all

all: ${NAME}

${NAME}: ${NAME}.c
	gcc $(CFLAGS) ${LDLIBS} ${LDFLAGS} -o ${NAME} ${NAME}.c

clean:
	rm -f ${NAME}