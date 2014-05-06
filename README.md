Name: Forking-web-server and epoll-server
Version: 1.0
Date: 4/23/2012
Author(s): MenghongLi

Description
===========
Forking-web-server:
It is a simple forking web server, it handle any type of files listed in mime-types.tsv, it also can deal with multiple simultaneous connections by using fork().

Epoll-server:
It is meant to be a single-threaded, single process web server that can handle multiple connections.


Compile and Run
===============

build all servers:
$ make all

build forking-web-server:
$ make forking

build epoll-server:
$ make epoll

run the server by typing:

./forking-web-server [your file] [port]
e.g. ./forking-web-server hello.html 8080;
or
./epoll-server 8080;



Known Issues or Bugs
====================
For the forking-web-server, An "connection was reset" error sometimes occurs
and disappeared by refreshing the page.

For the epoll-server, Currently it only handle a
"Hello, world!" string.
And some unkonwn reason it only works in FireFox,
so please test it in FireFox.




Credits
=======

Thanks to Professor Robert Merkel
