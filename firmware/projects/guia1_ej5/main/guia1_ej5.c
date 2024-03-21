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
#include "gpio_mcu.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

typedef struct {
	gpio_t pin;
	io_t dir;
}gpioConfig_t;

/*==================[internal functions declaration]=========================*/
void inicializarGPIO(gpioConfig_t *vectorGpio){
	for(int i=0; i<4; i++){
		GPIOInit(vectorGpio[i].pin, vectorGpio[i].dir);
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
/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Hello world!\n");
	//declaro el vector de GPIOs con los valores de los pines y el tipo de dir (que para todos van a ser salida)
	// se declara con doble llave porque tengo un vector de structs.
	gpioConfig_t vectorGpio[4] = {{GPIO_20, GPIO_OUTPUT},{GPIO_21, GPIO_OUTPUT},{GPIO_22, GPIO_OUTPUT},{GPIO_23, GPIO_OUTPUT}};
	inicializarGPIO(vectorGpio);
	
	uint8_t digitoBcd = 7;
	configurarBcdAGpio(digitoBcd, vectorGpio);
	
}
/*==================[end of file]============================================*/