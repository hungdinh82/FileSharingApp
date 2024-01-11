CFLAGS = -g -c -Wall
CC = gcc
LIBS =  -lm

all: server client

server: tcp-server protocol.o authenticate.o status.o validate.o path.o zipper.o
	${CC} -pthread server.o protocol.o authenticate.o status.o validate.o path.o zipper.o -o server -lzip

client: tcp-client protocol.o  status.o validate.o stack.o zipper.o
	${CC} client.o protocol.o status.o validate.o stack.o zipper.o -o client -lzip

tcp-server: server.c
	${CC} ${CFLAGS} -o server.o server.c -lzip

tcp-client: client.c
	${CC} ${CFLAGS} -o client.o client.c -lzip

protocol: protocol.c
	${CC} ${CFLAGS} protocol.c

authenticate: authenticate.c
	${CC} ${CFLAGS} authenticate.c

status: status.c
	${CC} ${CFLAGS} status.c

validate: validate.c
	${CC} ${CFLAGS} validate.c

path: path.c
	${CC} ${CFLAGS} path.c

stack: stack.c
	${CC} ${CFLAGS} stack.c

zipper: zipper.c
	${CC} ${CFLAGS} zipper.c -lzip

clean:
	rm -f *.o *~
	rm server
	rm client