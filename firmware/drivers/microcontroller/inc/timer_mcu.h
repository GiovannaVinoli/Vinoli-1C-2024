#ifndef TIMER_MCU_H
#define TIMER_MCU_H

/** \addtogroup Drivers_Programable Drivers Programable
 ** @{ */
/** \addtogroup Drivers_Microcontroller Drivers microcontroller
 ** @{ */
/** \addtogroup Timer Timer
 ** @{ */

/** \brief Timer driver for the ESP-EDU Board.
 * 
 * @author Albano Peñalva
 *
 * @section changelog
 *
 * |   Date	    | Description                                    						|
 * |:----------:|:----------------------------------------------------------------------|
 * | 20/10/2023 | Document creation		                         						|
 * Los timer debemos inicializarlos, podemos darle start y stop.
 * 
 **/

/*==================[inclusions]=============================================*/
#include "stdint.h"
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/
/**
 * @brief List of available timers in this driver
 */
typedef enum timers {
	TIMER_A,					/*!< Timer A */
	TIMER_B,					/*!< Timer B */
	TIMER_C						/*!< Timer C */
} timer_mcu_t;
/**
 * @brief Timer configuration struct
 */
typedef struct {				
	timer_mcu_t timer;		/*!< Selected timer : definimos que timer quiero usar*/ 
	uint32_t period;		/*!< Period (in us) : cada cuanto tiempo queremos que el timer me genere una interrupción*/
	void *func_p;			/*!< Pointer to callback function to call periodically : que función queremos que se ejecute cada
	ese lapso de tiempo*/
	void *param_p;			/*!< Pointer to callback function parameter */
} timer_config_t;
/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/
/**
 * @brief Timer initialization
 * 
 * @note Timer are stopped after init
 * 
 * @param timer_ini Pointer to timer configuration: estructura de inicialización que recibe 4 parametros
 */
void TimerInit(timer_config_t *timer_ini);

/**
 * @brief Start timer count
 * 
 * @param timer Timer number
 */
void TimerStart(timer_mcu_t timer);

/**
 * @brief Pause timer
 * 
 * @param timer Timer number
 */
void TimerStop(timer_mcu_t timer);

/**
 * @brief Reset timer count to 0
 * 
 * @param timer Timer number
 */
void TimerReset(timer_mcu_t timer);

/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
#endif 

/*==================[end of file]============================================*/
