CC = g++
FLAGS = 
LIBS =

PROGS = tcpservgroup tcpcligroup

all: ${PROGS}

tcpservgroup: tcpservgroup.o wrapsock.o
	${CC} ${FLAGS} -o $@ $^ ${LIBS}

tcpcligroup: tcpcligroup.o strcli.o wrapsock.o
	${CC} ${FLAGS} -o $@ $^ ${LIBS}

clean:
	rm -f ${PROGS} *.o
