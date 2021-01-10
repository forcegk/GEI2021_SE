/*----------------------------------------------------------------------------
 *      
 *----------------------------------------------------------------------------
 *      Name:    slcd.c
 *     
 *----------------------------------------------------------------------------
 *      
 *---------------------------------------------------------------------------*/

#include "slcd.h"												//Declarations

/*----------------------------------------------------------------------------
  Function that initializes sLCD
 *----------------------------------------------------------------------------*/
void slcdInitialize(void){
	
	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTC_MASK | /* Enable Clock to Port B & C */ 
								SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK | SIM_SCGC5_SLCD_MASK; /* Enable Clock to Port D & E and sLCD module*/ 
	LCD->GCR |= LCD_GCR_PADSAFE_MASK; // Set PADSAFE during configuration
	LCD->GCR &= ~LCD_GCR_LCDEN_MASK; // Clear LCDEN during configuration
	//multiplexers configuration for ports to act as sLCD controller
	
	//Configure pins *From Reference manual, set pins to MUX 0 for normal LCD display operation, only use MUX 7 if using LCD fault detection
	PORTB->PCR[7]  = PORT_PCR_MUX(0u);					//Set PTB7 to LCD_P7
	PORTB->PCR[8]  = PORT_PCR_MUX(0u);					//Set PTB8 to LCD_P8
	PORTB->PCR[10] = PORT_PCR_MUX(0u);					//Set PTB10 to LCD_P10
	PORTB->PCR[11] = PORT_PCR_MUX(0u);					//Set PTB11 to LCD_P11
	PORTB->PCR[21] = PORT_PCR_MUX(0u);					//Set PTB21 to LCD_P17
	PORTB->PCR[22] = PORT_PCR_MUX(0u);					//Set PTB22 to LCD_P18
	PORTB->PCR[23] = PORT_PCR_MUX(0u);					//Set PTB23 to LCD_P19
	PORTC->PCR[17] = PORT_PCR_MUX(0u);					//Set PTC17 to LCD_P37
	PORTC->PCR[18] = PORT_PCR_MUX(0u);					//Set PTC18 to LCD_P38
	PORTD->PCR[0]  = PORT_PCR_MUX(0u);					//Set PTD0 to LCD_P40
	PORTE->PCR[4]  = PORT_PCR_MUX(0u);					//Set PTE4 to LCD_P52
	PORTE->PCR[5]  = PORT_PCR_MUX(0u);					//Set PTE5 to LCD_P53
	
	//sLCD register configuration
	LCD->GCR = LCD_GCR_RVTRIM(0x00) | LCD_GCR_CPSEL_MASK | LCD_GCR_LADJ(0x03) |
						 LCD_GCR_VSUPPLY_MASK | LCD_GCR_ALTDIV(0x00) |
						 LCD_GCR_SOURCE_MASK | LCD_GCR_LCLK(0x01) | LCD_GCR_DUTY(0x03);
	//sLCD blinking configuration
	LCD->AR = LCD_AR_BRATE(0x03);	
	//FDCR register configuration
	LCD->FDCR = 0x00000000;
  //activation of 12 pins to control sLCD (2 registers, 32 bits each)
	LCD->PEN[0] = LCD_PEN_PEN(1u<<7) | // LCD_P7
								LCD_PEN_PEN(1u<<8) | // LCD_P8
								LCD_PEN_PEN(1u<<10) | // LCD_P10 
								LCD_PEN_PEN(1u<<11) | // LCD_P11
								LCD_PEN_PEN(1u<<17) | // LCD_P17
								LCD_PEN_PEN(1u<<18) | // LCD_P18
								LCD_PEN_PEN(1u<<19); // LCD_P19
	LCD->PEN[1] = LCD_PEN_PEN(1u<<5) | // LCD_P37
								LCD_PEN_PEN(1u<<6) | // LCD_P38
								LCD_PEN_PEN(1u<<8) | // LCD_P40
								LCD_PEN_PEN(1u<<20) | // LCD_P52
								LCD_PEN_PEN(1u<<21); // LCD_P53
	//configuration of 4 back plane pins (2 registers, 32 bits each)
	LCD->BPEN[0] = LCD_BPEN_BPEN(1u<<18) | //LCD_P18
								 LCD_BPEN_BPEN(1u<<19); //LCD_P19
	LCD->BPEN[1] = LCD_BPEN_BPEN(1u<<8) | //LCD_P40
								 LCD_BPEN_BPEN(1u<<20); //LCD_P52

	// waveform registers configuration – 4 active (because of 4 back planes)
	// (44.3.7 in KL46 Reference Manual)
	LCD->WF[4] =  LCD_WF_WF16(0x00) | LCD_WF_WF17(0x00) |
							  LCD_WF_WF18(0x88) | // COM3 (10001000)
							  LCD_WF_WF19(0x44);  // COM2 (01000100)
	LCD->WF[10] = LCD_WF_WF40(0x11) | // COM0 (00010001)
								LCD_WF_WF41(0x00) | LCD_WF_WF42(0x00) | LCD_WF_WF43(0x00);
	
	LCD->WF[13] = LCD_WF_WF52(0x22) | // COM1 (00100010)
								LCD_WF_WF53(0x00) | LCD_WF_WF54(0x00) | LCD_WF_WF55(0x00);
	
	LCD->GCR &= ~LCD_GCR_PADSAFE_MASK; // Clear PADSAFE at the end of configuration
	LCD->GCR |= LCD_GCR_LCDEN_MASK; // Set LCDEN at the end of configuration
}
/*----------------------------------------------------------------------------
  Function that outputs on sLCD (number) error message for debug purpose
 *----------------------------------------------------------------------------*/
