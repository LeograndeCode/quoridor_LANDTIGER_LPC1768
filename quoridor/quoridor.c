#include "quoridor.h"
#include "../RIT/RIT.h"
#include "../timer/timer.h"
#include "../button_EXINT/button.h"
#include "../GLCD/GLCD.h"

enum {FREE_SQUARE,PLAYER1,PLAYER2,SPACE,WALL};
enum {UP, DOWN, LEFT, RIGHT, DOWN_RIGHT};
enum{SINGLE_PLAYER,MULTIPLAYER,HUMAN,NPC};

volatile int matrix[MATRIX_DIM][MATRIX_DIM];
volatile int p1_i=0,p1_j=6,p2_i=12,p2_j=6;
volatile int wallsP1 = 8,wallsP2 = 8;
volatile uint32_t moves[100];
volatile int movesIndex = 0;
volatile unsigned int globalSeed = 123;
volatile int randomNumber;

extern int tmpX,tmpY; // adjust
extern int playerX,playerY;
extern int turnPlayer;
extern int wI,wJ,tmpWI,tmpWJ;
extern int mode;
extern int rotatedWallMode;
extern int jump;
extern int dir_jump;
extern int stop_timer_flag;
extern int menu_mode;
extern int menu_choice;
extern int menu_choice2;


extern void LCD_ColorSquare(uint16_t x0,	uint16_t y0,	uint16_t color,	int dim);

int checkWallMove(int i, int j, int dir, int player){
	// Check boarders
	if(i < 0 || i > 6 || j < 0 || j > 6 ) return 0;
	i = (i*2)+1; j = (j*2)+1; // conversion to get central point of the wall in the matrix
	switch(dir){
		case UP:
			if( (i-2) == 0) return 0;
			if(matrix[i-2][j] == WALL) return 0;
			break;
		case DOWN:
			if( (i+2) >=12 ) return 0;
			if(matrix[i+2][j] == WALL) return 0;
			break;
		case LEFT:
			if( (j-2) <= 0) return 0;
			if(matrix[i][j-2] == WALL) return 0;
			break;
		case RIGHT:
			if( (j+2) >= 12) return 0;
			if(matrix[i][j+2] == WALL) return 0;
			break;
	}
	return 1;
}
// 0 6
void conv(int i, int j){
	int shiftI = (i == 0)? 0 : (i/2), shiftJ =(j == 0) ? 0 : (j/2);		// Since square i in matrix 13x13 is square i/2 + 1 in matrix 7 x 7
	j= 1 + (shiftJ * SQUARE_DISTANCE);
	i= 3 + (shiftI * SQUARE_DISTANCE);
	tmpX = i;
	tmpY = j;
}
void init_matrix(){
	int i,j;
	for(i = 0; i < 13; i++){
		for(j = 0; j < 13; j++){
			if ( i % 2 == 0){
				// odd line	-> square for odd column, space for even column
				if ( j % 2 == 0){
					matrix[i][j] = FREE_SQUARE;
				}else{
					matrix[i][j] = SPACE;
				}
			}else{
				matrix[i][j] = SPACE;
			}
		}
	}
	//	Place Player initial position
	matrix[0][6] = PLAYER1;
	matrix[12][6] = PLAYER2;
	p1_i = 0;
	p1_j = 6;
	p2_i = 12;
	p2_j = 6; 
}
int checkWallInFrontOfPlayer(int i, int j, int dir){
	switch(dir){
		case UP:
			//	UP 
			if ((i-1) >= 0)	
				if (matrix[i-1][j] == WALL) {return 1;}
			break;
		case DOWN:
			// DOWN
			if ((i+1) <= 12)
				if (matrix[i+1][j] == WALL) {return 1; }
			break;
		case LEFT:
			// LEFT
			if ((j-1) >= 0)
				if (matrix[i][j-1] == WALL) {return 1; }
			break;
		case RIGHT:
			// RIGHT
			if ((j+1) <= 12)
				if (matrix[i][j+1] == WALL) {return 1;}
			break;
	}
	return 0;
}

