
CC=gcc
CFLAGS=-std=gnu99 -Wall -fsanitize=address,undefined
#LDFLAGS=-fsanitize=address,undefined
#LDLIBS=-lpthread -lm
NAME=main
MACROS=-D __DOOTSTR_DEBUG

.PHONY: clean all

all: ${NAME}

${NAME}: ${NAME}.c
	gcc $(CFLAGS) ${MACROS} ${LDLIBS} ${LDFLAGS} -o ${NAME} ${NAME}.c

clean:
	rm -f ${NAME}