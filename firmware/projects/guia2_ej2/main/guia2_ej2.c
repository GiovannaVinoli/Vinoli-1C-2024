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
 * |    EDUCIAA	  |   Periférico   	|
 * |:--------------:|:--------------|
 * | GPIO_3			|	ECHO
 * | GPIO_2			|  TRIGGER
 * |	+5V			|   +5V
 * |	GND 		|	GND
 *
 *	
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
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
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "switch.h"
#include "timer_mcu.h"

/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_1000 500000
#define CONFIG_BLINK_PERIOD_200 200

bool activar = true, congelar = false;
float distancia;
/*==================[internal data definition]===============================*/
TaskHandle_t sensar_task_handle = NULL;
TaskHandle_t leds_task_handle = NULL;
TaskHandle_t display_task_handle = NULL;

/*==================[internal functions declaration]=========================*/
/**
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimer(void* param){
    vTaskNotifyGiveFromISR(leds_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_1 */
	vTaskNotifyGiveFromISR(sensar_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_1 */
    vTaskNotifyGiveFromISR(display_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_1 */
//ponemos notificaciones
}
static void SensarTask(void *pvParameter){
	while(true){
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* Envía una notificación a la tarea asociada al LED_2 */
		printf("Sensando\n");
		if(activar == true){
			distancia = HcSr04ReadDistanceInCentimeters();
		}
	}
}

static void LedsTask(void *pvParameter){
	//el portMAXdelay te dice que espere infinito por la notificación. Yo podría decirle, che, espera 1 segundo por la notificaicón
	while(true){
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* Envía una notificación a la tarea asociada al LED_2 */
		printf("Leds\n");
		if(activar == true){
			if(distancia <= 10){
				LedsOffAll();
			}	
			else if( distancia <= 20 && distancia >10){
				LedOn(LED_1);
				LedOff(LED_2);
				LedOff(LED_3);
			}
			else if(distancia <= 30 && distancia > 20){
				LedOn(LED_1);
				LedOn(LED_2);
				LedOff(LED_3);
			}
			else if (distancia > 30){
				LedOn(LED_1);
				LedOn(LED_2);
				LedOn(LED_3);
			}
		}
		else {
			LedsOffAll();
		}
	}
}

static void DisplayTask(void *pvParameter){
	while(true){
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* Envía una notificación a la tarea asociada al LED_2 */
		if(activar == true){
			if(congelar == false){
				LcdItsE0803Write(distancia);
			}
		}
		else{
			LcdItsE0803Off();
		} 
	}
}
//creo las dos funciones que serán llamadas cuando se den las interrupciones

static void FuncionSwitch(void *pvParam){
	bool *puntero= (bool*)(pvParam);
	*puntero =! *puntero;
}


/*==================[external functions definition]==========================*/
void app_main(void){
	LedsInit();
	HcSr04Init(GPIO_3, GPIO_2);
	SwitchesInit();
	SwitchActivInt(SWITCH_1,FuncionSwitch, &activar);
	SwitchActivInt(SWITCH_2,FuncionSwitch, &congelar);
	LcdItsE0803Init();

	/* Inicialización de timers _: lo que me da diferentes timer es cuando quiero ponerle difeerntes bases de tiempo*/
    timer_config_t  timer_A= {
        .timer = TIMER_A,
        .period = CONFIG_BLINK_PERIOD_1000,
        .func_p = FuncTimer,
        .param_p = NULL
    };

	TimerInit(&timer_A);

	xTaskCreate(&SensarTask, "Sensar", 512, NULL, 4, &sensar_task_handle);
	xTaskCreate(&LedsTask, "Leds", 512, NULL, 4, &leds_task_handle);
	xTaskCreate(&DisplayTask, "Display", 512, NULL, 4, &display_task_handle);

	TimerStart(timer_A.timer);


	

}
/*==================[end of file]============================================*/