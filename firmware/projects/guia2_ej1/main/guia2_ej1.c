/*! @mainpage Proyecto 2 - Ejercicio 1
 * Medidor de distancia por ultrasonido
 *
 * @section genDesc General Description
 * Se diseña un firmware donde utilizamos un sensor ultrasónico para realizar medidas de distancias. 
 * Utilizando los valores de distancias obtenidos, podremos informarlo por dos medios: el primero con
 * una secuencia específica de iluminación de leds (si la distancia es menor a 10cm, no se enciende ningún
 * led; si está entre 10 y 20, se enciende el led 1; si está entre 20 y 30 se enciende el led 1 y 2; si es mayor
 * a 30, se enciende el led 1, 2 y 3); el segundo a través de un display, donde podemos tener la opción
 * de congelar el valor de medición tras apretar la tecla 2.
 * El sistema se activa o desactiva presionando la tecla 1.
 *
 * <a href="https://docs.google.com/document/d/13MwvkL35G1JcyPhDN2vl298YrCzqDJw_FseBSXWroOY/edit#heading=h.5fxbx7ywgd2l"></a>
 *
 * @section hardConn Hardware Connection
 *
 * |    ESP32c6	  	|   Hc sr04   	|
 * |:--------------:|:--------------|
 * | GPIO_3			|	ECHO
 * | GPIO_2			|  TRIGGER
 * |	+5V			|   +5V
 * |	GND 		|	GND
 * 
 * |	ESP32c6		|	Lcditse0803
 * |:--------------:|:--------------|	
 * | GPIO_20		|		D1
 * | GPIO_21		|		D2
 * | GPIO_22		|		D3
 * | GPIO_23		|		D4
 * | GPIO_19		|		SEL_1
 * | GPIO_18		|		SEL_2
 * | GPIO_9			|		SEL_3
 * | +5V			|		+5V
 * | GND			|		GND
 *
 *	
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * |:11/04/2024 | : creación de documentación
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

/** @def activar
 *  @brief variable booleana global que registra si el sistema está ON u OFF
*/
bool activar = true; 
/** @def congelar
 *  @brief variable booleana global que registra si el display queda en modo congelado
*/
bool congelar = false;

/**
 * @def distancia
 * @brief variable global que almacena el valor de distancia medido por el sensor
*/
float distancia;
/*==================[internal data definition]===============================*/

/**
 * @def sensar_task_handle
 * @brief identificador de la tarea encargada de sensar
*/
TaskHandle_t sensar_task_handle = NULL;
/**
 * @def leds_task_handle
 * @brief identificador de la tarea encargada de prender y apagar leds
*/
TaskHandle_t leds_task_handle = NULL;
/**
 * @def display_task_handle
 * @brief identificador de la tarea encargada de configurar el display
*/
TaskHandle_t display_task_handle = NULL;
/**
 * @def switches_task_handle
 * @brief identificador de la tarea encargada de reconocer los switches
*/
TaskHandle_t switches_task_handle = NULL;

/*==================[internal functions declaration]=========================*/

/**
 * @fn static void SensarTask(void *pvParameter)
 * @brief función que ejecuta la tarea de sensar, guardando en la variable distancia lo 
 * obtenido por el sensor
 * @param pvParameter puntero que no es utilizado
 * @return 
*/
static void SensarTask(void *pvParameter){
	while(true){
		printf("Sensando\n");
		if(activar == true){
			distancia = HcSr04ReadDistanceInCentimeters();
		}
		vTaskDelay(CONFIG_BLINK_PERIOD_1000 / portTICK_PERIOD_MS);
	}
}

/**
 * @fn static void LedsTask(void *pvParameter)
 * @brief función que enciende los diferentes leds, acorde al valor de distancia 
 * @param pvParameter puntero que no es utilizado
 * @return 
*/
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

/**
 * @fn static void DisplayTask(void *pvParameter)
 * @brief función que muestra por display el valor de la medición 
 * @param pvParameter puntero que no es utilizado
 * @return 
*/
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

/**
 * @fn static void SwitchesTask(void *pvParameter)
 * @brief función que de reconoce cuando se presionan los switches y cambia el estado 
 * de las variables que almacenan si el sistema está activo o no, y si se congela el display
 * @param pvParameter puntero que no es utilizado
 * @return 
*/
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
	/// Inicialización de los leds
	LedsInit();
	/// Inicialización del sensor ultrasónico, donde se define qué pin está conectado al echo y al trigger 
	/// del sensor
	HcSr04Init(GPIO_3, GPIO_2);
	/// Inicialización de los Switches
	SwitchesInit();
	/// Inicialización del display
	LcdItsE0803Init();

	/// Creación de nuevas tareas en FreeRTOS, donde le paso los parametros de cada tarea definida anteriormente
	xTaskCreate(&SwitchesTask, "Switches", 512, NULL, 4, &switches_task_handle);
	xTaskCreate(&SensarTask, "Sensar", 512, NULL, 4, &sensar_task_handle);
	xTaskCreate(&LedsTask, "Leds", 512, NULL, 4, &leds_task_handle);
	xTaskCreate(&DisplayTask, "Display", 512, NULL, 4, &display_task_handle);
	

}
/*==================[end of file]============================================*/