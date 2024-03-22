/** @mainpage Proyecto 1: Guía de ejercicios prácticos.
  * Configuración de placa para obtener por display datos de 32 bits

 * @section genDesc General Description
 *
 * Aplicación donde podemos mostrar por display un dato de 32 bits
 * Escriba una función que reciba un dato de 32 bits,
 * la cantidad de dígitos de salida y dos vectores de estructuras del tipo  gpioConf_t. 
 * Uno  de estos vectores es igual al definido en el punto anterior y el otro vector 
 * mapea los puertos con el dígito del LCD a donde mostrar un dato.
 * La función deberá mostrar por display el valor que recibe, pudiendo 
 * reutilizar las funciones de los ejercicios anteriores.
 * 
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
 * | 21/03/2024 | Document creation								 |
 * | 22/03/2024 | Commit with first version of the code          |
 *
 * @author Giovanna Viñoli (giovanna.vinoli@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include gpio_muc.h

/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
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
void inicializarGPIO(gpioConfig_t *vectorGpio){
	for(int i=0; i<len(vectorGpio); i++){
		GPIOInit(vectorGpio[i].pin, vectorGpio[i].dir);
	}
}

/**
 * @fn void convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcdNumber)
 * @brief Convierte un valor de 32 bits a un array de valores de 8 bits que almacena cada
 * dígito del valor
 * @param data valor de 32 bits del cual quiero obtener cada dígito por separado
 * @param digits número de 8 bits que representa la cantidad de dígitos que posee el valor de 32 bits
 * @param bcdNumber puntero array que contiene valores de 8 bits donde se almacenarán los dígitos del valor
 * @return 
*/
void convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcdNumber){
	//creo una variable auxiliar donde voy a guardar el resto de cada operación sobre el número
	uint8_t variableAuxiliar;
	//bucle con la cantidad de dígitos que tiene el número. 
	// arraco de "mayor" a "menor" para que me guarde en orden el número 
	// en el vector bcd_number: 4, 5, 6 en vez de 6, 5, 4.
	for(int i = digits-1; i>=0; i--){
		variableAuxiliar = data%10;
		data = data/10;
		bcdNumber[i] = variableAuxiliar;
	}
	
}
/**
 * @fn void configurarBcdAGpio(uint8_t digitoBcd, gpioConfig_t *vectorGpio)
 * @brief Coloca en alto o bajo las entradas de BCD a 7 segmentos.
 * @param digitoBcd número que quiero mostrar en el display
 * @param vectorGpio puntero a array que almacena los GPIO a configurar
 * @return 
*/
void configurarBcdAGpio(uint8_t digitoBcd, gpioConfig_t *vectorGpio){
	
	for(int i=0; i<4; i++){
		if((digitoBcd & 1<<i) == 0){
			GPIOOff(vectorGpio[i].pin);
		}
		else{
			GPIOOn(vectorGpio[i].pin);
		}
	}

}
/**
 * @fn void funcion(uint32_t dato, uint8_t digitos, gpioConfig_t *vectorGpio, gpioConfig_t *vectorLCD)
 * @brief Muestra por display un valor que recibe de 32 bits
 * @param dato valor que quiero mostrar en el display
 * @param digitos cantidad de dígitos que contiene el dato
 * @param vectorGpio vector que contiene los pines y direcciones para configurar los bits de entrada
 * @param vectorLCD vector que contiene los puertos a los dígitos del LCD que voy a configurar
 * @return 
*/
void funcion(uint32_t dato, uint8_t digitos, gpioConfig_t *vectorGpio, gpioConfig_t *vectorLCD)
{
	uint8_t vectorBcd[digitos];// creo un vector que almacene cada dígito del dato
	convertToBcdArray(dato, digitos, vectorBcd); // obtengo cada dígito del dato con la funcion converToBcdArray
	fot(int i=0; i<digitos; i++){ //creo un bucle for que se repite la cantidad de dígitos que posee el número
		
		configurarBcdAGpio(vectorBcd[i], vectorGpio); // para cada dígito, configuro el vector Gpio para que me muestre el número 
		
		GPIOOn(vectorLCD[i].pin);//genero el pulso en el LCD correspondiente para que me "fije" el valor que quiero
		GPIOOff(vectorLCD[i].pin);

	}
}

/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Hello world!\n");
	uint32_t dato = 456;
	uint8_t cantidadDigitos = 3;
	gpioConfig_t vectorGpio[4] = {{GPIO_20, GPIO_OUTPUT},{GPIO_21, GPIO_OUTPUT},{GPIO_22, GPIO_OUTPUT},{GPIO_23, GPIO_OUTPUT}};
	inicializarGPIO(vectorGpio);
	gpioConfig_t vectorLCD[3] = {{GPIO_19, GPIO_OUTPUT},{GPIO_18, GPIO_OUTPUT},{GPIO_9, GPIO_OUTPUT}};
	inicializarGPIO(vectorLCD);
	funcion(dato, cantidadDigitos, vectorGpio, vectorLCD);
}
/*==================[end of file]============================================*/