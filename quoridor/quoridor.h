#ifndef __QUORIDOR_H
#define __QUORIDOR_H
#define SQUARE_DISTANCE 33
#define BOXES_DISTANCE 75
#define MATRIX_DIM 13
#include <stdio.h>
#include <stdlib.h>
#include "lpc17xx.h"
typedef struct{
	int player;
	int op;
	int dir; 
	int x;
	int y;
}move_t;

void init_matrix();
int checkPlayerMove(int i, int j, int dir, int player);
int checkWallMove(int i, int j, int dir, int player);
int checkWallPlacement(int i, int j);
int checkTrap(int player);
static int dfs (int i, int j, int plater, int matrixVisited[MATRIX_DIM][MATRIX_DIM]);
void createMoveAndSave(uint32_t player, uint32_t move,uint32_t dir);
void makePlayerMove(int i, int j, int player);           /* Return 1 if the move is a winning move*/
int coordinateConversion(uint16_t LCD_Coordinate);
void placeWallMatrix(int i,int j, int dir);
int checkWallOverlay(int i, int j, int dir);
void placeWallRotated(int i, int j, uint16_t color);
void LCD_placeWall(int i, int j, uint16_t color, int rotated);
void wallPlacementMode();
void disablewallPlacementMode();
int decreaseWallCounter(int player);
void errorMessage(char message[100]);
int checkPlayerPresence(int i, int j, int dir);
void undoWallPlacing(int i, int j, int dir);
void win(int player);
void menu(int window);
void colorMenuBox(int boxNumber, uint16_t color);
void startGame();
move_t findBestMove(int player);

extern void confirmWallPlacement();
extern move_t buildMove(int player, int op, int dir, int x, int y);
#endif