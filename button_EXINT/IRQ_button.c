#include "button.h"
#include "lpc17xx.h"
#include "../timer/timer.h"
#include "../CAN/CAN.h"
extern int down;
extern int down2;
extern void startGame();
extern void menu();

extern int handshake;

void EINT0_IRQHandler (void)	  	/* INT0														 */
{		
	NVIC_DisableIRQ(EINT0_IRQn);
	menu(1);
	LPC_SC->EXTINT &= (1 << 0);     /* clear pending interrupt         */
}


void EINT1_IRQHandler (void)	  	/* KEY1														 */
{
	NVIC_DisableIRQ(EINT1_IRQn);		/* disable Button interrupts			 */
	LPC_PINCON->PINSEL4    &= ~(1 << 22);     /* GPIO pin selection */
	down=1;
	LPC_SC->EXTINT &= (1 << 1);     /* clear pending interrupt         */
}

void EINT2_IRQHandler (void)	  	/* KEY2														 */
{	
	down2 = 1;
	NVIC_DisableIRQ(EINT2_IRQn);
	LPC_PINCON->PINSEL4    &= ~(1 << 24);     /* GPIO pin selection */
  LPC_SC->EXTINT &= (1 << 2);     /* clear pending interrupt         */    
}


