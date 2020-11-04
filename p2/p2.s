; - Stack -------------------------------------------------
	AREA	STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem	SPACE   0x00000400
__initial_sp			; Stack initial pointer


; - Heap --------------------------------------------------
	AREA	STACK, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem	SPACE   0x00000400
__heap_limit			; Stack initial pointer


; - Directives --------------------------------------------
	PRESERVE8
	THUMB       


; - Vector Table ------------------------------------------
	AREA 	RESET, DATA, READONLY
	EXPORT  __Vectors	; Export required by linker
 
__Vectors				; Mapped to address 0 at reset
	DCD  __initial_sp	; Initial stack pointer
	DCD  Reset_Handler	; Address of reset handler


; - User data section -------------------------------------
	AREA	UserData, DATA, READWRITE, ALIGN=3
; (signed) int N = 410
N		DCD 410

; - Reset Handler -----------------------------------------
	AREA 	ResetCode, CODE, READONLY
Reset_Handler	PROC
	EXPORT Reset_Handler
	IMPORT ||Lib$$Request$$armlib||; 
	IMPORT __scatterload;
	
	BL __scatterload;   Init. values in RAM from ROM
	
	ENDP;
	
__rt_entry PROC
	EXPORT __rt_entry
	; jumps to main
	LDR R0, = main		; Cargamos la direccion del main
	BX R0				; Lanzamos el main
	ENDP


; - Main Program ------------------------------------------

	AREA 	MainCode, CODE, READONLY, ALIGN=3
		
;---------------------------------------------------------;
;														  ;
;  *-  *- C	  O	  N	  V	  E	  N	  T	  I	  O	  N	-*	-* 	  ;
;													      ;
;		  - En las funciones, los registros			      ;
;		tienen flechas tal que -> y <-				      ;
;		  - reg <- [recibe un parámetro]       			  ;
;		  - reg <= [devuelve un parámetro]				  ;
;		  - reg -> [indica qué valor almacenamos          ;
;		en ese registro y cómo cambia durante la		  ;
;		ejecución de la subrutina]       				  ;
;---------------------------------------------------------;

;- int pow(int n, int m) ----------------------------------
pow
	; r0 <= returned value n^m [nm]
	;	 <- base [n]
	;	 -> we store here partial power [nm]
	; r1 <- power [m]
	;    -> it decrements each for cycle
	; r2 -> we store here the base
	
	mov r2, r0
pow_for_1_begin
		cmp r1, #1
		ble pow_for_1_end
		
		mul r0, r0, r2
		sub r1, r1, #1
		b pow_for_1_begin

pow_for_1_end
		bx lr
;- END int pow(int n, int m) ------------------------------

;- int get_num_digits(int num) ----------------------------
get_num_digits
	; r0 <= returned value [count]
	;	 <- number [num]
	;	 -> it decrements each for cycle
	; r1 -> we store here partial count [count]
	; r2 -> here we store the constant value 10
	
	mov r1, #1
	mov r2, #10
get_num_digits_for_1_begin
		udiv r0, r0, r2
		cmp r0, #0
		ble get_num_digits_for_1_end
		
		add r1, r1, #1
		b get_num_digits_for_1_begin

get_num_digits_for_1_end
		mov r0, r1
		bx lr
;- END int get_num_digits(int num) ------------------------

;- int get_narcissist_count_under(int num) ----------------
get_narcissist_count_under
	; r0 <= returned value narcissist_count_under N [count]
	;	 <- number [num] (N)
	;
	; r4 -> we store here partial count [count]
	; r5 -> we copy here number [num] from r0
	; r2-6 -> the division result for each for inner loop[partial]
	;    -> also used while in the stack to store the result from calling pow
	; r3-7 -> the number of digits [num_digits]
	; r4-8 -> the accumulator for comparing to the original number [acc]
	; r5-9 -> the number modulo 10 [nmod10]
	; r7-10 -> constant value 10
	; r6-11 -> Now we use it for storing result of division
	
	push {r4, r5, r6, r7, r8, r9, r10, r11, lr}	; save the registers
	
	mov r4, #0 ; count = 0
	mov r5, r0 ; num = (received) num
	mov r10, #10; r10 = 10
	
get_narcissist_count_under_for_1_begin
		; Optimization: Single-digit numbers are always narcissistic.
		cmp		r5, #10
		ble		get_narcissist_count_under_for_1_end
		sub		r5, r5, #1  ; num -= 1
		
		mov		r6, r5	; partial = num
		
		; we don't care about r0-r3 so we do not need to push {r0, r1, r2, r3}
		mov		r0, r5
		bl		get_num_digits	; get_num_digits(r1 === num)
		mov		r7, r0			; num_digits = returned value
		; same here , we don't need pop {r0, r1, r2, r3}
		
		mov		r8, #0	; acc = 0
get_narcissist_count_under_for_2_begin
			cmp		r6, #0		; partival vs 0
			ble	get_narcissist_count_under_for_2_end	; if partial <= 0 { end inner loop }
			
			; modulo op
			udiv r9, r6, r10 ; get the int division (nmod10 = partial / 10)
			
			; we store partial/10 so we don't need to repeat
			mov r11, r9 ; (r11 holds partial / 10)
			
			mul	 r9, r9, r10 ; multiply by the divisor (nmod10 = nmod10 * 10 = ((int)(partial/10)) * 10)
			sub  r9, r6, r9 ; get the difference (nmod10 = partial-nmod10 = partial - ((int)(partial/10)) * 10 === remainder)
			
			
			mov r0, r9		;			 r9 === nmod10
			mov r1, r7		;		      				r7 === num_digits
			bl  pow			;        pow(r9 === nmod10, r7 === num_digits)
			add r8, r8, r0	; acc += pow(r9 === nmod10, r7 === num_digits)
			
			
			; and we restore partial/10 to r6
			mov  r6, r11
			
			b	get_narcissist_count_under_for_2_begin
get_narcissist_count_under_for_2_end
		
		cmp r8, r5	; acc vs num
		bne get_narcissist_count_under_skip_1
		add r4, r4, #1
		
get_narcissist_count_under_skip_1
		b get_narcissist_count_under_for_1_begin

get_narcissist_count_under_for_1_end
	;mov r0, r4
	add r0, r4, r5		; r0 = (number_of_narcissistic_over_one_digit + number_of_one_digit_numbers_left)
	pop  {r4, r5, r6, r7, r8, r9, r10, r11, lr}	; restore the registers
	bx lr
;- END int get_narcissist_count_under(int num) ------------


main			PROC
	EXPORT main
	
	LDR R0, =N
	LDR R0, [R0]
	
	; para medir ciclos (breakpoint 1)
	ORR R1, R1, R1
	
	bl get_narcissist_count_under
	
	; para medir ciclos (breakpoint 2)
	ORR R1, R1, R1
	
	ENDP
		
;- PROGRAM END -----
	END
