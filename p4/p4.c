/*----------------------------------------------------------------------------
 * Name : p4.c
 * Model: FRDM KL46Z
 * Info : Practica 4
 *----------------------------------------------------------------------------*/

#include "helper.h"

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
	

	// Bucle principal de ejecucion
	while(1) {
		if(sw1 || sw2){
			setLedG(0);
			setLedR(1);
		}else{
			setLedG(1);
			setLedR(0);
		}
	}
	
}
