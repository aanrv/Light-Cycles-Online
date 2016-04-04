CC	= gcc
LIBS	= -lncurses
FLAGS	= -Wall -Wextra

default: lco-server lco-client

lco-server:	src/lco-server.o src/player.o
	$(CC) src/lco-server.o src/player.o -o lco-server $(LIBS) $(FLAGS)

lco-client:	src/lco-client.o src/player.o
	$(CC) src/lco-client.o src/player.o -o lco-client $(LIBS) $(FLAGS)

src/lco-server.o:	src/lco-server.c
	$(CC) src/lco-server.c -c -o src/lco-server.o $(FLAGS)

src/lco-client.o:	src/lco-client.c
	$(CC) src/lco-client.c -c -o src/lco-client.o $(FLAGS)

src/player.o:	src/player.c
	$(CC) src/player.c -c -o src/player.o $(FLAGS)

clean:
	rm -r lco-server lco-client src/*.o