int checkPlayerMove(int i, int j, int dir, int player){
	if(i < 0 || i > 12 || j < 0 || j > 12 ) return 0;
	switch(dir){
		case UP:
			// out of bounds
			if ((i-2) < 0)	return 0;
			if (matrix[i-1][j] == WALL) return 0;
			if (matrix[i-2][j] == PLAYER1 || matrix[i-2][j] == PLAYER2) return 0;
			break;
		case DOWN:
			// out of bounds
			if ((i+2) > 12)	return 0;
			if (matrix[i+1][j] == WALL) return 0;
			if (matrix[i+2][j] == PLAYER1 || matrix[i+2][j] == PLAYER2) return 0;
			break;
		case LEFT:
            // out of bounds
			if ((j-2) < 0)	return 0;
			if (matrix[i][j-1] == WALL) return 0;
			if (matrix[i][j-2] == PLAYER1 || matrix[i][j-2] == PLAYER2) return 0;
			break;
		case RIGHT:
            // out of bounds
			if ((j+2) > 12)	return 0;
			if (matrix[i][j+1] == WALL) return 0;
			if (matrix[i][j+2] == PLAYER1 || matrix[i][j+2] == PLAYER2) return 0;
			break;

	}
	return 1;
}
int checkTrap(int player){
	int i,j,r,c;
  int matrixVisited[13][13], result = 0, tmpMatrix[13][13], tmpP1_i = p1_i,tmpP1_j = p1_j,tmpP2_i = p2_i,tmpP2_j = p2_j;
	int tmpJump = jump, tmpDirJump = dir_jump, tmpWallp1 = wallsP1, tmpWallp2 = wallsP2;
	
	i = (player == 1) ? p1_i : p2_i;
	j = (player == 1) ? p1_j : p2_j;
  for (r = 0; r < 13; r++){
      for (c = 0; c < 13; c++){
          matrixVisited[r][c] = 0;
					tmpMatrix[r][c] = matrix[r][c];
      }
  }
  matrixVisited[i][j] = 1;
  result = dfs(i,j,player, matrixVisited);
	// Restore everything
	for (r = 0; r < 13; r++){
      for (c = 0; c < 13; c++){
					matrix[r][c] = tmpMatrix[r][c];
      }
  }
	p1_i = tmpP1_i; p1_j = tmpP1_j; 
	p2_i = tmpP2_i; p2_j = tmpP2_j;
	jump = tmpJump, dir_jump = tmpDirJump;
  return result;
}
static int dfs (int i, int j, int player, int matrixVisited[MATRIX_DIM][MATRIX_DIM]){
	// dfs has to stop when it reaches the last line 12 (if player 1) or first line
    // 0 if player 2
    int term=-1,res=0;
    term = (player == 1) ? 12 : 0;
    if ( i == term){
        return 1;
    }
		
   if ((checkPlayerMove(i,j,UP,player) || dir_jump == UP)  && matrixVisited[i-2][j] == 0 ){
				if(jump == 1 && dir_jump == UP && checkPlayerMove(i-4,j,UP,player)) {
					i-=4;
					makePlayerMove(i,j,player);
					matrixVisited[i][j] = 1;
					if( (res = dfs(i,j,player,matrixVisited)) == 1 ) return 1;
					//  Backtrack
					matrixVisited[i][j] = 0;
					i+=4;
					makePlayerMove(i,j,player);
				}
				// This condition allow us to avoid the case in which the jump is possible but not legal
				if(dir_jump != UP){
					i-=2;
					makePlayerMove(i,j,player);
					matrixVisited[i][j] = 1;
					if( (res = dfs(i,j,player,matrixVisited)) == 1 ) return 1;
					//  Backtrack
					matrixVisited[i][j] = 0;
					i+=2;
					makePlayerMove(i,j,player);
				}
    }
		
    if ((checkPlayerMove(i,j,DOWN,player) || dir_jump == DOWN)  && matrixVisited[i+2][j] == 0 ){
				if(jump == 1 && dir_jump == DOWN && checkPlayerMove(i+4,j,DOWN,player)) {
					i+=4;
					makePlayerMove(i,j,player);
					matrixVisited[i][j] = 1;
					if( (res = dfs(i,j,player,matrixVisited)) == 1 ) return 1;
					//  Backtrack
					matrixVisited[i][j] = 0;
					i-=4;
					makePlayerMove(i,j,player);
				}
				// This condition allow us to avoid the case in which the jump is possible but not legal
				if(dir_jump != DOWN){
					i+=2;
					makePlayerMove(i,j,player);
					matrixVisited[i][j] = 1;
					if( (res = dfs(i,j,player,matrixVisited)) == 1 ) return 1;
					//  Backtrack
					matrixVisited[i][j] = 0;
					i-=2;
					makePlayerMove(i,j,player);
				}
    }

		
		if ((checkPlayerMove(i,j,LEFT,player) || dir_jump == LEFT)  && matrixVisited[i][j-2] == 0 ){
				if(jump == 1 && dir_jump == LEFT && checkPlayerMove(i,j-4,LEFT,player)) {
					j-=4;
					makePlayerMove(i,j,player);
					matrixVisited[i][j] = 1;
					if( (res = dfs(i,j,player,matrixVisited)) == 1 ) return 1;
					//  Backtrack
					matrixVisited[i][j] = 0;
					j+=4;
					makePlayerMove(i,j,player);
				}
				// This condition allow us to avoid the case in which the jump is possible but not legal
				if(dir_jump != LEFT){
					j-=2;
					makePlayerMove(i,j,player);
					matrixVisited[i][j] = 1;
					if( (res = dfs(i,j,player,matrixVisited)) == 1 ) return 1;
					//  Backtrack
					matrixVisited[i][j] = 0;
					j+=2;
					makePlayerMove(i,j,player);
				}
    }
		
    if ((checkPlayerMove(i,j,RIGHT,player) || dir_jump == RIGHT)  && matrixVisited[i][j+2] == 0 ){
				if(jump == 1 && dir_jump == RIGHT && checkPlayerMove(i+4,j,RIGHT,player)) {
					j+=4;
					makePlayerMove(i,j,player);
					matrixVisited[i][j] = 1;
					if( (res = dfs(i,j,player,matrixVisited)) == 1 ) return 1;
					//  Backtrack
					matrixVisited[i][j] = 0;
					j-=4;
					makePlayerMove(i,j,player);
				}
				// This condition allow us to avoid the case in which the jump is possible but not legal
				if(dir_jump != RIGHT){
					j+=2;
					makePlayerMove(i,j,player);
					matrixVisited[i][j] = 1;
					if( (res = dfs(i,j,player,matrixVisited)) == 1 ) return 1;
					//  Backtrack
					matrixVisited[i][j] = 0;
					j-=2;
					makePlayerMove(i,j,player);
				}
    }

    return 0;
}
/* 	
	Return 1 if the move is a winning move 
	Should check if after the move is done the next player has a jump position or not
*/
void makePlayerMove(int i, int j, int player){
	if(player == PLAYER1) matrix[p1_i][p1_j] = FREE_SQUARE;
	if(player == PLAYER2) matrix[p2_i][p2_j] = FREE_SQUARE;
	
	matrix[i][j] = player;
	// the jump flag will be resetted if we are no longer in a jump mode
	jump = 1; dir_jump = -1;
	// Check if after this move the next player is in a jump position
	//	UP 
	if ((i-2) >= 0)	
		if (matrix[i-2][j] == PLAYER1 || matrix[i-2][j] == PLAYER2) {jump = 1; dir_jump = DOWN;}
	// DOWN
	if ((i+2) <= 12)
		if (matrix[i+2][j] == PLAYER1 || matrix[i+2][j] == PLAYER2) {jump = 1; dir_jump = UP; }
	// LEFT
	if ((j-2) >= 0)
		if (matrix[i][j-2] == PLAYER1 || matrix[i][j-2] == PLAYER2) {jump = 1; dir_jump = RIGHT; }
	// RIGHT
	if ((j+2) <= 12)
		if (matrix[i][j+2] == PLAYER1 || matrix[i][j+2] == PLAYER2) {jump = 1; dir_jump = LEFT;}
	// Update pi and pj
	if(player == PLAYER1) {p1_i = i; p1_j = j;}
	if(player == PLAYER2) {p2_i = i; p2_j = j;}

}  
// 102 1
void HighLight(int x0,int y0,uint16_t color, int player){
	int d = SQUARE_DISTANCE;
	int j = (x0 == 3)? 0 :((x0/33))*2, i = (y0 == 1)? 0 :(((y0/33))*2);
		//	DOWN
		if(checkPlayerMove(i,j,DOWN,player) && !checkPlayerPresence(i,j,DOWN)) LCD_ColorSquare(x0,y0+d,color,30);
		//	UP
		if(checkPlayerMove(i,j,UP,player) && !checkPlayerPresence(i,j,UP)) LCD_ColorSquare(x0, y0-d,color,30);
		//	DX
		if(checkPlayerMove(i,j,RIGHT,player) && !checkPlayerPresence(i,j,RIGHT)) LCD_ColorSquare(x0+d,y0,color,30);
		//	SX
		if(checkPlayerMove(i,j,LEFT,player) && !checkPlayerPresence(i,j,LEFT) ) LCD_ColorSquare(x0-d,y0,color,30);
	
	// if jump is set we need to check also for another 3 possible squares
	// We don't jump if there's a wall between the player and the opponent in the direction of the jump
	if(jump == 1 && !checkWallInFrontOfPlayer(i,j,dir_jump)) {
		switch(dir_jump){
			case UP:
				if(checkPlayerMove(i-2,j,UP,player)) LCD_ColorSquare(x0, y0-(2*d),color,30);	// UP 
				if(matrix[i-3][j] == WALL){
					if(checkPlayerMove(i-2,j,RIGHT,player)) LCD_ColorSquare(x0+d, y0-d,color,30); //UP RIGHT
					if(checkPlayerMove(i-2,j,LEFT,player)) LCD_ColorSquare(x0-d, y0-d,color,30);	//UP LEFT
				}
				break;
			case DOWN:
				if(checkPlayerMove(i+2,j,DOWN,player)) LCD_ColorSquare(x0,y0+(2*d),color,30);	//	DOWN
				if(matrix[i+3][j] == WALL){
					if(checkPlayerMove(i+2,j,RIGHT,player)) LCD_ColorSquare(x0+d, y0+d,color,30); //DOWN RIGHT
					if(checkPlayerMove(i+2,j,LEFT,player)) LCD_ColorSquare(x0-d, y0+d,color,30);	//DOWN LEFT
				}
				break;
			case LEFT:
				if(checkPlayerMove(i,j-2,LEFT,player)) LCD_ColorSquare(x0-(2*d),y0,color,30);	//	LEFT
				if(matrix[i][j-3] == WALL){
					if(checkPlayerMove(i,j-2,UP,player)) LCD_ColorSquare(x0-d, y0-d,color,30); //UP LEFT
					if(checkPlayerMove(i,j-2,DOWN,player)) LCD_ColorSquare(x0-d, y0+d,color,30);	//DOWN LEFT
				}
				break;
			case RIGHT:
				if(checkPlayerMove(i,j+2,RIGHT,player)) LCD_ColorSquare(x0+(2*d),y0,color,30);	//	LEFT
				if(matrix[i][j+3] == WALL){
					if(checkPlayerMove(i,j+2,UP,player)) LCD_ColorSquare(x0+d, y0-d,color,30); //UP LEFT
					if(checkPlayerMove(i,j+2,DOWN,player)) LCD_ColorSquare(x0+d, y0+d,color,30);	//DOWN LEFT
				}
				break;
		}
			
	}
}
static void drawBoxes(uint16_t color){
	int x1,x2,x3,x4,y1,y2,y3,y4,i;
	char str[10];
	//	Cancel boxes
	x1 = 5; y1 = 250; x2 = 75;	y2 = 250;	y3 = 300; x3 = 75; x4= 5; y4 = 300;
	for (i = 0; i < 3; i++){
			LCD_DrawLine(x1, y1, x2, y2, color);
			LCD_DrawLine(x2, y2, x3, y3, color);
			LCD_DrawLine(x3, y3, x4, y4, color);
			LCD_DrawLine(x4, y4, x1, y1, color);
			x1+=BOXES_DISTANCE;
			x2+=BOXES_DISTANCE;
		  x3+=BOXES_DISTANCE;
		  x4+=BOXES_DISTANCE;
	}
	sprintf(str,"%d",wallsP1);
	GUI_Text(13, 253, (uint8_t *) "P1 Wall", color, Yellow);	
	GUI_Text(37, 275, (uint8_t *) str, color, Yellow);	
	sprintf(str,"%d",wallsP2);
	GUI_Text(163, 253, (uint8_t *) "P2 Wall", color, Yellow);	
	GUI_Text(185, 275, (uint8_t *) str, color, Yellow);	
}
void win(int player){
	char str[50];
	stop_timer_flag = 1;
	NVIC_DisableIRQ(TIMER0_IRQn);
	NVIC_DisableIRQ(RIT_IRQn);
	sprintf(str,"Player %d has won the match",player);
	LCD_Clear(White);
	LCD_Clear(White);
	GUI_Text(13, 253, (uint8_t *) str, Black, Yellow);
}
void DrawGame(){
	int x1=3,y1=1,x2=30,y2=1,x3=30,y3=30,x4=3,y4=30,i,j;
	LCD_Clear(Yellow);
	// Draw Board
	for(i=0; i < 7; i++){
		for (j=0; j < 7; j++){
			LCD_DrawLine(x1, y1, x2, y2, Black);
			LCD_DrawLine(x2, y2, x3, y3, Black);
			LCD_DrawLine(x3, y3, x4, y4, Black);
			LCD_DrawLine(x4, y4, x1, y1, Black);
			x1+=SQUARE_DISTANCE;
			x2+=SQUARE_DISTANCE;
			x3+=SQUARE_DISTANCE;
			x4+=SQUARE_DISTANCE;
		}
		y1+=SQUARE_DISTANCE;
		y2+=SQUARE_DISTANCE;
		y3+=SQUARE_DISTANCE;
		y4+=SQUARE_DISTANCE;
		x1 = 3; x2 = 30; x3 = 30; x4= 3;
	}
	drawBoxes(Black);
	//	Draw Player 1
	LCD_ColorSquare(102,1,Red,30);
	HighLight(102,1,Blue,1);
	
	//  Draw Player 2
	LCD_ColorSquare(102,199,White,30);
	
	
}
void startGame(){
	menu_mode = -1;
	init_matrix();		
	DrawGame();									
	enable_timer(0);
}
void colorPlayerBox(int player, uint16_t color){
	int x1=3,y1=1,x2=30,y2=1,x3=30,y3=30,x4=3,y4=30;
	x1 = 5; y1 = 250; x2 = 75;	y2 = 250;	y3 = 300; x3 = 75; x4= 5; y4 = 300;
	if (player == 1){
			LCD_DrawLine(x1, y1, x2, y2, color);
			LCD_DrawLine(x2, y2, x3, y3, color);
			LCD_DrawLine(x3, y3, x4, y4, color);
			LCD_DrawLine(x4, y4, x1, y1, color);
	}else if (player == 2){
			x1+=150;
			x2+=150;
		  x3+=150;
		  x4+=150;
			LCD_DrawLine(x1, y1, x2, y2, color);
			LCD_DrawLine(x2, y2, x3, y3, color);
			LCD_DrawLine(x3, y3, x4, y4, color);
			LCD_DrawLine(x4, y4, x1, y1, color);
	}
}
void placeWallMatrix(int i,int j, int dir){ // 5 3
	int x =(i*2)+1 ,y =(j*2)+1 ;
	if (dir == 1){						//	Vertical
		matrix[x][y] = WALL;
		matrix[x-1][y] = WALL;
		matrix[x+1][y] = WALL;
	}else{										//	Horizontal
		matrix[x][y] = WALL;
		matrix[x][y-1] = WALL;
		matrix[x][y+1] = WALL;
	}
	i = (turnPlayer == 1)? p1_i : p2_i;
	j = (turnPlayer == 1)? p1_j : p2_j;
	// Check if we are still in jump mode even after we place a wall
		//	UP 
	if ((i-2) >= 0)	
		if (matrix[i-2][j] == PLAYER1 || matrix[i-2][j] == PLAYER2) {jump = 1; dir_jump = DOWN;}
	// DOWN
	if ((i+2) <= 12)
		if (matrix[i+2][j] == PLAYER1 || matrix[i+2][j] == PLAYER2) {jump = 1; dir_jump = UP; }
	// LEFT
	if ((j-2) >= 0)
		if (matrix[i][j-2] == PLAYER1 || matrix[i][j-2] == PLAYER2) {jump = 1; dir_jump = RIGHT; }
	// RIGHT
	if ((j+2) <= 12)
		if (matrix[i][j+2] == PLAYER1 || matrix[i][j+2] == PLAYER2) {jump = 1; dir_jump = LEFT;}
}
int checkWallOverlay(int i, int j, int dir){
	int x = (j*2)+1 ,y = (i*2)+1 ;
	if(x <= 0 || y <= 0 || x >= 12 || y >= 12) return 0;
	//	Wall already placed there
	if(dir == 0){	//Horizontal
		if (matrix[x-1][y] == WALL || matrix[x+1][y] == WALL || matrix[x][y] == WALL)	return 0;
		if(matrix[x][y] == WALL || matrix[x][y-1] == WALL || matrix[x][y+1] == WALL  ) return 0;
	}
	if(dir == 1){	//vertical
		if (matrix[x][y-1] == WALL ||  matrix[x][y+1] == WALL || matrix[x][y] == WALL)	return 0;
		if(matrix[x][y] == WALL || matrix[x+1][y] == WALL || matrix[x-1][y] == WALL  ) return 0;
	}
	return 1;
}
static void placeWallRotated(int i, int j, uint16_t color){
	int x0,y0,s;
	x0 = (33*(j) +3 + 30 +2);
	y0 = 1+(33*i);
	for(s = 1; s < 3; s++){
		x0 = x0-s;	//	fill the wall
		LCD_DrawLine(	x0, y0, x0,y0+63,color);
	}
}
//	Given the coordinate i and j we know where to position the wall
// 	We need to specify the color so we can cancel a wall 
// 2 3
void LCD_placeWall(int i, int j, uint16_t color, int rotated){
		int x0,y0,s,r,c,x,y;
	x = (i*2)+1;
	y = (j*2)+1;
	//if(!checkWallOverlay(x,y,rotated)) return;
	if(rotated == 1){
		placeWallRotated(i,j,color);
		return;
	}
	
		x0 = (33*(j));
		y0 = (33*(i+1)+1);
			for(s = 1; s < 3; s++){
				y0 = y0-s;	//	fill the wall
				LCD_DrawLine(	x0, y0, x0+63,y0,color);
		}
}

