#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "checkers.h"
int player_count = 0;
pthread_mutex_t mutexcount;

void error(const char *msg)
{
    perror(msg);
    pthread_exit(NULL);
    exit(0);
}


int recv_int(int cli_sockfd)
{
    int msg = 0;
    int n = read(cli_sockfd, &msg, sizeof(int));
    
    if (n < 0 || n != sizeof(int)) 
        return -1;
    
    return msg;
}


void write_client_msg(int cli_sockfd, char * msg)
{
    int n = send(cli_sockfd, msg, strlen(msg), MSG_NOSIGNAL);
}

void write_client_int(int cli_sockfd, int msg)
{
    int n = send(cli_sockfd, &msg, sizeof(int), MSG_NOSIGNAL );
}

void write_clients_msg(int * cli_sockfd, char * msg)
{
    write_client_msg(cli_sockfd[0], msg);
    write_client_msg(cli_sockfd[1], msg);
}

void write_clients_int(int * cli_sockfd, int msg)
{
    write_client_int(cli_sockfd[0], msg);
    write_client_int(cli_sockfd[1], msg);
}


int setup_listener(int portno)
{
    int sockfd;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening listener socket.");
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;	
    serv_addr.sin_addr.s_addr = INADDR_ANY;	
    serv_addr.sin_port = htons(portno);		

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR binding listener socket.");


    return sockfd;
}

void get_clients(int lis_sockfd, int * cli_sockfd)
{
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    

    int num_conn = 0;
    while(num_conn < 2)
    {
	    listen(lis_sockfd, 253 - player_count);
        memset(&cli_addr, 0, sizeof(cli_addr));
        clilen = sizeof(cli_addr);
        cli_sockfd[num_conn] = accept(lis_sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (cli_sockfd[num_conn] < 0)
            error("ERROR accepting a connection from a client.");
        write(cli_sockfd[num_conn], &num_conn, sizeof(int));
        
        
        pthread_mutex_lock(&mutexcount);
        player_count++;
        printf("Number of players is now %d.\n", player_count);
        pthread_mutex_unlock(&mutexcount);

        if (num_conn == 0) {
            write_client_msg(cli_sockfd[0],"HLD");
            
        }

        num_conn++;
    }
}

void get_player_move(int cli_sockfd, int move[4])
{
    write_client_msg(cli_sockfd, "TRN");

    move[0]=recv_int(cli_sockfd);
    move[1]=recv_int(cli_sockfd);
    move[2]=recv_int(cli_sockfd);
    move[3]=recv_int(cli_sockfd);
}


void send_update(int * cli_sockfd, int move[4], int player_id)
{   
    write_clients_msg(cli_sockfd, "UPD");
    write_clients_int(cli_sockfd, player_id);
    
    write_clients_int(cli_sockfd, move[0]);
    write_clients_int(cli_sockfd, move[1]);
    write_clients_int(cli_sockfd, move[2]);
    write_clients_int(cli_sockfd, move[3]);
}

void send_player_count(int cli_sockfd)
{
    write_client_msg(cli_sockfd, "CNT");
    write_client_int(cli_sockfd, player_count);
}

void *run_game(void *thread_data) 
{
    int *cli_sockfd = (int*)thread_data; 
	int the_board[BOARD_SIZE][BOARD_SIZE];
	init_board(the_board);
	
	int x_from = 0, y_from = 0, x_to = 0, y_to = 0, jump, move;
	int *px_from = &x_from, *py_from = &y_from, *px_to = &x_to, *py_to = &y_to;
	int x_cur, y_cur , multi_jump;
	int win=FALSE;

    printf("Game on!\n");
    write_clients_msg(cli_sockfd, "SRT");
    print_board(the_board);
    
    int prev_player_turn = WHITE;
    int player_turn = RED;
    int game_over = 0;
    int turn_count = 0;
    while(win==FALSE) {
        if (prev_player_turn != player_turn)
            write_client_msg(cli_sockfd[(player_turn + 1) % 2], "WAT");

        int valid = 0;
        int move[4];
		jump = jump_exists(the_board, player_turn);
		do {
            get_player_move(cli_sockfd[player_turn % 2], move);
            for (int i = 0; i < 4; i++)
				if (move[i] == -1) break;
			
			if (move[0] == -1 || move[1] == -1 || move[2] == -1 || move[3] == -1) {
				break;
			}
            printf("Player %d moved %d %d to %d %d\n", player_turn-2, move[0]+1, move[1]+1, move[2]+1, move[3]+1);
                
            valid = check_move(the_board, player_turn, move[0], move[1], move[2], move[3], jump);
            if(multi_jump == TRUE && (x_cur != move[0] || y_cur != move[1])){
				valid = INVALID_MOVE;
			}
            if (valid == INVALID_MOVE) {
                printf("Move was invalid. Let's try this again...\n");
                if (jump == TRUE) write_client_msg(cli_sockfd[player_turn % 2], "IN2");
                else write_client_msg(cli_sockfd[player_turn % 2], "IN1");
            }
        } while(valid == INVALID_MOVE);

	    if (move[0] == -1 || move[1] == -1 || move[2] == -1 || move[3] == -1) { 
            printf("Player disconnected.\n");
            break;
        }
        else {
            move_piece(the_board, player_turn, move[0], move[1], move[2], move[3], jump);
            send_update( cli_sockfd, move, player_turn );
                
			print_board(the_board);

            win = check_board(the_board, player_turn);
            
            if (win == WIN_RED || win == WIN_WHITE) { 
                write_client_msg(cli_sockfd[player_turn % 2], "WIN");
                write_client_msg(cli_sockfd[(player_turn + 1) % 2], "LSE");
                printf("Player %d won.\n", player_turn % 2);
            }
            else if (win == DRAW) { 
                printf("Draw.\n");
                write_clients_msg(cli_sockfd, "DRW");
            }

            if(jump == TRUE && is_jumper(the_board, move[2], move[3])){
				x_cur = move[2];
				y_cur = move[3];
				multi_jump = TRUE;
				prev_player_turn=player_turn;
				continue;
			}
			multi_jump = FALSE;
            prev_player_turn = player_turn;
            player_turn==RED?player_turn=WHITE:player_turn=RED;
            turn_count++;
        }
    }

    printf("Game over.\n");

    close(cli_sockfd[0]);
    close(cli_sockfd[1]);

    pthread_mutex_lock(&mutexcount);
    player_count--;
    printf("Number of players is now %d.", player_count);
    player_count--;
    printf("Number of players is now %d.", player_count);
    pthread_mutex_unlock(&mutexcount);
    
    free(cli_sockfd);

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{   
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    
    int lis_sockfd = setup_listener(atoi(argv[1])); 
    pthread_mutex_init(&mutexcount, NULL);

    while (1) {
        if (player_count <= 252) {
            int *cli_sockfd = (int*)malloc(2*sizeof(int)); 
            memset(cli_sockfd, 0, 2*sizeof(int));
            
            get_clients(lis_sockfd, cli_sockfd);

            pthread_t thread; 
            int result = pthread_create(&thread, NULL, run_game, (void *)cli_sockfd); 
            if (result){
                printf("Thread creation failed with return code %d\n", result);
                exit(-1);
            }
            
        }
    }

    close(lis_sockfd);

    pthread_mutex_destroy(&mutexcount);
    pthread_exit(NULL); 
}
