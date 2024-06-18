/*! @mainpage Proyecto Integrador: Lanza pelotas
 *
 * @section genDesc General Description
 *
 * Proyecto donde se desarrollará un lanza pelotas, el cual a través 
 * de la detección de ladrido por micrófono accionará el sistema y 
 * lanzará la pelota. 
 * El sistema tendrá la capacidad de guardar el tiempo que demora
 * la pelota en retornar al dispositivo, enviándola a una aplicación 
 * móvil por bluetooth
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	l293	 	| 	GPIO_20, GPIO_21, GPIO_22		|
 * | tracker ir tcrt5000 	| 	GPIO_3		|
 * | 	micrófono	 	| 	GPIO_1		|
 * 
 * 
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 16/05/2024 | Creación del proyecto		                     |
 * | 18/06/2024 | Creación de documentación                      |
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
#include "pwm_mcu.h"
#include "ble_mcu.h"
#include "l293.h"
#include "switch.h"
#include "string.h"
#include "hc_sr04.h"
/*==================[macros and definitions]=================================*/
/// Definición de períodos de tiempo a utilizar
#define CONFIG_PERIOD_mS_1seg 1000
#define CONFIG_PERIOD_mS_4seg 4000
#define CONFIG_PERIOD_uS_A 1000 

/// Definición del tamaño del vector de muestras de audio
#define CHUNK 16
/*==================[internal data definition]===============================*/

/** @def gpio 
 * @brief variable booleana que registra la lectura del sensor
 * infrarrojo, informando si está o no la pelota
 */
bool gpio = false;

/** @def ladrar 
 * @brief variable booleana que informa si el micrófono registró
 * un ladrido
 */
bool ladrar = false;

/** @def botonLanzar 
 * @brief variable booleana que informa si se accionó el botón 
 * para activar el sistema de lanzar la pelota
 */
bool botonLanzar = false;

/** @def mensaje  
 * @brief cadena de caracteres utilizada para enviar el tiempo
 * que demora en retornar la pelota a la ubicación inicial
 */
char mensaje[20];

/** @def data 
 * @brief variable que almacena las muestras tomadas por el micrófono
 */
uint16_t data[50];

/** @def umbral 
 * @brief valor que debe superar la lectura del micrófono para reconocer
 * si el perro ladra o no
 */
int umbral = 1900;

/// Identificadores de las tareas implementadas
TaskHandle_t medir_tiempo_handle = NULL;
TaskHandle_t lanzar_pelota_handle = NULL;
TaskHandle_t switch_handle = NULL;
TaskHandle_t reconocer_Ladrar = NULL;
	
/*==================[internal functions declaration]=========================*/

/**
 * @fn void funcionTimerA(void *pParam)
 * @brief función que ejecutará el timer utilizado en el programa, para notificar
 * las diferentes tareas
*/
void funcionTimerA(void *pParam){
	vTaskNotifyGiveFromISR(reconocer_Ladrar, pdFALSE);
}

/**
 * @fn static void reconocerLadrar(void *pParam)
 * @brief función que reconoce si el perro ladra y cambia el estado de la variable
 * global a verdadero, de ser así.
*/
static void reconocerLadrar(void *pParam){
	int i = 0;
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		AnalogInputReadSingle(CH1, &data[i]);
		i++; 
		if(i == CHUNK){
			for(int j = 0; j < CHUNK; j++){
				if(data[j]>umbral){
					ladrar = true;
					printf("ladrar es true\n");
					//printf("vector%u\n", data[j]);
				}
			}
			i =0;
		}
		
	}
}

/**
 * @fn static void leerDatosBle(uint8_t data, uint8_t length)
 * @brief función encargada de leer la información que envía la aplicación del 
 * teléfono al dispositivo, por medio del bluetooth. Esta función se utiliza en 
 * el struct utilizado para inicializar el módulo bluetooth
*/
static void leerDatosBle(uint8_t * data, uint8_t length){
	switch(data[0]){
	case 'D':
		botonLanzar = true;
		break;
	case 'd':
		botonLanzar = false;
		break;
	}
}

/**
 * @fn static void enviarDatos(int tiempo)
 * @brief función encargada de enviar el tiempo que demora la pelota en retornar
 * a su posición inicial (en el dispositivo)
 * @param tiempo valor que se desea enviar
*/
static void enviarDatos(int tiempo){
	strcpy(mensaje, "");
	/* Para enviar al cuadro destinado para mostrar el tiempo en la app, 
	se debe enviar la cadena de caracteres con la información, pero al 
	inicio el caracter identificador del widget*/
	sprintf(mensaje, "*aTiempo: %u\n*", tiempo);
	BleSendString(mensaje);
}

