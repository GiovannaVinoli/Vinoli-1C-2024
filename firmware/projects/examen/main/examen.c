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
#include "analog_io_mcu.h"
#include "uart_mcu.h"
#include "timer_mcu.h"
#include "gpio_mcu.h"
#include "switch.h"

/*==================[macros and definitions]=================================*/
/// Definición de períodos de tiempo que utilizaré (1, 3 y 5 segundos)
#define CONFIG_PERIOD_ms_1_SEG 1000
#define CONFIG_PERIOD_mS_3_SEG 3000
#define CONFIG_PERIOD_mS_5_SEG 5000


/** @def encender
 *  @brief variable booleana global que registra si el sistema está ON u OFF
*/
bool encender = false;

/** @def bomba_pHA
 *  @brief variable booleana global que registra si la bomba de pHA está encendida 
*/
bool bomba_pHA = false;

/** @def bomba_pHB
 *  @brief variable booleana global que registra si la bomba de pHB está encendida 
*/
bool bomba_pHB = false;

/** @def bomba_agua
 *  @brief variable booleana global que registra si la bomba de agua está encendida 
*/
bool bomba_agua = false;

/** @def valores_pH
 *  @brief variable que almacena los valores de pH en voltios
*/
uint16_t valores_pH;
/*==================[internal data definition]===============================*/
/// Identificadores de las tareas implementadas
TaskHandle_t switches_task_handle = NULL;
TaskHandle_t sensar_humedad_task_handle = NULL;
TaskHandle_t sensar_pH_task_handle = NULL;
TaskHandle_t enviar_datos_UART_task_handle = NULL;

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
 * @fn static void Sensar_pH(void *pvParameter)
 * @brief función que define la tarea sensar el pH de la maceta y accionar las bombas
 * de ser necesario para normalizarlo.
*/
static void Sensar_pH(void *pvParameter){

	while(true){
		if(encender == true){
			AnalogInputReadSingle(CH3, &valores_pH);
			//convierto a valores de pH utilizando la función lineal: y = (3/14)* x
			if((valores_pH*(3/14)) < 6){
				GPIOOn(GPIO_23);//enciendo la bomba de pHB
				bomba_pHB = true;
			}
			if((valores_pH*(3/14)) > 6.7){
				GPIOOn(GPIO_22);//enciendo la bomba de pHA
				bomba_pHA = true;
			}
			else{
				GPIOOff(GPIO_22);
				GPIOOff(GPIO_23);
				bomba_pHA = false;
				bomba_pHB = false;
			}
		}
		else{
			GPIOOff(GPIO_22);
			GPIOOff(GPIO_23);
			bomba_pHA = false;
			bomba_pHB = false;
		}
		vTaskDelay(CONFIG_PERIOD_mS_3_SEG/portTICK_PERIOD_MS);
	}
}

/**
 * @fn static void Sensar_Humedad(void *pvParameter)
 * @brief función que define la tarea sensar la humedad de la maceta y accionar la bomba de agua 
 * de ser necesario para normalizarla.
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
				bomba_agua = true;
			}
			else{
				GPIOOff(GPIO_21);
				bomba_agua = false;
			}
		}
		else{
			GPIOOff(GPIO_21);
			bomba_agua = false;
		}
		vTaskDelay(CONFIG_PERIOD_mS_3_SEG/portTICK_PERIOD_MS);
	}
}

/**
 * @fn void funcionUART(void *param)
 * @brief función definida para configurar la UART
*/
void funcionUART(void* param){
	//como no leemos nada desde el puerto, no se configura
}

/**
 * @fn void enviar_datos_UART(void *param)
 * @brief función definida para enviar mensajes por la UART
*/
static void enviar_datos_UART(void* param){
	while (true)
	{
		if(encender == true){
			if(GPIORead(GPIO_1) == false){
				UartSendString(UART_PC, "pH:");
				UartSendString(UART_PC,(char*) UartItoa((valores_pH*(3/14)), 10));
				UartSendString(UART_PC, ", humedad correcta\n");
			}
			if(GPIORead(GPIO_1) == true){
				UartSendString(UART_PC, "pH:");
				UartSendString(UART_PC,(char*) UartItoa((valores_pH*(3/14)), 10));
				UartSendString(UART_PC, ", humedad incorrecta\n");
			}
			if(bomba_agua == true){
				UartSendString(UART_PC, "Bomba de agua encendida\n");
			}
			if(bomba_pHA == true){
				UartSendString(UART_PC, "Bomba de pHA encendida\n");
			}
			if(bomba_pHB == true){
				UartSendString(UART_PC, "Bomba de pHB encendida\n");
			}
		}
		vTaskDelay(CONFIG_PERIOD_mS_5_SEG/portTICK_PERIOD_MS);
	}
}

/**
 * @fn static void Encender_Sistema(void *pvParameter)
 * @brief función encargada de poner la variable encender en true
*/
static void Encender_Sistema(void *pvParam){
	bool *puntero= (bool*)(pvParam);
	*puntero = true;
}

/**
 * @fn static void Apagar_Sistema(void *pvParameter)
 * @brief función encargada de poner la variable encender en false
*/
static void Apagar_Sistema(void *pvParam){
	bool *puntero= (bool*)(pvParam);
	*puntero = false;
}
/*==================[external functions definition]==========================*/
void app_main(void){

	// inicialización:	
	SwitchesInit();
	
	gpioConfig_t vectorGpio[4] = {{GPIO_1, GPIO_INPUT},{GPIO_21, GPIO_INPUT},{GPIO_22, GPIO_INPUT},{GPIO_23, GPIO_INPUT}};
	inicializarGPIO(vectorGpio, 4);
	
	analog_input_config_t Analog_config = {
		.input = CH3,
		.mode = ADC_SINGLE
	};

	AnalogInputInit(&Analog_config);

	// configuración de la UART
	serial_config_t Uart = {
		.port = UART_PC,
		.baud_rate = 9800,
		.func_p = funcionUART,
		.param_p = NULL
	};

	/// inicializo la UART
	UartInit(&Uart);

	// configuración de interrupciones para las teclas de encendido y apagado
	SwitchActivInt(SWITCH_1,Encender_Sistema, &encender);
	SwitchActivInt(SWITCH_2,Apagar_Sistema, &encender);

	//tareas
	xTaskCreate(&Sensar_Humedad, "Sensar_humedad", 512, NULL, 4, &sensar_humedad_task_handle);
	xTaskCreate(&Sensar_pH, "Sensar_pH", 1024, NULL, 4, &sensar_pH_task_handle);
	xTaskCreate(&enviar_datos_UART, "Sensar_pH", 1024, NULL, 4, &enviar_datos_UART_task_handle);


}
/*==================[end of file]============================================*/