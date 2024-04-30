/*! @mainpage Template
 *
 * @section genDesc General Description
 * Se implementa un medidos de distancia por ultrasonido con 
 * interrupciones, enviando por el puerto serie los datos de los
 * valores obtenidos por el sensor y leyendo desde el mismo algunas
 * teclas para controlar la EDU-ESP32c6
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
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "switch.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "ctype.h"


/*==================[macros and definitions]=================================*/
/// defino el tiempo (en microsegundos) entre las interrupciones del timer. 
///Probamos ponerlo como 1000000, pero respondían muy lento las actualizaciones, así 
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
 * @def pulgadas
 * @brief variable global booleana para controlar cuando cambiar la unidad de medida del 
 * sensor de cm a pulgadas
*/
bool pulgadas = false;
/**
 * @def ver_maximo
 * @brief variable global booleana para controlar cuando se muestra el valor máximo de 
 * distancia adquirido por el sensor
*/
bool ver_maximo = false;
/// @brief definicion de la variable distancia (almacena los valores de distancia que adquiere
/// el sensor); maximo (almacena el valor máximo de distancia medido)
uint16_t distancia, maximo=0;
/// @brief definicion de variables de distancias, en cm, minima, media y máxima que utilizará el
/// programa para configurar la secuencia de encendido de leds
uint16_t distancia_min_cm = 10, distancia_media_cm =20, distamcia_max_cm = 30;
/// @brief definicion de variables de distancias, en pulgadas, minima, media y máxima que utilizará el
/// programa para configurar la secuencia de encendido de leds
uint16_t distancia_min_in = 10/2.54, distancia_media_in = 20/2.54, distamcia_max_in =30/2.54;

/*==================[internal data definition]===============================*/
/// @brief identificadores de FREERTOS utilizados para hacer referencia a las tareas y así
/// poder ejecutarlas (a partir de estar referidas por estos TaskHandle) en otros 
/// comandos de vTask como Delete, Create, etc.
TaskHandle_t sensar_task_handle = NULL;
TaskHandle_t leds_task_handle = NULL;
TaskHandle_t display_task_handle = NULL;
TaskHandle_t MandarDarosUART_handle = NULL;

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
	vTaskNotifyGiveFromISR(MandarDarosUART_handle, pdFALSE);
}

