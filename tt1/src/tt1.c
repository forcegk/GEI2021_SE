/*----------------------------------------------------------------------------
 * Name : tt1.c
 * Model: FRDM KL46Z
 * Info : Trabajo Tutelado 1
 *----------------------------------------------------------------------------*/

#include "helper.h"
#include "slcd.h"

// -----------------------------------------------------------------------------
// Binary Macros
// -----------------------------------------------------------------------------

#define B_0000    0
#define B_0001    1
#define B_0010    2
#define B_0011    3
#define B_0100    4
#define B_0101    5
#define B_0110    6
#define B_0111    7
#define B_1000    8
#define B_1001    9
#define B_1010    a
#define B_1011    b
#define B_1100    c
#define B_1101    d
#define B_1110    e
#define B_1111    f

#define _B2H(bits)    B_##bits
#define B2H(bits)    _B2H(bits)
#define _HEX(n)        0x##n
#define HEX(n)        _HEX(n)
#define _CCAT(a,b)    a##b
#define CCAT(a,b)   _CCAT(a,b)

#define U4L(a)								HEX( CCAT(B2H(0000),B2H(a)) )
#define U4H(a)								HEX( CCAT(B2H(a),B2H(0000)) )
#define U8(a,b)								HEX( CCAT(B2H(a),B2H(b)) )
#define U16(a,b,c,d)					HEX( CCAT(CCAT(B2H(a),B2H(b)),CCAT(B2H(c),B2H(d))) )
#define U32(a,b,c,d,e,f,g,h)	HEX( CCAT(CCAT(CCAT(B2H(a),B2H(b)),CCAT(B2H(c),B2H(d))),CCAT(CCAT(B2H(e),B2H(f)),CCAT(B2H(g),B2H(h)))) )


// -----------------------------------------------------------------------------
// Tipos
// -----------------------------------------------------------------------------

typedef const struct _state {
	const uint8_t output;	// Estructura bit-wise: tzyx dcba
												//		a: LED verde (1=on, 0=off)
												//    b: LED rojo encendido ('')
	const uint8_t led_time;  		// Tiempo en ms / 100 de encendido del LED (es decir, poner un 1 aquí hace esperar 100ms)
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

#define LED_GREEN U4L(0001)
#define LED_RED U4L(0010)


state fsm[] = {
	{0, 0, 0, 0, {s0, s1p, s1r}},										// s0
	
	{LED_GREEN, 30, 10, 0, {s0, s0, s2r}},					// s1r
	{LED_GREEN, 30, 10, 0, {s0, s0, s3r}},					// s2r
	{LED_GREEN, 30, 10, 0, {fsO, s0, s0}},					// s3r
	
	{LED_GREEN, 10, 10, 0, {s0, s2p, s1p1r}},				// s1p
	{LED_GREEN, 30, 10, 0, {fsA, s0, s1p2r}},				// s1p1r
	{LED_GREEN, 30, 10, 0, {s0, s0, s1p3r}},				// s1p2r
	{LED_GREEN, 30, 10, 0, {fsJ, s0, s0}},					// s1p3r
	
	{LED_GREEN, 10, 10, 0, {s0, s3p, s2p1r}},				// s2p
	{LED_GREEN, 30, 10, 0, {s0, s0, s2p2r}},				// s2p1r
	{LED_GREEN, 30, 10, 0, {s0, s0, s2p3r}},				// s2p2r
	{LED_GREEN, 30, 10, 0, {fs2, s0, s0}},					// s2p3r
	
	{LED_GREEN, 10, 10, 0, {fsS, s0, s0}},					// s3p
	
	{LED_RED, 30, 0, 'O', {s0, s0, s0}},						// fsO
	{LED_RED, 30, 0, 'J', {s0, s0, s0}},						// fsJ
	{LED_RED, 30, 0, '2', {s0, s0, s0}},						// fs2
	{LED_RED, 30, 0, 'A', {s0, s0, s0}},						// fsA
	{LED_RED, 30, 0, 'S', {s0, s0, s0}}							// fsS
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
// Funciones auxiliares
// -----------------------------------------------------------------------------

void ms_switch_conditionated_delay(unsigned int delay) {
  unsigned long long initTicks = msClock();
  while(msClock() - initTicks < delay){
		if(sw1 || sw2) return;
	};
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
		msDelay(100 * (int) fsm_state->led_time);
		
		setLedG(0);
		setLedR(0);
		
		// Y esperamos a la entrada
		msDelay(100 * (int) fsm_state->delay_time);
		
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
