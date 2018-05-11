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


int isamount(char * amount){
    int periodCount = 0;
    int len = strlen(amount);
    int i;
    for(i = 0 ; i < len ; i++){
        if(isdigit(amount[i]) == 0){
            if(amount[i] == '.'){
                if(periodCount++ > 1){
                    return 0;
                }
            }else{
                return 0;
            }
        }
    }
    return 1;
}

command_state determineCommand(char * command){
    int i;
    for(i = 0; i < strlen(command); i++){
        command[i] = tolower(command[i]);
    }

    if(strcmp(command,"open") == 0){
        return cs_open;
    }else if(strcmp(command,"start") == 0){
        return cs_start;
    }else if(strcmp(command,"add") == 0){
        return cs_add;
    }else if(strcmp(command,"withdraw") == 0){
        return cs_withdrawl;
    }else if(strcmp(command,"balance") == 0){
        return cs_balance;
    }else if(strcmp(command,"send") == 0){
        return cs_send;
    }else if(strcmp(command,"finish") == 0){
        return cs_finish;
    }else if(strcmp(command,"exit") == 0){
        return cs_exit;
    }else{
      return cs_notexit;
    }

    return cs_notexit;
}


void initAccounts(account ***arr_account, int **maxAccount){
	*maxAccount = malloc(sizeof(int));
    **maxAccount = 20;
    *arr_account = (account ** ) malloc(sizeof(account *) * (**maxAccount)); 
    int i;
    for(i = 0 ; i < (**maxAccount) ; i++){
        (*arr_account)[i] = (account *) malloc(sizeof(account));
        ((*arr_account)[i])->service_flag = -1;
        pthread_mutex_init(&(((*arr_account)[i])->lock), NULL);
    }
    return;
}

int searchAccount(account ** arr_account, char * name, int * maxAccount){
    int i;
    for(i = 0 ; i < (*maxAccount) ; i++){
        if(strcmp( (arr_account[i])->name ,name) == 0){
          return i;
        }
    }
    return -1;
}

void accOpen(int client_socket_fd, int *numAccount, account ***arr_account, char * name, int * session, pthread_mutex_t * openAuthLock, int **maxAccount){
    pthread_mutex_lock(openAuthLock);
    char obuffer[256];
    memset(obuffer, 0, sizeof(obuffer));
    if(strlen(name) <= 0){
      if ( write(client_socket_fd, obuffer, sprintf(obuffer, "ERROR: Must specify name of account\n") ) < 0 ){
          printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
          pthread_mutex_unlock(openAuthLock);
          exit(1);
      }
      pthread_mutex_unlock(openAuthLock);
      return;
    }
    if(*session >= 0){
        if ( write(client_socket_fd, obuffer, sprintf(obuffer, "ERROR: Already in session. Can't open an account: %s\n",name) ) < 0 ){
            printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
            pthread_mutex_unlock(openAuthLock);
            exit(1);
        }
        pthread_mutex_unlock(openAuthLock);
        return;
    }
    if(*numAccount >= (**maxAccount)){
        (**maxAccount) = (**maxAccount) + 20;
        *arr_account = (account ** ) realloc(*arr_account, sizeof(account *)*(**maxAccount));
        int i;
		for(i = (**maxAccount) - 20 ; i < (**maxAccount) ; i++){
			(*arr_account)[i] = (account *) malloc(sizeof(account));
			((*arr_account)[i])->service_flag = -1;
			pthread_mutex_init(&(((*arr_account)[i])->lock), NULL);
		}
    }
    if(searchAccount(*arr_account,name, (*maxAccount)) >= 0 ){
        if ( write(client_socket_fd, obuffer, sprintf(obuffer, "ERROR: Account exists already: %s\n",name) ) < 0 ){
            printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
            pthread_mutex_unlock(openAuthLock);
            exit(1);
        }
        pthread_mutex_unlock(openAuthLock);
        return;
    }
    account * client_account = (*arr_account)[*numAccount];

    strcpy(client_account->name, name);   
    client_account->balance = 0.0;       
    client_account->service_flag = 0;    

    if ( write(client_socket_fd, obuffer, sprintf(obuffer, "- - -Account has been created!: %s - - -\n",name) ) < 0 ){
        printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
        pthread_mutex_unlock(openAuthLock);
        exit(1);
    }

    *numAccount = *numAccount + 1;
    pthread_mutex_unlock(openAuthLock);
    return;

}

