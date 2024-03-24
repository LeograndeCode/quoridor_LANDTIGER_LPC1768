/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_RIT.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    RIT.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "lpc17xx.h"
#include "RIT.h"
#include "../quoridor/quoridor.h"
#include "../GLCD/GLCD.h"
#include "../CAN/CAN.h"
#include "../timer/timer.h"
#include <stdio.h>
/******************************************************************************
** Function name:		RIT_IRQHandler
**
** Descriptions:		REPETITIVE INTERRUPT TIMER handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
enum {UP,DOWN,LEFT,RIGHT,DOWN_RIGHT,DOWN_LEFT,UP_LEFT,UP_RIGHT,LEFT_UP,LEFT_DOWN,RIGHT_UP,RIGHT_DOWN};
enum{SINGLE_PLAYER,MULTIPLAYER,HUMAN,NPC};
volatile int down=0;
volatile int down2=0;

//Flags
volatile int reset_timer_flag = 0;
volatile int choice_confirmed = 0;
volatile int mode = 1;
volatile int turnPlayer = 1;
volatile int jump = 0;
volatile int selected_movement = -1;
volatile int dir_jump = -1;
volatile int stop_timer_flag = 0;
volatile move_t tmpMove;
volatile int tmpX = 102,tmpY = 1 ;
volatile int playerX =102 ,playerY = 1;
volatile int wI=2,wJ=3;	//	coordinates of the last insert wall
volatile int tmpWI=2,tmpWJ=3;
volatile int rotatedWallMode = 0;
volatile int alreadyMoved = 0;
volatile int menu_mode = -1;
volatile int menu_choice = -1;
volatile int menu_choice2 = -1;
volatile int flag_start = 0;
volatile int myplayer = CAN_CONTROLLER_FOR_GAME;	//Player number assigned in multiplayer mode by the handshake
extern int p1_i,p1_j,p2_i,p2_j;

void LCD_moveTurnPlayer(int dir);
move_t buildMove(int player, int op, int dir, int x, int y);
void LCD_moveWall(int dir, int rotated);
void confirmWallPlacement();
void confirmPlayerMove();
void decodeAndMakeMove(CAN_msg CAN_RxMsg);
move_t findBestMoveAndMakeIt(int player);
void activateNPC();
void sendMove(move_t move);
void NPC_Move();
void RIT_IRQHandler (void)
{					
	static int J_select=0;
	static int J_down = 0;
	static int J_up = 0;
	static int J_left = 0;
	static int J_right = 0;
	static int J_downRight = 0;
	static int J_downLeft = 0;
	static int J_upLeft = 0;
	static int J_upRight = 0;
	static int J_rightUp = 0;
	static int J_rightDown = 0;
	static int J_leftDown = 0;
	static int J_leftUp = 0;
	
	
 	if( menu_choice == SINGLE_PLAYER && menu_choice2 == NPC && turnPlayer == 2 && alreadyMoved == 0 ){
		NPC_Move();
	}
	if( menu_choice == MULTIPLAYER && menu_choice2 == NPC && turnPlayer == myplayer && alreadyMoved == 0 && flag_start ==1 ){
		NPC_Move();
		flag_start = 0;
	}
	
	/*
		Using the direction of the jump (dir_jump) to differentiate for example left-down from down-left
	*/
	// DOWN-RIGHT
	if((LPC_GPIO1->FIOPIN & (1<<26)) == 0 && (LPC_GPIO1->FIOPIN & (1<<28)) == 0 && jump == 1 && dir_jump == DOWN && mode == 1){		
		J_downRight++;
		switch(J_downRight){
			
			case 1:
				NVIC_DisableIRQ(TIMER0_IRQn);
				if((checkPlayerMove((turnPlayer == 1) ? (p1_i+2) : (p2_i+2),(turnPlayer == 1) ? p1_j : p2_j,RIGHT,turnPlayer))
					&& alreadyMoved == 0 ){
						selected_movement = DOWN_RIGHT;
						tmpMove = buildMove(turnPlayer,0,0,(turnPlayer == 1) ? p1_j/2 : p2_j/2,(turnPlayer == 1) ? p1_i/2 : p2_i/2);
						LCD_moveTurnPlayer(selected_movement);
					}
			default:
				break;
						
		}
		NVIC_EnableIRQ(TIMER0_IRQn);
		alreadyMoved = 1;
	}else{
			if(alreadyMoved == 0) J_downRight=0;
	}
	
	// DOWN-LEFT
	if((LPC_GPIO1->FIOPIN & (1<<26)) == 0 && (LPC_GPIO1->FIOPIN & (1<<27)) == 0 && jump == 1 && dir_jump == DOWN && mode == 1){		
		J_downLeft++;
		switch(J_downLeft){
			
			case 1:
				NVIC_DisableIRQ(TIMER0_IRQn);
				if((checkPlayerMove((turnPlayer == 1) ? (p1_i+2) : (p2_i+2),(turnPlayer == 1) ? p1_j : p2_j,LEFT,turnPlayer))
					&& alreadyMoved == 0 ){
						selected_movement = DOWN_LEFT;
						tmpMove = buildMove(turnPlayer,0,0,(turnPlayer == 1) ? p1_j/2 : p2_j/2,(turnPlayer == 1) ? p1_i/2 : p2_i/2);
						LCD_moveTurnPlayer(selected_movement);
					}
			default:
				break;
						
		}
		NVIC_EnableIRQ(TIMER0_IRQn);
		alreadyMoved = 1;
	}else{
			if(alreadyMoved == 0) J_downLeft=0;
	}
	
	// UP-LEFT
	if((LPC_GPIO1->FIOPIN & (1<<29)) == 0 && (LPC_GPIO1->FIOPIN & (1<<27)) == 0 && jump == 1 && dir_jump == UP && mode == 1){		
		J_upLeft++;
		switch(J_upLeft){
			
			case 1:
				NVIC_DisableIRQ(TIMER0_IRQn);
				if((checkPlayerMove((turnPlayer == 1) ? (p1_i-2) : (p2_i-2),(turnPlayer == 1) ? p1_j : p2_j,LEFT,turnPlayer))
					&& alreadyMoved == 0 ){
						selected_movement = UP_LEFT;
						tmpMove = buildMove(turnPlayer,0,0,(turnPlayer == 1) ? p1_j/2 : p2_j/2,(turnPlayer == 1) ? p1_i/2 : p2_i/2);
						LCD_moveTurnPlayer(selected_movement);
					}
			default:
				break;
						
		}
		NVIC_EnableIRQ(TIMER0_IRQn);
		alreadyMoved = 1;
	}else{
			if(alreadyMoved == 0) J_upLeft=0;
	}
	
	// UP-RIGHT
	if((LPC_GPIO1->FIOPIN & (1<<29)) == 0 && (LPC_GPIO1->FIOPIN & (1<<28)) == 0 && jump == 1 && dir_jump == UP && mode == 1){		
		J_upRight++;
		switch(J_upRight){
			
			case 1:
				NVIC_DisableIRQ(TIMER0_IRQn);
				if((checkPlayerMove((turnPlayer == 1) ? (p1_i-2) : (p2_i-2),(turnPlayer == 1) ? p1_j : p2_j,RIGHT,turnPlayer))
					&& alreadyMoved == 0 ){
						selected_movement = UP_RIGHT;
						tmpMove = buildMove(turnPlayer,0,0,(turnPlayer == 1) ? p1_j/2 : p2_j/2,(turnPlayer == 1) ? p1_i/2 : p2_i/2);
						LCD_moveTurnPlayer(selected_movement);
					}
			default:
				break;
						
		}
		NVIC_EnableIRQ(TIMER0_IRQn);
		alreadyMoved = 1;
	}else{
			if(alreadyMoved == 0) J_upRight=0;
	}
	
	// LEFT-UP
	if((LPC_GPIO1->FIOPIN & (1<<27)) == 0 && (LPC_GPIO1->FIOPIN & (1<<29)) == 0 && jump == 1 && dir_jump == LEFT && mode == 1){		
		J_leftUp++;
		switch(J_leftUp){
			
			case 1:
				NVIC_DisableIRQ(TIMER0_IRQn);
				if((checkPlayerMove((turnPlayer == 1) ? p1_i : p2_i,(turnPlayer == 1) ? (p1_j-2) : (p2_j-2),UP,turnPlayer))
					&& alreadyMoved == 0 ){
						selected_movement = LEFT_UP;
						tmpMove = buildMove(turnPlayer,0,0,(turnPlayer == 1) ? p1_j/2 : p2_j/2,(turnPlayer == 1) ? p1_i/2 : p2_i/2);
						LCD_moveTurnPlayer(selected_movement);
					}
			default:
				break;
						
		}
		NVIC_EnableIRQ(TIMER0_IRQn);
		alreadyMoved = 1;
	}else{
			if(alreadyMoved == 0) J_leftUp=0;
	}
	
	// LEFT-DOWN
	if((LPC_GPIO1->FIOPIN & (1<<27)) == 0 && (LPC_GPIO1->FIOPIN & (1<<26)) == 0 && jump == 1 && dir_jump == LEFT && mode == 1){		
		J_leftDown++;
		switch(J_leftDown){
			NVIC_DisableIRQ(TIMER0_IRQn);
			case 1:
				if((checkPlayerMove((turnPlayer == 1) ? p1_i : p2_i,(turnPlayer == 1) ? (p1_j-2) : (p2_j-2),DOWN,turnPlayer))
					&& alreadyMoved == 0 ){
						selected_movement = LEFT_DOWN;
						tmpMove = buildMove(turnPlayer,0,0,(turnPlayer == 1) ? p1_j/2 : p2_j/2,(turnPlayer == 1) ? p1_i/2 : p2_i/2);
						LCD_moveTurnPlayer(selected_movement);
					}
			default:
				break;
						
		}
		NVIC_EnableIRQ(TIMER0_IRQn);
		alreadyMoved = 1;
	}else{
			if(alreadyMoved == 0) J_leftDown=0;
	}
	
	// RIGHT-UP
	if((LPC_GPIO1->FIOPIN & (1<<28)) == 0 && (LPC_GPIO1->FIOPIN & (1<<29)) == 0 && jump == 1 && dir_jump == RIGHT && mode == 1){		
		J_rightUp++;
		switch(J_rightUp){
			NVIC_DisableIRQ(TIMER0_IRQn);
			case 1:
				if((checkPlayerMove((turnPlayer == 1) ? p1_i : p2_i,(turnPlayer == 1) ? (p1_j+2) : (p2_j+2),UP,turnPlayer))
					&& alreadyMoved == 0 ){
						selected_movement = RIGHT_UP;
						tmpMove = buildMove(turnPlayer,0,0,(turnPlayer == 1) ? p1_j/2 : p2_j/2,(turnPlayer == 1) ? p1_i/2 : p2_i/2);
						LCD_moveTurnPlayer(selected_movement);
					}
			default:
				break;
						
		}
		NVIC_EnableIRQ(TIMER0_IRQn);
		alreadyMoved = 1;
	}else{
			if(alreadyMoved == 0) J_rightUp=0;
	}
	
	// RIGHT-DOWN
	if((LPC_GPIO1->FIOPIN & (1<<28)) == 0 && (LPC_GPIO1->FIOPIN & (1<<26)) == 0 && jump == 1 && dir_jump == RIGHT && mode == 1){		
		J_rightDown++;
		switch(J_rightDown){
			NVIC_DisableIRQ(TIMER0_IRQn);
			case 1:
				if((checkPlayerMove((turnPlayer == 1) ? p1_i : p2_i,(turnPlayer == 1) ? (p1_j+2) : (p2_j+2),DOWN,turnPlayer))
					&& alreadyMoved == 0 ){
						selected_movement = RIGHT_DOWN;
						tmpMove = buildMove(turnPlayer,0,0,(turnPlayer == 1) ? p1_j/2 : p2_j/2,(turnPlayer == 1) ? p1_i/2 : p2_i/2);
						LCD_moveTurnPlayer(selected_movement);
					}
			default:
				break;
						
		}
		NVIC_EnableIRQ(TIMER0_IRQn);
		alreadyMoved = 1;
	}else{
			if(alreadyMoved == 0) J_rightDown=0;
	}
	
	//	JOYSTICK DOWN
	if((LPC_GPIO1->FIOPIN & (1<<26)) == 0){	
		/* Joytick J_Select pressed p1.25*/
		/* Joytick J_Down pressed p1.26 --> using J_DOWN due to emulator issues*/
		
		J_down++;
		switch(J_down){
			case 1:
				selected_movement = DOWN;
				if(menu_mode == -1 && 
					((turnPlayer == myplayer && menu_choice == MULTIPLAYER) 
					|| (menu_choice == SINGLE_PLAYER && menu_choice2 == NPC && turnPlayer == 1)
					|| (menu_choice == SINGLE_PLAYER && menu_choice2 == HUMAN))){
					NVIC_DisableIRQ(TIMER0_IRQn);
					if(mode == 1 
						&& (checkPlayerMove((turnPlayer == 1) ? p1_i : p2_i,(turnPlayer == 1) ? p1_j : p2_j,DOWN,turnPlayer) || jump == 1)
						&& alreadyMoved == 0){
						selected_movement = DOWN;
						
						if(jump == 0 || dir_jump != selected_movement){
							tmpMove = buildMove(turnPlayer,0,0,(turnPlayer == 1) ? p1_j/2 : p2_j/2,(turnPlayer == 1) ? p1_i/2 : p2_i/2);
							LCD_moveTurnPlayer(selected_movement);
							
						}else if(jump == 1 && dir_jump == selected_movement){
							tmpMove = buildMove(turnPlayer,0,0,(turnPlayer == 1) ? p1_j/2 : p2_j/2,(turnPlayer == 1) ? (p1_i+2)/2 : (p2_i+2)/2);
							LCD_moveTurnPlayer(selected_movement);
						}
						
					}else if(mode == 2 && checkWallMove(tmpWI,tmpWJ,selected_movement,turnPlayer)){
						selected_movement = DOWN;
						tmpMove = buildMove(turnPlayer,1,(rotatedWallMode == 1) ? 0 : 1,tmpWI,tmpWJ);
						LCD_moveWall(selected_movement,rotatedWallMode);
					}
					NVIC_EnableIRQ(TIMER0_IRQn);
					alreadyMoved = 1;
				}else if (menu_mode == 1 && menu_choice == SINGLE_PLAYER){
					menu_choice = MULTIPLAYER;
					colorMenuBox(1,Black);
					colorMenuBox(2,Red);
				}else if (menu_mode == 2 && menu_choice2 == HUMAN){
					menu_choice2 = NPC;
					colorMenuBox(1,Black);
					colorMenuBox(2,Red);
				}
				break;
				default:
					break;
		}
	}
	else{
			if(alreadyMoved == 0 || mode == 2) J_down=0;
	}
	
			//	JOYSTICK UP
	if((LPC_GPIO1->FIOPIN & (1<<29)) == 0 ){	
		/* Joytick J_Up pressed p1.29 --> using J_UP due to emulator issues*/
		J_up++;
		//	Debug
		switch(J_up){
			case 1:
				selected_movement = UP;
				if(menu_mode == -1 && 
					((turnPlayer == myplayer && menu_choice == MULTIPLAYER) 
					|| (menu_choice == SINGLE_PLAYER && menu_choice2 == NPC && turnPlayer == 1)
					|| (menu_choice == SINGLE_PLAYER && menu_choice2 == HUMAN))){
					NVIC_DisableIRQ(TIMER0_IRQn);
					if(mode == 1 
						&& (checkPlayerMove((turnPlayer == 1) ? p1_i : p2_i,(turnPlayer == 1) ? p1_j : p2_j,UP,turnPlayer) || jump == 1)
						&& alreadyMoved == 0) {
						selected_movement = UP;
						
						if(jump == 0 || dir_jump != selected_movement){
							tmpMove = buildMove(turnPlayer,0,0,(turnPlayer == 1) ? p1_j/2 : p2_j/2,(turnPlayer == 1) ? p1_i/2 : p2_i/2);
							LCD_moveTurnPlayer(selected_movement);
							
						}else if(jump == 1 && dir_jump == selected_movement){
							
							tmpMove = buildMove(turnPlayer,0,0,(turnPlayer == 1) ? p1_j/2 : p2_j/2,(turnPlayer == 1) ? (p1_i-2)/2 : (p2_i-2)/2);
							LCD_moveTurnPlayer(selected_movement);
						}
						
					}else if(mode == 2 && checkWallMove(tmpWI,tmpWJ,selected_movement,turnPlayer)){
						selected_movement = UP;
						tmpMove = buildMove(turnPlayer,1,(rotatedWallMode == 1) ? 0 : 1,tmpWI,tmpWJ);
						LCD_moveWall(selected_movement,rotatedWallMode);
					}
					NVIC_EnableIRQ(TIMER0_IRQn);
					alreadyMoved = 1;
				}else if (menu_mode == 1 && menu_choice == MULTIPLAYER){
					menu_choice = SINGLE_PLAYER;
					colorMenuBox(2,Black);
					colorMenuBox(1,Red);
				}else if (menu_mode == 2 && menu_choice2 == NPC){
					menu_choice2 = HUMAN;
					colorMenuBox(2,Black);
					colorMenuBox(1,Red);
				}
				break;
			default:
				// Do Nothing
				break;
		}
	}
	else{
			//If you already moved you can't move another time unless it's wallPlacement
			if(alreadyMoved == 0 || mode == 2 )	J_up=0;
	}
	
			//	JOYSTICK LEFT
	if((LPC_GPIO1->FIOPIN & (1<<27)) == 0){	
		/* Joytick J_Left pressed p1.27 --> using J_Left due to emulator issues*/
		J_left++;
		//	Debug
		switch(J_left){
			case 1:
				selected_movement = LEFT;
  			NVIC_DisableIRQ(TIMER0_IRQn);
				if((turnPlayer == myplayer && menu_choice == MULTIPLAYER) 
					|| (menu_choice == SINGLE_PLAYER && menu_choice2 == NPC && turnPlayer == 1)
					|| (menu_choice == SINGLE_PLAYER && menu_choice2 == HUMAN)){
					if(mode == 1 
						&& (checkPlayerMove((turnPlayer == 1) ? p1_i : p2_i,(turnPlayer == 1) ? p1_j : p2_j,LEFT,turnPlayer) || jump == 1) 
						&& alreadyMoved == 0){
						selected_movement = LEFT;
						
						if(jump == 0 || dir_jump != selected_movement){
							tmpMove = buildMove(turnPlayer,0,0,(turnPlayer == 1) ? p1_j/2 : p2_j/2,(turnPlayer == 1) ? p1_i/2 : p2_i/2);
							LCD_moveTurnPlayer(selected_movement);
							
						}else if(jump == 1 && dir_jump == selected_movement){
							
							tmpMove = buildMove(turnPlayer,0,0,(turnPlayer == 1) ? (p1_j-2)/2 : (p2_j-2)/2,(turnPlayer == 1) ? p1_i/2 : p2_i/2);
							LCD_moveTurnPlayer(selected_movement);
						}
						
					}else if(mode == 2 && checkWallMove(tmpWI,tmpWJ,selected_movement,turnPlayer)){
						selected_movement = LEFT;
						tmpMove = buildMove(turnPlayer,1,(rotatedWallMode == 1) ? 0 : 1,tmpWI,tmpWJ);
						LCD_moveWall(selected_movement,rotatedWallMode);
					}
				}
				NVIC_EnableIRQ(TIMER0_IRQn);
				alreadyMoved = 1;
				break;
			default:
				// Do Nothing
				break;
		}
	}
	else{
			//If you already moved you can't move another time unless it's wallPlacement
			if(alreadyMoved == 0 || mode == 2)	J_left=0;
	}
	
			//	JOYSTICK RIGHT
	if((LPC_GPIO1->FIOPIN & (1<<28)) == 0 ){	
		/* Joytick J_right pressed p1.28 --> using J_right due to emulator issues*/
		J_right++;
		//	Debug
		switch(J_right){
			case 1:
  			NVIC_DisableIRQ(TIMER0_IRQn);
			selected_movement = RIGHT;
			if((turnPlayer == myplayer && menu_choice == MULTIPLAYER) 
					|| (menu_choice == SINGLE_PLAYER && menu_choice2 == NPC && turnPlayer == 1)
					|| (menu_choice == SINGLE_PLAYER && menu_choice2 == HUMAN)){
					if(mode == 1 
						&& (checkPlayerMove((turnPlayer == 1) ? p1_i : p2_i,(turnPlayer == 1) ? p1_j : p2_j,RIGHT,turnPlayer) || jump == 1)
						&& alreadyMoved == 0){
						selected_movement = RIGHT;
						
						if(jump == 0 || dir_jump != selected_movement){
							tmpMove = buildMove(turnPlayer,0,0,(turnPlayer == 1) ? p1_j/2 : p2_j/2,(turnPlayer == 1) ? p1_i/2 : p2_i/2);
							LCD_moveTurnPlayer(selected_movement);
							
						}else if(jump == 1 && dir_jump == selected_movement){
							
							tmpMove = buildMove(turnPlayer,0,0,(turnPlayer == 1) ? (p1_j+2)/2 : (p2_j+2)/2,(turnPlayer == 1) ? p1_i/2 : p2_i/2);
							LCD_moveTurnPlayer(selected_movement);
						}
						
					}else if(mode == 2 && checkWallMove(tmpWI,tmpWJ,selected_movement,turnPlayer)){
						selected_movement = RIGHT;
						tmpMove = buildMove(turnPlayer,1,(rotatedWallMode == 1) ? 0 : 1,tmpWI,tmpWJ);
						LCD_moveWall(selected_movement,rotatedWallMode);
					}
				}
				NVIC_EnableIRQ(TIMER0_IRQn);
				alreadyMoved = 1;
				break;
			default:
				// Do Nothing
				break;
		}
	}
	else{
			//If you already moved you can't move another time unless it's wallPlacement
			if(alreadyMoved == 0 || mode == 2)	J_right=0;
	}
	
	
	// SELECT
	if((LPC_GPIO1->FIOPIN & (1<<25)) == 0){	
		/* Joytick J_select pressed p1.25 --> using J_UP due to emulator issues*/
		J_select++;
		//	Debug
   		switch(J_select){
			case 1:
					// if we press select without moving we pass the turn
			if(menu_mode == (-1)){
				NVIC_DisableIRQ(TIMER0_IRQn); 
				if(alreadyMoved != 0){
					if(mode == 1){
						confirmPlayerMove();
					}else if(mode == 2){
						if(checkWallOverlay(tmpWI,tmpWJ,rotatedWallMode)){
							confirmWallPlacement();
						}else{
							LCD_placeWall(tmpWI,tmpWJ,Yellow,rotatedWallMode);
							errorMessage("Can't overlay walls!");
							LCD_placeWall(2,3,Red,rotatedWallMode);
							tmpWI = 2;	tmpWJ = 3;
						}
					}
				}else{
					reset_timer_flag = 0;
				}
			}else if(menu_mode == 1){
				if(menu_choice == SINGLE_PLAYER){
					menu(2);
				}else if(menu_choice == MULTIPLAYER){
					LCD_Clear(White);
					GUI_Text(13, 253, (uint8_t *) "waiting for opponent...", Black, White);	
					enable_timer(1);
					// Send Handshake ACK
					CAN_TxMsg.len = 0;
					CAN_TxMsg.id = CAN_HANDSHAKE_ID;
					CAN_TxMsg.format = DATA_FRAME;
					CAN_wrMsg(CAN_CONTROLLER_FOR_GAME, &CAN_TxMsg);
					menu(2);
				}
			}else if (menu_mode == 2){
				if(menu_choice == SINGLE_PLAYER){
					if(menu_choice2 == HUMAN){
						startGame();
						menu_mode = -1;
					}else if(menu_choice2 == NPC){
						menu_mode = -1;
						startGame();
						
					}
				}else if (menu_choice == MULTIPLAYER){
					if(menu_choice2 == HUMAN){
						GUI_Text(13, 253, (uint8_t *) "starting...", Black, White);	
						// Send Handshake ACK
						CAN_TxMsg.len = 0;
						CAN_TxMsg.id = CAN_HANDSHAKE_ACK_ID;
						CAN_TxMsg.format = DATA_FRAME;
						CAN_wrMsg(CAN_CONTROLLER_FOR_GAME, &CAN_TxMsg);
						menu_mode = -1;
					}else  if(menu_choice2 == NPC){

						GUI_Text(13, 253, (uint8_t *) "starting...", Black, White);	
						// Send Handshake ACK
						CAN_TxMsg.len = 0;
						CAN_TxMsg.id = CAN_HANDSHAKE_ACK_ID;
						CAN_TxMsg.format = DATA_FRAME;
						CAN_wrMsg(CAN_CONTROLLER_FOR_GAME, &CAN_TxMsg);
						menu_mode = -1;
						//if i'm playing against a bot i'm player 1 
						if(menu_choice == SINGLE_PLAYER  ) myplayer = 1;
						if (myplayer==1) flag_start =1;
					}
				}
			}
			default:
				// Do Nothing
				break;
		}
		NVIC_EnableIRQ(TIMER0_IRQn);
	}
	else{
			J_select=0;
	}
	

	
	
