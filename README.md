# Chat Project

This is the readme file for the Earlham College CS 410 chat project.

## Files

The following files are included in this project.

* `c-s-socket.h` - the header file containing the required include libraries
* `server.c` - the source code for the chat server
* `client.c` - the source code for the chat client
* `Makefile` - the Makefile for compiling the programs
* `screenshot.png` - the screenshot showing three clients and a server interacting with each other
* `README.md` - this readme file

## Compiling the Programs

Just run `make` to build both the client and server executables. To build each program separately, run `make client` or `make server`. To remove the executables, run `make clean`.

## Running the Programs

Start the server first by running `./server [port]` in the desired TCP port. The server will automatically log all connections and interactions with every client to the terminal. Now, start the client on other machines by running `./client [hostname] [port]`, noting the name of the machine running the server program at the given port.

All clients have the following commands available to interact with each other.

* `/user [name]` - login with the given name
* `/post [message]` - post message to all users
* `/who` - get list of logged in users
* `/help` - display this help message
* `/quit` - disconnect from server

All commands return some response from the server that indicate success or failure. In addition, `/post`ing a message to the server will also echo the message back to the originating client. If a message is received from another client while typing a command, the current input will not be overwritten.

## Functionality

All major components of the project have been successfully implemented, free of noticeable bugs and able to handle most errors. The two suggested additions for this project, however, have not been implemented for the sake of time. Features from the GNU Readline library have also not yet been implemented for the same reason. Another possible improvement for the chat server is to add a way to safely close the server without having to forcefully `kill` the process, as the server will always listen for new client connections.

## Last Updated

15 December 2016