/**
 * @fn static void cambiarEstado(void *pvParameter)
 * @brief función encargada de invertir el estado de la variable booleana que recibe
*/
static void cambiarEstado(bool *pvParam){
	bool *puntero= (bool*)(pvParam);
	*puntero =! *puntero;
}
/**
 * @fn void FuncionUART(void *param)
 * @brief función encargada de leer lo que ingresa por el puerto serie
*/
void FuncionUART(void* param){
	uint8_t uart;
	UartReadByte(UART_PC, &uart);
	uart = toupper(uart);
	
	switch (uart)
	{
	case 'O':
		cambiarEstado(&activar);
		//activar=! activar;
		break;
	
	case 'H':
		cambiarEstado(&congelar);
		//congelar =! congelar;
		break;

	case 'I':
		cambiarEstado(&pulgadas);
		//pulgadas =! pulgadas;
		break;

	case 'M':
		cambiarEstado(&ver_maximo);
		//ver_maximo =! ver_maximo;
		break;
	}
}
/**
 * @fn static void MandarDatosUART(void *pvParameter)
 * @brief función encargada de enviar datos por el puerto serie 
*/
static void MandarDatosUART(void *pvParameter){
	while(true){ // agregarle la condicion de si esta apagado o prendido, agregarle los cm. y un \n
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if(activar == true){
			if(ver_maximo == false){
				if(pulgadas == true){
					UartSendString(UART_PC,(char*) UartItoa(distancia, 10));
					UartSendString(UART_PC, " in\n");
				}
				else{
					UartSendString(UART_PC,(char*) UartItoa(distancia, 10));
					UartSendString(UART_PC, " cm\n");
				}
			}
			else{
				UartSendString(UART_PC, " El valor máximo en cm es: ");
				UartSendString(UART_PC,(char*) UartItoa(maximo, 10));
				UartSendString(UART_PC, "\n");

			}

		}
		
	}//pyserial ver 
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
		//printf("Sensando\n");
		if(activar == true){
			if(pulgadas == false){
				distancia = HcSr04ReadDistanceInCentimeters();
				if(distancia > maximo){
					maximo = distancia;
				}
			}
			else{
				distancia = HcSr04ReadDistanceInInches();
				if((distancia*2.54) > maximo){
					maximo = (distancia*2.54);
				}
			}
		}
	}
}
/**
 * @fn void configurarLeds(void *pvParameter)
 * @brief función que enciende y apaga los leds dependiendo el valor de la distancia medida por el sensor
*/
void configurarLeds(uint16_t distancia_min, uint16_t distancia_media, uint16_t distancia_max){
	if(distancia <= distancia_min){
		LedsOffAll();
	}	
	else if( distancia <= distancia_media && distancia >distancia_min){
		LedOn(LED_1);
		LedOff(LED_2);
		LedOff(LED_3);
	}
	else if(distancia <= distancia_max && distancia > distancia_media){
		LedOn(LED_1);
		LedOn(LED_2);
		LedOff(LED_3);
	}
	else if (distancia > distancia_max){
		LedOn(LED_1);
		LedOn(LED_2);
		LedOn(LED_3);
	}
}
/**
 * @fn static void LedsTask(void *pvParameter)
 * @brief función que verifica si el sistema está activo, verifica en qué unidad de debe
 * trabajar la distancia, y enciende o no los leds
*/
static void LedsTask(void *pvParameter){
	//el portMAXdelay te dice que espere infinito por la notificación. Yo podría decirle, che, espera 1 segundo por la notificaicón
	while(true){
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* Envía una notificación a la tarea asociada al LED_2 */
		if(activar == true){
			//printf("%i\n", distancia);
			if(pulgadas == false){
				configurarLeds(distancia_min_cm, distancia_media_cm, distamcia_max_cm);
			}
			else{
				configurarLeds(distancia_min_in, distancia_media_in, distamcia_max_in);
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


/*==================[external functions definition]==========================*/
void app_main(void){
	/// inicializo todos los componentes que voy a utilizar: Leds, Lcd, Sensor, Switches
	LedsInit();
	HcSr04Init(GPIO_3, GPIO_2);
	LcdItsE0803Init();
	SwitchesInit();

	/// defino que cuando se presiona los switch, que se ejecute determinada función, y le mando 
	/// otro parámetro
	SwitchActivInt(SWITCH_1,cambiarEstado, &activar);
	SwitchActivInt(SWITCH_2,cambiarEstado, &congelar);

	// Congifuracion de puertos:
	//GPIOInit(pin , dirección: input u  output);

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

	/// configuración de la UART
	serial_config_t Uart = {
		.port = UART_PC,
		.baud_rate = 9600,
		.func_p = FuncionUART,
		.param_p = NULL
	};

	/// Inicializo el UART
	UartInit(&Uart);

	/// creación de las tareas que quiero ejecutar 
	xTaskCreate(&SensarTask, "Sensar", 512, NULL, 4, &sensar_task_handle);
	xTaskCreate(&LedsTask, "Leds", 512, NULL, 4, &leds_task_handle);
	xTaskCreate(&DisplayTask, "Display", 512, NULL, 4, &display_task_handle);
	xTaskCreate(&MandarDatosUART, "UART", 512, NULL, 4, &MandarDarosUART_handle);

	/// línea donde defino que comienza a funcionar el timer. En este caso, timer_A.timer me devuelve TIMER_A
	/// porque es de la estructura definida en la línea 179, el primer ítem
	TimerStart(timer_A.timer);	

}
/*==================[end of file]============================================*/