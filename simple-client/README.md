What's this?
============

This is a primitive client for a very stripped-down version of the chat system developed in the system programming lab.
In fact, it merely connects to a given server and then waits for text messages to be entered on standard input and for
network messages received from the server.
Entered messages are sent to the server in the form described in [message.h](message.h) and received network messages
are printed to standard output.

Message format
==============

The exact format of sent and received network messages is described by the header [message.h](message.h) in the
source code.
The client sends line breaks as entered and also prints them as received, so the server is expected not to strip them.

Compile and run
===============

There is a [Makefile](Makefile) included.
To build, just type `make` in the project directory.

The client can then be started in one of the following ways:
* `./simple-client`: Connect to `localhost` using the default port (8111).
* `./simple-client HOSTNAME`: Connect to `HOSTNAME` using the default port.
* `./simple-client HOSTNAME PORT`: Connect to `HOSTNAME` using the given `PORT`

Messages are sent as soon as the return key is pressed.
Received messages are printed after being received completely.
