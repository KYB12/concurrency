//
// client.c: A very, very primitive HTTP client.
// 
// To run, try: 
//      client hostname portnumber filename
//
// Sends one HTTP request to the specified HTTP server.
// Prints out the HTTP response.
//
// For testing your server, you will want to modify this client.  
// For example:
// You may want to make this multi-threaded so that you can 
// send many requests simultaneously to the server.
//
// You may also want to be able to request different URIs; 
// you may want to get more URIs from the command line 
// or read the list from a file. 
//
// When we test your server, we will be using modifications to this client.
//

#include "io_helper.h"
#include <time.h>
#include <stdlib.h>
#include <pthread.h>

#define MAXBUF (8192)
char *filenames[4] = {"/index.html", "/acceptman.html","/man.html","/statman.html"};
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

//
// Send an HTTP request for the specified file 
//
void client_send(int fd, char *filename) {
    char buf[MAXBUF];
    char hostname[MAXBUF];
    
    gethostname_or_die(hostname, MAXBUF);
    
    /* Form and send the HTTP request */
    sprintf(buf, "GET %s HTTP/1.1\n", filename);
    sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
    printf("%s",buf);
    write_or_die(fd, buf, strlen(buf));
}

//
// Read the HTTP response and print it out
//
void client_print(int fd) {
    char buf[MAXBUF];  
    int n;
    
    // Read and display the HTTP Header 
    n = readline_or_die(fd, buf, MAXBUF);
    while (strcmp(buf, "\r\n") && (n > 0)) {
	printf("Header: %s", buf);
	n = readline_or_die(fd, buf, MAXBUF);
	
	// If you want to look for certain HTTP tags... 
	// int length = 0;
	//if (sscanf(buf, "Content-Length: %d ", &length) == 1) {
	//    printf("Length = %d\n", length);
	//}
    }
    
    // Read and display the HTTP Body 
    n = readline_or_die(fd, buf, MAXBUF);
    while (n > 0) {
	printf("%s", buf);
	n = readline_or_die(fd, buf, MAXBUF);
    }
}

typedef struct{
    char host[100];
    int port;
} clientArgs;

void *client(void *arg){
    pthread_mutex_lock(&lock);
    clientArgs *args = (clientArgs*) arg; 
    char* host = (*args).host;
    int port = (*args).port;
    int clientfd = open_client_fd_or_die(host, port);
    //get random file
    int r = rand() % 4;
    char *filename = filenames[r];
    client_send(clientfd, filename);
    client_print(clientfd);
    close_or_die(clientfd);
    pthread_mutex_unlock(&lock);
    return NULL;
}


//edited main function to send mutliple requets at once by implementing the client function
int main(int argc, char *argv[]) {
    char *host; //*filename;
    int port;
    //int clientfd;
    
//    if (argc != 4) {
//	fprintf(stderr, "Usage: %s <host> <port> <filename>\n", argv[0]);
//	exit(1);
//    }
    
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <host> <port> <loops>\n", argv[0]);
        exit(1);
    }

    host = argv[1];
    port = atoi(argv[2]);
    int loops = atoi(argv[3]);
    
    /* Open a single connection to the specified host and port */

    pthread_t threads[loops];
    clientArgs args;
    strcpy(args.host, host);
    args.port = port;

    int i;
    for(i = 0; i < loops; i++){
        pthread_create(&threads[i], NULL,client,&args);
    }
    //wait for all the threads to finish
    int j;
    for(j= 0; j < loops; j++){
        pthread_join(threads[j], NULL);
    }
    
    exit(0);
}