/* button management KEY1*/
 	if(down!=0 && (selected_movement == (-1) || mode == 2) && menu_mode == (-1)){ 
		if((LPC_GPIO2->FIOPIN & (1<<11)) == 0){	/* KEY1 pressed */
			down++;				
			switch(down){
				case 2:		
					// If KEY1 is pressed while in wallPlacementMode we should go back to playerMove mode
					if(mode !=2){
						NVIC_DisableIRQ(TIMER0_IRQn);
						wallPlacementMode();
						NVIC_EnableIRQ(TIMER0_IRQn);
					}else{
						NVIC_DisableIRQ(TIMER0_IRQn);
						LCD_placeWall(tmpWI,tmpWJ,Yellow,rotatedWallMode);
						disableWallPlacementMode();
						alreadyMoved = 0;
						NVIC_EnableIRQ(TIMER0_IRQn);				
					}
					break;
					
				default:
					break;
			}
		}
		else {	/* button released */
			down=0;			
			NVIC_EnableIRQ(EINT1_IRQn);							 /* enable Button interrupts			*/
			LPC_PINCON->PINSEL4    |= (1 << 22);     /* External interrupt 0 pin selection */
		}
	}
	// KEY2 Button Management
	if(down2!=0 && mode == 2 && menu_mode == (-1)){  
			down2 ++;  
		if((LPC_GPIO2->FIOPIN & (1<<12)) == 0){
			switch(down2){
			case 2:
				NVIC_DisableIRQ(TIMER0_IRQn);
			  LCD_placeWall(tmpWI,tmpWJ,Yellow,rotatedWallMode);
				rotatedWallMode = (rotatedWallMode == 0) ? 1 : 0;
				LCD_placeWall(tmpWI,tmpWJ,Red,rotatedWallMode);
				NVIC_EnableIRQ(TIMER0_IRQn);
				break;
			default:			
				// Do Nothing
				break;
			}
		}
		else {	/* button released */
			down2=0;			
			NVIC_EnableIRQ(EINT2_IRQn); 							 /* enable Button interrupts			*/
			LPC_PINCON->PINSEL4    |= (1 << 24);     /* External interrupt 2 pin selection */
		}
	}
	
  LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
	
  return;

}
void LCD_moveTurnPlayer(int dir){
	conv(tmpMove.x*2,tmpMove.y*2);
	LCD_ColorSquare(playerX,playerY,Yellow, 30);
	switch(dir){
		case DOWN:
			// Update temporary new player position in both LCD and matrix variable
			HighLight(playerX,playerY,Yellow,turnPlayer);
			tmpMove.y = tmpMove.y +1 ; 
			tmpY+=(SQUARE_DISTANCE);
			break;
		case UP:
			
			// Update temporary new player position in both LCD and matrix variable
			
			HighLight(playerX,playerY,Yellow,turnPlayer);
			tmpMove.y = tmpMove.y -1 ; 
			tmpY-=(SQUARE_DISTANCE);
			break;
		case RIGHT:
			
			// Update temporary new player position in both LCD and matrix variable
			HighLight(playerX,playerY,Yellow,turnPlayer);
			tmpMove.x = tmpMove.x +1 ; 
			tmpX+=(SQUARE_DISTANCE);
			break;
		case LEFT:
			
			// Update temporary new player position in both LCD and matrix variable
			
			HighLight(playerX,playerY,Yellow,turnPlayer);
			tmpMove.x = tmpMove.x -1; 
			tmpX-=(SQUARE_DISTANCE);
			break;
		case DOWN_RIGHT:
			HighLight(playerX,playerY,Yellow,turnPlayer);
			tmpMove.y = tmpMove.y +1 ; 
			tmpMove.x = tmpMove.x +1 ; 
			tmpY+=(SQUARE_DISTANCE);
			tmpX+=(SQUARE_DISTANCE);
			break;
		case DOWN_LEFT:
			HighLight(playerX,playerY,Yellow,turnPlayer);
			tmpMove.y = tmpMove.y +1 ; 
			tmpMove.x = tmpMove.x -1 ; 
			tmpY+=(SQUARE_DISTANCE);
			tmpX-=(SQUARE_DISTANCE);
			break;
		case UP_RIGHT:
			HighLight(playerX,playerY,Yellow,turnPlayer);
			tmpMove.y = tmpMove.y -1 ; 
			tmpY-=(SQUARE_DISTANCE);
			tmpMove.x = tmpMove.x +1 ; 
			tmpX+=(SQUARE_DISTANCE);
			break;
		case UP_LEFT:
			HighLight(playerX,playerY,Yellow,turnPlayer);
			tmpMove.y = tmpMove.y -1 ; 
			tmpY-=(SQUARE_DISTANCE);
			tmpMove.x = tmpMove.x -1 ; 
			tmpX-=(SQUARE_DISTANCE);
			break;
		case LEFT_DOWN:
			HighLight(playerX,playerY,Yellow,turnPlayer);
			tmpMove.x = tmpMove.x -1 ; 
			tmpX-=(SQUARE_DISTANCE);
			tmpMove.y = tmpMove.y +1 ; 
			tmpY+=(SQUARE_DISTANCE);
			break;
		case LEFT_UP:
			HighLight(playerX,playerY,Yellow,turnPlayer);
			tmpMove.x = tmpMove.x -1 ; 
			tmpX-=(SQUARE_DISTANCE);
			tmpMove.y = tmpMove.y -1 ; 
			tmpY-=(SQUARE_DISTANCE);
			break;
		case RIGHT_DOWN:
			HighLight(playerX,playerY,Yellow,turnPlayer);
			tmpMove.x = tmpMove.x +1 ; 
			tmpX+=(SQUARE_DISTANCE);
			tmpMove.y = tmpMove.y +1 ; 
			tmpY+=(SQUARE_DISTANCE);
			break;
		case RIGHT_UP:
			HighLight(playerX,playerY,Yellow,turnPlayer);
			tmpMove.x = tmpMove.x +1 ; 
			tmpX+=(SQUARE_DISTANCE);
			tmpMove.y = tmpMove.y -1 ; 
			tmpY-=(SQUARE_DISTANCE);
			break;
	}
	LCD_ColorSquare(tmpX,tmpY,(turnPlayer == 1)? Red : White, 30);
}