void accStart(int client_socket_fd,  account ***arr_account, char * name, int *session, int * maxAccount){
    char obuffer[256];
    memset(obuffer, 0, sizeof(obuffer));
    if(strlen(name) <= 0){
      if ( write(client_socket_fd, obuffer, sprintf(obuffer, "ERROR: Must specify name of account\n") ) < 0 ){
          printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
          exit(1);
      }
      return;
    }
    if(*session >= 0){
        if ( write(client_socket_fd, obuffer, sprintf(obuffer, "ERROR: Already in session. Can't open another seesion: %s\n",name) ) < 0 ){
            printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
            exit(1);
        }
        return;
    }

    if( (*session = searchAccount(*arr_account,name, maxAccount)) < 0 ){
        if ( write(client_socket_fd, obuffer, sprintf(obuffer, "ERROR: Account not found: %s\n",name) ) < 0 ){
            printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
            exit(1);
        }
        return;
    }

    account * client_account = (*arr_account)[*session];

    if( pthread_mutex_trylock(&(client_account->lock)) != 0 ){   //pthread_mutex_lock(&(client_account->lock));
        if ( write(client_socket_fd, obuffer, sprintf(obuffer, "- - -Please try again later. The account is currently being used: %s- - -\n",name) ) < 0 ){
            printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
            exit(1);
        }
        *session = -1;    
          return;
    }

    client_account->service_flag = 1;


    if ( write(client_socket_fd, obuffer, sprintf(obuffer, "- - -Your session has begun for account: %s - - -\n",name) ) < 0 ){
        printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
        exit(1);
    }

    return;

}

void accAdd(int client_socket_fd, account ***arr_account, char * amount, int *session){
    char obuffer[256];
    memset(obuffer, 0, sizeof(obuffer));

    if(strlen(amount) <= 0){
        if ( write(client_socket_fd, obuffer, sprintf(obuffer, "ERROR: Must specify amount to add.\n") ) < 0 ){
            printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
            exit(1);
        }
        return;
    }
    if(!isamount(amount)){
        if ( write(client_socket_fd, obuffer, sprintf(obuffer, "ERROR: You must specify a non-negative numerical amount to add.\n") ) < 0 ){
            printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
            exit(1);
        }
        return;
    }

    if(*session < 0){
        if ( write(client_socket_fd, obuffer, sprintf(obuffer, "ERROR: No session exists.\n") ) < 0 ){
            printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
            exit(1);
        }
        return;
    }

    float addval = atof(amount);

    account * client_account = (*arr_account)[*session];
    client_account->balance += addval;

    if ( write(client_socket_fd, obuffer, sprintf(obuffer, "- - -Money have been added to your account.- - -\n") ) < 0 ){
        printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
        exit(1);
    }

    return;

}

void accWithdrawl(int client_socket_fd, account ***arr_account, char * amount, int *session){
    char obuffer[256];
    memset(obuffer, 0, sizeof(obuffer));

    if(strlen(amount) <= 0){
        if ( write(client_socket_fd, obuffer, sprintf(obuffer, "ERROR: Must specify amount to withdraw.\n") ) < 0 ){
            printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
            exit(1);
        }
        return;
    }
    if(!isamount(amount)){
        if ( write(client_socket_fd, obuffer, sprintf(obuffer, "ERROR: You must specify a non-negative numerical amount to withdraw.\n") ) < 0 ){
            printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
            exit(1);
        }
        return;
    }

    if(*session < 0){
        if ( write(client_socket_fd, obuffer, sprintf(obuffer, "ERROR: No session exists.\n") ) < 0 ){
            printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
            exit(1);
        }
        return;
    }

    float withdrawlval = atof(amount);

    account * client_account = (*arr_account)[*session];
    if(withdrawlval > client_account->balance){
      if( write(client_socket_fd, obuffer, sprintf(obuffer, "ERROR: You cannot withdraw more than your balance.\n") ) < 0 ){
          printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
          exit(1);
      }
      return;
    }

    client_account->balance -= withdrawlval;

    if( write(client_socket_fd, obuffer, sprintf(obuffer, "- - -Money have been withdrawn.- - -\n") ) < 0 ){
        printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
        exit(1);
    }

    return;

}

