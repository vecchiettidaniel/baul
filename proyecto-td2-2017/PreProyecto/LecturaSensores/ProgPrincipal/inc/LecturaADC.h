/*
 * LecturaADC.h
 *
 *  Created on: Sep 13, 2017
 *      Author: daniel
 */

#ifndef PROGPRINCIPAL_INC_LECTURAADC_H_
#define PROGPRINCIPAL_INC_LECTURAADC_H_

#include <chip.h>
#include <semphr.h>
#include <FreeRTOS.h>

//Hardware y configuracion del ADC
#define AD0a3_port	((uint8_t) 0)
#define AD0_pin		((uint8_t) 23)
#define AD1_pin		((uint8_t) 24)
#define AD2_pin		((uint8_t) 25)
#define AD3_pin		((uint8_t) 26)

#define AD4y5_port	((uint8_t) 1)
#define AD4_pin		((uint8_t) 30)
#define AD5_pin		((uint8_t) 31)

#define AD_Frec_Muestreo	ADC_MAX_SAMPLE_RATE

// Demoras y Tiempos
#define Tiempo_Entre_Lecturas	(1000/portTICK_RATE_MS)
#define Tiempo_Entre_Muestras	(50/portTICK_RATE_MS)
#define Tiempo_Cola_Llena		portMAX_DELAY//(200/portTICK_RATE_MS)
#define Tiempo_Cola_Vacia		portMAX_DELAY//(200/portTICK_RATE_MS)

#define Tiempo_Refresco_Pantalla	portMAX_DELAY //(200/portTICK_RATE_MS)

//Toma de Muestras
#define Cantidad_Sensores_Max			6	//máximo 6
#define Cantidad_Muestras_Tomadas		10
#define Cantidad_Muestras_Descartadas	4	//Debera ser numero par; se elimina los valores de los extremos

#define Descartar_Desde	(Cantidad_Muestras_Descartadas/2)
#define Descartar_Hasta (Cantidad_Muestras_Tomadas-(Cantidad_Muestras_Descartadas/2))

#define Descartar_N_Bits	3 	// Descartamos bits menos significativos
#define Descartar_Bits		((uint16_t) ~((1<<Descartar_N_Bits)-1)) // Mascara para borrar los bits descartados
#define Sensibilidad_Temp	127	// Valor obtenido de tablas, minima diferencia dicernible por el sistema

#define ADC_Valor_Sin_Conexion	((uint16_t) 740)	// Valor de 4[mA]

#define Error_Conversion	((uint16_t) 5000)
#define Error_Conexion 		((int32_t) 10000)//corresponde a un valor de 100 ºC
#define Sensor_Anulado 		((int32_t) 20000)//corresponde a un valor de 200 ºC


typedef struct esDatos_ADC
 {
	uint8_t ID;
	uint16_t dato[Cantidad_Muestras_Tomadas];
 }esDatos_ADC;

/*=====================================
 *===			Funciones			===
 *=====================================*/

void ADC_Inicializar(void);
void ADC_leer_canal (ADC_CHANNEL_T canal, uint16_t *guardar);
uint16_t ADC_Procesar_Muestras (uint16_t *p);
int32_t ADC_Convertir_Valor (uint16_t dato, uint16_t ValorRef, int32_t ValorRefTemp);
void Mostrar_Valor_Temp (int32_t temp,uint8_t posY,uint8_t posX);
void Mostrar_Titulos (void);


/*=====================================
 *		Handlers Globales
 *=====================================*/
//Semaforos
SemaphoreHandle_t	SemADCConversion;

// Colas

/* =====================================
 * ADC_Inicializar
 * ----
 * Configura el hardware, pines y perifericos
 * ======================================*/

