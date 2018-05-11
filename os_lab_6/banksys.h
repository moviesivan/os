#ifndef BANKSYS_H
#define BANKSYS_H

#include <pthread.h>
#include <ctype.h>

    typedef enum command_state_ {
        cs_open,
        cs_start,
        cs_add,
        cs_withdrawl,
        cs_balance,
        cs_finish,
        cs_send,
        cs_exit,
        cs_notexit,
    } command_state;

    typedef struct account_ {
        char name[100];
        float balance;
        int service_flag;              
        pthread_mutex_t lock;
    } account;

    int isamount(char * amount);

    command_state determineCommand(char * command);

    void initAccounts(account ***arr_account, int **maxAccount);

    int searchAccount(account ** arr_account, char * name, int * maxAccount);

    void accOpen(int client_socket_fd, int *numAccount, account ***arr_account, char * name, int *session, pthread_mutex_t * openAuthLock, int **maxAccount);

    void accStart(int client_socket_fd,  account ***arr_account, char * name, int *session, int * maxAccount);

    void accAdd(int client_socket_fd, account ***arr_account, char * amount, int *session);

    void accWithdrawl(int client_socket_fd, account ***arr_account, char * amount, int *session);

    void accBalance(int client_socket_fd,  account ***arr_account, int *session);

    void accSend(int client_socket_fd,  account ***arr_account, char * amount, int *session, char * dest, int * maxAccount); 

    void accFinish(int client_socket_fd, account ***arr_account,  int *session);

    //void accExit(int client_socket_fd, int *session);
#endif
