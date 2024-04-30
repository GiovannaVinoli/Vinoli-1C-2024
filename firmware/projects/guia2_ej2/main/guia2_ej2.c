/*! @mainpage Actividad 2 Proyecto: Medidor de distancia por ultrasonido con interrupciones
 *
 * @section genDesc General Description
 * Se utiliza el medidor de distancia con un sensor ultrasónico de la actividad anterior, donde 
 * las funcionalidades de las teclas 1 y 2 de la placa, se implementan como interrupciones y las
 * tareas se le agregan timers
 *

 *
 * @section hardConn Hardware Connection
 *
 * |    ESP32c6	  |   Periférico   	|
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
 * |:12/04/2024:|: Generación de documentación					 |
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
/// defino el tiempo (en microsegundos) entre las interrupciones del timer. 
/// Probamos ponerlo como 1000000, pero respondían muy lento las actualizaciones, así 
/// que lo bajamos a 500000 
#define CONFIG_BLINK_PERIOD_uS 500000
/**
 * @def activar
 * @brief variable global booleana para controlar cuando el sistema está encendido o apagado
*/
bool activar = true;
/**
 * @def congelar
 * @brief variable global booleana para controlar cuando coneglar el valor en el display
*/
bool congelar = false;
/**
 * @def activar
 * @brief variable global flotante para almacenar la distancia adquirida por el sensor
*/
float distancia;

/*==================[internal data definition]===============================*/
/// tipo (struct) de FREERTOS utilizado para hacer referencia a las tareas y así
/// usar las tareas (a partir de estar referidas por estos TaskHandle) en otros 
/// comandos de vTask como Delete, Create, etc.
TaskHandle_t sensar_task_handle = NULL;
TaskHandle_t leds_task_handle = NULL;
TaskHandle_t display_task_handle = NULL;

/*==================[internal functions declaration]=========================*/
/**
 * @fn void FuncTimer(void* param)
 * @brief Función invocada en la interrupción del timer A. Con esta función defino que va 
 * a ocurrir cuando defino y utilizo el timer. Dentro de ella, creo notificaciones a las 
 * tareas que quiero ejecutar
 */
void FuncionTimer(void* param){
	//ponemos notificaciones
    vTaskNotifyGiveFromISR(leds_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada a los leds*/
	vTaskNotifyGiveFromISR(sensar_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al sensor*/
    vTaskNotifyGiveFromISR(display_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al display */
}

/**
 * @fn static void SensarTask(void *pvParameter)
 * @brief función que define la tarea sensar. Ingresa en un bucle While infinito para que siempre esté en ejecución
 * y dentro del while tenemos un ukTaskNotifyTake.
*/
static void SensarTask(void *pvParameter){
	while(true){
		/// TaskHandle_t y ulTaskNotifyTake se utilizan juntos para crear un mecanismo
		/// de sincronización entre diferentes partes de tu código
	
		/// hasta que la tarea no se notifique desde el "vTaskMptifyGiveFromISR", no se va a ejecutar esta tarea
		/// como tenemos el portMAX_DELAY: nos dice que esperará indefinidamente hasta que llegue la notificación para +
		/// ejecutarse
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* Envía una notificación a la tarea asociada al LED_2 */
		/// a partir de acá defino lo que quiero que haga mi función SensarTask
		printf("Sensando\n");
		if(activar == true){
			distancia = HcSr04ReadDistanceInCentimeters();
		}
	}
}
/**
 * @fn static void LedsTask(void *pvParameter)
 * @brief función que enciende y apaga los leds dependiendo el valor de la distancia medida por el sensor
*/
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
/**
 * @fn static void DisplayTask(void *pvParameter)
 * @brief función encargada de mostrar por display el valor de la medición
*/
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
/* para manejar los switches, a diferencia del ejercicio anterior, debo cambiar la forma de implementarlo. 
Ya no voy a revisar qué switch es el que se presiona. Ahora voy a configurar directamente cada switch por 
separado y con la función de la librería switch, "SwitchActivInt" le voy a decir qué tecla se apretó y 
qué función debe ejecutar cuando esto sucede:
1) creo dos funciones, "FuncionSwitch1" y "FuncionSwitch2" en donde adentro de la funcion cambio el 
estado de activar y congelar respectivamente
2) creo una funcion genérica, en donde le paso por parámetro en la funcion de SwitchActivInt(-- , -- , xx)
(donde se encuentra la xx) y le paso cuál tendré que modificar: activar o congelar
*/
//creo las dos funciones que serán llamadas cuando se den las interrupciones

/**
 * @fn static void FuncionSwitch(void *pvParameter)
 * @brief función encargada de invertir el estado de la variable booleana que recibe
*/
static void FuncionSwitch(void *pvParam){
	bool *puntero= (bool*)(pvParam);
	*puntero =! *puntero;
}


/*==================[external functions definition]==========================*/
void app_main(void){
	/// inicializo todos los componentes que voy a utilizar: Leds, Lcd, Sensor, Switches
	LedsInit();
	HcSr04Init(GPIO_3, GPIO_2);
	LcdItsE0803Init();
	SwitchesInit();
	/// defino que cuando se presiona los switch, que se ejecute determinada función, y le mando 
	/// los parámetros que quiero modificar al presionar cada uno
	SwitchActivInt(SWITCH_1,FuncionSwitch, &activar);
	SwitchActivInt(SWITCH_2,FuncionSwitch, &congelar);

	/// Definición del timer: lo que me hace definir más de un timer es cuando quiero trabajar con diferntes bases de tiempo.
	/// Como ahora tengo la misma base de tiempo que es 1 segundo = 1000000 microsegundos, voy a crear solo uno para las mismas tareas
    
	timer_config_t  timer_A= {
        .timer = TIMER_A, /// nombre del timer
        .period = CONFIG_BLINK_PERIOD_uS, /// período de tiempo
        .func_p = FuncionTimer, /// puntero a función que queremos que se ejecute cada el tiempo que definí en la 
		/// línea anterior. 
        .param_p = NULL
    };

	/// Inicialización del timer
	TimerInit(&timer_A);

	/// creación de las tareas que quiero ejecutar 
	xTaskCreate(&SensarTask, "Sensar", 512, NULL, 4, &sensar_task_handle);
	xTaskCreate(&LedsTask, "Leds", 512, NULL, 4, &leds_task_handle);
	xTaskCreate(&DisplayTask, "Display", 512, NULL, 4, &display_task_handle);

	/// línea donde defino que comienza a funcionar el timer. En este caso, timer_A.timer me devuelve TIMER_A
	/// porque es de la estructura definida en la línea 179, el primer ítem
	TimerStart(timer_A.timer);	

}
/*==================[end of file]============================================*/