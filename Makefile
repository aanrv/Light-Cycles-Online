CC		= gcc
LIBS		= -lncurses
FLAGS		= -Wall -Wextra
S_DIR		= src

default: lco-server lco-client

lco-server:	$(S_DIR)/lco-server.o $(S_DIR)/h.o
	$(CC) $(FLAGS) $(S_DIR)/lco-server.o $(S_DIR)/h.o -o lco-server $(LIBS)

lco-client:	$(S_DIR)/lco-client.o $(S_DIR)/visuals.o $(S_DIR)/mainmenu.o $(S_DIR)/connectionmenu.o $(S_DIR)/instructionsmenu.o $(S_DIR)/gameovermenu.o $(S_DIR)/player.o $(S_DIR)/h.o
	$(CC) $(FLAGS) $(S_DIR)/lco-client.o $(S_DIR)/visuals.o $(S_DIR)/mainmenu.o $(S_DIR)/connectionmenu.o $(S_DIR)/instructionsmenu.o $(S_DIR)/gameovermenu.o $(S_DIR)/player.o $(S_DIR)/h.o -o lco-client $(LIBS)

$(S_DIR)/lco-server.o:	$(S_DIR)/lco-server.c
	$(CC) $(FLAGS) $(S_DIR)/lco-server.c -c -o $(S_DIR)/lco-server.o

$(S_DIR)/lco-client.o:	$(S_DIR)/lco-client.c
	$(CC) $(FLAGS) $(S_DIR)/lco-client.c -c -o $(S_DIR)/lco-client.o

$(S_DIR)/visuals.o:	$(S_DIR)/visuals.c
	$(CC) $(FLAGS) $(S_DIR)/visuals.c -c -o $(S_DIR)/visuals.o

$(S_DIR)/mainmenu.o:	$(S_DIR)/mainmenu.c
	$(CC) $(FLAGS) $(S_DIR)/mainmenu.c -c -o $(S_DIR)/mainmenu.o

$(S_DIR)/connectionmenu.o:	$(S_DIR)/connectionmenu.c
	$(CC) $(FLAGS) $(S_DIR)/connectionmenu.c -c -o $(S_DIR)/connectionmenu.o

$(S_DIR)/instructionsmenu.o:	$(S_DIR)/instructionsmenu.c
	$(CC) $(FLAGS) $(S_DIR)/instructionsmenu.c -c -o $(S_DIR)/instructionsmenu.o

$(S_DIR)/gameovermenu.o:	$(S_DIR)/gameovermenu.c
	$(CC) $(FLAGS) $(S_DIR)/gameovermenu.c -c -o $(S_DIR)/gameovermenu.o

$(S_DIR)/player.o:	$(S_DIR)/player.c
	$(CC) $(FLAGS) $(S_DIR)/player.c -c -o $(S_DIR)/player.o

$(S_DIR)/h.o:	$(S_DIR)/h.c
	$(CC) $(FLAGS) $(S_DIR)/h.c -c -o $(S_DIR)/h.o

clean:
	rm -f lco-server lco-client $(S_DIR)/*.o

