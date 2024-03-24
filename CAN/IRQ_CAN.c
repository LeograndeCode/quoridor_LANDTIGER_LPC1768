/*----------------------------------------------------------------------------
 * Name:    Can.c
 * Purpose: CAN interface for for LPC17xx with MCB1700
 * Note(s): see also http://www.port.de/engl/canprod/sv_req_form.html
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2009 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <lpc17xx.h>                  /* LPC17xx definitions */
#include "CAN.h"                      /* LPC17xx CAN adaption layer */
#include "../GLCD/GLCD.h"
#include "../quoridor/quoridor.h"

extern uint8_t icr ; 										//icr and result must be global in order to work with both real and simulated landtiger.
extern uint32_t result;
extern CAN_msg       CAN_TxMsg;    /* CAN message for sending */
extern CAN_msg       CAN_RxMsg;    /* CAN message for receiving */                                

static int puntiRicevuti1 = 0;
static int puntiInviati1 = 0;

static int puntiRicevuti2 = 0;
static int puntiInviati2 = 0;
enum{SINGLE_PLAYER,MULTIPLAYER,HUMAN,NPC};

volatile int handshake = 0;
volatile int handshake_successful = 0;
volatile int sent_flag = 0;
extern void decodeAndMakeMove(CAN_msg CAN_RxMsg);
extern int myplayer;
extern int menu_mode;
extern int flag_start;
extern void NPC_Move();
extern int menu_choice2;
extern int turnPlayer;
/*----------------------------------------------------------------------------
  CAN interrupt handler
 *----------------------------------------------------------------------------*/
void CAN_IRQHandler (void) {
	if(CAN_CONTROLLER_FOR_GAME == 1){
		/* check CAN controller 1 */
		icr = 0;
		icr = (LPC_CAN1->ICR | icr) & 0xFF;               /* clear interrupts */
		if (icr & (1 << 0)) {                          		/* CAN Controller #1 meassage is received */
			uint8_t str;
			CAN_rdMsg(1,&CAN_RxMsg);
			LPC_CAN1->CMR = (1 << 2);
			if(CAN_RxMsg.id == CAN_HANDSHAKE_ID){
				// Debug
				GUI_Text(13, 253, (uint8_t *) "Arrivato primo handshake", Black, White);		
				handshake_successful = 1;	// No longer have to wait for handshake, there's someone linked to me
				GUI_Text(13, 253, (uint8_t *) "waiting for opponent...", White, White);
			}
			// We can procede with the game
			if(CAN_RxMsg.id == CAN_HANDSHAKE_ACK_ID){
				// Debug
				while(menu_mode != -1);
				GUI_Text(13, 253, (uint8_t *) "Arrivato primo handshake", Black, White);	
				GUI_Text(13, 253, (uint8_t *) "Arrivato secondo handshake", Black, White);
				startGame();
				flag_start = 1;
			}
			// FROM CAN2 to CAN1 player 2 move
			if(CAN_RxMsg.id == 2){
				//GUI_Text(50,100,(uint8_t *) "Ricevuta mossa da player 1",Red,Yellow);
				// Receive the move
				decodeAndMakeMove(CAN_RxMsg);
				NVIC_EnableIRQ(TIMER0_IRQn);
			}
		}
		else
			if (icr & (1 << 1)) {                         /* CAN Controller #1 meassage is transmitted */
				// try this at lab
			}
	}
	else if(CAN_CONTROLLER_FOR_GAME == 2){
		/* check CAN controller 2 */
			
		icr = 0;
		icr = (LPC_CAN2->ICR | icr) & 0xFF;             /* clear interrupts */

		if (icr & (1 << 0)) {                          	/* CAN Controller #2 meassage is received */
			uint8_t str;
			CAN_rdMsg(2,&CAN_RxMsg);
			LPC_CAN2->CMR = (1 << 2);
			if(CAN_RxMsg.id == CAN_HANDSHAKE_ID){
				// Debug
				GUI_Text(13, 253, (uint8_t *) "Arrivato primo handshake", Black, White);		
				handshake_successful = 1;	// No longer have to wait for handshake, there's someone linked to me	
				GUI_Text(13, 253, (uint8_t *) "waiting for opponent...", White, White);
			}
			if(CAN_RxMsg.id == CAN_HANDSHAKE_ACK_ID){
				// Debug
				while(menu_mode != -1);
				GUI_Text(13, 253, (uint8_t *) "Arrivato primo handshake", Black, White);	
				GUI_Text(13, 253, (uint8_t *) "Arrivato secondo handshake", Black, White);
				startGame();
				flag_start = 1;
			}
			// FROM CAN1 to CAN2 player 1 move
			if(CAN_RxMsg.id == 1){
				//GUI_Text(50,100,(uint8_t *) "Ricevuta mossa da player 1",Red,Yellow);
				// Receive the move
				decodeAndMakeMove(CAN_RxMsg);
				NVIC_EnableIRQ(TIMER0_IRQn);
			}
		}
		else{
			if (icr & (1 << 1)) {                         /* CAN Controller #2 meassage is transmitted */
				// try this at lab
			}
		}
	}
}
