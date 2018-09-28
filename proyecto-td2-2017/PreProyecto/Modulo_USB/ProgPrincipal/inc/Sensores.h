/*
 * LecturaADC.h
 *
 *  Created on: Sep 13, 2017
 *      Author: daniel
 */

#ifndef PROGPRINCIPAL_INC_SENSORES_H_
#define PROGPRINCIPAL_INC_SENSORES_H_

#include <chip.h>
#include <semphr.h>
#include <FreeRTOS.h>
#include <GLCD.h>

#define Tiempo_Cola_Llena		portMAX_DELAY//(200/portTICK_RATE_MS)
#define Tiempo_Cola_Vacia		portMAX_DELAY//(200/portTICK_RATE_MS)

//Toma de Muestras
#define Cantidad_Sensores_Max			6	//máximo 6

#define Sensibilidad_Temp	127	// Valor obtenido de tablas, minima diferencia dicernible por el sistema

#define Sensor_Valor_Sin_Conexion	((uint16_t) 740)	// Valor de 4[mA]

#define Error_Conversion	((uint16_t) 5000)
#define Error_Conexion 		((int32_t) 10000)//corresponde a un valor de 100 ºC
#define Sensor_Anulado 		((int32_t) 20000)//corresponde a un valor de 200 ºC

#define Descartar_N_Bits	3 	// Descartamos bits menos significativos
#define Descartar_Bits		((uint16_t) ~((1<<Descartar_N_Bits)-1)) // Mascara para borrar los bits descartados
#define Sensibilidad_Temp	127	// Valor obtenido de tablas, minima diferencia dicernible por el sistema

#define SENSOR_OFF  ((uint8_t) 0)
#define SENSOR_ON 	((uint8_t) 1)

#define Valor_Ref_Cuentas	((uint16_t) 2056)	// Nº de cuentas
#define Valor_Ref_Temp		((int32_t) 25790)		// TEMP ºC

/*=====================
  *		typedefs
  *====================*/

typedef struct esSenData
{
	 uint8_t 	ID;				// Nº de sensor [0:5]
	 uint16_t 	UltimaLectura;	// Valor de ultima lectura en cuentas
 	 uint8_t 	Estado;			// ON/OFF
}esSenData;

 typedef struct esSenConf
 {
	 esSenData	Sensor;
/*	 uint8_t ID;				// Nº de sensor [0:5]
 	 uint16_t UltimaLectura;	// Valor de ultima lectura en cuentas
 	 uint8_t Estado;			// ON/OFF
*/	 uint16_t 	ValorRefCuent;	// Valor de referencia en Cuentas
 	 int32_t 	ValorRefTemp;		// Valor de referencia en [ºC]
 	 uint16_t 	Alarma_Value;		// Temperatura de Alarma
 	 uint8_t 	Alarma_Estado;		// Alarma Habilitada ON/OFF
 }esSenConf;

 /*=====================================
  *		Handlers Globales
  *=====================================*/
 //Semaforos

 // Colas
/*=====================================
 *===			Funciones			===
 *=====================================*/

int32_t Sensor_Convertir_Valor (uint16_t dato, uint16_t ValorRef, int32_t ValorRefTemp);
void Sensor_Mostrar_Temp (int32_t temp,uint8_t posY,uint8_t posX);
void Sensor_Mostrar_Titulos (void);
void sensores_configuracion_fabrica(esSenConf *p);
void sensores_configuracion_fabrica(esSenConf *p);

/*=====================================*/


void sensores_configuracion_fabrica(esSenConf *p)
{
	uint8_t i;
	for(i=0;i<6;i++)
	{

		(p+i)->Sensor.ID=(uint8_t)i;
		(p+i)->Sensor.Estado=SENSOR_OFF;
		(p+i)->Sensor.UltimaLectura=Sensor_Anulado;
		(p+i)->Alarma_Estado=SENSOR_OFF;
		(p+i)->Alarma_Value=(uint16_t)1405;
		(p+i)->ValorRefCuent=Valor_Ref_Cuentas;
		(p+i)->ValorRefTemp=Valor_Ref_Temp;
	}

	// Sensor 1 y 2 encendidos
	p->Sensor.ID=SENSOR_ON;
	(p+i)->Sensor.ID=SENSOR_ON;

}

/*****************************************************************
 * Handlers de Interrupción
 *****************************************************************/

/* =====================================
 *
 * ----
 *
 * ======================================*/



/*=====================================
 *===			Funciones			===
 *=====================================*/
/* =====================================
 * Sensor_Convertir_Valor
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
int32_t Sensor_Convertir_Valor (uint16_t dato, uint16_t ValorRef, int32_t ValorRefTemp)
{
	int32_t aux;

	if(dato>Sensor_Valor_Sin_Conexion)
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

/* =====================================
 * Mostrar_Valores
 * -----
 * 	Muestra el valor de temperatura
 * ======================================*/
void Sensor_Mostrar_Temp (int32_t temp,uint8_t posY,uint8_t posX)
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
 * Mostrar_Titulos
 * ----
 * Muestra en pantalla los valores de temperaturas
 *
 * ======================================*/

void Sensor_Mostrar_Titulos (void)
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

#endif /* PROGPRINCIPAL_INC_SENSORES_H_ */
