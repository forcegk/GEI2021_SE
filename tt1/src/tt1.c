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
	uint8_t output;
	uint8_t time;
	/*{no_input, dot, line}*/
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


state fsm[] = {
	{U4L(0000), 100, {s0, s1r, s1p}}/*,
	{},
	{},
	{},
	{},
	{},
	{},
	{},
	{},
	{},*/
};

// -----------------------------------------------------------------------------
// Variables globales
// -----------------------------------------------------------------------------

volatile int sw1 = 0, sw2 = U8(0000,0000);

// -----------------------------------------------------------------------------
// Rutinas de interrupción
// -----------------------------------------------------------------------------



// -----------------------------------------------------------------------------
// Programa principal
// -----------------------------------------------------------------------------

int main (void) {
	
	// Inicializar rutinas de interrupción
	//initSwitch1(sw1_pressed);
	//initSwitch3(sw2_pressed);
	
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
