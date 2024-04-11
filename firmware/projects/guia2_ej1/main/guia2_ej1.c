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
 * | GPIO_2			|	ECHO
 * | GPIO_3			|  TRIGGER
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

/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_1000 1000
#define CONFIG_BLINK_PERIOD_200 200

bool activar = true, congelar = false;
float distancia;
/*==================[internal data definition]===============================*/
TaskHandle_t sensar_task_handle = NULL;
TaskHandle_t leds_task_handle = NULL;
TaskHandle_t display_task_handle = NULL;
TaskHandle_t switches_task_handle = NULL;

/*==================[internal functions declaration]=========================*/
static void SensarTask(void *pvParameter){
	while(true){
		printf("Sensando\n");
		if(activar == true){
			distancia = HcSr04ReadDistanceInCentimeters();
		}
		vTaskDelay(CONFIG_BLINK_PERIOD_1000 / portTICK_PERIOD_MS);
	}
}

static void LedsTask(void *pvParameter){
	while(true){
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
		vTaskDelay(CONFIG_BLINK_PERIOD_1000 / portTICK_PERIOD_MS);
	}
}

static void DisplayTask(void *pvParameter){
	while(true){
		if(activar == true){
			if(congelar == false){
				LcdItsE0803Write(distancia);
			}
		}
		else{
			LcdItsE0803Off();
		} 
		vTaskDelay(CONFIG_BLINK_PERIOD_1000 / portTICK_PERIOD_MS);
	}
}

static void SwitchesTask(void *pvParameter){
	uint8_t teclas;
	while(true){
		printf("switches\n");
		teclas = SwitchesRead();
		switch (teclas)
		{
		case SWITCH_1:
			activar = !activar;
			printf("switch 1 presionado\n");
			break;
		
		case SWITCH_2:
			congelar = !congelar;
			printf("switch 2 presionado\n");
			break;
		default:
			break;
		}
		vTaskDelay(CONFIG_BLINK_PERIOD_200 / portTICK_PERIOD_MS);
	}
}

/*==================[external functions definition]==========================*/
void app_main(void){
	LedsInit();
	HcSr04Init(GPIO_3, GPIO_2);
	SwitchesInit();
	LcdItsE0803Init();

	xTaskCreate(&SwitchesTask, "Switches", 512, NULL, 4, &switches_task_handle);
	xTaskCreate(&SensarTask, "Sensar", 512, NULL, 4, &sensar_task_handle);
	xTaskCreate(&LedsTask, "Leds", 512, NULL, 4, &leds_task_handle);
	xTaskCreate(&DisplayTask, "Display", 512, NULL, 4, &display_task_handle);
	

}
/*==================[end of file]============================================*/