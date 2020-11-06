#include <stdio.h>                /* prototype declarations for I/O functions */

extern void SER_Init(void);                                   /* see Serial.c */

/*----------------------------------------------------------------------------
  Scalar Product Function in In-line Assembly
 *----------------------------------------------------------------------------*/
__asm int asm_scalar_product(int * a, int * b, int n){
	/** We store:
	*		r0 = *a
	*		r1 = *b
	*		r2 = n
	*		r3 = a[i]
	*		r4 = b[i]
	*		r5 = acc
	*/
	
	push	{r4, r5}				// We store these because we can not dirty modify these
	mov		r5, #0					// Set acc initial value (acc = 0)
	b			for_asp_entry		// we could afford this branch if we did not care about N being 0 or negative,
												// but we want to keep this function as general as possible
	
for_asp
	ldr		r3, [r0], #4		// ta = *(a++)
	ldr		r4, [r1], #4		// tb = *(b++)
	mla		r5, r3, r4, r5	// acc += ta * tb
for_asp_entry
	subs	r2, r2, #1			// n--
	bpl		for_asp					// Putting this on the end, saves us the branch test at the beginning of the for loop
												// And also the final branch to the exit procedure.
	// Exit procedure
	mov		r0, r5
	pop		{r4, r5}
	bx		lr
}

/*----------------------------------------------------------------------------
  Scalar Product Function in C
 *----------------------------------------------------------------------------*/
int c_scalar_product(int * a, int * b, int n){
	int acc=0;
	while (n > 0) {
		acc += *(a++) * *(b++);
		n--;
	}
	return acc;
}

/*----------------------------------------------------------------------------
  main program
 *----------------------------------------------------------------------------*/

#define N 64

int main (void) {
  int a[N], b[N], axb, i;

  SER_Init();                                  /* initialize serial interface */
	
	for(i=0; i<N; i++){
		a[i] = i+1;
		b[i] = -i-1;
		printf("A[%d]=%d\tB[%d]=%d\n", i, a[i], i, b[i]);
	}
	
	i = 0;
	axb = c_scalar_product(a, b, N);
	i = 0;
	printf("\nA*B = %d\n", axb);
	
	i = 0;
	axb = asm_scalar_product(a, b, N);
	i = 0;
	printf("\nA*B = %d\n", axb);

	i = 0;
	
}