void slcdErr(uint8_t number){
	LCD->WF8B[LCD_Front_Pin[0]] = (LCD_S_G | LCD_S_E | LCD_S_D | LCD_S_F); //1st digit
	LCD->WF8B[LCD_Front_Pin[1]] = (LCD_S_A);
	
	LCD->WF8B[LCD_Front_Pin[2]] = (LCD_S_E | LCD_S_G); //2nd digit
	LCD->WF8B[LCD_Front_Pin[3]] = (LCD_C);
	
	LCD->WF8B[LCD_Front_Pin[4]] = (LCD_S_E | LCD_S_G); //3rd digit
	LCD->WF8B[LCD_Front_Pin[5]] = (LCD_C);
	
	switch(number){  //4 digit depends on provided number
		case 0x00:
			LCD->WF8B[LCD_Front_Pin[6]] = (LCD_S_D | LCD_S_E | LCD_S_F); /* To display '0' we must active segments: d, e & f on first front plane */
			LCD->WF8B[LCD_Front_Pin[7]] = (LCD_S_A | LCD_S_B | LCD_S_C); /* To display '0' we must active segments: a, b & c on second front plane */
			break;
		case 0x01:
			LCD->WF8B[LCD_Front_Pin[6]] = (LCD_S_B | LCD_S_C);
			break;
		case 0x02:
			LCD->WF8B[LCD_Front_Pin[6]] = (LCD_S_G | LCD_S_E | LCD_S_D );
			LCD->WF8B[LCD_Front_Pin[7]] = (LCD_S_A | LCD_S_B);
			break;
		case 0x03:
			LCD->WF8B[LCD_Front_Pin[6]] = (LCD_S_G | LCD_S_D);
			LCD->WF8B[LCD_Front_Pin[7]] = (LCD_S_A | LCD_S_B | LCD_S_C);
			break;
		case 0x04:
			LCD->WF8B[LCD_Front_Pin[6]] = (LCD_S_G | LCD_S_F);
			LCD->WF8B[LCD_Front_Pin[7]] = (LCD_S_B | LCD_S_C);
			break;
		default:   //as default display 'r'
			LCD->WF8B[LCD_Front_Pin[6]] = (LCD_S_E | LCD_S_G);
			LCD->WF8B[LCD_Front_Pin[7]] = (LCD_C);
			break;
	}
}
/*----------------------------------------------------------------------------
  Writing on sLCD (value) on specified position (digit)
*
* FP0 - Front Plane 0
*  f |_g
*  e |_
*    d
*
* FP1 - Front Plane 1
*    a_
*      | b
*    c |.
*        dot
 *----------------------------------------------------------------------------*/
