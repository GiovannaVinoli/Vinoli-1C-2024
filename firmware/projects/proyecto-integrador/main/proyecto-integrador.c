/*! @mainpage Proyecto Integrador: Lanza pelotas
 *
 * @section genDesc General Description
 *
 * Proyecto donde se desarrollará un lanza pelotas, el cual a través de la detección de ladrido por micrófono
 * accionará el sistema y lanzará la pelota.  El sistema tendrá la capacidad de guardar el tiempo que demora
 * la pelota en retornar al dispositivo, enviándola a una aplicación móvil por bluetooth
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 16/05/2024 | Creación del proyecto		                         |
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
//#include "iir_filter.h"
#include "hc_sr04.h"
/*==================[macros and definitions]=================================*/
// defino todos los tiempos en mili segundos.
#define CONFIG_PERIOD_mS_1seg 1000
#define CONFIG_PERIOD_mS_4seg 4000
#define CONFIG_PERIOD_uS_1seg 1000000
//#define CONFIG_PERIOD_mS_125uS 0.125
#define CONFIG_PERIOD_uS_A 1000 // trabajo a una frecuencia de 8KHz --> 1/8000 = 125 uSegundos.
#define fm 8000
#define CHUNK 16
/*==================[internal data definition]===============================*/
TaskHandle_t medir_tiempo_task_handle = NULL;
bool gpio = false;
bool ladrar = false;
bool botonLanzar = false;
char mensaje[20];
int distancia = 0;
//volatile uint16_t tiempo;
uint16_t data[50];
float data_filt[CHUNK];
int umbral = 1900;

//TaskHandle_t enviar_datos_handle = NULL;
TaskHandle_t medir_tiempo_handle = NULL;
TaskHandle_t lanzar_pelota_handle = NULL;
TaskHandle_t switch_handle = NULL;
TaskHandle_t reconocer_Ladrar = NULL;
	
/*==================[internal functions declaration]=========================*/
void funcionTimerA(void *pParam){
	//agrego notificaciones
	vTaskNotifyGiveFromISR(reconocer_Ladrar, pdFALSE);
}

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
// envío los datos del tiempo que demoró en volver la pelota al dispositivo
// por el módulo bluetooth a mi app. Como quiero mostrarlo en un panel de texto, 
// yo lo configuré para que si tiene un "*a" al inicio del mensaje se envíe ahí

static void enviarDatos(int tiempo){
	strcpy(mensaje, "");
	sprintf(mensaje, "*aTiempo: %u\n*", tiempo);
	BleSendString(mensaje);
}
// leo desde el sensor infrarrojo si está o no la pelota, para
// saber si accionar o no el dispositivo (no voy a "lanzar pelota" si no está)

static void medir_tiempo(void *pParam){
	bool timerStart = false, gpio_ant = false, enviar = false;
	uint16_t contador=0;
	while(true){
		gpio = GPIORead(GPIO_3);
		//printf("Mido tiempo\n");
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
			//tiempo = contador;
			enviarDatos(contador);
			printf("Tiempo: %u\n", contador);
			contador = 0;
			enviar = false;
		}
		gpio_ant = gpio;
		vTaskDelay(CONFIG_PERIOD_mS_1seg/portTICK_PERIOD_MS);
	}
}
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
static void lanzarPelota(void *pParam){
	while(true){
		//leerDatosBle();
		//printf("entro a lanzar pelota\n");
		gpio = GPIORead(GPIO_3);
		//printf("leo gpio\n");
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

static void switchTask(void *pvParameter){
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

	SwitchesInit();

	//HcSr04Init(GPIO_3, GPIO_2);
	// GPIO 1 configurado con el sensor infrarrojo
	GPIOInit(GPIO_3, GPIO_INPUT);

 
 	/// configuración de la UART
	serial_config_t Uart = {
		.port = UART_PC,
		.baud_rate = 921600,
		.func_p = NULL,
		.param_p = NULL
	};
	UartInit(&Uart);
	
	ble_config_t ble_configuration = {
        "EDU_GIO",
        leerDatosBle
    };

	BleInit(&ble_configuration);

	L293Init();

	analog_input_config_t Analog_config = {
		.input = CH1,
		.mode = ADC_SINGLE
	};

	AnalogInputInit(&Analog_config);
	AnalogOutputInit();

	timer_config_t timerA = {
		.timer = TIMER_A,
		.period = CONFIG_PERIOD_uS_A,
		.func_p = funcionTimerA,
		.param_p = NULL
	};
	/// inicialización del timerA
	TimerInit(&timerA);

	//xTaskCreate(&enviarDatos, "enviar_datos", 2048, NULL, 4, &enviar_datos_handle);
	xTaskCreate(&medir_tiempo, "medir_tiempo", 2048, NULL, 4, &medir_tiempo_handle);
	xTaskCreate(&lanzarPelota, "lanzar_pelota", 2048, NULL, 4, &lanzar_pelota_handle);
	xTaskCreate(&switchTask, "switch", 2048, NULL, 4, &switch_handle);
	xTaskCreate(&reconocerLadrar, "switch", 2048, NULL, 4, &reconocer_Ladrar);

	TimerStart(timerA.timer);

}

/*==================[end of file]============================================*/