move_t buildMove(int player, int op, int dir, int x, int y){
	move_t move;
	move.player = player;	
	move.op = op;
	move.dir = dir;
	move.x = x;
	move.y = y;
	return move;
}
/*
	make a wall move on the LCD and update the tmp variable with wall coordinate
*/
void LCD_moveWall(int dir, int rotated){
	switch(dir){
		case DOWN:
			LCD_placeWall(tmpWI,tmpWJ,Yellow,rotated);
			tmpWI++;
			LCD_placeWall(tmpWI,tmpWJ,Red,rotated);
			break;
		case UP:
			LCD_placeWall(tmpWI,tmpWJ,Yellow,rotated);
			tmpWI--;
			LCD_placeWall(tmpWI,tmpWJ,Red,rotated);
			break;
		case RIGHT:
			LCD_placeWall(tmpWI,tmpWJ,Yellow,rotated);
			tmpWJ++;
			LCD_placeWall(tmpWI,tmpWJ,Red,rotated);
			break;
		case LEFT:
			LCD_placeWall(tmpWI,tmpWJ,Yellow,rotated);
			tmpWJ--;
			LCD_placeWall(tmpWI,tmpWJ,Red,rotated);
			break;
	}
}
/*
	Send a player move over the CAN bus 
*/
void sendMove(move_t move){
	int x,y,op,dir,playerID;
	x =move.x; y = move.y; op = move.op; dir = move.dir; playerID = move.player;
	CAN_TxMsg.data[1] = (move.x & 0xF0)>>4;	//High part x
	CAN_TxMsg.data[0] = move.x & 0x0F;	// Low part x
	CAN_TxMsg.data[3] = (move.y & 0xF0)>>4;	//High part y
	CAN_TxMsg.data[2] = move.y & 0x0F;	// Low part y
	CAN_TxMsg.data[4] =  move.dir;
	CAN_TxMsg.data[5] = move.op;
	CAN_TxMsg.data[6] = move.player & 0x0F;	// Low part player
	CAN_TxMsg.data[7] = (move.player & 0xF0)>>4;	// High part player
	CAN_TxMsg.len = 8;
	CAN_TxMsg.id = move.player;
	CAN_TxMsg.format = DATA_FRAME;
	// Send from CAN 1 to CAN 2
	CAN_wrMsg(CAN_CONTROLLER_FOR_GAME, &CAN_TxMsg);
	
}

