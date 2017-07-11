/*
 ============================================================================
 Name        : client.c
 Author      : Van Petrosyan
 Version     :
 Copyright   : Your copyright notice
 Description : client in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <curses.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>


#define BUFSIZE 1024

void *sender();
void *listener();
void sigint_handler();

static int sockfd;

int main(int argc, char **argv) {
	int portno;
	struct sockaddr_in serveraddr;
	struct hostent *server;
	char *hostname;	

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
	fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK); // set socket in non-blocking state

    // init ncurses
    initscr();
    nodelay(stdscr, TRUE);				// getch will return ERR if there is no input 
    cbreak();							// don't wait for new line
    noecho();							// don't echo chars  
    clear(); 							/* Clear the screen */

    signal(SIGINT, sigint_handler);		// handle CTRL+C
    int n = 0;
    char ch;
    char rcv_buf[BUFSIZE];	
	while(1){
		usleep(1000);
		// send message from keyboard to server
		ch = getch();
		if (ch != ERR){
			n = write(sockfd, &ch, 1);
			if (n < 0){
			  	perror("ERROR writing to socket");
			  	endwin();
				close(sockfd);
		  		exit(EXIT_FAILURE);
			}
		} 
		// Listen for messages and display them
		memset(rcv_buf, 0, BUFSIZE);
        n = read(sockfd, rcv_buf, BUFSIZE);
		if (n > 0)
		 	write( STDOUT_FILENO, rcv_buf, strlen( rcv_buf ));		 
	}
	return EXIT_SUCCESS;
}

// Catch SIGINT
void sigint_handler(){
	char sig = 0x03; // send ctrl+c
	write(sockfd, &sig, 1);
}
