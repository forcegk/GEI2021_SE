/*----------------------------------------------------------------------------
 * Name : p4.c
 * Model: FRDM KL46Z
 * Info : Practica 4
 *----------------------------------------------------------------------------*/

#include "helper.h"
#include "slcd.h"

#define _PARTE_OPCIONAL_

// -----------------------------------------------------------------------------
// Variables globales
// -----------------------------------------------------------------------------

volatile int sw1 = 0, sw2 = 0; // Casi prefiero curarme en salud haciendolo volátil,
															 //  que miedo me dan los compiladores

// -----------------------------------------------------------------------------
// Rutinas de interrupción
// -----------------------------------------------------------------------------

void sw1_pressed(){
	sw1 = !sw1;
}

void sw2_pressed(){
	sw2 = !sw2;
}


// -----------------------------------------------------------------------------
// Programa principal
// -----------------------------------------------------------------------------

int main (void) {
	
	// Inicializar rutinas de interrupción
	initSwitch1(sw1_pressed);
	initSwitch3(sw2_pressed);
	
	#ifdef _PARTE_OPCIONAL_
	slcdInitialize();
	slcdSet(1, 1);
	slcdSet(2, 3);
	slcdEnableDot(1);
	slcdEnableDot(3);
	#endif
	

	// Bucle principal de ejecucion
	while(1) {
		if(sw1 || sw2){
			setLedG(0);
			setLedR(1);
		}else{
			setLedG(1);
			setLedR(0);
		}
		
		#ifdef _PARTE_OPCIONAL_
		if(sw1) slcdSet(0, 2);
		else		slcdSet(0xc, 2);
		
		if(sw2) slcdSet(0, 4);
		else		slcdSet(0xc, 4);
		#endif
	}
	
}
