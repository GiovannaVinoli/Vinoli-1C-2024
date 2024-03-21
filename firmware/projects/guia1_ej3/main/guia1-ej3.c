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
#include "led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*==================[macros and definitions]=================================*/
// le pongo numeros para poder usarlo en el switch (porque solo me va a registrar los números, no los modos)
#define ON 1
#define OFF 0
#define TOGGLE 2

/*enum {ON, OFF, TOGGLE} en vez de poner los tres define anteriores, puedo usar la estructura enum*/
#define CONFIG_BLINK_PERIOD 100
/*==================[internal data definition]===============================*/
struct leds
{
    uint8_t mode;       /*ON, OFF, TOGGLE*/
	uint8_t n_led;        /*indica el número de led a controlar*/
	uint8_t n_ciclos;   /*indica la cantidad de ciclos de ncendido/apagado*/
	uint16_t periodo;    /*indica el tiempo de cada ciclo*/

} my_leds;
/*==================[internal functions declaration]=========================*/

void ControlLeds(struct leds *led_ptr);

/*siempre debo definir que es una estructura, sino puedo poner cuando
defino la estructura un typedef adelante del struct y las instancias que se me van a generar no va a ser 
necesario*/
/*==================[external functions definition]==========================*/



void ControlLeds(struct leds *led_ptr)
{
	switch (led_ptr->mode){
		case 1:/*ON*/
			switch (led_ptr->n_led){
			case 1:
				LedOn(LED_1);
				break;
			case 2:
				LedOn(LED_2);
				break;
			
			case 3:
				LedOn(LED_3);
				break;
			}
			break;
		case 0:/*OFF*/
			switch (led_ptr->n_led){
			case 1:
				LedOff(LED_1);
				break;
			case 2:
				LedOff(LED_2);
				break;
			
			case 3:
				LedOff(LED_3);
				break;
			}
			break;
		case 2: /*TOGGLE*/
			for(int i=0; i<led_ptr->n_ciclos; i++){
				
				switch (led_ptr->n_led){
				case 1:
					LedToggle(LED_1);
					break;
				case 2:
					LedToggle(LED_2);
					break;
				case 3:
					LedToggle(LED_3);
					break;
				}
				for(int j=0; j<led_ptr->periodo;j++)
				{
					vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS); //defino cuántos milisegundos va a ser el delay. 
					//Dependiendo la cantidad de períodos que defina, voy a tener diferentes valores	
				}
				
			}
			break;
			
	}
}

void app_main(void){
	/*debemos probar que la funcion ControlLed funciona*/
	LedsInit();
	my_leds.mode = 2;
	my_leds.n_led = 3;
	my_leds.n_ciclos = 10;
	my_leds.periodo = 5;
	ControlLeds(&my_leds);

}
/*==================[end of file]============================================*/