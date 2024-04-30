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
 * @brief 
*/
uint16_t valores;
int i =0;

#define CONFIG_BLINK_PERIOD_uS_A 2000
#define CONFIG_BLINK_PERIOD_uS_B 4329

/*==================[internal data definition]===============================*/

void funcionUART(void* param);
static void conversionADC(void *pParam);
static void conversionDAC(void *pParam);

TaskHandle_t conversion_ADC = NULL;
TaskHandle_t conversion_DAC = NULL;
int contador = 0;
/*==================[internal functions declaration]=========================*/

static void conversionADC(void *pParam){

	while(true){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		AnalogInputReadSingle(CH1, &valores);
		UartSendString(UART_PC,(char*) UartItoa(valores, 10));
		UartSendString(UART_PC, "\r");
	}

}

static void conversionDAC(void *pParam){
	while(true){
		if(i<231){
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
			AnalogOutputWrite(ecg[i]);
			i++;
		}
		else{
			i = 0;
		}
	}
}
void funcionUART(void* param){
	//ver si tenemos que usarlo para leer algo desde el puerto serie.
}

void funcionTimerA(void *pParam){
	//agrego notificaciones
	vTaskNotifyGiveFromISR(conversion_ADC, pdFALSE);

}

void funcionTimerB(void *pParam){
	//agrego notificaciones
	vTaskNotifyGiveFromISR(conversion_DAC, pdFALSE);

}
/*==================[external functions definition]==========================*/
void app_main(void){

	analog_input_config_t Analog_config = {
		.input = CH1,
		.mode = ADC_SINGLE
	};

	AnalogInputInit(&Analog_config);
	AnalogOutputInit();

	timer_config_t timerA = {
		.timer = TIMER_A,
		.period = CONFIG_BLINK_PERIOD_uS_A,
		.func_p = funcionTimerA,
		.param_p = NULL
	};
	TimerInit(&timerA);

	timer_config_t timerB = {
		.timer = TIMER_B,
		.period = CONFIG_BLINK_PERIOD_uS_B,
		.func_p = funcionTimerB,
		.param_p = NULL
	};
	TimerInit(&timerB);

	serial_config_t Uart = {
		.port = UART_PC,
		.baud_rate = 38400,
		.func_p = funcionUART,
		.param_p = NULL
	};
	UartInit(&Uart);

	xTaskCreate(&conversionADC, "digitalizar", 512, NULL, 3, &conversion_ADC);
	xTaskCreate(&conversionDAC, "digital-analogico", 512, NULL, 3, &conversion_DAC);


	TimerStart(timerA.timer);
	TimerStart(timerB.timer);

}
/*==================[end of file]============================================*/