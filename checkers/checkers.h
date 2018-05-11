//checkers.h
#include <stdio.h>
#include <stdlib.h>

#define BOARD_SIZE 8
#define RED_PLAYER 0
#define WHITE_PLAYER 1
#define INVALID_MOVE 1
#define WIN_RED 1
#define WIN_WHITE 2
#define DRAW 3
#define VALID_MOVE 2
#define _NULL 0
#define EMPTY 1
#define RED 2
#define WHITE 3
#define RED_KING 4
#define WHITE_KING 5
#define TRUE 1
#define FALSE 0

void print_board(int the_board[BOARD_SIZE][BOARD_SIZE]);
int jump_exists(int the_board[BOARD_SIZE][BOARD_SIZE], int color);
int step_exists(int the_board[BOARD_SIZE][BOARD_SIZE], int color);
int check_move(int the_board[BOARD_SIZE][BOARD_SIZE], int color, int x_from, int y_from, int x_to, int y_to, int jump);
void move_piece(int the_board[BOARD_SIZE][BOARD_SIZE], int color,int x_from, int y_from, int x_to, int y_to, int jump);
int check_step(int the_board[BOARD_SIZE][BOARD_SIZE], int x_from, int y_from, int x_to, int y_to, int dir);
int check_jump(int the_board[BOARD_SIZE][BOARD_SIZE], int x_from, int y_from, int x_to, int y_to, int dir);
int is_jumper(int the_board[BOARD_SIZE][BOARD_SIZE], int x, int y);
int is_stepper(int the_board[BOARD_SIZE][BOARD_SIZE], int x, int y);

