#include "quoridor.h"
#include "RIT/RIT.h"
#include "timer/timer.h"
#include "button_EXINT/button.h"
#include "GLCD/GLCD.h"

enum {FREE_SQUARE,PLAYER1,PLAYER2,SPACE,WALL};
enum {UP, DOWN, LEFT, RIGHT};

volatile int matrix[MATRIX_DIM][MATRIX_DIM];
volatile int p1_i=0,p1_j=6,p2_i=12,p2_j=6;
volatile int wallsP1 = 8,wallsP2 = 8;
volatile uint32_t moves[100];
volatile int movesIndex = 0;

extern int tmpX,tmpY; // adjust
extern int playerX,playerY;
extern int turnPlayer;
extern int wI,wJ,tmpWI,tmpWJ;
extern int mode;
extern int rotatedWallMode;
extern int jump;

extern void LCD_ColorSquare(uint16_t x0,	uint16_t y0,	uint16_t color,	int dim);
// 0 6
void conv(int i, int j){
	int shiftI = (i == 0)? 0 : (i/2), shiftJ =(j == 0) ? 0 : (j/2);		// Since square i in matrix 13x13 is square i/2 + 1 in matrix 7 x 7
	i= 1 + (shiftI * SQUARE_DISTANCE);
	j= 3 + (shiftJ * SQUARE_DISTANCE);
	tmpX = j;
	tmpY = i;
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

int checkPlayerMove(int i, int j, int dir, int player){
	switch(dir){
		case UP:
			// out of bounds
			if ((i-1) < 0)	return 0;
			if (matrix[i-1][j] == WALL) return 0;
			if (matrix[i-2][j] == PLAYER1 || matrix[i-2][j] == PLAYER2) jump = 1;
			break;
		case DOWN:
			// out of bounds
			if ((i+1) > 12)	return 0;
			if (matrix[i+1][j] == WALL) return 0;
			if (matrix[i+2][j] == PLAYER1 || matrix[i+2][j] == PLAYER2) jump = 1;
			break;
		case LEFT:
            // out of bounds
			if ((j-1) < 0)	return 0;
			if (matrix[i][j-1] == WALL) return 0;
			if (matrix[i][j-2] == PLAYER1 || matrix[i][j-2] == PLAYER2) jump = 1;
			break;
		case RIGHT:
            // out of bounds
			if ((j+1) > 12)	return 0;
			if (matrix[i][j+1] == WALL) return 0;
			if (matrix[i][j+2] == PLAYER1 || matrix[i][j+2] == PLAYER2) jump = 1;
			break;

	}
	return 1;
}
int checkWallMove(int i, int j, int dir, int player){
	i = (i*2)+1; j = (j*2)+1; // conversion to get central point of the wall in the matrix
	switch(dir){
		case UP:
			if(matrix[i-2][j] == WALL) return 0;
			break;
		case DOWN:
			if(matrix[i+2][j] == WALL) return 0;
			break;
		case LEFT:
			if(matrix[i][j-2] == WALL) return 0;
			break;
		case RIGHT:
			if(matrix[i][j+2] == WALL) return 0;
			break;
	}
	return 1;
}
int checkTrap(int i, int j,int player);
static int dfs (int i, int j, int plater, int matrixVisited[MATRIX_DIM][MATRIX_DIM]);
void createMoveAndSave(uint32_t player, uint32_t move,uint32_t dir);
/* 	
	Return 1 if the move is a winning move 
*/
void makePlayerMove(int i, int j, int dir, int player, int jump){
	if ( jump == 0){
		matrix[i][j] = player;
	}else{
		switch(dir){
			case UP:
				matrix[i-4][j] = player;
				break;
			case DOWN:
				matrix[i+4][j] = player;
				break;
			case LEFT:
				matrix[i][j-4] = player;
				break;
			case RIGHT:
				matrix[i][j+4] = player;
				break;
		}
	}
}  
// 102 1
void HighLight(int x0,int y0,uint16_t color, int player){
	int d = SQUARE_DISTANCE;
	int j = (x0 == 3)? 0 :((x0/33))*2, i = (y0 == 1)? 0 :(((y0/33))*2);
		//	DOWN
		if(checkPlayerMove(i,j,DOWN,player)) LCD_ColorSquare(x0,y0+d,color,30);
		//	UP
		if(checkPlayerMove(i,j,UP,player)) LCD_ColorSquare(x0, y0-d,color,30);
		//	DX
		if(checkPlayerMove(i,j,RIGHT,player)) LCD_ColorSquare(x0+d,y0,color,30);
		//	SX
		if(checkPlayerMove(i,j,LEFT,player)) LCD_ColorSquare(x0-d,y0,color,30);
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
	enable_RIT();		
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
	int x = (i*2)+1 ,y = (j*2)+1 ;
	if (dir == 1){						//	Vertical
		matrix[x][y] = WALL;
		matrix[x-1][y] = WALL;
		matrix[x+1][y] = WALL;
	}else{										//	Horizontal
		matrix[x][y] = WALL;
		matrix[x][y-1] = WALL;
		matrix[x][y+1] = WALL;
	}
}
int checkWallOverlay(int i, int j, int dir){
	int x = (i*2)+1 ,y = (j*2)+1 ;
	//	Wall already placed there
	if(dir == 0){	//Horizontal
		if (matrix[x-1][y] == WALL || matrix[x+1][y] == WALL)	return 0;
	}
	if(dir == 1){	//vertical
		if (matrix[x][y-1] == WALL || matrix[x][y+1] == WALL)	return 0;
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
		int x0,y0,s;
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
			NVIC_DisableIRQ(TIMER0_IRQn);
			drawBoxes(Yellow);
			GUI_Text(13, 253, (uint8_t *) message, Black, Yellow);	
			GUI_Text(13, 253, (uint8_t *) message, Yellow, Yellow);	
			drawBoxes(Black);
			NVIC_EnableIRQ(TIMER0_IRQn);
}