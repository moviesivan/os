
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

 #include "banksys.h"

 account **arr_account;
 int numAccount;
 int * maxAccount;
 int pauseFlag = 0;
 pthread_mutex_t openAuthLock;


 void * clientServiceFunction(void * arg){
      printf("Server has just accepted a client connection.\n");
      int client_socket_fd = *(int *) arg;
      char command[10];
      char name[100];
      char name2[100];
      char ibuffer[256];
      char obuffer[256];
      memset(ibuffer, 0, sizeof(ibuffer));
      memset(obuffer, 0, sizeof(obuffer));
      memset(command, 0, sizeof(command));
      memset(name, 0, sizeof(name));
      command_state cs;
      int session = -1;
      char c_options[255] = "COMMANDS:\n\topen (name)\n\tstart (name)\n\tadd (amount)\n\twithdraw (amount)\n\tsend (amount name)\n\tbalance\n\tfinish\n\texit";
      if ( write(client_socket_fd, obuffer, sprintf(obuffer, "%s\n",c_options) ) < 0 ){
          printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
          exit(1);
        }
      memset(obuffer, 0, sizeof(obuffer));
      // read
      while( read(client_socket_fd, ibuffer, 255) > 0){
          sscanf(ibuffer, "%s %s %s",command, name, name2);

          cs = determineCommand(command);

          if(pauseFlag == 0) switch(cs){
              case cs_open:
                  accOpen(client_socket_fd,&numAccount,&arr_account,name,&session,&openAuthLock, &maxAccount);
                  break;
              case cs_start:
                  accStart(client_socket_fd,&arr_account,name,&session, maxAccount);
                  break;
              case cs_add:
                  accAdd(client_socket_fd,&arr_account,name,&session);
                  break;
              case cs_withdrawl:
                  accWithdrawl(client_socket_fd,&arr_account,name,&session);
                  break;
              case cs_balance:
                  accBalance(client_socket_fd,&arr_account,&session);
                  break;
              case cs_send:
                  accSend(client_socket_fd, &arr_account, name, &session, name2, maxAccount); 
                  break;
              case cs_finish:
                  accFinish(client_socket_fd,&arr_account,&session);
                  break;
              case cs_exit:    
                  if(session < 0){
                      if ( write(client_socket_fd, obuffer, sprintf(obuffer, "- - - EXITED - - -\n- - -Thank you. Please come again! - - -.\n") ) < 0 ){
                          printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
                          exit(1);
                        }
                      printf("Server has closed a client connection\n");
                      close(client_socket_fd);
                      free(arg);
                      return 0;
                    }
                    arr_account[session]->service_flag = 0;
                    pthread_mutex_unlock(&(arr_account[session]->lock));
                    session = -1;
                      if ( write(client_socket_fd, obuffer, sprintf(obuffer, "- - - EXITED - - -\n- - -Thank you. Please come again! - - -.\n") ) < 0 ){
                          printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
                          exit(1);
                        }
                    printf("Server has closed a client connection\n");
                    close(client_socket_fd);
                    free(arg);
                  return 0;
              default:
                  if ( write(client_socket_fd, obuffer, sprintf(obuffer, "ERROR: Invalid argument: %s\n",command) ) < 0 ){
                      printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
                      exit(1);
                  }
                  break;

          }
          if(pauseFlag == 1 && write(client_socket_fd, obuffer, sprintf(obuffer, "- - - Server is in pause mode. - - -\n") ) < 0 ){
               printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
               exit(1);
          }
          if ( write(client_socket_fd, obuffer, sprintf(obuffer, "%s\n",c_options) ) < 0 ){
              printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
              exit(1);
            }
          memset(ibuffer, 0, sizeof(ibuffer));
          memset(obuffer, 0, sizeof(obuffer));
          memset(command, 0, sizeof(command));
          memset(name, 0, sizeof(name));
      }

      free(arg);
      return 0;
 }

 void *print_every(void * args){

      while(1){
          printf("BANK INFORMATION: \n");
          if(numAccount == 0){
              printf("\tNo Account have been opened yet\n");
          }else{
              int i;
              for(i = 0 ; i < (*maxAccount) ; i++){
                  account * ca = arr_account[i];
                  if(ca->service_flag >= 0){
                      if(ca->service_flag == 0){
                          printf("Account name: %s, Balance: %f, In Service: %s\n",ca->name,ca->balance,"NO");
                      }else{
                          printf("Account name: %s, Balance: %f, In Service: %s\n",ca->name,ca->balance,"YES");
                      }
                  }
              }
          }
          sleep(20);
      }
 }

 void *scan_for_stop(void * args){

      char command[100];
      while(1){
          scanf("%s", command);
          if(strcmp(command,"pause") == 0){
              pauseFlag = 1;
          }else if (strcmp(command,"unpause") == 0){
              pauseFlag = 0;
          }
      }
 }

 int main(int argc, char *argv[]) {
    int server_socket_fd;
    int client_socket_fd;
    struct addrinfo request;
    request.ai_flags = AI_PASSIVE;
    request.ai_family = AF_INET;
    request.ai_socktype = SOCK_STREAM;
    request.ai_protocol = 0;
    request.ai_addrlen = 0;
    request.ai_canonname = NULL;
    request.ai_next = NULL;
    struct addrinfo *result;
       
    if(argc < 2){
        printf("ERROR: MUST SPECIFY PORT\n");
        exit(1);
    }

    if ( getaddrinfo(0, argv[1], &request, &result )){
        printf("ERROR: COULD NOT GET INFO\n");
        exit(1);
    }

    if ( (server_socket_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol) ) < 0 ){
        printf("ERROR: SERVER COULD NOT BE CREATED: %s\n", strerror(errno));
        exit(1);
    }

    if( bind(server_socket_fd, result->ai_addr, result->ai_addrlen) < 0){
        printf("ERROR: SERVER COULD NOT BE CREATED: %s\n", strerror(errno));
        exit(1);
    }
    int optval = 1;
    setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR , &optval, sizeof(int));

    numAccount = 0;
    initAccounts(&arr_account, &maxAccount);
    pthread_mutex_init(&openAuthLock, NULL);


    pthread_t get_account_info;
    pthread_create(&get_account_info,NULL,&print_every,NULL);

    pthread_t get_command;
    pthread_create(&get_command,NULL,&scan_for_stop,NULL);

    listen(server_socket_fd,5); 
    void * client_socket_arg;   
    pthread_t client;
    while(1){

        if( (client_socket_fd = accept(server_socket_fd, NULL, NULL)) < 0){
            printf("ERROR: FAILED TO ACCEPT: %s\n", strerror(errno));
            exit(1);
        }

        client_socket_arg = malloc(sizeof(int));
        memcpy(client_socket_arg, &client_socket_fd, sizeof(int));

        if (pthread_create(&client, NULL, &clientServiceFunction, client_socket_arg ) != 0){
            printf("ERROR: Can't create user server thread: %s\n", strerror(errno));
            exit(1);
        }

        if (pthread_detach(client) != 0){
            printf("ERROR: Could not detach client thread: %s\n", strerror(errno));
            exit(1);
        }



    }
 }