int get_move(int the_board[BOARD_SIZE][BOARD_SIZE], int player, int *x_from, int *y_from, int *x_to, int *y_to);
void init_board(int the_board[BOARD_SIZE][BOARD_SIZE]);
int check_board(int the_board[BOARD_SIZE][BOARD_SIZE], int color);

  int get_move(int the_board[BOARD_SIZE][BOARD_SIZE], int player, int *px_from, int *py_from, int *px_to, int *py_to){
  if(player == WHITE){ 
    printf("WHITE's move:  ");
    scanf("%d %d %d %d" , py_from, px_from, py_to, px_to);
  }else{ 
    printf("RED's move:  ");
    scanf("%d %d %d %d" , py_from, px_from, py_to, px_to);
  } 
  *py_from-=1;
  *px_from-=1;
  *py_to-=1;
  *px_to-=1;
  return 0;
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

int jump_exists(int the_board[BOARD_SIZE][BOARD_SIZE], int color){
  int i , j;
  for(i = 0; i < BOARD_SIZE; i++){
    for(j = 0; j < BOARD_SIZE; j++){
	if(color == WHITE){
	  if((the_board[i][j] == WHITE || the_board[i][j] == WHITE_KING) && is_jumper(the_board, i , j)){
	    return TRUE;
	  }
	}else if(color == RED){ 	 
	  if((the_board[i][j] == RED || the_board[i][j] == RED_KING) && is_jumper(the_board, i , j)){
	    return TRUE;
	  }
	}
    }
    }
    
    return FALSE;
}

int step_exists(int the_board[BOARD_SIZE][BOARD_SIZE], int color){
  int i , j;
  for(i = 0; i < BOARD_SIZE; i++){
    for(j = 0; j < BOARD_SIZE; j++){
	if(color == WHITE){
	  if((the_board[i][j] == WHITE || the_board[i][j] == WHITE_KING) && is_stepper(the_board, i , j)){
	    return TRUE;
	  }
	}else if(color == RED){ 	 
	  if((the_board[i][j] == RED || the_board[i][j] == RED_KING) && is_stepper(the_board, i , j)){
	    return TRUE;
	  }
	}
    }
    }
    
    return FALSE;
}

int check_move(int the_board[BOARD_SIZE][BOARD_SIZE], int color, int x_from, int y_from, int x_to, int y_to, int jump){

  if(x_from < 0 || x_from > 7 || y_from < 0 || y_from > 7) return INVALID_MOVE;
  if(x_to < 0 || x_to > 7 || y_to < 0 || y_to > 7) return INVALID_MOVE;

  int check_piece = the_board[x_from] [y_from];
  if(check_piece == EMPTY || check_piece == _NULL) return INVALID_MOVE;
  if(color == WHITE && !(check_piece == WHITE || check_piece == WHITE_KING))return INVALID_MOVE;
  if(color == RED && !(check_piece == RED || check_piece == RED_KING))return INVALID_MOVE;
  
  if(jump == TRUE){
    if(color == WHITE && (check_jump(the_board, x_from, y_from, x_to, y_to, -1) == VALID_MOVE))return VALID_MOVE;
    if(check_piece == WHITE_KING &&  (check_jump(the_board, x_from, y_from, x_to, y_to, 1) == VALID_MOVE)) return VALID_MOVE;
    
    if(color == RED && (check_jump(the_board, x_from, y_from, x_to, y_to, 1) == VALID_MOVE))return VALID_MOVE;
    if(check_piece == RED_KING &&  (check_jump(the_board, x_from, y_from, x_to, y_to, -1) == VALID_MOVE))return VALID_MOVE;
    
  }else{
    if(color == RED){
      if(check_step(the_board, x_from, y_from, x_to, y_to, -1) == VALID_MOVE)return VALID_MOVE;
      if(check_piece == RED_KING && (check_step(the_board, x_from, y_from, x_to, y_to, 1) == VALID_MOVE)) return TRUE;
    }else if(color == WHITE){
      if(check_step(the_board, x_from, y_from, x_to, y_to, 1) == VALID_MOVE)return VALID_MOVE;
      if(check_piece == WHITE_KING &&  (check_step(the_board, x_from, y_from, x_to, y_to, -1) == VALID_MOVE))return VALID_MOVE;
    }
  }
  return INVALID_MOVE;
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

int check_step(int the_board[BOARD_SIZE][BOARD_SIZE], int x_from, int y_from, int x_to, int y_to, int dir){
  if(x_from < 0 || x_from > 7 || y_from < 0 || y_from > 7)return INVALID_MOVE;
  else if(x_to < 0 || x_to > 7 || y_to < 0 || y_to > 7) return INVALID_MOVE;
  if(abs(x_from - x_to) != 1 || (y_from - y_to) != dir){
    return INVALID_MOVE;
  }
  
  if(the_board[x_to][y_to] == EMPTY){
    return VALID_MOVE;
  }
  return INVALID_MOVE;
}

  
int check_jump(int the_board[BOARD_SIZE][BOARD_SIZE], int x_from, int y_from, int x_to, int y_to, int dir){
  if(x_from < 0 || x_from > 7 || y_from < 0 || y_from > 7)return INVALID_MOVE;
  else if(x_to < 0 || x_to > 7 || y_to < 0 || y_to > 7) return INVALID_MOVE;
  if(abs(y_to - y_from) != 2) return INVALID_MOVE; 
  
  int check_piece = the_board[x_from][y_from];
  int x_dir = (x_to - x_from)/2;
  
  if(!(x_dir == 1 || x_dir == -1)) return INVALID_MOVE;

  if(check_piece == WHITE || check_piece == WHITE_KING){
    if(the_board[x_from + x_dir][y_from + dir] == RED || the_board[x_from + x_dir][y_from + dir] == RED_KING){
      if(the_board[x_from + 2*x_dir][y_from + 2*dir] == EMPTY) return VALID_MOVE;
    }
  }else if(check_piece == RED || check_piece == RED_KING){
    if(the_board[x_from + x_dir][y_from + dir] == WHITE || the_board[x_from + x_dir][y_from + dir] == WHITE_KING){
      if(the_board[x_from + 2*x_dir][y_from + 2*dir] == EMPTY) return VALID_MOVE;
    }
  }
  return INVALID_MOVE;
}

int is_jumper(int the_board[BOARD_SIZE][BOARD_SIZE], int x, int y){
  int check_piece = the_board[x][y];
  if(check_piece == EMPTY){
    return FALSE;
  }
  else if(check_piece == WHITE || check_piece == WHITE_KING){
    if(check_jump(the_board, x , y , x+2, y-2, -1) == VALID_MOVE) return TRUE;
    else if(check_jump(the_board, x , y , x-2, y-2, -1) == VALID_MOVE) return TRUE;
    
    if(check_piece == WHITE_KING){
      if(check_jump(the_board, x , y , x+2, y+2, 1) == VALID_MOVE) return TRUE;
      else if(check_jump(the_board, x , y , x-2, y+2, 1) == VALID_MOVE) return TRUE;
    } 
  }
  else if(check_piece == RED || check_piece == RED_KING){
    if(check_jump(the_board, x , y , x+2, y+2, 1) == VALID_MOVE) return TRUE;
    else if(check_jump(the_board, x , y , x-2, y+2, 1) == VALID_MOVE)return TRUE;
    
    if(check_piece == RED_KING){
      if(check_jump(the_board, x , y , x+2, y-2, -1) == VALID_MOVE) return TRUE;
      else if(check_jump(the_board, x , y , x-2, y-2, -1) == VALID_MOVE) return TRUE;
    } 
  }
  return FALSE;
}

int is_stepper(int the_board[BOARD_SIZE][BOARD_SIZE], int x, int y){
  int check_piece = the_board[x][y];
  if(check_piece == EMPTY){
    return FALSE;
  }
  else if(check_piece == WHITE || check_piece == WHITE_KING){
    if(check_step(the_board, x , y , x+1, y-1, 1) == VALID_MOVE) return TRUE;
    else if(check_step(the_board, x , y , x-1, y-1, 1) == VALID_MOVE) return TRUE;
    
    if(check_piece == WHITE_KING){
      if(check_step(the_board, x , y , x+1, y+1, -1) == VALID_MOVE) return TRUE;
      else if(check_step(the_board, x , y , x-1, y+1, -1) == VALID_MOVE) return TRUE;
    } 
  }
  else if(check_piece == RED || check_piece == RED_KING){
    if(check_step(the_board, x , y , x+1, y+1, -1) == VALID_MOVE) return TRUE;
    else if(check_step(the_board, x , y , x-1, y+1, -1) == VALID_MOVE)return TRUE;
    
    if(check_piece == RED_KING){
      if(check_step(the_board, x , y , x+1, y-1, 1) == VALID_MOVE) return TRUE;
      else if(check_step(the_board, x , y , x-1, y-1, 1) == VALID_MOVE) return TRUE;
    } 
  }
  return FALSE;
}

int check_board(int the_board[BOARD_SIZE][BOARD_SIZE], int color){
  int i , j;
  int counter_r = 0;
  int counter_w = 0;
  for(i = 0; i < BOARD_SIZE; i++){
    for(j = 0; j < BOARD_SIZE; j++){
	 if((the_board[i][j] == WHITE || the_board[i][j] == WHITE_KING)){
	    ++counter_w;
	  }else if((the_board[i][j] == RED || the_board[i][j] == RED_KING)){
	    ++counter_r;
	  }
	}
    }
    if (counter_w == 0){
		return WIN_RED;
	} else if (counter_r == 0){
		return WIN_WHITE;
	}
	if (jump_exists(the_board, color) || step_exists(the_board, color)) {
		return FALSE;
	}
    return DRAW; 
}
