/*----------------------------------------------------------------------------
 * Name : helper.h
 * Model: FRDM KL46Z
 * Info : Modulo auxiliar para gestionar el hardware en la practica 5
 *----------------------------------------------------------------------------*/
 
#ifndef __HELPER_H__
#define __HELPER_H__
 
// -----------------------------------------------------------------------------
// Controla el estado del led rojo
// -----------------------------------------------------------------------------
void setLedR(int state);

// -----------------------------------------------------------------------------
// Controla el estado del led verde
// -----------------------------------------------------------------------------
void setLedG(int state);

// -----------------------------------------------------------------------------
// Instala un manejador (puntero a funcion) que controla el switch SW1
// -----------------------------------------------------------------------------
void initSwitch1( void(*handler) );

// -----------------------------------------------------------------------------
// Instala un manejador (puntero a funcion) que controla el switch SW3
// -----------------------------------------------------------------------------
void initSwitch3( void(*handler) );

// -----------------------------------------------------------------------------
// Devuelve el numero de ticks desde la primera llamada a clock() o wait()
// -----------------------------------------------------------------------------
unsigned long long msClock(void);

// -----------------------------------------------------------------------------
// Detiene la ejecucion en un bucle activo hasta que se alcanza delay
// -----------------------------------------------------------------------------
void msDelay(unsigned int delay);

#endif // __HELPER_H__