void ADC_Inicializar(void)
{
	ADC_CLOCK_SETUP_T ADC_Config;

	//pines del adc
	Chip_IOCON_PinMux(LPC_IOCON,AD0a3_port,AD0_pin,IOCON_MODE_INACT,IOCON_FUNC1);
	Chip_IOCON_PinMux(LPC_IOCON,AD0a3_port,AD1_pin,IOCON_MODE_INACT,IOCON_FUNC1);
	Chip_IOCON_PinMux(LPC_IOCON,AD0a3_port,AD2_pin,IOCON_MODE_INACT,IOCON_FUNC1);
	Chip_IOCON_PinMux(LPC_IOCON,AD0a3_port,AD3_pin,IOCON_MODE_INACT,IOCON_FUNC1);

	Chip_IOCON_PinMux(LPC_IOCON,AD4y5_port,AD4_pin,IOCON_MODE_INACT,IOCON_FUNC3);
	Chip_IOCON_PinMux(LPC_IOCON,AD4y5_port,AD5_pin,IOCON_MODE_INACT,IOCON_FUNC3);

	/*ADC*/
	//Configurar el ADC
	Chip_ADC_Init(LPC_ADC,&ADC_Config);//power on y inicio
	Chip_ADC_SetBurstCmd(LPC_ADC,DISABLE);//modo rafaga desabilitado
	Chip_ADC_SetSampleRate(LPC_ADC,&ADC_Config,AD_Frec_Muestreo);
	//los canales los habilito/desabilito al momento de leerlos

	SemADCConversion=xSemaphoreCreateBinary();

}

/*****************************************************************
 * Handlers de Interrupción
 *****************************************************************/

/* =====================================
 * ADC
 * ----
 * Interrumpe cuando finaliza una conversión
 * Solo libera un semaforo, el resto se realiza en la tarea
 * ======================================*/
void ADC_IRQHandler ( void )
{
	volatile int i;
	portBASE_TYPE Cambio=pdFALSE;

	for(i=0;i<6;i++)
	{
		if(Chip_ADC_ReadStatus(LPC_ADC,(ADC_CH0+i),ADC_DR_DONE_STAT))
		{	//desabilito Interrupciones. Leo el dato en la tarea por lo que recien ahí se limpia el flag
			NVIC_DisableIRQ(ADC_IRQn);
			xSemaphoreGiveFromISR(SemADCConversion,&Cambio);
		}
	}
	portEND_SWITCHING_ISR(Cambio);
}

/* =====================================
 * ADC_leer_canal
 * ----
 *
 * ======================================*/
void ADC_leer_canal (ADC_CHANNEL_T canal, uint16_t *guardar)
{
	uint8_t i;

	Chip_ADC_EnableChannel(LPC_ADC,canal,ENABLE);//habilito el canal
	Chip_ADC_Int_SetChannelCmd(LPC_ADC,canal,ENABLE);//interrupe al finalizar comversión

	for(i=0;i<Cantidad_Muestras_Tomadas;i++)
	{
		NVIC_EnableIRQ(ADC_IRQn);//habilito ints en el nvic;
		Chip_ADC_SetStartMode(LPC_ADC,ADC_START_NOW,ADC_TRIGGERMODE_RISING);//comienzo de conversión en el ADC

		if(xSemaphoreTake(SemADCConversion,Tiempo_Cola_Llena)==pdTRUE)//se libera en la interrupción deberia tardar 65 ciclos
				Chip_ADC_ReadValue(LPC_ADC,canal,(guardar+i));//leo y reseteo el flag de int
		else
			{
				*(guardar+i)=(uint16_t) Error_Conversion; //valor error
			}

		vTaskDelay(Tiempo_Entre_Muestras);
	}

	Chip_ADC_EnableChannel(LPC_ADC,canal,DISABLE);// Desabilitamos el canal y las interrupciones
	Chip_ADC_Int_SetChannelCmd(LPC_ADC,canal,DISABLE);
}

/* =====================================
 * Tarea Conversion
 * ----
 * Procesa un array de muestras obtenidas del ADC
 * 1º - Ordena en forma decreciente
 * 2º - Descarta las muestras de los extremos
 * 3º - Realiza un promedio con las muestras restante
 * 4º - Descarta los bits menos significativos
 *
 * ======================================*/
uint16_t ADC_Procesar_Muestras (uint16_t *p)
{

	uint16_t aux,i,j;

	aux=0;

	//ordeno de menor a mayor burbujeo
	for(i=0;i<Cantidad_Muestras_Tomadas;i++)
		for(j=i+1;j<Cantidad_Muestras_Tomadas;j++)
			if( (*(p+j)) < (*(p+i)))
			{
				aux=*(p+i);
				*(p+i)=*(p+j);
				*(p+j)=aux;
			}

	//Descartamos las muestras de los extremos y
	//Tomamos el promedio de las restantes
	aux=0;
	for(i=Descartar_Desde;i<Descartar_Hasta;i++)
	{
		//Si el dato es valido hacemos un promedio
		if( *(p+i) != Error_Conversion)
			aux+=*(p+i);
	}

	aux=aux / (Cantidad_Muestras_Tomadas-Cantidad_Muestras_Descartadas);
	aux = aux & Descartar_Bits; //Elimino los bits menos significativos

	return(aux);
}

