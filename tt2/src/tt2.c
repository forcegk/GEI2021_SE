/*----------------------------------------------------------------------------
 * Name : tt2.c
 * Model: FRDM KL46Z
 * Info : Trabajo Tutelado 2
 *----------------------------------------------------------------------------*/

#include "helper.h"
#include "slcd.h"
#include "RTL.h"

#define DEFAULT_WAIT_TIME 10
#define ANGLE_ARRAY_LEN 10

// Defines de los flags
#define FLAG_ANGLE_NANGL 1UL
#define FLAG_ANGLE_COMPL 1UL<<1
#define FLAG_ANGLE_SUPPL 1UL<<2
const uint16_t flags_by_task[3] = {FLAG_ANGLE_COMPL, FLAG_ANGLE_SUPPL, FLAG_ANGLE_NANGL};


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
OS_MUT mut_1;

const angle angles1[ANGLE_ARRAY_LEN] = {
	{45, 0, 0}, // compl
	{90, 0, 0}, // supl
	{91, 0, 0}, // nada
  {45, 30, 0}, // complementario si suma minutos
	{90, 35, 0}, // suplementario si suma minutos
	{45, 30, 30}, // complementario si suma minutos y segundos
	{90, 35, 35}, // suplementario si suma minutos y segundos
	{131, 0, 0}, // agregamos un par de suplementarios para diferenciarlos
	{47, 59, 59}, // ...
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
	{49, 0, 0},
	{132, 0, 1},
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

void clear_phy(void){
	setLedG(0); setLedR(0);
	sw1 = 0; sw2 = 0;
}

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
__task void task_5(void){
	
	while(os_evt_wait_and(1, 0xFFFF) != OS_R_EVT){
		os_dly_wait(DEFAULT_WAIT_TIME);
	}
	
	os_mut_wait(mut_1, 0xFFFF); // No es necesario, pero por si acaso, no hace
															// daño a nadie
	slcdClear();
	setLedG(1);
	setLedR(1);
	
	while(!sw1 && !sw2){
		os_dly_wait(DEFAULT_WAIT_TIME);
	}
	
	clear_phy();
	
	os_mut_release(mut_1);

	os_tsk_delete_self();
}

__task void task_generic(void* argv){
	uint16_t count = 0;
	
	#define my_flag (*(uint16_t*) argv)
	//uint16_t my_flag = *(uint16_t*) argv;
	
	while(1){
		if(os_evt_wait_or(my_flag, 0xFFFF) == OS_R_EVT){
			
			os_mut_wait(mut_1, 0xFFFF);
			
			count++;
			slcdDisplay(count, 10);
			
			switch(my_flag){
				case FLAG_ANGLE_COMPL:
					setLedG(1);
					while(!sw1){os_dly_wait(DEFAULT_WAIT_TIME);}
					break;
				case FLAG_ANGLE_SUPPL:
					setLedR(1);
					while(!sw1){os_dly_wait(DEFAULT_WAIT_TIME);}
					break;
				case FLAG_ANGLE_NANGL:
					while(!sw1){
						setLedG(0);
						setLedR(1);
						os_dly_wait(DEFAULT_WAIT_TIME);
						setLedG(1);
						setLedR(0);
						os_dly_wait(DEFAULT_WAIT_TIME);
					}
					break;
			}

			clear_phy();
			os_evt_set(my_flag, t_1);
			
			os_mut_release(mut_1);
		}
	}
	#undef my_flag
}

__task void task_1(void){
	uint16_t i, curr_flag, tasks_waiting=0UL;
	angle_match_type result;
	OS_TID curr_tid;
	
	for(i = 0; i<ANGLE_ARRAY_LEN; i++){
		result = compare_angles(angles1[i], angles2[i]);
		
		switch(result){
			case ANGLE_COMPL:
				curr_flag = FLAG_ANGLE_COMPL;
				curr_tid = t_2;
				break;
			case ANGLE_SUPPL:
				curr_flag = FLAG_ANGLE_SUPPL;
				curr_tid = t_3;
				break;
			case ANGLE_NANGL:
				curr_flag = FLAG_ANGLE_NANGL;
				curr_tid = t_4;
				break;
		}
		
		if(tasks_waiting & curr_flag){
			os_evt_wait_and(curr_flag, 0xFFFF);
		}else{
			tasks_waiting |= curr_flag;
		}
		
		os_evt_set(curr_flag, curr_tid);
	}
	
	os_evt_wait_and(tasks_waiting, 0xFFFF);
	os_tsk_delete(t_2);
	os_tsk_delete(t_3);
	os_tsk_delete(t_4);
	os_evt_set(1, t_5);
	
	os_tsk_delete_self(); // Finalmente cuando hayamos terminado, simplemente salimos
}

__task void init(void){
	os_mut_init(mut_1);
	t_5 = os_tsk_create(task_5, 1);
	t_4 = os_tsk_create_ex(task_generic, 1, (void*) &flags_by_task[2]);
	t_3 = os_tsk_create_ex(task_generic, 1, (void*) &flags_by_task[1]);
	t_2 = os_tsk_create_ex(task_generic, 1, (void*) &flags_by_task[0]);
	t_1 = os_tsk_create(task_1, 1);
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
	
	// Inicializar los LEDs poniendolos a apagado
	setLedG(0);
	setLedR(0);
	
	os_sys_init(init);
}
