/*----------------------------------------------------------------------------
 * Name : helper.c
 * Model: FRDM KL46Z
 * Info : Modulo auxiliar para gestionar el hardware en la practica 5
 *----------------------------------------------------------------------------*/
 
// Cabecera correspondiente al modulo actual
#include "helper.h"
 
// Cabecera con definiciones para acceder facilmente a los recursos de la placa
#include <MKL46Z4.H>

// -----------------------------------------------------------------------------
// Internal: Inicializa el led en el puerto especificado
// -----------------------------------------------------------------------------
int initLed(const char letra, const unsigned int puerto) {
	// Lista de mascaras para activar los relojes de los puertos
	const unsigned int clockMask[] = { SIM_SCGC5_PORTA_MASK,
		SIM_SCGC5_PORTB_MASK, SIM_SCGC5_PORTC_MASK,
		SIM_SCGC5_PORTD_MASK, SIM_SCGC5_PORTE_MASK };

	// Lista con las direcciones de memoria de cada uno de los puertos
	PORT_Type* const portAddr[] = { PORTA, PORTB, PORTC, PORTD, PORTE };	
		
	// Lista con las direcciones para FPGIO para registros de entrada/salida
	FGPIO_Type* const fgpioAddr[] = { FPTA, FPTB, FPTC, FPTD, FPTE };

	// Calculamos la mascara asociada al puerto
	const unsigned int ledMask = 1 << puerto;
	
	// Convertimos la letra en numero empezando en A=0
	const char letraVal = letra - 'A';
	if(letra < 'A' || letra > 'E') return -1;
	
	// Verificamos que el puerto esta dentro del rango permitido
	if(puerto > 31) return -1;
	
	// Activa el reloj del puerto, por defecto desactivado para reducir el consumo
	SIM->SCGC5 |= clockMask[letraVal];
		
	// Establece el multiplexor del pin y puerto deseado en modo GPIO (leds en 1)
	portAddr[letraVal]->PCR[puerto] = PORT_PCR_MUX(1);

	// Habilitamos el puerto como salida PDOR (Port Data Output Register)
	fgpioAddr[letraVal]->PDOR = ledMask;
	
	// Seleccionamos direccion salida en puerto PDDR (Port Data Direction Register)
	fgpioAddr[letraVal]->PDDR = ledMask;
	
	// Retornamos '0' para indicar que no ocurrio ningun error
	return 0;
}

// -----------------------------------------------------------------------------
// Internal: Conmuta el led en el puerto especificado usando el estado 'state'
// -----------------------------------------------------------------------------
int setLed(const char letra, const char puerto, const char state) {
	// Lista con las direcciones para FPGIO para registros de entrada/salida
	FGPIO_Type* const fgpioAddr[] = { FPTA, FPTB, FPTC, FPTD, FPTE };
	
	// Obtenemos el puerto FPGIO en funcion de la letra empezando en A=0
	FGPIO_Type* const fgpio = fgpioAddr[letra - 'A'];

	// Calculamos la mascara asociada al puerto
	const unsigned int ledMask = 1 << puerto;
	
	// Si state es 0 entonces encendemos usando PSOR (Port Set Output Register)	
	if(!state) fgpio->PSOR = ledMask;
	// En otro caso apagamos usando PCOR (Port Clear Output Register)
	else fgpio->PCOR = ledMask;
	
	// Retornamos '0' para indicar que no ocurrio ningun error
	return 0;
}

// -----------------------------------------------------------------------------
// Controla el estado del led rojo
// -----------------------------------------------------------------------------
void setLedR(int state) {
	const char letter = 'E', number = 29; // PTE29
	static int init = 0;
	if(!init) {
		init = 1;
		initLed(letter, number);
	}		
	setLed(letter, number, state);
}

// -----------------------------------------------------------------------------
// Controla el estado del led verde
// -----------------------------------------------------------------------------
void setLedG(int state) {
	const char letter = 'D', number = 5; // PTD5
	static int init = 0;
	if(!init) {
		init = 1;
		initLed(letter, number);
	}		
	setLed(letter, number, state);
}

