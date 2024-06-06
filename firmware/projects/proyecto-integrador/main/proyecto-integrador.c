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
#include "servo_sg90.h"
#include "pwm_mcu.h"
#include "ble_mcu.h"
#include "l293.h"
#include "switch.h"
#include "string.h"
/*==================[macros and definitions]=================================*/
// defino todos los tiempos en mili segundos.
#define CONFIG_PERIOD_mS_1seg 1000
#define CONFIG_PERIOD_mS_6seg 6000
#define CONFIG_PERIOD_200ms 200
#define CONFIG_PERIOD_uS_1seg 1
/*==================[internal data definition]===============================*/
TaskHandle_t medir_tiempo_task_handle = NULL;
bool lanzar = false;
bool ladrar = false;
bool botonLanzar = false;
char mensaje[10];
int distancia = 0;
uint64_t tiempo = 0;

TaskHandle_t enviar_datos_handle = NULL;
TaskHandle_t medir_tiempo_handle = NULL;
TaskHandle_t lanzar_pelota_handle = NULL;
TaskHandle_t switch_handle = NULL;
	
	timer_config_t timer_A ={
		.timer = TIMER_A,
		.period = CONFIG_PERIOD_uS_1seg,
		.func_p = funcionTimerA,
		.param_p = NULL
	};
/*==================[internal functions declaration]=========================*/
static void funcionTimerA(void *pParam){}

static void reconocerLadrar(void *pParam){
	//registro con el micrófono la señal, si detecta
	//si detecta ladrido: ladrar = true
}

static void moverMotor(bool mover){
	if(mover == true){
		L293SetSpeed(MOTOR_1, 100);
	}
	else{
		L293SetSpeed(MOTOR_1, 0);
	}
}

// leo lo que me devuelve el bluetooth
// en mi caso solo acciono un botón para lanzar o no la pelota

static void leerDatosBle(uint8_t * data, uint8_t length){
	switch(data[0]){
	case 'D':
		lanzar = true;
		break;
	case 'd':
		lanzar = false;
		break;
	}
}
// envío los datos del tiempo que demoró en volver la pelota al dispositivo
// por el módulo bluetooth a mi app. Como quiero mostrarlo en un panel de texto, 
// yo lo configuré para que si tiene un "*a" al inicio del mensaje se envíe ahí

static void enviarDatos(void *pParam){
	sprintf(mensaje, "*a%lld*", tiempo);
	BleSendString(mensaje);
	vTaskDelay(CONFIG_PERIOD_mS_6seg/portTICK_PERIOD_MS);
}
// leo desde el sensor infrarrojo si está o no la pelota, para
// saber si accionar o no el dispositivo (no voy a "lanzar pelota" si no está)

static void medir_tiempo(void *pParam){
	bool gpio, gpio_ant = false;
	while(true){
		gpio = GPIORead(GPIO_23);
		
		if(gpio_ant = true && gpio == false){
			TimerStart(timer_A.timer);
		}

		if(gpio_ant == false && gpio == true){
			TimerStop(timer_A.timer);
			tiempo = TimerGetCount(timer_A.timer);
			TimerReset(timer_A.timer);
		}
		gpio_ant = gpio;
		vTaskDelay(CONFIG_PERIOD_mS_1seg/portTICK_PERIOD_MS);
	}
}

static void lanzarPelota(void *pParam){
	while(true){
		//leerDatosBle();
		if(lanzar == true){
			if(ladrar == true || botonLanzar == true){
				// encender motor 
				moverMotor(true);
				//espero 6 segundos y luego apago todo
				lanzar = false;
			}
		}
		moverMotor(false);
		vTaskDelay(CONFIG_PERIOD_mS_6seg/portTICK_PERIOD_MS);
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
			break;
		}
		vTaskDelay(CONFIG_PERIOD_200ms / portTICK_PERIOD_MS);
	}
}
/*==================[external functions definition]==========================*/
void app_main(void){

	SwitchesInit();

	// GPIO configurado con el sensor infrarrojo
	GPIOInit(GPIO_3, GPIO_INPUT);
	TimerInit(&timer_A);
	
	ble_config_t ble_configuration = {
        "ESP_EDU_1",
        leerDatosBle
    };

	BleInit(&ble_configuration);

	L293Init();

	xTaskCreate(&enviarDatos, "enviar_datos", 512, NULL, 4, &enviar_datos_handle);
	//xTaskCreate(&medir_tiempo, "medir_tiempo", 512, NULL, 4, &medir_tiempo_handle);
	xTaskCreate(&lanzarPelota, "lanzar_pelota", 512, NULL, 4, &lanzar_pelota_handle);
	xTaskCreate(&switchTask, "switch", 512, NULL, 4, &switch_handle);

}

/*==================[end of file]============================================*/