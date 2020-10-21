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
; Unsigned short = 16 bits - 0,31,++
LIST_v	DCW  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
; Unsigned char = 8 bits - Inicializados a cero
LIST_m	DCB  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

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

; OBJETIVO:
; for(int i = 1; i<N && v[i]^2 <= N; i++){
;	 if(m[i] != 1){
;		 for(int j = v[i]^2; j<N; j = j + v[i])
;			 m[j] = 1
; 	 }
; }

	AREA 	MainCode, CODE, READONLY, ALIGN=3

main			PROC
	; Asumo short = 16 bits
	; 		char  = 8 bits
	
	EXPORT main
	MOV R9, #1	; Cargamos el 1 en R9
	LDR R2, =LIST_v			;(unsigned short) == 2 Bytes
	LDR R3, =LIST_m			;(signed char) == 1 Byte
	MOV R10, R3				; Hacemos copia en R10 de =LIST_m para evitar hace más adelante
							; un LDR y hacer un MOV, el cual nos ahorra un ciclo
	
	; R4 == 2a copia de LIST_m     Haremos otra copia de &LIST_m para usar con m[j] más tarde
	MOV R5, #32				; Guardamos int N = 32 (signed)
	
	;; ------ Optimizaciones implementadas ------ ;;
	; Contexto: En la primera versión de la implementación, utilizábamos R0 para guardar el valor de i
	;			y R1 para guardar el valor de j.
	; 			Sin embargo se puede inferir el valor de éstas calculando con direcciones máximas
	
	; Queremos dejar de usar R0 para almacenar el valor de i, para usar un valor que calculemos ya de por si,
	; y no tener que realizar el ADD R0, R0, #1 para contar el numero de iteraciones
	; Para esto vamos a almacenar en R0, en vez del valor de i, el valor de la dirección que tomaría v[i] en la última iteración (v{i_max])
	; Guardamos en R0 la dirección base de v, mas N*<tamaño en bytes de unsigned short> == N*2 == N<<1
	ADD R0, R2, R5, LSL #1
	
	; Aquí la calculamos para j:
	; Calculamos la dirección máxima que puede alcanzar m[j] (m[j_max]), con esto podemos
	; llevar la cuenta de j sin tener que usar un registro para ello, y nos ahorramos operaciones
	; Recordemos que m es array de shorts
	ADD R1, R3, R5, LSL #0
	
	; Ahora ya podemos poner el offset al principio, ya que como empezamos a contar por uno, hay que sumarle 1*sizeof(short)
	ADD R2, R2, #2			; No podíamos hacerlo antes porque dependíamos de R2 para la suma anterior
	ADD R3, R3, #1			; Igual para m[i_m]
					
	
OUTER_LOOP
	; Primera exit condition [ i < N ]
	CMP R2, R0;
	BGE DONE			; if i >= N exit
	
	; Segunda exit condition [ v[i]^2 <= N ]
	LDRH R6, [R2], #2	; R6 = v[i_v] ; i_v++
	MUL R8, R6, R6		; R8 = v[i_v]^2
	CMP R8, R5;
	BGT DONE			; if v[i]^2 > N exit
	
	
	; Comprobamos si m[i] == 1, para ejecutar el código interior, esto es equivalente a escribir:
	; 	if (m[i] == 1) { continue; }
	LDRSB R7, [R3], #1	; R7 = m[i] ; i_m++
	TEQ R7, #1
	BEQ OUTER_LOOP		; if m[i] == 1 continue
	
	; Si no hemos hecho el "continue", ejecutamos el código aquí
	; Calculamos m[j] para la primera iteración
	
	; Ahorramos un ciclo si hacemos un MOV en vez de un LDR
	MOV R4, R10 				; An una versión anterior del programa hacíamos la línea de abajo, pero esto nos ahorra un ciclo
								; Cuanto mayor sea N más se notará este pequeño cambio
	; LDR R4, =LIST_m			; Cargamos la dirección base de m (la necesitamos ya que en cada iteración j es diferente)
	ADD R4, R4, R8, LSL #0		; Aquí calculamos la dirección de la primera m[j] (m[j_1])


INNER_LOOP
	CMP R4, R1		; &m[j_curr] vs &m[j_max]
	BGE OUTER_LOOP	; if j>=N exit  <==> if &m[j] >= &m[N] exit --- Es decir, que salimos del for interior cuando j>=N
	
	; Guardamos 1 en m[j]
	; Otra optimización que hice aquí es cargar el 1 en R9 al principio.
	; Ya que puedo usar todos los registros a mi conveniencia, por qué no dejar el 1 ahí desde el principio, en vez
	; de ejecutar una instrucción MOV en cada iteración del bucle interior?
	STRB R9, [R4]  ; R9 = 1
				   ;&m[j_next] = ( (&m[j_curr]) + v[i]*sizeof(char) )
				   ;R4 = &m[j]
				   ;R6 = v[i]
	
	; Me gustaría poder integrar este ADD en el anterior STRB pero parece que es imposible
	; en este chip, con las directivas actuales, o algo similar
	; Recordemos que el LSL #0 lo elimina el ensamblador porque es redundante, y solo está ahí
	; por si se quisiese cambiar el tipo de los datos de m.
	ADD R4, R4, R6, LSL #0	
	
	; Si funcionase se podría agrupar las dos instrucciones superiores con 
	; STRB R9, [R4], R6, LSL #0
	
	
	B INNER_LOOP
;- INNER_LOOP END -----

	B OUTER_LOOP
;- OUTER_LOOP END -----
	
DONE

	; Para medir ciclos. Poner aquí un breakpoint y eso
	; AND R0, R0, R0;

	ENDP
		
;- PROGRAM END -----
	END
