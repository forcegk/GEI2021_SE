/*----------------------------------------------------------------------------
 * Name : PR05.c
 * Model: FRDM KL46Z
 * Info : Practica 5
 *----------------------------------------------------------------------------*/
 
 #include "helper.h"
 #include "slcd.h"	
 
// -----------------------------------------------------------------------------
// Programa principal
// -----------------------------------------------------------------------------

int main (void) {
	
	// LEDs Inicialmente apagados
	setLedG(0); setLedR(0);
	slcdInitialize();

	// Bucle principal de ejecucion
	while(1) {
		slcdDemo();
		setLedG(1);			// Enciende el LED verde
		msDelay(500);		// Espera medio segundo
		setLedG(0);			// Apage el LED verde
			
		setLedR(1);			// Enciende el LED rojo
		msDelay(500);		// Espera medio segundo
		setLedR(0);			// Apaga el LED rojo
	}
	
}
