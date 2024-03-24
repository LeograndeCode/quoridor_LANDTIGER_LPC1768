/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_timer.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    timer.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "lpc17xx.h"
#include "timer.h"
#include "../quoridor/quoridor.h"
#include "../GLCD/GLCD.h"
#include "../CAN/CAN.h"

extern int reset_timer_flag;
extern int choice_confirmed;
extern int mode;
extern int turnPlayer;
extern int tmpX,tmpY;
extern int tmpWI,tmpWJ;
extern int playerX,playerY;
extern int selected_movement;
extern int rotatedWallMode;
extern int p1_i,p1_j,p2_i,p2_j;
extern int alreadyMoved;
extern int jump,dir_jump;
extern int stop_timer_flag;
extern int handshake_successful;
extern int menu_mode;
extern int menu_choice;
extern int menu_choice2;
extern int myplayer;
extern int flag_start;
extern void sendMove(move_t move);

enum{SINGLE_PLAYER,MULTIPLAYER,HUMAN,NPC};

/******************************************************************************
** Function name:		Timer0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

void TIMER0_IRQHandler (void)
{
	static int time = 20;
	char time_in_char[5] = "";
	move_t move;
	if(stop_timer_flag == 1) return;
	if(time < 0 || reset_timer_flag == 1)	{
		// Moved but not confirmed
		if(choice_confirmed == 0 && selected_movement != (-1)){
			//createMoveAndSave(turnPlayer,0,1);
			if(mode == 1 ){
				// Restore old position
				LCD_ColorSquare(tmpX,tmpY,Yellow,30);
				LCD_ColorSquare(playerX,playerY,(turnPlayer == 1)? Red : White,30);
			}else if(mode == 2){
				//createMoveAndSave(turnPlayer,0,1);
				LCD_placeWall(tmpWI,tmpWJ,Yellow,rotatedWallMode);
			}	
			if(menu_choice == MULTIPLAYER && menu_choice2 == HUMAN){
				move = buildMove(turnPlayer,0,1,0,0);
				sendMove(move);
			}
		}
		
		// 	Not even Moved but not received a CAN move
		if( selected_movement == (-1) && (menu_choice2 != NPC) ){
			if(menu_choice == MULTIPLAYER && menu_choice2 == HUMAN){
				move = buildMove(turnPlayer,0,1,0,0);
				sendMove(move);
			}
			if(mode == 1){
				HighLight(playerX,playerY,Yellow,turnPlayer);
			}else if(mode == 2){
				LCD_placeWall(tmpWI,tmpWJ,Yellow,rotatedWallMode);
			}
			if(menu_choice == MULTIPLAYER && menu_choice2 == HUMAN){
				move = buildMove(turnPlayer,0,1,0,0);
				sendMove(move);
			}
		}
		
		//	Pass turn procedure 
		time = 20;
		// Changing coordinates for the turn Player
		turnPlayer = (turnPlayer == 1) ? 2 : 1;
		conv((turnPlayer == 1)? p1_j : p2_j,(turnPlayer == 1)? p1_i : p2_i );
		playerX = tmpX;
		playerY = tmpY;
		//Graphics
		if((menu_choice == SINGLE_PLAYER && menu_choice2 == HUMAN)
			|| (menu_choice == SINGLE_PLAYER && menu_choice2 == NPC && turnPlayer == 1)
			|| (menu_choice == MULTIPLAYER && menu_choice2 == HUMAN && turnPlayer == myplayer) ){
			HighLight(tmpX,tmpY,Blue,turnPlayer);
			LCD_ColorSquare(tmpX,tmpY,(turnPlayer == 1) ? Red : White,30);
		}
		
		// Flag resetting
		reset_timer_flag = 0;
		selected_movement = -1;
		choice_confirmed = 0;
		mode = 1;
		alreadyMoved = 0;
		flag_start = 1;
	}
	sprintf(time_in_char,"%4d",time);		//convert from system time to real time
	GUI_Text(95, 268, (uint8_t *) time_in_char, Black, Yellow);
	time--;
  LPC_TIM0->IR = 1;			/* clear interrupt flag */
  return;
}


/******************************************************************************
** Function name:		Timer1_IRQHandler
**
** Descriptions:		Timer/Counter 1 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void TIMER1_IRQHandler (void)
{
	static int countdown = 5;
	
	if(handshake_successful == 1){
		disable_timer(1);
	}
	
	if(countdown == 0){
		LCD_Clear(White);
		GUI_Text(50,100,(uint8_t *) "Errore connessione:",Black,White);
		GUI_Text(10,130,(uint8_t *) "nessun avversario trovato",Black,White);
	}
	countdown--;
	
  LPC_TIM1->IR = 1;			/* clear interrupt flag */
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
