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
LIST	DCD 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60	; Lista de elementos

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
		
main			PROC
	EXPORT main
	MOV R0, #1;
	LDR R1, =LIST;		Puntero al primer valor de la lista
	LDR R2,[R1];		Cargamos el valor
LOOP
	STR R0, [R1]
	ADD R1,R1,#4
	LDR R2, [R1]
	CMP R2,#60
	BNE LOOP
	
	ENDP
;- LOOP END -----
	END


; OBJETIVO:
; for(int i = 1; i<N && v[i]^2 <= N; i++){
;	 if(m[i] != 1){
;		 for(int j = v[i]^2; j<N; j = j + v[i])
;			 m[j] = 1
; 	 }
; }
