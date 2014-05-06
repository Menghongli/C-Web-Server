all: forking epoll
forking: forking-web-server.c
	gcc -Wall -g forking-web-server.c -o forking-web-server
epoll: epoll-server.c
	gcc -Wall -g epoll-server.c -o epoll-server

clean:
	rm forking-web-server epoll-server 
