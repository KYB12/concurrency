#include <stdio.h>
#include <pthread.h>
#include "request.h"
#include "io_helper.h"

char default_root[] = ".";

//Implemented producer and consumer functions
pthread_cond_t emptyBuffer = PTHREAD_COND_INITIALIZER;
pthread_cond_t fullBuffer = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int* buffer;
int buffers = 1;
//current number of file descriptors in the buffer
int curBuf = 0;
//holds the current get and post position in the buffer
int post = 0;
int get = 0;

void *producer(void *arg){
	int listen_fd = *((int *) arg);
	while(1){
		//printf("buffers: %i\n", buffers);
		pthread_mutex_lock(&lock);
		while(curBuf == buffers){
			//Wait for buffer to be empty
			pthread_cond_wait(&emptyBuffer,&lock);
		}
		struct sockaddr_in client_addr;
		int client_len = sizeof(client_addr);
		int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
		//printf("accepted file %i\n", conn_fd);
		buffer[post] = conn_fd;  
		post = (post + 1)%buffers;
		curBuf += 1;
		//there is something in the buffer for consumer to pull now
		pthread_cond_signal(&fullBuffer);
		//printf("signaled from producer\n");
		//unlock
		pthread_mutex_unlock(&lock);
	}
}

void *consumer(void *arg){
	//printf("created consumer %i\n", *((int *)arg));
	while(1){
		pthread_mutex_lock(&lock);
		while(curBuf == 0){
			printf("consumer %i sleep\n",*((int *)arg));
			//wait until buffer is full again
			pthread_cond_wait(&fullBuffer,&lock);
		}
		printf("waked consumer %i\n",*((int *)arg));
		int conn_fd = buffer[get];
		request_handle(conn_fd);
		get = (get + 1) % buffers;
		curBuf -= 1;
		close_or_die(conn_fd);
		pthread_cond_signal(&emptyBuffer);
		pthread_mutex_unlock(&lock);
	}
}

int main(int argc, char *argv[]) {
    int c;
    char *root_dir = default_root;
    int port = 10000;
		int threads = 1;
		char *schedule = "FIFO";
		//Changed while loop to also check for thread, buffer, and schedule arguments
    while ((c = getopt(argc, argv, "d:p:t:b:s:")) != -1)
	switch (c) {
	case 'd':
	    root_dir = optarg;
	    break;
	case 'p':
	    port = atoi(optarg);
	    break;
	case 't':
			threads = atoi(optarg);
			break;
	case 'b':
			buffers = atoi(optarg);
			break;
	case 's':
			schedule = optarg;
			break;
	default:
	    fprintf(stderr, "usage: wserver [-d basedir] [-p port] [-t threads] [-b buffers] [-s schedule]\n");
	    exit(1);
	}
		//Initialize Buffer based on arg
		buffer = (int*) malloc(buffers * sizeof(int));

    // run out of this directory
    chdir_or_die(root_dir);

    // now, get to work
		int listen_fd = open_listen_fd_or_die(port);
		pthread_t producerID, consumersID[threads];
		pthread_create(&producerID,NULL,producer,&listen_fd);
		int i;
		int array[6] ={1,2,3,4,5,6};
		for(i = 0; i < threads; i++){
			pthread_create(&consumersID[i], NULL, consumer, &array[i]);
		}
		//only wait for producer since none of the consumers have a termination condition
		pthread_join(producerID, NULL);
    return 0;
}


    


 
