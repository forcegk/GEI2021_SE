#include <stdio.h>
#define N 410

int my_pow(int n, int m){
	/*
	// Original
	int nm = 1;
	for(int i = 0; i<m; i++){
		nm = nm * n;
	}
	*/
	
	// Optimizacion
	int nm = n;
	for(; m>1; m--){
		nm = nm * n;
	}
	return nm;
}

int get_num_digits(int num){
	int count = 1;
	
	REPEAT:
	if((num = num/10) > 0){
		count++;
		goto REPEAT;
	}

	printf("[%d] has %d digits\n", num, count);

	return count;
}

int get_narcissist_count_under(int num){
	int count = 0; // return in r0 (we store it in r4)
	// num === receive in r0 --> move to r5

	// MOV in place
	int partial, num_digits, acc, nmod10;
	//     r6        r7       r8   r9
	gncu_begin:
	num = num - 1; 
	if(num < 0) goto gncu_end;

	partial = num;
														printf("num=%d\t", num);
	num_digits = get_num_digits(num);
	
	acc = 0;
	for( ; partial > 0; partial /= 10){
		nmod10 = partial%10;
		acc += my_pow(nmod10, num_digits);
		//partial = partial / 10;
	}

	if(acc != num) goto gncu_skip_1;
	count = count + 1;

	gncu_skip_1:
	goto gncu_begin;

	gncu_end: return count;
}


int main(){
	int n;
	n = get_narcissist_count_under(N);
	printf("%d narcisistas\n", n);
	return 0;
}