#ifndef __QUORIDOR_H
#define __QUORIDOR_H
#define SQUARE_DISTANCE 33
#define BOXES_DISTANCE 75
#define MATRIX_DIM 13
#include <stdio.h>
#include "lpc17xx.h"
void init_matrix();
int checkPlayerMove(int i, int j, int dir, int player, int* jump);
int checkWallMove(int i, int j, int dir, int player);
int checkWallPlacement(int i, int j);
static int checkTrap(int i, int j,int player);
static int dfs (int i, int j, int plater, int matrixVisited[MATRIX_DIM][MATRIX_DIM]);
void createMoveAndSave(uint32_t player, uint32_t move,uint32_t dir);
void makePlayerMove(int i, int j, int dir, int player, int jump);           /* Return 1 if the move is a winning move*/
void placeWall(int i, int j, int player);
void HighLight(int x0,int y0,uint16_t color, int player);
static void drawBoxes(uint16_t color);
void win(int player);
void DrawGame();
void startGame();
void colorPlayerBox(int player, uint16_t color);
#endif
