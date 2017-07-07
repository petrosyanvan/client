/*
 ============================================================================
 Name        : client.c
 Author      : Van Petrosyan
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <curses.h>

#define BUFSIZE 1024


int main(int argc, char **argv) {
	int sockfd, portno, n;
	struct sockaddr_in serveraddr;
	struct hostent *server;
	char *hostname;
	char buf[BUFSIZE];

	/* check command line arguments */
	if (argc != 3) {
	   fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
	   exit(0);
	}
	hostname = argv[1];
	portno = atoi(argv[2]);

	/* socket: create the socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0){
		perror("ERROR opening socket");
		exit(EXIT_FAILURE);
	}

	/* gethostbyname: get the server's DNS entry */
	server = gethostbyname(hostname);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host as %s\n", hostname);
		exit(EXIT_FAILURE);
	}

	/* build the server's Internet address */
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
	serveraddr.sin_port = htons(portno);

	/* connect: create a connection with the server */
	if (connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0){
	  perror("ERROR connecting");
	  exit(EXIT_FAILURE);
	}

	initscr();
	//noecho();
	nodelay(stdscr, TRUE);
	cbreak();
	keypad(stdscr, TRUE);
	int ch;
	int i = 0;
	char rcv_buf[BUFSIZE];
	/* we have connected to the server */
	/* get message line from the user */
	while(1){

		if ((ch = getch()) == ERR){

		}
		else{
			rcv_buf[i++] = ch;
			if(ch == '\n'){
				rcv_buf[i] = '\0';
				n = write(sockfd, rcv_buf, strlen(buf));
				if (n < 0){
				  perror("ERROR writing to socket");
				  exit(EXIT_FAILURE);
				}
				i = 0;
				bzero(rcv_buf, BUFSIZE);
			}
		}

		//bzero(buf, BUFSIZE);
		//fgets(buf, BUFSIZE, stdin);

		/* send the message line to the server */
		/*n = write(sockfd, buf, strlen(buf));
		if (n < 0){
		  perror("ERROR writing to socket");
		  exit(EXIT_FAILURE);
		} */

		/* print the server's reply */
		bzero(buf, BUFSIZE);
		n = read(sockfd, buf, BUFSIZE);
		if (n < 0){
		  perror("ERROR reading from socket");
		  exit(EXIT_FAILURE);
		}
		printf("%s\n", buf);
	}
	endwin();
	close(sockfd);
	return EXIT_SUCCESS;
}