void accBalance(int client_socket_fd,  account ***arr_account, int *session){
    char obuffer[256];
    memset(obuffer, 0, sizeof(obuffer));

    if(*session < 0){
        if ( write(client_socket_fd, obuffer, sprintf(obuffer, "ERROR: No session exists.\n") ) < 0 ){
            printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
            exit(1);
        }
        return;
    }

    float balance = (*arr_account)[*session]->balance;

    if ( write(client_socket_fd, obuffer, sprintf(obuffer, "- - -Your balance is: %f.- - -\n",balance) ) < 0 ){
        printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
        exit(1);
    }


    return;


}

void accSend(int client_socket_fd,  account ***arr_account, char * amount, int *session, char * dest, int * maxAccount){
    char obuffer[256];
    int receiver;
    memset(obuffer, 0, sizeof(obuffer));
    if(strlen(dest) <= 0){
      if ( write(client_socket_fd, obuffer, sprintf(obuffer, "ERROR: Must specify name of receiving account\n") ) < 0 ){
          printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
          exit(1);
      }
      return;
    }

    if( (receiver = searchAccount(*arr_account,dest, maxAccount)) < 0 ){
        if ( write(client_socket_fd, obuffer, sprintf(obuffer, "ERROR: Account not found: %s\n",dest) ) < 0 ){
            printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
            exit(1);
        }
        return;
    }

    account * client_account = (*arr_account)[*session];

    
    account * receiver_account = (*arr_account)[receiver];

    if(strlen(amount) <= 0){
        if ( write(client_socket_fd, obuffer, sprintf(obuffer, "ERROR: Must specify amount to send.\n") ) < 0 ){
            printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
            exit(1);
        }
        return;
    }
    if(!isamount(amount)){
        if ( write(client_socket_fd, obuffer, sprintf(obuffer, "ERROR: You must specify a non-negative numerical amount to send.\n") ) < 0 ){
            printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
            exit(1);
        }
        return;
    }

    if(*session < 0){
        if ( write(client_socket_fd, obuffer, sprintf(obuffer, "ERROR: No session exists.\n") ) < 0 ){
            printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
            exit(1);
        }
        return;
    }

    float withdrawlval = atof(amount);

    if(withdrawlval > client_account->balance){
      if( write(client_socket_fd, obuffer, sprintf(obuffer, "ERROR: You cannot send more than your balance.\n") ) < 0 ){
          printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
          exit(1);
      }
      return;
    }

    client_account->balance -= withdrawlval;
    receiver_account->balance += withdrawlval;

    if( write(client_socket_fd, obuffer, sprintf(obuffer, "- - -Money have been sent.- - -\n") ) < 0 ){
        printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
        exit(1);
    }

    return;

}


void accFinish(int client_socket_fd, account ***arr_account,  int *session){
    char obuffer[256];
    memset(obuffer, 0, sizeof(obuffer));
    if(*session < 0){
        if ( write(client_socket_fd, obuffer, sprintf(obuffer, "ERROR: No session exists.\n") ) < 0 ){
            printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
            exit(1);
        }
        return;
    }

    account * client_account = (*arr_account)[*session];
    char * name = client_account->name;

    if ( write(client_socket_fd, obuffer, sprintf(obuffer, "- - -You have finised your account session: %s- - -\n",name) ) < 0 ){
        printf("ERROR: WRITE FAILED: %s\n", strerror(errno));
        exit(1);
    }

    client_account->service_flag = 0;
    *session = -1;

    pthread_mutex_unlock(&(client_account->lock));

    return;

}