// Tipo de dato que tiene que tener el puntero a los manejadores de excepcion
typedef void(*irqHandler)();

// Tabla con todos los manejadores de excepciones (static inicializa a 0)
static irqHandler irqHandlers[5][32];

// -----------------------------------------------------------------------------
// Internal: Instala un manejador para la interruptcion deseada
// -----------------------------------------------------------------------------
int initInterrupt(const char letra, const unsigned int puerto, irqHandler handler) {
	// Lista de mascaras para activar los relojes de los puertos
	const unsigned int clockMask[] = { SIM_SCGC5_PORTA_MASK,
		SIM_SCGC5_PORTB_MASK, SIM_SCGC5_PORTC_MASK,
		SIM_SCGC5_PORTD_MASK, SIM_SCGC5_PORTE_MASK };

	// Lista con las direcciones de memoria de cada uno de los puertos
	PORT_Type* const portAddr[] = { PORTA, PORTB, PORTC, PORTD, PORTE };	
		
	// Lista con las direcciones para FPGIO para registros de entrada/salida
	FGPIO_Type* const fgpioAddr[] = { FPTA, FPTB, FPTC, FPTD, FPTE };

	// Numero de interrupcion para cada uno de los puertos
	const enum IRQn portIrq[] = { PORTA_IRQn, DMA0_IRQn, PORTC_PORTD_IRQn,
		PORTC_PORTD_IRQn, DMA0_IRQn };
	
	// Calculamos la mascara asociada al puerto
	const unsigned int switchMask = 1 << puerto;
	
	// Convertimos la letra en numero empezando en A=0
	const char letraVal = letra < 'A' || letra > 'E' ? 0 : letra - 'A';

	// Calculamos la interrupcion asociada al puerto
	const enum IRQn irq = portIrq[letraVal];
	if(!irq) return -1;
	
	// Verificamos que el puerto esta dentro del rango permitido
	if(puerto > 31) return -1;
	if(letra < 'A' || letra > 'E') return -1;

	// Activa el reloj del puerto, por defecto desactivado para reducir el consumo
	SIM->SCGC5 |= clockMask[letraVal];
		
	// Establece el multiplexor del pin y puerto deseado en modo GPIO
	portAddr[letraVal]->PCR[puerto] = PORT_PCR_MUX(1);
	
	// En el mismo puerto activamos la resistencia de pull-up interna
	// para evitar lecturas incorrectas (PE:Pull Enable, PS:Pull Up)
	portAddr[letraVal]->PCR[puerto] |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;

	// En el mismo puerto activamos las interruptiones al soltar presionar
	// IRQC(9) = raising, IRQC(10) = falling, IRQC(11) = either, disabled(0)
	portAddr[letraVal]->PCR[puerto] |= PORT_PCR_IRQC(10);

	// Seleccionamos direccion entrada en puerto PDDR (Port Data Direction Register)
	fgpioAddr[letraVal]->PDDR &= switchMask;

	// Establecemos el manejador de excepcion 'handler', que puede ser NULL
	irqHandlers[letraVal][puerto] = handler;

	// Establecemos una prioridad baja para la interrupcion (por defecto 0)
	NVIC_SetPriority(irq, 128);
	
	// Limpiamos cualquier interrupcion anterior que quedara pendiente
	NVIC_ClearPendingIRQ(irq);
	
	// Activamos las interrupciones
	NVIC_EnableIRQ(irq);
	
	// Retornamos '0' para indicar que no ocurrio ningun error
	return 0;

}

// -----------------------------------------------------------------------------
// Internal: Manejador de excepciones para el puerto A
// Simplemente definimos un simbolo de 'Vector Table' (fichero startup_XXXX.s)
// -----------------------------------------------------------------------------
void PORTA_IRQHandler() {
	int i; // Variable para iterar en los bucles
	
	// Limpiamos las excepciones pendientes en el puerto correspondiente
	NVIC_ClearPendingIRQ(PORTA_IRQn);

	// Iteramos sobre cada uno de los componentes del puerto
	for(i = 0; i < 32; i++)
		// Comprobamos si la excepcion 'i' esta activada o no
		if(PORTA->ISFR & (1 << i) ) {
			// En caso de que dicha excepcion tenga un manejador lo lanzamos
			if(irqHandlers[0][i]) irqHandlers[0][i]();
		}

		// Limpiamos todos los puerto en ISFR (Interrupt Status Flag Register)
	PORTA->ISFR = 0xffffffff;
}

