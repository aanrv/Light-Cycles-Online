# Tron-Light-Cycles-Online

![Menu](img/lco_menu.png) ![Game](img/lco_gameplay.png)

An implementation of the Tron Light Cycles game.
The graphics are text based and were designed using `ncurses`.

It is a multiplayer LAN game.
Server and client were written using the C Sockets API.

Runs on Linux and Mac.

# Dependencies

`ncurses` is required to play the game.

| OS / Distro    | Installation                          |
| -------------- | ------------------------------------- |
| Mac            | Should be included in the [Xcode Tools](https://developer.apple.com/library/mac/documentation/Darwin/Reference/ManPages/man3/ncurses.3x.html).|
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

