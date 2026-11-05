#include <calculadora.h>

#include <stdio.h>

int main ( void )
{
	float a = 9.0f;
	float b = 3.0f;

	printf ( "Suma: %.2f\n", op_suma ( a, b ) ) ;
	printf ( "Resta: %.2f\n", op_resta ( a, b ) ) ;
	printf ( "Multiplicacion: %.2f\n", op_mult ( a, b ) ) ;
	printf ( "Division: %.2f\n", op_div ( a, b ) ) ;
	printf ( "Raiz cuadrada de %.2f: %.2f\n", a, op_sqrt ( a ) ) ;

	return 0;
}