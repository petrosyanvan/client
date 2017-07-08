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
#include <pthread.h>
#include <termios.h>

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
	//char buf[BUFSIZE];

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

	// Set up threads
    pthread_t threads[2];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    // Spawn the listen/receive deamons    
    pthread_create(&threads[0], &attr, listener, NULL);
    pthread_create(&threads[1], &attr, sender, NULL);

    // init ncurses
    initscr();
    nodelay(stdscr, FALSE);				// getch will return ERR if there is no input 
    //keypad(stdscr, TRUE);
    noecho();							// don't echo chars
    raw();								// don't wait for new line

    signal(SIGINT, sigint_handler);		// handle CTRL+C

	while(1)
		sleep(1);										
	return EXIT_SUCCESS;
}

// Send message from keyboard to server
void *sender(){
	int n = 0;
	char ch;	
    while(1){	
		ch = getch();
		if (ch != ERR){
			n = write(sockfd, &ch, 1);
			if (n < 0){
			  	perror("ERROR writing to socket");
			  	endwin();
				pthread_exit(NULL);
				close(sockfd);
		  		exit(EXIT_FAILURE);
			}
		}
    }
}

// Listen for messages and display them
void *listener(){
    char rcv_buf[BUFSIZE];
    int n = 0;
    while(1) {
        memset(rcv_buf, 0, BUFSIZE);
        //Receive message from server
        n = read(sockfd, rcv_buf, BUFSIZE);
		if (n < 0){
		 	perror("ERROR reading from socket");
		  	endwin();
			pthread_exit(NULL);
			close(sockfd);
		  	exit(EXIT_FAILURE);
		}   
		write( STDOUT_FILENO, rcv_buf, strlen( rcv_buf ));
    }
}


// Catch SIGINT
void sigint_handler(){
	char sig = 0x03; // send ctrl+c
	write(sockfd, &sig, 1);
}
