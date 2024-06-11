/*! @mainpage Sistema de Irrigación automática de plantas
 *
 * @section genDesc General Description
 *
 * 
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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "switch.h"

/*==================[macros and definitions]=================================*/

/** @def encender
 *  @brief variable booleana global que registra si el sistema está ON u OFF
*/
bool encender = false;
/*==================[internal data definition]===============================*/
/**
 * @def switches_task_handle
 * @brief identificador de la tarea encargada de reconocer los switches
*/
TaskHandle_t switches_task_handle = NULL;

/*==================[internal functions declaration]=========================*/

/**
 * @fn static void SwitchesTask(void *pvParameter)
 * @brief función que reconoce cuando se presionan los switches y cambia el estado 
 * de las variables que almacenan si el sistema está activo o no
 * @param pvParameter puntero que no es utilizado
 * @return 
*/
static void SwitchesTask(void *pvParameter){
	uint8_t teclas;
	while(true){
		teclas = SwitchesRead();
		switch (teclas)
		{
		case SWITCH_1:
			encender = true;
			break;
		
		case SWITCH_2:
			encender = false;
			break;
		default:
			break;
		}
		vTaskDelay(CONFIG_BLINK_PERIOD_200 / portTICK_PERIOD_MS);
	}
}

/*==================[external functions definition]==========================*/
void app_main(void){

	// inicialización de teclas	
	SwitchesInit();


	//tareas
	xTaskCreate(&SwitchesTask, "Switches", 512, NULL, 4, &switches_task_handle);


}
/*==================[end of file]============================================*/