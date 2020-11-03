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

int is_narcissist(int num){
	int partial = num;
	printf("Passed num=%d\t", num);
	int n_dig = get_num_digits(num);
	int acc = 0;
	
	int nmod10;
	
	for(int i = 0; i<n_dig; i++){
		nmod10 = partial%10;
		partial = partial / 10;
		acc += my_pow(nmod10, n_dig);
	}

	return (acc == num);
	
}


int main(){
	int n = 0;
	for(int i = 0; i<N; i++){
		n += is_narcissist(i);
	}
	printf("%d narcisistas\n", n);
	return 0;
}