#include <stdio.h>
#include "biblioteca.h"

int main(void){
	int a=1;
	int b=2;

	float c=16.0f;
	
	printf("%d\n",sumar(a,b));
	printf("%d\n",restar(b,a));
	printf("%d\n",multiplicar(a,b));
	printf("%d\n",dividir(a,b));
	printf("%.2f\n",raiz_cuadrada(c));

	return 0;
}
