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
#include<sys/sendfile.h>
#include<netdb.h>
#include<string.h>
#include<unistd.h>

#define BACKLOG 20 // how many pending connections queue will hold
#define MIMETYPE "mime-types.tsv"

void send_http_response(int conn, char *name);
char * get_mime_type(char *name);

int main(int argc, char *argv[])
{
	int status;
	int sockfd, new_fd;
	int send_fd;
	off_t offset = 0L;
	struct stat sb;
	struct addrinfo hints;
	struct addrinfo *servinfo; 				// will point to the results 
	struct addrinfo *p;
	struct sockaddr_storage client_addr;
	socklen_t addr_size;
	
	if (argc != 3) {
		fprintf(stderr, "usage: forking web server need more arguments\n");
		return 1;
	}

	
	
	memset(&hints, 0, sizeof hints);	// make sure the struct is empty
	hints.ai_family = AF_UNSPEC; 			// don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; 	// TCP stream sockets
	hints.ai_flags = AI_PASSIVE;			// fill in my IP for me

	if((status = getaddrinfo(NULL, argv[2], &hints, &servinfo)) != 0 ) {
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

	while(1) { //loop for accept incoming connection

		addr_size = sizeof client_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}
		
		printf("server: connection established...\n");

		// read to communicate on socket descriptor new_fd
		if (!fork()) {
			close(sockfd);
			char content_type[50];
			sprintf(content_type, "Content-Type: %s\n", get_mime_type(argv[1]));
			int len = strlen(content_type);
			//printf("%s", content_type);
			if(send(new_fd, "HTTP/1.1 200 OK\n", 16, 0) == -1) {
				perror("send header");
			}
			if(send(new_fd, content_type, len, 0) == -1) {
				perror("send header");
			}
		//	if(send(new_fd, "Server: Leo/0.1\n", 16, 0) == -1) {
		//		perror("send header");
		//	}
		//	if(send(new_fd,"\n", 1, 0) == -1) {
		//		perror("send header");
		//	}
			//open the file
			send_fd = open(argv[1], O_RDONLY);
			if (send_fd == -1 ) {
				perror("Couldn't open file");
				return(1);
			}
			fstat(send_fd, &sb);
			if(sendfile(new_fd, send_fd, &offset, sb.st_size) == -1) {
				perror("send file");
			}
		//	send(new_fd, "Hello, world!", 13, 0);
			close(new_fd);
			close(send_fd);
			exit(0);
		}
		close(new_fd);

	}

	return 0;
}

char * get_mime_type(char *name) {
	char *ext = strrchr(name, '.');
  char delimiters[] = " ";
	char *mime_type = NULL;
	mime_type = malloc (128 * sizeof(char)) ;
	char line[128];
	char *token;
	int line_counter = 1;
	ext++; // skip the '.';
	FILE *mime_type_file = fopen(MIMETYPE, "r");
	if (mime_type_file != NULL) {
		while(fgets(line, sizeof line, mime_type_file) != NULL) {
			if (line_counter > 1)
			{
				if((token = strtok(line,delimiters)) != NULL) {
					if(strcmp(token,ext) == 0) {
						token = strtok(NULL, delimiters);
						strcpy(mime_type, token);
						break;
					}
				}
			}
			line_counter++;
		}
		fclose( mime_type_file );
	} else {
		perror("open");
	}
	return mime_type;
}