void confirmWallPlacement(){
	placeWallMatrix(tmpWI,tmpWJ,rotatedWallMode);
	if(!checkTrap(1) || !checkTrap(2)){
		undoWallPlacing(tmpWI,tmpWJ,rotatedWallMode);
		LCD_placeWall(tmpWI,tmpWJ,Yellow,rotatedWallMode);
		errorMessage("Can't trap a player!");
		LCD_placeWall(2,3,Red,rotatedWallMode);
		tmpWI = 2;	tmpWJ = 3;
		return;
	}	
	decreaseWallCounter(turnPlayer);
	tmpMove = buildMove(turnPlayer,1,(rotatedWallMode == 0) ? 1 : 0,tmpWI,tmpWJ);
	if(menu_choice == MULTIPLAYER && tmpMove.player == myplayer){
		flag_start = 0;
		tmpMove.op = 1;
		tmpMove.dir = (rotatedWallMode == 0)? 1 : 0;
		sendMove(tmpMove);
	}
	wI = tmpWI; wJ = tmpWJ;
	choice_confirmed = 1;
	reset_timer_flag = 1;
	rotatedWallMode = 0;
}

void confirmPlayerMove(){
	makePlayerMove(tmpMove.y*2,tmpMove.x*2,tmpMove.player);
	if(tmpMove.player == 1 && (tmpMove.y*2) == 12) win(1);
	if(tmpMove.player == 2 && tmpMove.y == 0) win(2);
	playerX = tmpX; playerY = tmpY;
	// Here we should send in Multiplayer mode the move
	// sendMove() 
	if(menu_choice == MULTIPLAYER && turnPlayer == myplayer){
		flag_start = 0;
		tmpMove.op = 0;
		tmpMove.dir = 0;
		sendMove(tmpMove);
	}
	choice_confirmed = 1;
	reset_timer_flag = 1;
}
/*
	Given a recieved messagge from the CAN controller, decode it into a move_t data and make 
	the move on the screen
*/
void decodeAndMakeMove(CAN_msg CAN_RxMsg){
	int x,y,player;
	NVIC_DisableIRQ(TIMER0_IRQn);
	x = (CAN_RxMsg.data[1]<<4) | CAN_RxMsg.data[0];
	y = (CAN_RxMsg.data[3]<<4) | CAN_RxMsg.data[2];
	player = (CAN_RxMsg.data[7]<<4) | CAN_RxMsg.data[6];
	tmpMove = buildMove(player,CAN_RxMsg.data[5],CAN_RxMsg.data[4], x,y);
	//no move done
	if(tmpMove.op == 0 && tmpMove.dir == 1){
		choice_confirmed = 1;
		reset_timer_flag = 1;
		return;
	}
	if( tmpMove.op == 0){	//player movement 
		// LCD player movement
		conv(tmpMove.x*2,tmpMove.y*2);
		LCD_ColorSquare(playerX,playerY,Yellow, 30);
		HighLight(playerX,playerY,Yellow,turnPlayer);
		LCD_ColorSquare(tmpX,tmpY,(turnPlayer == 1)? Red : White, 30);
		//	Internal Logic player movement
		confirmPlayerMove();
		NVIC_EnableIRQ(TIMER0_IRQn);
	}else{
		// wall Move
		tmpWI = tmpMove.x;
		tmpWJ = tmpMove.y;
		LCD_placeWall(tmpWI,tmpWJ,Red,(tmpMove.dir == 0) ? 1 : 0);
		rotatedWallMode = (tmpMove.dir == 0) ? 1 : 0;
		confirmWallPlacement();
		NVIC_EnableIRQ(TIMER0_IRQn);
	}
}
void activateNPC(){
	
	if(menu_choice2 == NPC && menu_choice == MULTIPLAYER){
		tmpMove = findBestMove(myplayer);
	}
	if(menu_choice2 == NPC && menu_choice == SINGLE_PLAYER){
		tmpMove = findBestMove(2);
	}
}
void NPC_Move(){
	NVIC_DisableIRQ(TIMER0_IRQn);
		alreadyMoved = 1;
		//Updates tmpMove with next move
		activateNPC();
		if(tmpMove.op == 0){
			HighLight(playerX,playerY,Yellow,turnPlayer);
			LCD_ColorSquare(playerX,playerY,Yellow, 30);
			conv(tmpMove.x*2,tmpMove.y*2);
			LCD_ColorSquare(tmpX,tmpY,(turnPlayer == 1)? Red : White, 30);
			confirmPlayerMove();
		}
		if(tmpMove.op == 1){
			tmpWI = tmpMove.x;
			tmpWJ = tmpMove.y;
			LCD_placeWall(tmpWI,tmpWJ,Red,(tmpMove.dir == 0)? 1 : 0);
			rotatedWallMode = (tmpMove.dir == 0)? 1 : 0;
			confirmWallPlacement();
		}
		NVIC_EnableIRQ(TIMER0_IRQn);
}


/******************************************************************************
**                            End Of File
******************************************************************************/
