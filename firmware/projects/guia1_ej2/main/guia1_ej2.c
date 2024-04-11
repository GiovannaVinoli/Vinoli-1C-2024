
/*! @mainpage Blinking switch
 *
 * \section genDesc General Description
 *
 * This example makes LED_1 and LED_2 blink if SWITCH_1 or SWITCH_2 are pressed.
 *
 * @section changelog Changelog
 *
 *
 * @author Giovanna Viñoli (giovanna.vinoli@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 1000
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Hello world!\n");
	uint8_t teclas;
	LedsInit();
	SwitchesInit();
    while(1)    {
    	teclas  = SwitchesRead();
    	switch(teclas){
    		case SWITCH_1:
    			LedToggle(LED_1);
    		break;
    		case SWITCH_2:
    			LedToggle(LED_2);
    		break;
			// case (SWITCH_1 & SWITCH_2): //el SWITCH_1 es un "1", que en binario equivale a 01. El SWITCH_2 es un "2", que en binario
			// //es 01. Al hacer la operación &, vamos a multiplicar ambos valores y 01 * 10 nos da 00. Va a estar prendido siempre.
			// 	LedOn(LED_3);
			// break;
			// case SWITCH_1 + SWITCH_2: // forma de resolverlo de forma algebraica.
			// 	LedToggle(LED_3);
			// break;
			// case SWITCH_1 | SWITCH_2: // al poner un or, nos sirve para ambas cosas- forma de resolverlo de forma lógica.
			// 	LedToggle(LED_3);
			// break;
			// case ((teclas & 1) && (teclas & 2)): // forma de resolverlo por máscaras. Generamos las comparaciones. 
			// 	LedToggle(LED_3);
			// break;
    	}
//	    LedToggle(LED_3);
		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
	}
}

/*==================[end of file]============================================*/