/**
 * @fn static void medir_tiempo(void *pParam)
 * @brief función encargada de medir el tiempo que transcurre entre que la 
 * pelota es lanzada y cuando retorna a la posición inicial
*/
static void medir_tiempo(void *pParam){
	bool timerStart = false, gpio_ant = false, enviar = false;
	uint16_t contador=0;
	while(true){
		gpio = GPIORead(GPIO_3);
		if(gpio_ant == true && gpio == false){
			timerStart = true;
			//printf("Comienzo a contar\n"); 
		}
		if(gpio_ant == false && gpio == true && timerStart == true){
			timerStart = false;
			//printf("Paro de contar\n"); 
			//printf("Tiempo: %d\n", tiempo);
		}
		if(timerStart == true){
			contador +=1;
			enviar = true;
		}
		if(timerStart == false && enviar == true){
			enviarDatos(contador);
			//printf("Tiempo: %u\n", contador);
			contador = 0;
			enviar = false;
		}
		gpio_ant = gpio;
		vTaskDelay(CONFIG_PERIOD_mS_1seg/portTICK_PERIOD_MS);
	}
}

/**
 * @fn static void moverMotor(bool mover)
 * @brief función que acciona el motor dependiendo el valor de la variable que
 * recibe como parámetro
 * @param mover variable booleana que define si se enciende o no el motor
*/
static void moverMotor(bool mover){
	if(mover == true){
		L293SetSpeed(MOTOR_1, 100);
		//printf("Motor on\n");
	}
	else{
		L293SetSpeed(MOTOR_1, 0);
		//printf("Motor off\n");
	}
}

/**
 * @fn static void lanzarPelota(void *pParam)
 * @brief función encargada de lanzar la pelota si se dan las condiciones deseadas
 * (que el botón de lanzar esté accionado, que el perro esté ladrando y que 
 * la pelota esté en el dispositivo)
*/
static void lanzarPelota(void *pParam){
	while(true){
		gpio = GPIORead(GPIO_3);
		if(botonLanzar == true){
			if(ladrar==true && gpio==true){
				moverMotor(true);
				ladrar=false;
			}
			else{
				moverMotor(false);
			}
		}
		else{
			moverMotor(false);
		}
		vTaskDelay(CONFIG_PERIOD_mS_4seg/portTICK_PERIOD_MS);
	}
}

/**
 * @fn static void switchTask(void *pParam)
 * @brief función encargada de cambiar el estado de la variable booleana 
 * 'lanzarPelota' si se activa la tecla 1
*/
static void switchTask(void *pvParam){
	uint8_t teclas;
	while(true){
		teclas = SwitchesRead();
		switch (teclas)
		{
			case SWITCH_1:
				botonLanzar =! botonLanzar;
				printf("tecla 1 pulsada\n");
				break;
		}
		vTaskDelay(CONFIG_PERIOD_mS_1seg / portTICK_PERIOD_MS);
	}
}
/*==================[external functions definition]==========================*/
void app_main(void){
	/// inicialización del switch
	SwitchesInit();

	/// inicialización del GPIO 3 como entrada
	GPIOInit(GPIO_3, GPIO_INPUT);
	
	/// definicion del struct para configurar el bluetooth 
	ble_config_t ble_configuration = {
        "Lanzador-de-pelotas",
        leerDatosBle
    };

	/// inicialización del bluetooth
	BleInit(&ble_configuration);

	/// inicialización del L293D
	L293Init();

	/// configuración e inicialización del canal 1 para leer los datos del micrófono
	analog_input_config_t Analog_config = {
		.input = CH1,
		.mode = ADC_SINGLE
	};
	AnalogInputInit(&Analog_config);
	AnalogOutputInit();

	/// configuración e inicialización del timer
	timer_config_t timerA = {
		.timer = TIMER_A,
		.period = CONFIG_PERIOD_uS_A,
		.func_p = funcionTimerA,
		.param_p = NULL
	};
	TimerInit(&timerA);

	/// creación de tareas
	xTaskCreate(&medir_tiempo, "medir_tiempo", 2048, NULL, 4, &medir_tiempo_handle);
	xTaskCreate(&lanzarPelota, "lanzar_pelota", 2048, NULL, 4, &lanzar_pelota_handle);
	xTaskCreate(&switchTask, "switch", 2048, NULL, 4, &switch_handle);
	xTaskCreate(&reconocerLadrar, "switch", 2048, NULL, 4, &reconocer_Ladrar);

	/// activación del timer
	TimerStart(timerA.timer);
}

/*==================[end of file]============================================*/