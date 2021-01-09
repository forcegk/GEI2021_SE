/*----------------------------------------------------------------------------
 * Name : tt2.c
 * Model: FRDM KL46Z
 * Info : Trabajo Tutelado 2
 *----------------------------------------------------------------------------*/

#include "helper.h"
#include "slcd.h"
#include "RTL.h"

#define DEFAULT_TIMEOUT 10
#define ANGLE_ARRAY_LEN 10

#define FLAG_ANGLE 1UL
#define FLAG_TAKEOVER 1UL<<1
#define FLAG_ACK 1UL<<2


typedef uint8_t angle_match_type;
#define ANGLE_NANGL 0x0
#define ANGLE_COMPL 0x1
#define ANGLE_SUPPL 0x2


// -----------------------------------------------------------------------------
// Tipos
// -----------------------------------------------------------------------------

typedef struct _angle {
	uint16_t deg;
	uint8_t min;
	uint8_t sec;
} angle;


// -----------------------------------------------------------------------------
// Variables globales
// -----------------------------------------------------------------------------

volatile uint8_t sw1 = 0, sw2 = 0;
OS_TID t_5, t_4, t_3, t_2, t_1;

const angle angles1[ANGLE_ARRAY_LEN] = {
	{45, 0, 0}, // compl
	{90, 0, 0}, // supl
	{91, 0, 0}, // nada
  {45, 30, 0}, // complementario si suma minutos
	{90, 35, 0}, // suplementario si suma minutos
	{45, 30, 30}, // complementario si suma minutos y segundos
	{90, 35, 35}, // suplementario si suma minutos y segundos
	{90, 0, 0}, // agregamos un par de suplementarios para diferenciarlos
	{90, 0, 0}, // ...
	{0, 30, 45} // para usar al menos una vez la condicion de guarda...
};

const angle angles2[ANGLE_ARRAY_LEN] = {
	{45, 0, 0},
	{90, 0, 0},
	{90, 0, 0},
  {44, 30, 0},
	{89, 25, 0},
	{44, 29, 30},
	{89, 24, 25},
	{90, 0, 0},
	{90, 0, 0},
	{0, 0, 0}
};


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

angle sum_angle(const angle angle1, const angle angle2){
	// Precondición: Ambos ángulos están normalizados, es decir:
	//   0 <= deg < 360
	//   0 <= min < 60
	//   0 <= sec < 60
	
	angle angle_ret;
	
	angle_ret.deg = angle1.deg + angle2.deg;
	angle_ret.min = angle1.min + angle2.min;
	angle_ret.sec = angle1.sec + angle2.sec;
	
	// Ajustamos minutos con segundos > 60
	angle_ret.min += angle_ret.sec>=60?1:0;
	angle_ret.sec -= angle_ret.sec>=60?60:0;
	
	// Ajustamos grados con minutos > 60
	angle_ret.deg += angle_ret.min>=60?1:0;
	angle_ret.min -= angle_ret.min>=60?60:0;
	
	// Ajustamos grados a una sola vuelta
	angle_ret.deg -= angle_ret.deg>=360?360:0;
	
	return angle_ret;
}

angle_match_type compare_angles(const angle angle1, const angle angle2){
	angle angle_tmp = sum_angle(angle1, angle2);
	
	// guard conditions
	if(angle_tmp.min != 0) return ANGLE_NANGL;
	if(angle_tmp.sec != 0) return ANGLE_NANGL;
	
	switch(angle_tmp.deg){
		case 90:
			return ANGLE_COMPL;
		case 180:
			return ANGLE_SUPPL;
		default:
			return ANGLE_NANGL;
	}
}

// -----------------------------------------------------------------------------
// Tasks
// -----------------------------------------------------------------------------
__task void task5(void){
	//os_evt_wait_or(FLAG_ACK, 0xFFFF);
	os_tsk_delete_self();
}

__task void task4(void){
	//os_evt_wait_or(FLAG_ACK, 0xFFFF);
	os_tsk_delete_self();
}

__task void task3(void){
	uint16_t count = 0, ret_flags;
	setLedG(1);
	
	while(!sw1){
		if(os_evt_wait_or(FLAG_ANGLE, DEFAULT_TIMEOUT) == OS_R_EVT){
			ret_flags = os_evt_get();
		
			if(ret_flags & FLAG_ANGLE){
				count++;
				slcdDisplay(count, BASE10);
				
				// Send ACK to Task1
				os_evt_set(FLAG_ACK, t_1);
			}
		}	
	}
	sw1 =0; sw2 = 0;
	slcdDisplay(0xdead, BASE16);
	os_tsk_delete_self();
}

__task void task2(void){
	uint16_t count = 0, ret_flags;
	setLedG(1);
	
	while(!sw1){
		if(os_evt_wait_or(FLAG_ANGLE, DEFAULT_TIMEOUT) == OS_R_EVT){
			ret_flags = os_evt_get();
		
			if(ret_flags & FLAG_ANGLE){
				count++;
				slcdDisplay(count, BASE10);
				
				// Send ACK to Task1
				os_evt_set(FLAG_ACK, t_1);
			}
		}	
	}
	sw1 =0; sw2 = 0;
	slcdDisplay(0xdead, BASE16);
	os_tsk_delete_self();
}

__task void task1(void){
	uint16_t i;
	angle_match_type result;
	OS_TID tid;
	
	for(i = 0; i<ANGLE_ARRAY_LEN; i++){
		result = compare_angles(angles1[i], angles2[i]);
		
		switch(result){
			case ANGLE_COMPL:
				tid = t_2;
				break;
			case ANGLE_SUPPL:
				tid = t_3;
				break;
			case ANGLE_NANGL:
				tid = t_4;
				break; // se que no es necesario siendo el último, pero confío en que
			  // el compilador optimizará esto, y por mantenibilidad lo pongo siempre
			  // para evitar problemas, por si alguien añadiese casos y se olvidase
				// de introducirlo.
		}
		
		if(tid == t_2){
			os_evt_set(FLAG_ANGLE, tid); // Seteamos el evento a la tarea correspondiente
			// Y esperamos a que se haya "recibido"
			os_evt_wait_and(FLAG_ACK, 0xFFFF);
		}
	}
	
	os_tsk_delete_self(); // Finalmente cuando hayamos terminado, simplemente salimos
}

__task void init(void){
	t_5 = os_tsk_create(task5, 1);
	t_4 = os_tsk_create(task4, 1);
	t_3 = os_tsk_create(task3, 1);
	t_2 = os_tsk_create(task2, 1);
	t_1 = os_tsk_create(task1, 1);
	os_tsk_delete_self();
}

// -----------------------------------------------------------------------------
// Programa principal
// -----------------------------------------------------------------------------
int main (void) {
	// Inicializar rutinas de interrupción
	initSwitch1(sw1_pressed);
	initSwitch3(sw2_pressed);
	
	// Inicializar sLcd
	slcdInitialize();
	
	setLedG(0);
	setLedR(0);
	
	os_sys_init(init);
}
