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
;		  - reg <- [recibe o devuelve un parámetro]       ;
;		  - reg -> [indica qué valor almacenamos          ;
;		en ese registro y cómo cambia durante la		  ;
;		ejecución de la subrutina]       				  ;
;---------------------------------------------------------;

;- int pow(int n, int m) ----------------------------------
pow
	; r0 <- returned value n^m [nm]
	;	 -> we store here partial power [nm]
	; r1 <- base [n]
	; r2 <- power [m]
	;    -> it decrements each for cycle
	
	mov r0, r1
pow_for_1_begin
		cmp r2, #1
		ble pow_for_1_end
		
		mul r0, r0, r1
		sub r2, r2, #1
		b pow_for_1_begin

pow_for_1_end
		bx lr
;- END int pow(int n, int m) ------------------------------

;- int get_num_digits(int num) ----------------------------
get_num_digits
	; r0 <- returned value [count]
	;	 -> we store here partial count [count]
	; r1 <- number [num]
	;    -> it decrements each for cycle
	; r2 -> here we store the constant value 10
	
	mov r0, #1
	mov r2, #10
get_num_digits_for_1_begin
		udiv r1, r1, r2
		cmp r1, #0
		ble get_num_digits_for_1_end
		
		add r0, r0, #1
		b get_num_digits_for_1_begin

get_num_digits_for_1_end
		bx lr
;- END int get_num_digits(int num) ------------------------

;- int get_narcissist_count_under(int num) ----------------
get_narcissist_count_under
	; r0 <- returned value narcissist_count_under N [count]
	;	 -> we store here partial count [count]
	; r1 <- number [num] (N)
	; r2 -> the division result for each for inner loop[partial]
	;    -> also used while in the stack to store the result from calling pow
	; r3 -> the number of digits [num_digits]
	; r4 -> the accumulator for comparing to the original number [acc]
	; r5 -> the number modulo 10 [nmod10]
	; r6 -> iterator [i]
	; r7 -> constant value 10
	
	push {r4, r5, r6, r7, lr}	; save the registers
	
	mov r0, #0 ; count = 0
	mov r7, #10; r7 = 10
	
get_narcissist_count_under_for_1_begin
		sub		r1, r1, #1  ; num -= 1
		cmp		r1, #0
		blt		get_narcissist_count_under_for_1_end
		
		mov		r2, r1	; partial = num
		
		push	{r0, r1, r2}	; save values (we don't care about r3)
		bl		get_num_digits	; get_num_digits(r1 === num)
		mov		r3, r0			; num_digits = returned value
		pop		{r0, r1, r2}	; restore pre-subroutine values (but r3)
		
		mov		r4, #0	; acc = 0
		mov		r6, #0	; i = 0
get_narcissist_count_under_for_2_begin
			cmp	r6, r3	; i vs num_digits
			bge	get_narcissist_count_under_for_2_end	; if i>=num_digits => end inner loop
			
			; modulo op
			udiv r5, r2, r7 ; get the int division
			
			; we store partial/10 so we don't need to repeat
			push {r5}		; we store partial / 10 in the stack
			
			mul	 r5, r5, r7 ; multiply by the divisor
			sub  r5, r2, r5 ; get the difference
			
			
			push {r0, r1, r2, r3} ; store the values in the stack
			mov r1, r5		;			 r5 === nmod10
			mov r2, r3		;		      				r3 === num_digits
			bl  pow			;        pow(r5 === nmod10, r3 === num_digits)
			add r4, r4, r0	; acc += pow(r5 === nmod10, r3 === num_digits)
			pop  {r0, r1, r2, r3} ; aaand restore them
			
			
			; and we restore partial/10 to r2 manually (because we need to restore it to a different register)
			pop  {r5}
			mov  r2, r5
			
			add	r6, r6, #1
			b	get_narcissist_count_under_for_2_begin
get_narcissist_count_under_for_2_end
		
		cmp r4, r1	; acc vs num
		bne get_narcissist_count_under_skip_1
		add r0, r0, #1
		
get_narcissist_count_under_skip_1
		b get_narcissist_count_under_for_1_begin

get_narcissist_count_under_for_1_end
	pop {r4, r5, r6, r7, lr}	; restore the registers
	bx lr
;- END int get_narcissist_count_under(int num) ------------


main			PROC
	EXPORT main
	
	LDR R1, =N
	LDR R1, [R1]
	
	ORR R0, R0, R0
	ORR R0, R0, R0
	
	; push {r0, r1, r2, r3} ; no nos sirve para nada en especial
	
	bl get_narcissist_count_under
	
	; pop  {r0, r1, r2, r3} ; idem
	
	ORR R0, R0, R0
	ORR R0, R0, R0
	
	


DONE
	ENDP
		
;- PROGRAM END -----
	END
