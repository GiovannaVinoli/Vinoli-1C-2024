/*! @mainpage Template
 *
 * @section genDesc General Description
 * Se implementa una aplicación para digitalizar una señal analógica 
 * y viceversa, transmitiéndola por un graficador de puerto serie de la PC.
 * 
 *
 * @section hardConn Hardware Connection
 *
 * |    ESP32c6	  |   Periférico   	|
 * |:--------------:|:--------------|
 * | 			|	
 * | 			|  
 * |			|   
 * |	 		|	
 *	
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * |:12/04/2024:|: Generación del proyecto					 |
 * |:30/04/2024 |: Generación de la documentación                |
 *
 * @author Giovanna Viñoli (giovanna.vinoli@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "analog_io_mcu.h"
#include "uart_mcu.h"
#include "timer_mcu.h"
#include "gpio_mcu.h"
#include "ecg.h"

/*==================[macros and definitions]=================================*/
/**
 * @def valores
 * @brief variable que almacena lo que se lee desde el canal CH1
*/
uint16_t valores;
/**
 * @def contador
 * @brief variable para controlar el valor del índice del vector de ECG
*/
int contador =0;

/**
 * @brief Declaración de los tiempos que utilizan los timers
*/
#define CONFIG_PERIOD_uS_A 2000
#define CONFIG_PERIOD_uS_B 4329

/*==================[internal data definition]===============================*/

void funcionUART(void* param);
static void conversionADC(void *pParam);
static void conversionDAC(void *pParam);

/// @brief identificadores de FREERTOS utilizados para hacer referencia a las tareas y así
/// poder ejecutarlas (a partir de estar referidas por estos TaskHandle) en otros 
/// comandos de vTask como Delete, Create, etc.
TaskHandle_t conversion_ADC = NULL;
TaskHandle_t conversion_DAC = NULL;

/*==================[internal functions declaration]=========================*/
/**
 * @fn static void conversionADC(void *pParam)
 * @brief función que lee desde el canal 1 una señal analógica
*/
static void conversionADC(void *pParam){

	while(true){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		AnalogInputReadSingle(CH1, &valores);
		// la funcion Itoa convierte de int --> ascii
		UartSendString(UART_PC,(char*) UartItoa(valores, 10));
		UartSendString(UART_PC, "\r");
	}

}
 /**
  * @fn static void conversionDAC(void *pParam)
  * @brief funcion para escribir los valores de ECG como señal analógica
 */
static void conversionDAC(void *pParam){
	while(true){
		if(contador<BUFFER_SIZE){
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
			AnalogOutputWrite(ecg[contador]);
			contador++;
		}
		else{
			contador = 0;
		}
	}
}
/**
 * @fn void funcionUART(void *param)
 * @brief función definida para configurar la UART
*/
void funcionUART(void* param){
	//ver si tenemos que usarlo para leer algo desde el puerto serie.
}
/**
 * @fn void FuncTimerA(void* param)
 * @brief Función invocada en la interrupción del timer A. Con esta función defino que va 
 * a ocurrir cuando defino y utilizo el timer. Dentro de ella, creo notificaciones a la 
 * tarea para realizar la conversion ADC (leer la señal y mostrarla por la UART)
 */
void funcionTimerA(void *pParam){
	//agrego notificaciones
	vTaskNotifyGiveFromISR(conversion_ADC, pdFALSE);

}
/**
 * @fn void FuncTimerB(void* param)
 * @brief Función invocada en la interrupción del timer B. Con esta función defino que va 
 * a ocurrir cuando defino y utilizo el timer. Dentro de ella, creo notificaciones a la 
 * tarea conversion_DAC para escribir el ECG
 */
void funcionTimerB(void *pParam){
	//agrego notificaciones
	vTaskNotifyGiveFromISR(conversion_DAC, pdFALSE);

}
/*==================[external functions definition]==========================*/
void app_main(void){
	/// inicializo el struct para configurar qué puerto voy a utilizar
	/// y en qué modo
	analog_input_config_t Analog_config = {
		.input = CH1,
		.mode = ADC_SINGLE
	};

	/// inicializo el analógico para entrada y salida
	AnalogInputInit(&Analog_config);
	AnalogOutputInit();

	/// Definición del timerA: lo que me hace definir más de un timer es cuando quiero trabajar 
	/// con diferntes bases de tiempo. Como ahora tengo la misma base de tiempo que es 1 
	/// segundo = 1000000 microsegundos, voy a crear solo uno para las mismas tareas 
	timer_config_t timerA = {
		.timer = TIMER_A,
		.period = CONFIG_PERIOD_uS_A,
		.func_p = funcionTimerA,
		.param_p = NULL
	};
	/// inicialización del timerA
	TimerInit(&timerA);

	/// Definición del timerB: lo que me hace definir más de un timer es cuando quiero trabajar con diferntes bases de tiempo.
	/// Como ahora tengo la misma base de tiempo que es 1 segundo = 1000000 microsegundos, voy a crear solo uno para las mismas tareas 
	timer_config_t timerB = {
		.timer = TIMER_B,
		.period = CONFIG_PERIOD_uS_B,
		.func_p = funcionTimerB,
		.param_p = NULL
	};
	 /// inicialización del timer B
	TimerInit(&timerB);

	/// configuración de la UART
	serial_config_t Uart = {
		.port = UART_PC,
		.baud_rate = 38400,
		.func_p = funcionUART,
		.param_p = NULL
	};

	/// inicializo la UART
	UartInit(&Uart);

	/// creación de las tareas que quiero ejecutar 
	xTaskCreate(&conversionADC, "digitalizar", 512, NULL, 3, &conversion_ADC);
	xTaskCreate(&conversionDAC, "digital-analogico", 512, NULL, 3, &conversion_DAC);

	/// comienza a ejecutarse los timers
	TimerStart(timerA.timer);
	TimerStart(timerB.timer);

}
/*==================[end of file]============================================*/