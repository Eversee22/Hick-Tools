CC=gcc
FLAGS= 
LIBS=

PROGS=serv cli
DEPS=$(wildcard *.h) Makefile 

all: ${PROGS}

serv: serv.o
	${CC} ${FLAGS} -o $@ $^ ${LIBS}

cli: cli.o
	${CC} ${FLAGS} -o $@ $^ ${LIBS}

%.o: %.c $(DEPS)
	${CC} ${FLAGS} -c $< -o $@ ${LIBS}

clean:
	rm -f ${PROGS} *.o
