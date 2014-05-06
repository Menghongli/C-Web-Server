/*
 ** forking-web-server.c -- This is a command-line application which taked two command-line arguments- the name of a 
 ** file in the current working directory,and a port number, in that order. The file name must end with one of the 
 ** extensions listed in mime-types.tsv. E.g:
 **	
 ** forking-web-server hello.html 80
 **
 */
#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<netdb.h>
#include<string.h>
#include<sys/sendfile.h>
#include<sys/epoll.h>
#include<unistd.h>
#include<errno.h>

#define MAXEPOLLSIZE 1000
#define BACKLOG 200 // how many pending connections queue will hold


int set_non_blocking(int sockfd)
{
	int flags, s;
	flags = fcntl(sockfd, F_GETFL, 0);
	if(flags == -1)
	{
		perror("fcntl");
		return -1;
	}
	flags |= O_NONBLOCK;
	s = fcntl(sockfd, F_SETFL, flags);
	if(s == -1)
	{
		perror("fcntl");
		return -1;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int status;
	int sockfd, new_fd, kdpfd, nfds, n, curfds;
	struct addrinfo hints;
	struct addrinfo *servinfo; 				// will point to the results 
	struct addrinfo *p;
	struct sockaddr_storage client_addr;
	struct epoll_event ev;
	struct epoll_event *events;
	socklen_t addr_size;

	if (argc != 2) {
		fprintf(stderr, "usage: epoll web server need more arguments\n");
		return 1;
	}



	memset(&hints, 0, sizeof hints);	// make sure the struct is empty
	hints.ai_family = AF_UNSPEC; 			// don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; 	// TCP stream sockets
	hints.ai_flags = AI_PASSIVE;			// fill in my IP for me

	if((status = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0 ) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		return 2;
	}
	// servinfo now points to a linked list of 1 or more struct addrinfos
	for (p = servinfo; p != NULL; p = p->ai_next ) {

		// make a socket:
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		// make the sock non blocking
		set_non_blocking(sockfd);

		// bind it to the port
		if ((bind(sockfd, p->ai_addr, p->ai_addrlen)) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if(p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		return 2; // ???????????
	}

	freeaddrinfo(servinfo); // free the linked-list

	// listen for incoming connection
	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	kdpfd = epoll_create(MAXEPOLLSIZE);
	ev.events = EPOLLIN|EPOLLET;
	ev.data.fd = sockfd;
	if(epoll_ctl(kdpfd, EPOLL_CTL_ADD, sockfd, &ev) < 0)
	{
		fprintf(stderr, "epoll set insert error.");
		return -1;
	} else {
		printf("success insert listening socket into epoll.\n");
	}
	events = calloc (MAXEPOLLSIZE, sizeof ev);
	curfds = 1;
	while(1) 
	{ //loop for accept incoming connection

		nfds = epoll_wait(kdpfd, events, curfds, -1);
		if(nfds == -1)
		{
			perror("epoll_wait");
			break;
		}		
		for (n = 0; n < nfds; ++n)
		{
			if(events[n].data.fd == sockfd){
				addr_size = sizeof client_addr;
				new_fd = accept(events[n].data.fd, (struct sockaddr *)&client_addr, &addr_size);
				if (new_fd == -1)
				{
					if((errno == EAGAIN) ||
						 (errno == EWOULDBLOCK))
					{
						break;
					}
					else
					{
						perror("accept");
						break;
					}
				}
				printf("server: connection established...\n");
				set_non_blocking(new_fd);
				ev.events = EPOLLIN|EPOLLET;
				ev.data.fd = new_fd;
				if(epoll_ctl(kdpfd,EPOLL_CTL_ADD, new_fd, &ev)<0)
				{
					printf("Failed to insert socket into epoll.\n");
				}
				curfds++;
			} else {
				if(send(events[n].data.fd, "Hello, world!", 13, 0) == -1)
				{
					perror("send");
					break;
				}
				epoll_ctl(kdpfd, EPOLL_CTL_DEL, events[n].data.fd, &ev);
				curfds--;
				close(events[n].data.fd);
			}
		}
	}
	free(events);
	close(sockfd);			
	return 0;
}

