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
/*==================[macros and definitions]=================================*/
#define CONFIG_PERIOD_mS_1seg 1000
#define CONFIG_PERIOD_mS_6seg 6000000
#define CONFIG_PERIOD_uS_medio_seg 500000
/*==================[internal data definition]===============================*/
TaskHandle_t medir_tiempo_task_handle = NULL;
bool lanzar = false;
bool ladrar = false;
bool motor = false;
int distancia = 0;
uint64_t tiempo = 0;
int8_t anguloMovimiento = 90;
servo_out_t servo = SERVO_0;
/*==================[internal functions declaration]=========================*/
static void funcionTimerA(void *pParam){
	vTaskNotifyGiveFromISR(medir_tiempo_task_handle,pdFALSE);

}

static void leerSensor(void *pParam){
	while(true){
		distancia = HcSr04ReadDistanceInCentimeters();
		if(distancia <= 5){
			lanzar = true;
		}
		else{
			lanzar = false;
		}
	}
}
static void ladrar(void *pParam){
	//registro con el micrófono la señal, si detecta
	//si detecta ladrido: ladrar = true
}
static void moverMotor(bool mover){
	//pwm_mcu configuro el motor (que tengo que ponerle en qué GPIO va conectado), 
	//el ciclo de trabajo y una frecuencia que será fija.
}
static void moverServo(void *pParam){
	ServoMove(servo, anguloMovimiento);
	vTaskDelay(CONFIG_PERIOD_mS_1seg/portTICK_PERIOD_MS);
	ServoMove(servo, -anguloMovimiento);
}
static void enviarDatos(void *pParam){

}
static void medir_tiempo(void *pParam){
	while(true){
		if(lanzar == false){
			TimerStart(TIMER_A);
		}
		else{
			TimerStop(TIMER_A);
			tiempo = TimerGetCount(TIMER_A);
			TimerReset(TIMER_A);
		}
	}
}
static void lanzarPelota(void *pParam){
	while(true){
		if(lanzar == true){
			if(ladrar == true){
				// encender motor 
				motor = true;
				vTaskDelay(CONFIG_PERIOD_mS_6seg/portTICK_PERIOD_MS);
				// encender servo 
				moverServo();
				//espero 6 segundos y luego apago todo
				lanzar = false;
				ladrar = false;
				vTaskDelay(CONFIG_PERIOD_mS_6seg/portTICK_PERIOD_MS);

			}
		}
		motor = false;
	}

}

/*==================[external functions definition]==========================*/
void app_main(void){
	HcSr04Init(GPIO_3, GPIO_2);

	timer_config_t timer_A ={
		.timer = TIMER_A,
		.period = CONFIG_PERIOD_uS_1seg,
		.func_p = funcionTimerA,
		.param_p = NULL
	};
	TimerInit(&timer_A);
	ServoInit(servo, GPIO_0);


	TimerStart(timer_A.timer);
}

/*==================[end of file]============================================*/