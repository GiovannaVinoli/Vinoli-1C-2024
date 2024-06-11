/*! @mainpage Examen
 *
 * @section genDesc General Description
 *
 *  irrigación automática de plantas
 * Se diseña un dispositivo que controle la humedad y el pH de una plantera. 
 * Utilizando un sensor de humedad, se accionará o no la bomba de riego.
 * A partir de un sensor de pH, determinaremos si es necesario aumentarlo o 
 * disminuirlo, y en base a ello se accionará la bomba de pH (ácido o básico)
 * que corresponda.
 * Se requiere que el sistema realice las mediciones cada 3 segundos y que cada 5 
 * segundos informe sobre el estado del mismo. 
 * Utilizando las teclas 1 y 2 se podrá encender o apagar el sistema, respectivamente.
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | Sensor humedad	| 	GPIO_1		|
 * | 	Sensor pH	| 	GPIO_3		|
 * |bomba de agua	| 	GPIO_21		|
 * | 	bomba pHA	| 	GPIO_22		|
 * | 	bomba pHB	| 	GPIO_23		| 
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 11/06/2024 | Creación del proyecto 	                         |
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

/*==================[external functions definition]==========================*/
void app_main(void){
	
}
/*==================[end of file]============================================*/