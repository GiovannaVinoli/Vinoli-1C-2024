/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Giovanna Viñoli (giovanna.vinoli@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
//en el tp figura sin el void, porque generalmente se devuelven valores para controlar 
// errores. Nosotros lo vamos a declarar como void porque no vamos a devolver nada
void convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcdNumber){
	//creo una variable auxiliar donde voy a guardar el resto de cada operación sobre el número
	uint8_t variableAuxiliar;
	//bucle con la cantidad de dígitos que tiene el número. 
	// arraco de "mayor" a "menor" para que me guarde en orden el número 
	// en el vector bcd_number: 4, 5, 6 en vez de 6, 5, 4.
	for(int i = digits-1; i>=0; i--){
		variableAuxiliar = data%10;
		data = data/10;
		bcdNumber[i] = variableAuxiliar;
	}
	
}
void mostrarVector(uint8_t *bcdNumber){
	for(int i = 0; i<3; i++){
		printf(" %d", bcdNumber[i]);
	}
}
/*==================[external functions definition]==========================*/
void app_main(void){
	uint32_t numero = 456;
	uint8_t bcdArray [3];
	//el nombre de un array ya actúa como puntero, no hay que poner el &
	convertToBcdArray(numero, 3, bcdArray);
	mostrarVector(bcdArray);
}
/*==================[end of file]============================================*/