// -----------------------------------------------------------------------------
// Internal: Manejador de excepciones para el puerto D
// Simplemente definimos el simbolo de 'Vector Table' (fichero startup_XXXX.s)
// -----------------------------------------------------------------------------
void PORTD_IRQHandler() {
	int i; // Variable para iterar en los bucles
	
	// Limpiamos las excepciones pendientes en el puerto correspondiente
	NVIC_ClearPendingIRQ(PORTC_PORTD_IRQn);
	
	// Si hay excepciones pendientes en el puerto C
	if(PORTC->ISFR) {
		// Iteramos sobre cada uno de los componentes del puerto C
		for(i = 0; i < 32; i++)
			// Comprobamos si la excepcion 'i' esta activada o no
			if(PORTC->ISFR & (1 << i) ) {
				// En caso de que dicha excepcion tenga un manejador lo lanzamos
				if(irqHandlers[2][i]) irqHandlers[2][i]();
			}

		// Limpiamos el ISFR (Interrupt Status Flag Register) del puerto
		PORTC->ISFR = 0xffffffff;
	}

	// Si hay excepciones pendientes en el puerto D
	if(PORTD->ISFR) {
		// Iteramos sobre cada uno de los componentes del puerto C
		for(i = 0; i < 32; i++)
			// Comprobamos si la excepcion 'i' esta activada o no
			if(PORTD->ISFR & (1 << i) ) {
				// En caso de que dicha excepcion tenga un manejador lo lanzamos
				if(irqHandlers[3][i]) irqHandlers[3][i]();
			}

		// Limpiamos el ISFR (Interrupt Status Flag Register) del puerto
		PORTD->ISFR = 0xffffffff;
	}
	
}

// -----------------------------------------------------------------------------
// Instala un manejador que controla el switch SW1
// -----------------------------------------------------------------------------
void initSwitch1( void(*handler) ) { initInterrupt('C', 3, (irqHandler)handler); }

// -----------------------------------------------------------------------------
// Instala un manejador que controla el switch SW3
// -----------------------------------------------------------------------------
void initSwitch3( void(*handler) ) { initInterrupt('C', 12, (irqHandler)handler); }


// Variable global para almacenar el numero de ticks en milisegundos
volatile static unsigned long long msTicks = 0;

// -----------------------------------------------------------------------------
// Devuelve el numero de ticks desde la primera llamada a clock() o wait()
// -----------------------------------------------------------------------------
unsigned long long msClock(void) {
	static int init = 0;
	
	// Si se inicializo el contador de tics devuelve el valor correspondiente
	if(init) return msTicks; else init++;
	
	// Actualiza SystemCoreClock leyendo la frecuencia de reloj de un registro interno
  SystemCoreClockUpdate();
	
	// Genera interrupciones periodicas en los intervalos de tiempo deseados
	// ticksPerUsec(SystemCoreClock) / 1000 usecsPerMsec = 1 ms
  SysTick_Config(SystemCoreClock / 1000);
	
	return msTicks;
}

// -----------------------------------------------------------------------------
// Detiene la ejecucion en un bucle activo hasta que se alcanza msDelay
// -----------------------------------------------------------------------------
void msDelay(unsigned int delay) {
  unsigned long long initTicks = msClock();
  while(msClock() - initTicks < delay);
}

// -----------------------------------------------------------------------------
// Internal: Instala un manejador para el contador de ticks
// Simplemente definimos un simbolo de 'Vector Table' (fichero startup_XXXX.s)
// -----------------------------------------------------------------------------
void SysTick_Handler(void) {
	msTicks++;
}


