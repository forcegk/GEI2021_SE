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
;LIST	DCD 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60	; Lista de elementos
; Unsigned short = 16 bits - 0,31,++
LIST_v	DCW 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
; Unsigned char = 8 bits - Inicializados a cero
LIST_m	DCB  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

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
		
;main			PROC
;	EXPORT main
;	MOV R0, #1;
;	LDR R1, =LIST;		Puntero al primer valor de la lista
;	LDR R2,[R1];		Cargamos el valor
;LOOP
;	STR R0, [R1]
;	ADD R1,R1,#4
;	LDR R2, [R1]
;	CMP R2,#60
;	BNE LOOP
	
;	ENDP
;- LOOP END -----
;	END


; OBJETIVO:
; for(int i = 1; i<N && v[i]^2 <= N; i++){
;	 if(m[i] != 1){
;		 for(int j = v[i]^2; j<N; j = j + v[i])
;			 m[j] = 1
; 	 }
; }

main			PROC
	; Asumo short = 16 bits
	; 		char  = 8 bits
	
	EXPORT main
	MOV R0, #1; int i = 1 (signed)
	;   R1 == j
	LDR R2, =LIST_v;		(unsigned short) Puntero al primer valor de v  (v[1] ???)
	ADD R2, R2, #2
	LDR R3, =LIST_m;		(signed char)
	; R4 == 2a copia de LIST_m     Haremos otra copia de &LIST_m para usar con m[j] más tarde
	MOV R5, #32; int N = 32 (signed)
	
OUTER_LOOP
	; first loop exit conditions
	CMP R0, R5;
	ADD R0, R0, #1
	BGE GIT_OUTA_HERE_BOI; if i>=N exit
	
	LDRH R6, [R2], #2	; R6 = v[i_v]<-- short ; i_v++
	MUL R8, R6, R6		; R8 = v[i_v]^2
	CMP R8, R5;
	BGT GIT_OUTA_HERE_BOI; if v[i]^2 > N exit
	
	
	; if (m[i] == 1) { continue; }
	LDRSB R7, [R3], #1	; R7 = m[i] ; i_m++
	TEQ R7, #1
	BEQ OUTER_LOOP		; if m[i] == 1 continue
	
	; else
	MOV R1, R8	; j = v[i_v]^2
	MOV R8, #1
	; Calculamos m[j] para la primera iteración
	LDR R4, =LIST_m	; m[]
	ADD R4, R4, R1		; m[j]
INNER_LOOP
	; R6 = v[i_v]
	CMP R1, R5	; j vs N
	BGE OUTER_LOOP	; salimos del for interior cuando j>=N
	
	;STRB #1, [R5]
	; guardar m[j] =1 
	STRB R4, [R8] ; R4 = 1
	ADD R1, R1, R6; j = j + v[i]
	ADD R4, R4, R6, LSL #0; m[j] es m[j_antiguo] + 1*v[i] (1 y no 4 porque son chars)
	B INNER_LOOP
;- INNER_LOOP END -----

	B OUTER_LOOP

;- OUTER_LOOP END -----
	
GIT_OUTA_HERE_BOI
	ENDP
;- LOOP END -----
	END
