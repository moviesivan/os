#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define BOARD_SIZE 8
#define _NULL 0
#define EMPTY 1
#define RED 2
#define WHITE 3
#define RED_KING 4
#define WHITE_KING 5
#define TRUE 1
#define FALSE 0

void error(const char *msg)
{
    #ifdef DEBUG
    perror(msg);
    #else
    printf("Either the server shut down or the other player disconnected.\nGame over.\n");
    #endif 

    exit(0);
}

void recv_msg(int sockfd, char * msg)
{
    memset(msg, 0, 4);
    int n = read(sockfd, msg, 3);
    
    if (n < 0 || n != 3)  
        error("ERROR reading message from server socket.");

}

int recv_int(int sockfd)
{
    int msg = 0;
    int n = read(sockfd, &msg, sizeof(int));
    
    if (n < 0 || n != sizeof(int)) 
        error("ERROR reading int from server socket");
    
    return msg;
}

void write_server_int(int sockfd, int msg)
{
    int n = write(sockfd, &msg, sizeof(int));
    if (n < 0)
        error("ERROR writing int to server socket");
}

int connect_to_server(char * hostname, int portno)
{
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
    if (sockfd < 0) 
        error("ERROR opening socket for server.");
	
    server = gethostbyname(hostname);
	
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
	
	memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    memmove(server->h_addr, &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno); 

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting to server");

    
    return sockfd;
}

 void print_board(int the_board[BOARD_SIZE][BOARD_SIZE]){
	int i , j ,count, posx, posy;
	count = 0;
	i = 0;
	j = 0;
	printf(" ");
	for(i = 0; i < BOARD_SIZE; i++){
		printf("  %d " , count+1);
		count++;
	}
	printf("\n");
	count = 0; 
  
	for(i = 0; i < BOARD_SIZE*2 + 1; i++){ 
    
    if((i + 1) % 2 == 1){
		printf(" |");
		for(j = 0; j < BOARD_SIZE; j++){
			printf("---|");
		}
		printf("\n");
    }
    
    else{
		printf("%i|" , count+1);
		count++;
		for(j = 0; j < BOARD_SIZE * 2 ; j++){
			posx = j / 2;
			posy = i / 2;
	  
			if ((j  + 1) % 2 == 1){
				if(the_board[posx][posy] == _NULL) printf("   ");
				else if(the_board[posx][posy] == EMPTY) printf("   ");  
				else if(the_board[posx][posy] == RED) printf(" r ");
				else if(the_board[posx][posy] == WHITE) printf(" w ");
				else if(the_board[posx][posy] == RED_KING) printf(" R ");
				else if(the_board[posx][posy] == WHITE_KING) printf(" W ");
			}
			else printf("|");
		}
		printf("\n");
	}   
	} 
	printf("\n");
 }

void take_turn(int sockfd)
{
    
	int x_from = 0, y_from = 0, x_to = 0, y_to = 0;
    
    while (1) {  
        printf("Enter your move in format \"y_from x_from y_to x_to\": ");
		scanf("%d %d %d %d" , &y_from, &x_from, &y_to, &x_to); 
		y_from-=1;
		x_from-=1;
		y_to-=1;
  		x_to-=1;
        if (y_from <= 7 && y_from >= 0 && x_from <= 7 && x_from >= 0 && 
			y_to <= 7 && y_to >= 0 && x_to <= 7 && x_to >= 0){
            printf("\n");
            write_server_int(sockfd, x_from);   
            write_server_int(sockfd, y_from); 
            write_server_int(sockfd, x_to); 
            write_server_int(sockfd, y_to); 
            break;
        } 
        else
            printf("\nInvalid input. Try again.\n");
    }
}

void move_piece(int the_board[BOARD_SIZE][BOARD_SIZE], int color,int x_from, int y_from, int x_to, int y_to, int jump){
  int piece = the_board[x_from][y_from]; 
  the_board[x_from][y_from] = EMPTY;
  the_board[x_to][y_to] = piece;
  
  if(jump == TRUE){
    the_board[x_from + ((x_to - x_from) /2)][y_from + ((y_to - y_from) / 2)] = EMPTY;
  }
  
  if((piece == WHITE) && (y_to == 0)){
    the_board[x_to][y_to] = WHITE_KING;
  }else if((piece == RED) && (y_to == 7)){
    the_board[x_to][y_to] = RED_KING;
  }
}

void get_update(int sockfd, int the_board[BOARD_SIZE][BOARD_SIZE])
{
    int player_id = recv_int(sockfd);
    int x_from = recv_int(sockfd);
    int y_from = recv_int(sockfd);
    int x_to = recv_int(sockfd);
    int y_to = recv_int(sockfd);
	int jump = abs(y_to - y_from) == 2 ? TRUE : FALSE; 
    move_piece(the_board, player_id+2, x_from, y_from, x_to, y_to,jump);    
}

void init_board(int the_board[BOARD_SIZE][BOARD_SIZE]){
  int i;
  int j;
  int count = 0;
  
  for(i = 0; i < 8; i++){
    for(j = 0; j < 3; j++){
      if (count % 2 == 0){
		the_board[i][j] = _NULL;
      }else the_board[i][j] = RED; 
      count++;
    }

    for(j = 3; j < 5; j++){
      if (count % 2 == 0){
		the_board[i][j] = _NULL;
      }else the_board[i][j] = EMPTY; 
      count++;
    }

    for(j = 5; j < 8; j++){
      if (count % 2 == 0){
	the_board[i][j] = _NULL;
      }else the_board[i][j] = WHITE;      
      count++;
    }
    count++;
  }
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    int sockfd = connect_to_server(argv[1], atoi(argv[2]));

    int id = recv_int(sockfd);
    
    char msg[4];
    int the_board[BOARD_SIZE][BOARD_SIZE];
	init_board(the_board);

    printf("Checkers\n------------\n");

    do {
        recv_msg(sockfd, msg);
        if (!strcmp(msg, "HLD"))
            printf("Waiting for a second player...\n");
    } while ( strcmp(msg, "SRT") );

    printf("Game on!\n");
    printf("Your are %s\n", id ? "White" : "Red");

    print_board(the_board);

    while(1) {
        recv_msg(sockfd, msg);

        if (!strcmp(msg, "TRN")) { 
	        printf("Your move...\n");
	        take_turn(sockfd);
        }
        else if (!strcmp(msg, "IN1")) { 
            printf("Invalid move. Try again.\n"); 
        }
        else if (!strcmp(msg, "IN2")) {
            printf("Invalid move. Jump is required. Try again.\n"); 
        }
        else if (!strcmp(msg, "CNT")) {
            int num_players = recv_int(sockfd);
            printf("There are currently %d active players.\n", num_players); 
        }
        else if (!strcmp(msg, "UPD")) { 
            get_update(sockfd, the_board);
            print_board(the_board);
        }
        else if (!strcmp(msg, "WAT")) { 
            printf("Waiting for other players move...\n");
        }
        else if (!strcmp(msg, "WIN")) { 
            printf("You win!\n");
            break;
        }
        else if (!strcmp(msg, "LSE")) { 
            printf("You lost.\n");
            break;
        }
        else if (!strcmp(msg, "DRW")) { 
            printf("Draw.\n");
            break;
        }
        else 
            error("Unknown message.");
    }
    
    printf("Game over.\n");

    close(sockfd);
    return 0;
}