void wallPlacementMode(){
	HighLight(playerX,playerY,Yellow,turnPlayer);
	// check if there's already a wall in initial position
	// ...
	wI = 2;wJ = 3;
	tmpWI = 2; tmpWJ = 3;
	LCD_placeWall(2 ,3 , Red, 0);
	mode = 2;
}
void disableWallPlacementMode(){
	HighLight(playerX,playerY,Blue,turnPlayer);
	mode = 1;
	rotatedWallMode = 0;
}
int decreaseWallCounter(int player){
	char str[20];
	NVIC_DisableIRQ(TIMER0_IRQn);
	if ( player == 1){
		if (wallsP1 != 0){
			wallsP1--;
			sprintf(str,"%d",wallsP1);
			GUI_Text(37, 275, (uint8_t *) str, Black, Yellow);	
			NVIC_EnableIRQ(TIMER0_IRQn);
			return 1;
		}else{
			// Cancel boxes
			drawBoxes(Yellow);
			GUI_Text(13, 253, (uint8_t *) "No walls available, move the token", Black, Yellow);	
			GUI_Text(13, 253, (uint8_t *) "No walls available, move the token", Yellow, Yellow);	
			drawBoxes(Black);
			NVIC_EnableIRQ(TIMER0_IRQn);
			return 0;
		}
	}else if (player == 2){
		if(wallsP2!=0){
			wallsP2--;
			sprintf(str,"%d",wallsP2);
			GUI_Text(185, 275, (uint8_t *) str, Black, Yellow);	
			NVIC_EnableIRQ(TIMER0_IRQn);
			return 1;
		}else{
			// Cancel boxes
			drawBoxes(Yellow);
			GUI_Text(13, 253, (uint8_t *) "No walls available, move the token", Black, Yellow);	
			GUI_Text(13, 253, (uint8_t *) "No walls available, move the token", Yellow, Yellow);	
			drawBoxes(Black);
			NVIC_EnableIRQ(TIMER0_IRQn);
			return 0;
		}
	}
	NVIC_EnableIRQ(TIMER0_IRQn);
	return 1;
}
void errorMessage(char message[100]){
	// Cancel boxes
			stop_timer_flag = 1;
			NVIC_DisableIRQ(TIMER0_IRQn);
			drawBoxes(Yellow);
			GUI_Text(13, 253, (uint8_t *) message, Black, Yellow);	
			GUI_Text(13, 253, (uint8_t *) message, Yellow, Yellow);	
			drawBoxes(Black);
			stop_timer_flag = 0;
			NVIC_EnableIRQ(TIMER0_IRQn);
}       
int checkPlayerPresence(int i, int j, int dir){
	switch(dir){
		case UP:
			//	UP 
			if ((i-2) >= 0)	
				if (matrix[i-2][j] == PLAYER1 || matrix[i-2][j] == PLAYER2) {return 1;}
			break;
		case DOWN:
			// DOWN
			if ((i+2) <= 12)
				if (matrix[i+2][j] == PLAYER1 || matrix[i+2][j] == PLAYER2) {return 1; }
			break;
		case LEFT:
			// LEFT
			if ((j-2) >= 0)
				if (matrix[i][j-2] == PLAYER1 || matrix[i][j-2] == PLAYER2) {return 1; }
			break;
		case RIGHT:
			// RIGHT
			if ((j+2) <= 12)
				if (matrix[i][j+2] == PLAYER1 || matrix[i][j+2] == PLAYER2) {return 1;}
			break;
	}
	return 0;
}
void undoWallPlacing(int i, int j, int dir){
	int x = (i*2)+1 ,y = (j*2)+1 ;
	if (dir == 1){						//	Vertical
		matrix[x][y] = SPACE;
		matrix[x-1][y] = SPACE;
		matrix[x+1][y] = SPACE;
	}else{										//	Horizontal
		matrix[x][y] = SPACE;
		matrix[x][y-1] = SPACE;
		matrix[x][y+1] = SPACE;
	}
}
void colorMenuBox(int boxNumber, uint16_t color){
	int x1,x2,x3,x4,y1,y2,y3,y4,i,dim=90;
	x1 = 50; y1 = 150; x2 = 190;	y2 = 150;	y3 = 190; x3 = 190; x4= 50; y4 = 190;
	if(boxNumber == 1){
				LCD_DrawLine(x1, y1, x2, y2, color);
				LCD_DrawLine(x2, y2, x3, y3, color);
				LCD_DrawLine(x3, y3, x4, y4, color);
				LCD_DrawLine(x4, y4, x1, y1, color);
	}else{
				y1+=dim;
				y2+=dim;
				y3+=dim;
				y4+=dim;
				LCD_DrawLine(x1, y1, x2, y2, color);
				LCD_DrawLine(x2, y2, x3, y3, color);
				LCD_DrawLine(x3, y3, x4, y4, color);
				LCD_DrawLine(x4, y4, x1, y1, color);
	}
}
void menu(int window){
	enable_RIT();		
	LCD_Clear(White);
	if(window == 1){
		menu_mode = 1;
		menu_choice = SINGLE_PLAYER;
		GUI_Text(40,50,(uint8_t *) "Select the GAME MODE",Black,White);
		colorMenuBox(1,Red);
		colorMenuBox(2,Black);
		GUI_Text(70,165,(uint8_t *) "Single Board",Black,White);
		GUI_Text(80,255,(uint8_t *) "Two Board",Black,White);
	}
	if(window == 2){
		menu_mode = 2;
		menu_choice2 = HUMAN;
		//Remember to adjust this
		GUI_Text(20,50,(uint8_t *) "Select the opposing player",Black,White);
		colorMenuBox(1,Red);
		colorMenuBox(2,Black);
		GUI_Text(120,165,(uint8_t *) "Human",Black,White);
		GUI_Text(140,255,(uint8_t *) "NPC",Black,White);
	}
	return;
}

