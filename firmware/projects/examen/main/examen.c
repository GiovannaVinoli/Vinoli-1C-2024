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
#include "uart_mcu.h"
#include "timer_mcu.h"
#include "gpio_mcu.h"
#include "switch.h"

/*==================[macros and definitions]=================================*/

#define CONFIG_PERIOD_ms_1_SEG 1000
#define CONFIG_PERIOD_mS_3_SEG 3000

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
TaskHandle_t sensar_humedad_task_handle = NULL;

/** @def gpioConfig_t
 *  @brief Estructura que utilizaremos para confirugrar los diferentes pines GPIO
*/
typedef struct {
	gpio_t pin;
	io_t dir;
}gpioConfig_t;
/*==================[internal functions declaration]=========================*/
/**
 * @fn void inicializarGPIO(gpioConfig_t *vectorGpio)
 * @brief Inicializa los pines y la dirección de salida en los que van a trabajar (input o output)
 * @param vectorGpio puntero a vector que contiene datos de tipo struct
 * @return 
*/
void inicializarGPIO(gpioConfig_t *vectorGpio, int cantidad){
	for(int i=0; i<cantidad ; i++){
		GPIOInit(vectorGpio[i].pin, vectorGpio[i].dir);
	}
}
/**
 * @fn static void Sensar_Humedad(void *pvParameter)
 * @brief función que define la tarea sensar la humedad de la maceta.
*/
static void Sensar_Humedad(void *pvParameter){
	bool gpio = false;
	while (true)
	{
		if(encender == true){
			gpio = GPIORead(GPIO_1);
			if(gpio == true){
				//Si necesito agua, enciendo la bomba de agua
				GPIOOn(GPIO_21);
			}
			else{
				GPIOOff(GPIO_21);
			}
		}
		else{
			GPIOOff(GPIO_21);
		}
		vTaskDelay(CONFIG_PERIOD_mS_3_SEG/portTICK_PERIOD_MS);
	}
}
/**
 * @fn static void Leer_Teclas(void *pvParameter)
 * @brief función que reconoce cuando se presionan los switches y cambia el estado 
 * de las variables que almacenan si el sistema está activo o no
 * @param pvParameter puntero que no es utilizado
 * @return 
*/
static void Leer_Teclas(void *pvParameter){
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
		vTaskDelay(CONFIG_PERIOD_ms_1_SEG / portTICK_PERIOD_MS);
	}
}

/*==================[external functions definition]==========================*/
void app_main(void){

	// inicialización de teclas	
	SwitchesInit();
	gpioConfig_t vectorGpio[5] = {{GPIO_1, GPIO_INPUT},{GPIO_3, GPIO_INPUT},{GPIO_21, GPIO_INPUT},{GPIO_22, GPIO_INPUT},{GPIO_23, GPIO_INPUT}};
	inicializarGPIO(vectorGpio, 5);

	//tareas
	xTaskCreate(&Leer_Teclas, "Switches", 512, NULL, 4, &switches_task_handle);
	xTaskCreate(&Sensar_Humedad, "Sensar", 512, NULL, 4, &sensar_humedad_task_handle);


}
/*==================[end of file]============================================*/