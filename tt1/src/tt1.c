/*----------------------------------------------------------------------------
 * Name : tt1.c
 * Model: FRDM KL46Z
 * Info : Trabajo Tutelado 1
 *----------------------------------------------------------------------------*/

#include "helper.h"
#include "slcd.h"

#define TICKLEN 750 // ms

// -----------------------------------------------------------------------------
// Máquina de estados
// -----------------------------------------------------------------------------

typedef const struct _state {
	const uint8_t output;	// Estructura bit-wise: tzyx dcba
												//		a: LED verde (1=on, 0=off)
												//    b: LED rojo encendido ('')
	const uint8_t led_time;  		// Tiempo en ticks. Este valor será multiplicado por TICKLEN ms (definido arriba)
	const uint8_t delay_time;		// Idem pero para el retardo despues del encendido del LED
	
	const char character;
	
	/* {no_input, dot, line} */
	const struct _state *next[3];
} state;


#define s0		&fsm[0]

#define s1r		&fsm[1]
#define s2r		&fsm[2]
#define s3r		&fsm[3]

#define s1p		&fsm[4]
#define s1p1r	&fsm[5]
#define s1p2r	&fsm[6]
#define s1p3r	&fsm[7]

#define s2p		&fsm[8]
#define s2p1r	&fsm[9]
#define s2p2r	&fsm[10]
#define s2p3r	&fsm[11]

#define s3p		&fsm[12]

#define fsO		&fsm[13]
#define fsJ		&fsm[14]
#define fs2		&fsm[15]
#define fsA		&fsm[16]
#define fsS		&fsm[17]

#define LED_GREEN 0x1
#define LED_RED 0x1<<1


state fsm[] = {
/*{output, led_time, delay_time, character, *next[]}*/
	{0, 0, 0, 0, {s0, s1p, s1r}},										// s0
	
	{LED_GREEN, 3, 1, 0, {s0, s0, s2r}},						// s1r
	{LED_GREEN, 3, 1, 0, {s0, s0, s3r}},						// s2r
	{LED_GREEN, 3, 1, 0, {fsO, s0, s0}},						// s3r
	
	{LED_GREEN, 1, 1, 0, {s0, s2p, s1p1r}},					// s1p
	{LED_GREEN, 3, 1, 0, {fsA, s0, s1p2r}},					// s1p1r
	{LED_GREEN, 3, 1, 0, {s0, s0, s1p3r}},					// s1p2r
	{LED_GREEN, 3, 1, 0, {fsJ, s0, s0}},						// s1p3r
	
	{LED_GREEN, 1, 1, 0, {s0, s3p, s2p1r}},					// s2p
	{LED_GREEN, 3, 1, 0, {s0, s0, s2p2r}},					// s2p1r
	{LED_GREEN, 3, 1, 0, {s0, s0, s2p3r}},					// s2p2r
	{LED_GREEN, 3, 1, 0, {fs2, s0, s0}},						// s2p3r
	
	{LED_GREEN, 1, 1, 0, {fsS, s0, s0}},						// s3p
	
	{LED_RED, 3, 0, 'O', {s0, s0, s0}},							// fsO
	{LED_RED, 3, 0, 'J', {s0, s0, s0}},							// fsJ
	{LED_RED, 3, 0, '2', {s0, s0, s0}},							// fs2
	{LED_RED, 3, 0, 'A', {s0, s0, s0}},							// fsA
	{LED_RED, 3, 0, 'S', {s0, s0, s0}}							// fsS
};

// -----------------------------------------------------------------------------
// Variables globales
// -----------------------------------------------------------------------------

volatile uint8_t sw1 = 0, sw2 = 0;


// -----------------------------------------------------------------------------
// Rutinas de interrupción
// -----------------------------------------------------------------------------

void sw1_pressed(){
	sw1 = 1;
}

void sw2_pressed(){
	sw2 = 1;
}


// -----------------------------------------------------------------------------
// Programa principal
// -----------------------------------------------------------------------------

int main (void) {
	
	// Poner el estado inicial de la fsm
	state * fsm_state = s0;
	
	// Inicializar rutinas de interrupción
	initSwitch1(sw1_pressed);
	initSwitch3(sw2_pressed);
	
	// Inicializar el LCD
	slcdInitialize();

	// Bucle principal de ejecucion
	while(1) {
		
		setLedG(fsm_state->output & LED_GREEN);
		setLedR(fsm_state->output & LED_RED);	
		
		if(fsm_state->character){
			// Por el momento mostramos los dos ultimos caracteres
			slcdSetChar(fsm_state->character, 1);
		}
		
		// Al finalizar todo hacemos el delay correspondiente
		msDelay(TICKLEN * fsm_state->led_time);
		
		setLedG(0);
		setLedR(0);
		
		// Y esperamos a la entrada
		msDelay(TICKLEN * fsm_state->delay_time);
		
		if(sw1){
			fsm_state = fsm_state->next[1];
		}else if(sw2){
			fsm_state = fsm_state->next[2];
		}else{
			fsm_state = fsm_state->next[0];
		}
		
		sw1 = 0;
		sw2 = 0;
		
	}
	
}