static move_t bestMoveDFS(int i, int j, int player, int matrixVisited[13][13], int cost, int *bestCost, int *bestI, int *bestJ, int tmpI, int tmpJ){
	int term,res,rotated,flag = 0,wI,wj, walls = (player == 1) ? wallsP1 : wallsP2,tmpi = i, tmpj = j;
	move_t move,bestMoveForOpponent;
  term = (player == 1) ? 12 : 0;
  if ( i == term){
      // At this point by using pruning we should be sure that the cost 
			// is better
		*bestCost = cost;
		*bestI = tmpI;
		*bestJ = tmpJ;
		move = buildMove(player,0,0,*bestI,*bestJ);
		return move;
	}
	if( randomNumber%2 != 0 && walls != 0 ){	// WallPlacement
			do{
				bestMoveForOpponent = findBestMove(( player == 1)? 2 : 1);
				globalSeed++;
			}while(bestMoveForOpponent.op != 0);
		//Place wall in the direction of best opponent move
			i = (player == 1) ? p2_i : p1_i;
			j = (player == 1) ? p2_j : p1_j;
			// Scale it back to matrix coordinate
			bestMoveForOpponent.x*=2;
			bestMoveForOpponent.y*=2;
			if(i-bestMoveForOpponent.y >0){	//best move is UP
				//check left wall placement
				wI = (i == 0 || i == 1)? 0 : (((i - 1)-1)/2);
				wJ = (j == 0 || j == 1)? 0 : (((j - 1)-1)/2);
				if(checkWallOverlay(	wI,wJ,0)){
					placeWallMatrix(	wI,	wJ,0	);
					if(checkTrap(1) && checkTrap(2)) return move = buildMove(player,1,1, wI,wJ );
				}
				//check right wall placement
				wI = (i == 0 || i == 1)? 0 : (((i - 1)-1)/2);
				wJ = (j == 0 || j == 1)? 0 : (((j + 1)-1)/2);
				if(checkWallOverlay(	wI,wJ,0)){
					placeWallMatrix(	wI,	wJ,0	);
					if(checkTrap(1) && checkTrap(2)) return move = buildMove(player,1,1, wI,wJ );
				}
			}
			if(i-bestMoveForOpponent.y < 0){	//best move is DOWN
				//check left wall placement
				wI = (i == 0 || i == 1)? 0 : (((i + 1)-1)/2);
				wJ = (j == 0 || j == 1)? 0 : (((j - 1)-1)/2);
				if(checkWallOverlay(	wI,wJ,0)){
					placeWallMatrix(	wI,	wJ,0	);
					if(checkTrap(1) && checkTrap(2)) return move = buildMove(player,1,1, wI,wJ );
				}
				//check right wall placement
				wI = (i == 0 || i == 1)? 0 : (((i + 1)-1)/2);
				wJ = (j == 0 || j == 1)? 0 : (((j + 1)-1)/2);
				if(checkWallOverlay(	wI,wJ,0)){
					placeWallMatrix(	wI,	wJ,0	);
					if(checkTrap(1) && checkTrap(2)) return move = buildMove(player,1,1, wI,wJ );
				}
			}
			if(j-bestMoveForOpponent.x >0){	//best move is left
				//check up wall placement
				wI = (i == 0 || i == 1)? 0 : (((i - 1)-1)/2);
				wJ = (j == 0 || j == 1)? 0 : (((j - 1)-1)/2);
				if(checkWallOverlay(	wI,wJ,1)){
					placeWallMatrix(	wI,	wJ,1	);
					if(checkTrap(1) && checkTrap(2)) return move = buildMove(player,1,0, wI,wJ );
				}
				//check down wall placement
				wI = (i == 0 || i == 1)? 0 : (((i + 1)-1)/2);
				wJ = (j == 0 || j == 1)? 0 : (((j - 1)-1)/2);
				if(checkWallOverlay(	wI,wJ,1)){
					placeWallMatrix(	wI,	wJ,1	);
					if(checkTrap(1) && checkTrap(2)) return move = buildMove(player,1,0, wI,wJ );
				}
			}
			if(j-bestMoveForOpponent.x < 0){	//best move is right
				//check up wall placement
				wI = (i == 0 || i == 1)? 0 : (((i - 1)-1)/2);
				wJ = (j == 0 || j == 1)? 0 : (((j + 1)-1)/2);
				if(checkWallOverlay(	wI,wJ,1)){
					placeWallMatrix(	wI,	wJ,1	);
					if(checkTrap(1) && checkTrap(2)) return move = buildMove(player,1,0, wI,wJ );
				}
				//check up wall placement
				wI = (i == 0 || i == 1)? 0 : (((i + 1)-1)/2);
				wJ = (j == 0 || j == 1)? 0 : (((j + 1)-1)/2);
				if(checkWallOverlay(	wI,wJ,1)){
					placeWallMatrix(	wI,	wJ,1	);
					if(checkTrap(1) && checkTrap(2)) return move = buildMove(player,1,0, wI,wJ );
				}
			}	
		}
			i = tmpi;
			j = tmpj;
			if ((checkPlayerMove(i,j,UP,player) || dir_jump == UP)  && matrixVisited[i-2][j] == 0 ){
					if(jump == 1 && dir_jump == UP && checkPlayerMove(i-2,j,UP,player) && cost < *bestCost) {
						
						i-=4;
						makePlayerMove(i,j,player);
						matrixVisited[i][j] = 1;
						// recursive path with initial move UP
						if(cost == 0) {
							tmpI = i;
							tmpJ = j;
						}
						cost++;
						move = bestMoveDFS(i,j,player,matrixVisited,cost,bestCost,bestI,bestJ,tmpI,tmpJ);
						//  Backtrack
						matrixVisited[i][j] = 0;
						i+=4;
						makePlayerMove(i,j,player);
						cost--;
					}
					// This condition allow us to avoid the case in which the jump is possible but not legal
					if(dir_jump != UP && (cost+1) < *bestCost){
						
						i-=2;
						makePlayerMove(i,j,player);
						matrixVisited[i][j] = 1;
						if(cost == 0) {
							tmpI = i;
							tmpJ = j;
						}
						cost++;
						move = bestMoveDFS(i,j,player,matrixVisited,cost,bestCost,bestI,bestJ,tmpI,tmpJ);
						//  Backtrack
						matrixVisited[i][j] = 0;
						i+=2;
						makePlayerMove(i,j,player);
						cost--;
					}
			}
			if ((checkPlayerMove(i,j,DOWN,player) || dir_jump == DOWN)  && matrixVisited[i+2][j] == 0 ){
					if(jump == 1 && dir_jump == DOWN && checkPlayerMove(i+2,j,DOWN,player) && cost < *bestCost) {
						
						i+=4;
						makePlayerMove(i,j,player);
						matrixVisited[i][j] = 1;
						// recursive path with initial move UP
						if(cost == 0) {
							tmpI = i;
							tmpJ = j;
						}
						cost++;
						move = bestMoveDFS(i,j,player,matrixVisited,cost,bestCost,bestI,bestJ,tmpI,tmpJ);
						//  Backtrack
						matrixVisited[i][j] = 0;
						i-=4;
						makePlayerMove(i,j,player);
						cost--;
					}
					// This condition allow us to avoid the case in which the jump is possible but not legal
					if(dir_jump != DOWN && (cost+1) < *bestCost){
						
						i+=2;
						makePlayerMove(i,j,player);
						matrixVisited[i][j] = 1;
						if(cost == 0) {
							tmpI = i;
							tmpJ = j;
						}
						cost++;
						move = bestMoveDFS(i,j,player,matrixVisited,cost,bestCost,bestI,bestJ,tmpI,tmpJ);
						//  Backtrack
						matrixVisited[i][j] = 0;
						i-=2;
						makePlayerMove(i,j,player);
						cost--;
					}
			}
			if ((checkPlayerMove(i,j,LEFT,player) || dir_jump == LEFT)  && matrixVisited[i][j-2] == 0 ){
					if(jump == 1 && dir_jump == LEFT && checkPlayerMove(i,j-2,LEFT,player) && cost < *bestCost) {
						
						j-=4;
						makePlayerMove(i,j,player);
						matrixVisited[i][j] = 1;
						// recursive path with initial move UP
						if(cost == 0) {
							tmpI = i;
							tmpJ = j;
						}
						cost++;
						move = bestMoveDFS(i,j,player,matrixVisited,cost,bestCost,bestI,bestJ,tmpI,tmpJ);
						//  Backtrack
						matrixVisited[i][j] = 0;
						j+=4;
						makePlayerMove(i,j,player);
						cost--;
					}
					// This condition allow us to avoid the case in which the jump is possible but not legal
					if(dir_jump != LEFT && (cost+1) < *bestCost){
						
						j-=2;
						makePlayerMove(i,j,player);
						matrixVisited[i][j] = 1;
						if(cost == 0) {
							tmpI = i;
							tmpJ = j;
						}
						cost++;
						move = bestMoveDFS(i,j,player,matrixVisited,cost,bestCost,bestI,bestJ,tmpI,tmpJ);
						//  Backtrack
						matrixVisited[i][j] = 0;
						j+=2;
						makePlayerMove(i,j,player);
						cost--;
					}
			}
			if ((checkPlayerMove(i,j,RIGHT,player) || dir_jump == RIGHT)  && matrixVisited[i][j+2] == 0 ){
					if(jump == 1 && dir_jump == RIGHT && checkPlayerMove(i,j+2,RIGHT,player) && cost < *bestCost) {
						
						j+=4;
						makePlayerMove(i,j,player);
						matrixVisited[i][j] = 1;
						// recursive path with initial move UP
						if(cost == 0) {
							tmpI = i;
							tmpJ = j;
						}
						cost++;
						move = bestMoveDFS(i,j,player,matrixVisited,cost,bestCost,bestI,bestJ,tmpI,tmpJ);
						//  Backtrack
						matrixVisited[i][j] = 0;
						j-=4;
						makePlayerMove(i,j,player);
						cost--;
					}
					// This condition allow us to avoid the case in which the jump is possible but not legal
					if(dir_jump != RIGHT && (cost+1) < *bestCost){
						
						j+=2;
						makePlayerMove(i,j,player);
						matrixVisited[i][j] = 1;
						if(cost == 0) {
							tmpI = i;
							tmpJ = j;
						}
						cost++;
						move = bestMoveDFS(i,j,player,matrixVisited,cost,bestCost,bestI,bestJ,tmpI,tmpJ);
						//  Backtrack
						matrixVisited[i][j] = 0;
						j-=2;
						makePlayerMove(i,j,player);
						cost--;
					}
			}
	return move;
}

