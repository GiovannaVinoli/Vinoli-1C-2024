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
#include gpio_muc.h

/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
typedef struct {
	gpio_t pin;
	io_t dir;
}gpioConfig_t;
/*==================[internal functions declaration]=========================*/
void inicializarGPIO(gpioConfig_t *vectorGpio){
	for(int i=0; i<len(vectorGpio); i++){
		GPIOInit(vectorGpio[i].pin, vectorGpio[i].dir);
	}
}

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
void configurarBcdAGpio(uint8_t digitoBcd, gpioConfig_t *vectorGpio){
	
	for(int i=0; i<4; i++){
		if((digitoBcd & 1<<i) == 0){
			GPIOOff(vectorGpio[i].pin);
		}
		else{
			GPIOOn(vectorGpio[i].pin);
		}
	}

}
void funcion(uint32_t dato, uint8_t digitos, gpioConfig_t *vectorGpio, gpioConfig_t *vectorLCD)
{
	uint8_t vectorBcd[digitos];// creo un vector que almacene cada dígito del dato
	convertToBcdArray(dato, digitos, vectorBcd); // obtengo cada dígito del dato con la funcion converToBcdArray
	fot(int i=0; i<digitos; i++){ //creo un bucle for que se repite la cantidad de dígitos que posee el número
		
		configurarBcdAGpio(vectorBcd[i], vectorGpio); // para cada dígito, configuro el vector Gpio para que me muestre el número 
		
		GPIOOn(vectorLCD[i].pin);//genero el pulso en el LCD correspondiente para que me "fije" el valor que quiero
		GPIOOff(vectorLCD[i].pin);

	}
}

/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Hello world!\n");
	uint32_t dato = 456;
	uint8_t cantidadDigitos = 3;
	gpioConfig_t vectorGpio[4] = {{GPIO_20, GPIO_OUTPUT},{GPIO_21, GPIO_OUTPUT},{GPIO_22, GPIO_OUTPUT},{GPIO_23, GPIO_OUTPUT}};
	inicializarGPIO(vectorGpio);
	gpioConfig_t vectorLCD[3] = {{GPIO_19, GPIO_OUTPUT},{GPIO_18, GPIO_OUTPUT},{GPIO_9, GPIO_OUTPUT}};
	inicializarGPIO(vectorLCD);
	funcion(dato, cantidadDigitos, vectorGpio, vectorLCD);
}
/*==================[end of file]============================================*/