/* =====================================
 * ADC_Convertir_Valor
 * ----
 *  Convierte un Nº de cuentas en un Valor de Temperaturas en lugar de usar decimales
 *  usamos enteros y multiplicamos los valores por 1000 (35,51[ºC] equivale a 35510[ºC])
  *  al fianlizar la convercion hacemos una división por 10 para quedarnos solo con 2 decimales
 *
 * 					 	Sensibilidad [ºC]
 * Vx[ºC] - Vref[ºC]=  ------------------- ( Vx[Ns] - ValorRef[Ns] )
 * 					 	2^(Nbits Descarte)
 *
 * Ej:	Vref=25[ºC] (25000)=2056[Ns] Sens=0.127[ºC/8cuentas] = 127
 * 		Vx =1408[N] => 25790 + (127/8)* (1408-2056) = 25790+(-10287) = 15503(/10)= 1550
 * 		VX =2848[N] => 25790 + (127/8)* (2848-2056) = 25790+ (12573) = 38363(/10) = 3836
 *
 * Sensibilidad es cuantos ºC representan las 8 cuentas...
 * ======================================*/
int32_t ADC_Convertir_Valor (uint16_t dato, uint16_t ValorRef, int32_t ValorRefTemp)
{
	int32_t aux;

	if(dato>ADC_Valor_Sin_Conexion)
	{
		aux= (int32_t) ((Sensibilidad_Temp)*(dato-(ValorRef))/(1<<Descartar_N_Bits));
		aux+=ValorRefTemp;
		aux=aux/10;		// Tomamos solo 2 decimales
	}
	else {
			aux= Error_Conexion;
		}

	return(aux);
}

/*************************************
 * 		Mostrar_Valores
 * 	Muestra el valor de temperatura
 */
void Mostrar_Valor_Temp (int32_t temp,uint8_t posY,uint8_t posX)
{
	if(temp < Error_Conexion )
	{// Es un valor válido de temperaturas,
		if( temp < 0 )
		{
			GLCD_Escribir_Frase((uint8_t*)"-",posY,posX,Letras_Blancas);
			temp = 0 - temp;
		}
		else GLCD_Escribir_Frase((uint8_t*)" ",posY,posX,Letras_Blancas);

		posY+=6;
		GLCD_Escribir_Numero((temp/100),posY,posX,Letras_Blancas);
		posY+=(6*2);
		GLCD_Escribir_Frase((uint8_t*)".",posY,posX,Letras_Blancas);
		posY+=6;
		GLCD_Escribir_Numero((temp%100),posY,posX,Letras_Blancas);
		posY+=(6*2);
		GLCD_Escribir_Caracter_Tabla(98,posY,posX,Letras_Blancas); // "º"
		posY+=6;
		GLCD_Escribir_Frase((uint8_t*)"C",posY,posX,Letras_Blancas);
	}
	else
	{// Recibi un valor por debajo de 4[mA] S-ERROR = error en sensor
		GLCD_Escribir_Frase((uint8_t*)" S-ERROR",posY,posX,Letras_Blancas);
	}
}
/* =====================================
 * Mostrar_Valore
 * ----
 * Muestra en pantalla los valores de temperaturas
 *
 * ======================================*/

void Mostrar_Titulos (void)
{
	GLCD_Escribir_Centrado((uint8_t*)"TEMPERATURAS",1,Letras_Blancas);
	GLCD_Escribir_Centrado((uint8_t *)" MENU | x | x | x ",7,Letras_Negras);

	// Pongo un Separador en el medio de la pantalla
	GLCD_Escribir_Caracter_Tabla(92,58,3,Letras_Blancas);
	GLCD_Escribir_Caracter_Tabla(92,64,3,Letras_Blancas);
	GLCD_Escribir_Caracter_Tabla(92,58,4,Letras_Blancas);
	GLCD_Escribir_Caracter_Tabla(92,64,4,Letras_Blancas);
	GLCD_Escribir_Caracter_Tabla(92,58,5,Letras_Blancas);
	GLCD_Escribir_Caracter_Tabla(92,64,5,Letras_Blancas);

}

#endif /* PROGPRINCIPAL_INC_LECTURAADC_H_ */