void slcdSet(uint8_t value, uint8_t digit){	

	switch(value){
		case 0x00:
			LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_D | LCD_S_E | LCD_S_F); /* To display '0' we must active segments: d, e & f on first front plane */
			LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A | LCD_S_B | LCD_S_C); /* To display '0' we must active segments: a, b & c on second front plane */
			break;
		case 0x01:
			LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = 0; // si no lo ponemos queda lo anterior
			LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_B | LCD_S_C);
			break;
		case 0x02:
			LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_G | LCD_S_E | LCD_S_D );
			LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A | LCD_S_B);
			break;
		case 0x03:
			LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_G | LCD_S_D);
			LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A | LCD_S_B | LCD_S_C);
			break;
		case 0x04:
			LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_G | LCD_S_F);
			LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_B | LCD_S_C);
			break;
		case 0x05:
			LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_G | LCD_S_F | LCD_S_D);
			LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A | LCD_S_C);
			break;
		case 0x06:
			LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_G | LCD_S_E | LCD_S_D  | LCD_S_F);
			LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A | LCD_S_C);
			break;
		case 0x07:
			LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_C);
			LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A | LCD_S_B | LCD_S_C);
			break;
		case 0x08:
			LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_G | LCD_S_E | LCD_S_D  | LCD_S_F);
			LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A | LCD_S_B | LCD_S_C);
			break;
		case 0x09:
			LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_F | LCD_S_G | LCD_S_D);
			LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A | LCD_S_B | LCD_S_C);
			break;
		case 0x0A:
			LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_F | LCD_S_E | LCD_S_G);
			LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A | LCD_S_B | LCD_S_C);
			break;
		case 0x0B:
			LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_G | LCD_S_E | LCD_S_D | LCD_S_F);
			LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_C);
			break;
		case 0x0C:
			LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_F | LCD_S_E | LCD_S_D);
			LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A);
			break;
		case 0x0D:
			LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_G | LCD_S_E | LCD_S_D);
			LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_B | LCD_S_C);
			break;
		case 0x0E:
			LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_G | LCD_S_E | LCD_S_D | LCD_S_F);
			LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A);
			break;
		case 0x0F:
			LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_F | LCD_S_E | LCD_S_G);
			LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A);
			break;
		default:   //as default display 'r'
			LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_E | LCD_S_G);
			LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_C);
			break;
	}
	if(digit>4){ 
		slcdErr(1);
	}
}
/*----------------------------------------------------------------------------
  Function for conversion purpose, displaying (value) in specified (format)
 *----------------------------------------------------------------------------*/
void slcdDisplay(uint16_t value ,uint16_t format){	
	uint8_t i = 0;
  uint16_t temp_value = value;
	
	if(format < 2 || format > 16){
		slcdSet(0xe, 1);
		slcdSet(0xFF, 2);
		slcdSet(0xFF, 3);
	}

	for(i=4; i >= 1; i--){
		slcdSet(temp_value%format, i);
		temp_value = temp_value / format;
	}
}
/*----------------------------------------------------------------------------
  Clear sLCD
 *----------------------------------------------------------------------------*/
void slcdClear(void){
	uint8_t i = 0;
	
	for(i=0; i < 8; i++){
		LCD->WF8B[LCD_Front_Pin[i]] = LCD_C;
	}
}
/*----------------------------------------------------------------------------
  Function for demo purpose
 *----------------------------------------------------------------------------*/
void slcdDemo(void){	
	uint8_t i;
	
	for(i=0; i < 0x10; i++){
		slcdClear();
		slcdSet(i, i%4 + 1);
		msDelay(300);
		slcdClear();
	}
}

/*----------------------------------------------------------------------------
	Student-added function:
	Enable dot at digit position
 *----------------------------------------------------------------------------*/
void slcdEnableDot(uint8_t digit){	
	if(digit>4 || digit<1){ 
		slcdErr(1);
	}
	LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] |= LCD_S_DEC; /* Enable decimal dot */
}


void slcdSetChar(char c, uint8_t digit){

	switch(c){
		case 'O':
			LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_D | LCD_S_E | LCD_S_F);
			LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A | LCD_S_B | LCD_S_C);
			break;
		case '2':
			LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_G | LCD_S_E | LCD_S_D );
			LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A | LCD_S_B);
			break;
		case 'S':
			LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_G | LCD_S_F | LCD_S_D);
			LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A | LCD_S_C);
			break;
		case 'J':
			LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_E | LCD_S_D);
			LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_B | LCD_S_C);
			break;
		case 'A':
			LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_F | LCD_S_E | LCD_S_G);
			LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A | LCD_S_B | LCD_S_C);
			break;
		default:   //as default display 'r'
			LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_E | LCD_S_G);
			LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_C);
			break;
	}
	if(digit>4){ 
		slcdErr(1);
	}
}