move_t findBestMove(int player){
	move_t bestMove;
	int i,j,r,c, bestCost = 1000, bestDir = -1, bestI=-1,bestJ=-1;// 100 is a logic MAX
  int matrixVisited[13][13], result = 0, tmpMatrix[13][13], tmpP1_i = p1_i,tmpP1_j = p1_j,tmpP2_i = p2_i,tmpP2_j = p2_j,tmpJump = jump, tmpDirJump = dir_jump;
	i = (player == 1) ? p1_i : p2_i;
	j = (player == 1) ? p1_j : p2_j;
  for (r = 0; r < 13; r++){
      for (c = 0; c < 13; c++){
          matrixVisited[r][c] = 0;
					tmpMatrix[r][c] = matrix[r][c];
      }
  }
	//Place player
  matrixVisited[i][j] = player;
	srand(globalSeed);
	randomNumber = rand()%100;
	// number shuffling
	globalSeed++;
  bestMove = bestMoveDFS(i,j,player, matrixVisited,0,&bestCost,&bestI,&bestJ,-1,-1);
	// Restore everything
	for (r = 0; r < 13; r++){
      for (c = 0; c < 13; c++){
					matrix[r][c] = tmpMatrix[r][c];
      }
  }
	p1_i = tmpP1_i; p1_j = tmpP1_j; 
	p2_i = tmpP2_i; p2_j = tmpP2_j;
	jump = tmpJump, dir_jump = tmpDirJump;
	//Scale x and y to 7x7 matrix if is a playerMove
	if(bestMove.op == 0){
		bestMove.x = bestJ/2;
		bestMove.y = bestI/2;
	}
	return bestMove;
}
/*
	if mode is 0 we should move only when turnPlayer == myplayer
	if mode is 1 NPC will move only when he is player 2
*/
