CC	= gcc
LIBS	= -lncurses
FLAGS	= -Wall -Wextra

default: lco-server lco-client

lco-server:	src/lco-server.o src/h.o
	$(CC) $(FLAGS) src/lco-server.o src/h.o -o lco-server $(LIBS)

lco-client:	src/lco-client.o src/visuals.o src/mainmenu.o src/connectionmenu.o src/instructionsmenu.o src/gameovermenu.o src/player.o src/h.o
	$(CC) $(FLAGS) src/lco-client.o src/visuals.o src/mainmenu.o src/connectionmenu.o src/instructionsmenu.o src/gameovermenu.o src/player.o src/h.o -o lco-client $(LIBS)

src/lco-server.o:	src/lco-server.c
	$(CC) $(FLAGS) src/lco-server.c -c -o src/lco-server.o

src/lco-client.o:	src/lco-client.c
	$(CC) $(FLAGS) src/lco-client.c -c -o src/lco-client.o

src/visuals.o:	src/visuals.c
	$(CC) $(FLAGS) src/visuals.c -c -o src/visuals.o

src/mainmenu.o:	src/mainmenu.c
	$(CC) $(FLAGS) src/mainmenu.c -c -o src/mainmenu.o

src/connectionmenu.o:	src/connectionmenu.c
	$(CC) $(FLAGS) src/connectionmenu.c -c -o src/connectionmenu.o

src/instructionsmenu.o:	src/instructionsmenu.c
	$(CC) $(FLAGS) src/instructionsmenu.c -c -o src/instructionsmenu.o

src/gameovermenu.o:	src/gameovermenu.c
	$(CC) $(FLAGS) src/gameovermenu.c -c -o src/gameovermenu.o

src/player.o:	src/player.c
	$(CC) $(FLAGS) src/player.c -c -o src/player.o

src/h.o:	src/h.c
	$(CC) $(FLAGS) src/h.c -c -o src/h.o

clean:
	rm -f lco-server lco-client src/*.o

