#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netdb.h>

#include <pthread.h>
#include	<sys/time.h>
#include	<signal.h>

#include "banksys.h"

int n_fd; 

static void sigint_handler( int signo )
{
  char command[5] = "exit";
	printf( "Received Signal SIGNIT: Sending exit command to Server before Client dies. . .\n");

  if ( (write(n_fd, command, strlen(command) ) ) < 0){
         printf("- - - Sorry, your session has expired. - - -\n");
  }
  return;
}

void * rd_Server_User(void * args){
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    int network_socket_fd = *(int *)args;
    int status;

    while( (status = read(network_socket_fd, buffer, sizeof(buffer)) ) > 0 ){
        printf("%s", buffer);
        memset(buffer, 0, sizeof(buffer));
    }
    printf("- - - The Bank Server has closed connection - - -\n- - -   You can no longer send/receive.     - - -\n");
    close(network_socket_fd);
    free(args);
    return 0;
}

int main(int argc, char *argv[]){

    char buffer[256];
    int network_socket_fd;
    void *socket_arg1 = malloc(sizeof(network_socket_fd));
    void *socket_arg2 = malloc(sizeof(network_socket_fd));
    struct addrinfo request;
    request.ai_flags = 0;
    request.ai_family = AF_INET;
    request.ai_socktype = SOCK_STREAM;
    request.ai_protocol = 0;
    request.ai_addrlen = 0;
    request.ai_canonname = NULL;
    request.ai_next = NULL;
    struct addrinfo *result;

    if(argc < 3){
        printf("ERROR: MUST SPECIFY SERVER IP AND PORT\n");
        exit(1);
    }

    if ( getaddrinfo(argv[1], argv[2], &request, &result )){
        printf("ERROR: COULD NOT GET INFO\n");
        exit(1);
    }


    network_socket_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    memcpy(socket_arg1, &network_socket_fd, sizeof(int));
    memcpy(socket_arg2, &network_socket_fd, sizeof(int));
    int status = connect(network_socket_fd, result->ai_addr, result->ai_addrlen);
    while( status < 0 ){
        close(network_socket_fd);
        network_socket_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if(errno != ECONNREFUSED){
              printf("ERROR: %s\n", strerror(errno));
              exit(1);
        }
        printf("Could not find server. Attempting to reconnect in 3 sec. . .\n");
        sleep(3);
        status = connect(network_socket_fd, result->ai_addr, result->ai_addrlen);
    }

    n_fd = network_socket_fd;
    struct sigaction	action;
    action.sa_flags = 0;
	  action.sa_handler = sigint_handler;	/* short form */
	  sigemptyset( &action.sa_mask );		/* no additional signals blocked */
	  sigaction( SIGINT, &action, 0 );

    printf(" - - - Welcome to the Virtual Bank! - - -\n");



    pthread_t server_user;

    if (pthread_create(&server_user, NULL, &rd_Server_User, socket_arg2 ) != 0){
         printf("ERROR: Can't create server user thread: %s\n", strerror(errno));
         exit(1);
    }

    pthread_detach(server_user);

    memset(buffer, 0, sizeof(buffer));

    while( read(0, buffer, sizeof(buffer)) > 0) {
        if ( (status = write(network_socket_fd, buffer, strlen(buffer) ) ) < 0){
               printf("- - - Sorry, your session has expired. - - -\n");
               break;
        }
        memset(buffer, 0, sizeof(buffer));
        sleep(2);
    }

    return 0;

}
