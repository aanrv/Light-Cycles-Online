# Tron-Light-Cycles

![Menu](img/lco_menu.png) ![Gameplay](img/lco_gameplay.png)

An implementation of the Tron Light Cycles game.

It is a multiplayer game.
The server and the client were written using the C Sockets API.
The graphics are text based, as they were not my priority, and were designed using `ncurses`.

Runs on Linux and Mac. To run on a Windows PC, install Linux.

# Dependencies

`ncurses` is required to play the game.

| OS / Distro    | Installation                          |
| -------------- | ------------------------------------- |
| OS X           | Should be included in the [Xcode Tools](https://developer.apple.com/library/mac/documentation/Darwin/Reference/ManPages/man3/ncurses.3x.html).|
| Ubuntu/Debian  | `apt-get install libncurses5-dev`     |
| Fedora         | `yum install ncurses-devel`           |

#  Installation

Simply run `make` to create the server and client executables.

# Starting the Game

1. `./lco-server` to first start the server.
2. `./lco-client` to start the game client.

The game will begin after two clients have connected to the server.

Note: When running client from a different host, you must provide the server host's IP Address. See below.

# Optional Arguments

`lco-server [portnum]`

- `portnum` Port number the server should attempt to run on. Default value is 1337.

`lco-client [-a ipaddr] [-p portnum]`

- `ipaddr` IP Address of the server's host. If not specified, loopback address is used.
- `portnum` Server's port. Default value is 1337.

# Implementation

A client/server model was used to implement the game. The server can be seen as the brain and the clients are the mindless interpreters of information. The game begins once both clients have connected to the server.

**Message Types**

Let us call each bundle of information sent to or from the client or the server a message. Each message has a one byte header that indicates its type. Usually, this is a standard message containing player directions. Additionally, if a client's player collides, the client will send a "collision" message to the server. If a server receives a collision message, it determines the other player as the winner. It then sends both clients a message that declares the winner and tells both clients to end the game. A list of the different message types:

![Message Types](img/message_types.png)

_Client-to-Server Messages_:

1. Standard: Contains client's player's direction [`2 bytes`].
2. Collision: Indicates that client's player has collided [`1 byte`].

_Server-to-Client Messages_:

1. Standard: Contains directions for all players [`3 bytes`] i.e [`(1 + number of players per game) bytes`].
2. End: Notifies clients that game has ended. Contains winning player number [`2 bytes`].

![Client Server Communication Model](img/client_server_communication_model.png)

_Client_:

Along with (re)drawing the screen at each loop, the client's purpose is to keep note of a direction change performed by its player. At each loop, the client checks if an arrow key is pressed. At the end of each loop, the current direction (which would be the new direction, if a key was pressed) is sent to the server. The client also checks if its player has collided. If it has, it sends a collision message to the server. At this point, the client simply waits for the server's response to continue.
Note that it is unnecessary to keep track of locations, since each player's initial location is identical across every game.

_Server_:

The server keeps track of the current directions of both players. It starts the game by sending each client the initial direction of each player. The clients will use this to draw the next screen i.e the next direction in which each player will move. The server now waits for both clients to send information back. This will usually be the direction of the client's player, in which case, the server updates each player's variables and sends out identical information about all the players to each client. In the case that the server receives a collision message, it will determine the winning player and send out a termination message to all clients.

This entire process keeps both clients perfectly synchronized. Additionally, each client will receive identical information so there should be no continuity errors. The downside of this is that one bad connection = high latency for everyone, since the server waits for a response from both clients before proceeding.

# References

1. I learned about the C Sockets API mostly from [The Linux Programming Interface](https://www.amazon.com/Linux-Programming-Interface-System-Handbook/dp/1593272200). It is a textbook for programming in the Linux environment in general.
2. [Beej's Guide](http://beej.us/guide/bgnet/) is a concise overview of Network Programming and is quite popular. I still mostly used the textbook, however.
2. [This](http://gafferongames.com/networking-for-game-programmers/) site has some interesting articles regarding game-specific Network Programming.
