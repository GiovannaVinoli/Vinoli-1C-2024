/*! @mainpage Blinking
 *
 * \section genDesc General Description
 *
 * This example makes LED_1, LED_2 and LED_3 blink at different rates, using FreeRTOS tasks.
 * 
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_LED_1 1000
#define CONFIG_BLINK_PERIOD_LED_2 1500
#define CONFIG_BLINK_PERIOD_LED_3 500
/*==================[internal data definition]===============================*/
TaskHandle_t led1_task_handle = NULL;
TaskHandle_t led2_task_handle = NULL;
TaskHandle_t led3_task_handle = NULL;
/*==================[internal functions declaration]=========================*/
static void Led1Task(void *pvParameter){
    while(true){
        printf("LED_1 ON\n");
        LedOn(LED_1);
        vTaskDelay(CONFIG_BLINK_PERIOD_LED_1 / portTICK_PERIOD_MS); //es del kernel del FreeRTOS
        //que me permite liberar el micro.
        printf("LED_1 OFF\n");
        LedOff(LED_1);
        vTaskDelay(CONFIG_BLINK_PERIOD_LED_1 / portTICK_PERIOD_MS);
    }
    //en esta funcion estoy en un bucle while infinito, nunca voy a salir. Ahora como tengo el vTaskDelay, 
    //esa funcion me permite "liberar el micro" y permitir ejecutar otra tarea de la lista de espera. Pasado el 
    //tiempo de delay que definimos allí, esta tarea estará nuevamente disponible para que cuando el micro se 
    //libere, pueda ejecutarla de nuevo.
    //El schedule tiene el control de decidir a quien le da o elige que tarea ejecutar, pero hasta que las 
    //tareas no lo liberan, no se puede cambiar.
}

static void Led2Task(void *pvParameter){
    while(true){
        printf("LED_2 ON\n");
        LedOn(LED_2);
        vTaskDelay(CONFIG_BLINK_PERIOD_LED_2 / portTICK_PERIOD_MS);
        printf("LED_2 OFF\n");
        LedOff(LED_2);
        vTaskDelay(CONFIG_BLINK_PERIOD_LED_2 / portTICK_PERIOD_MS);
    }
}

static void Led3Task(void *pvParameter){
    while(true){
        printf("LED_3 ON\n");
        LedOn(LED_3);
        vTaskDelay(CONFIG_BLINK_PERIOD_LED_3 / portTICK_PERIOD_MS);
        printf("LED_3 OFF\n");
        LedOff(LED_3);
        vTaskDelay(CONFIG_BLINK_PERIOD_LED_3 / portTICK_PERIOD_MS);
    }
}
/*==================[external functions definition]==========================*/
void app_main(void){ // acá voy a crear tareas 
    LedsInit();
    //xTaskCreate: funciones del Kernel que crea una tarea. La tarea Main crea nuevas tareas
    //debo parametrizarla: nomrbe de tarea, etiqueta, tamaño de memoria RAM que le asigno, 
    //campo para pasar parametros (si no quiero nada, arranco con un NULL), nivel de prioridad, puntero a la tarea
    xTaskCreate(&Led1Task, "LED_1", 512, NULL, 5, &led1_task_handle);
    xTaskCreate(&Led2Task, "LED_2", 512, NULL, 5, &led2_task_handle);
    xTaskCreate(&Led3Task, "LED_3", 512, NULL, 5, &led3_task_handle);
}
