/*
 * MenuFunciones.h
 *
 *  Created on: Sep 8, 2017
 *      Author: daniel
 */

#ifndef PROGPRINCIPAL_INC_MENUFUNCIONES_H_
#define PROGPRINCIPAL_INC_MENUFUNCIONES_H_

#include <chip.h>
#include <menues.h>

#define SENSOR_OFF  ((uint8_t) 0)
#define SENSOR_ON 	((uint8_t) 1)

#define Temp_Pasos_Alarma	(8*8) 	/* Pasos para el seteo de la alarma
									* (8*X) => DT[ºC]=0.13*X
									* (8*1) => 0.13 ºC
									* (8*2) => 0.26 ºC
									* (8*4) => 0.52 ºC
									* (8*8) => 1.01 ºC								 *
									* */

#define Temp_Pasos_Ajuste0	8 // Pasos para el seteo del 0 en N cuentas (deberia coincidir con bits_descarte)

// Valor de referencia a partir del cual se convierten todos los valores
#define Valor_Ref_Cuentas	((uint16_t) 2056)	// Nº de cuentas
#define Valor_Ref_Temp		((int32_t) 25790)		// TEMP ºC

#define AJ0PosTitulo3	((64-(5*6))/2)
#define AJ0PosCuenta4	((64-(4*6))/2)
#define AJ0PosTemper5	((64-(8*6))/2)

/*=====================================
 *		Estructuras Propias
 *=====================================*/
typedef struct esSenConf
{
	 uint8_t ID;			// Nº de sensor [0:5]
	 uint8_t Estado;		// ON/OFF
	uint16_t ValorRefCuent;	// Valor de referencia en Cuentas
	 int32_t ValorRefTemp;	// Valor de referencia en [ºC]
	uint16_t Alarma_Value;	// Temperatura de Alarma
	uint8_t Alarma_Estado;	// Alarma Habilitada ON/OFF
	uint16_t UltimaLectura;	// Valor de ultima lectura en cuentas
}esSenConf;

/*=====================================
 *		Handlers Globales
 *=====================================*/
extern QueueHandle_t colaDatosSensores;

QueueHandle_t		ColaConfigSens;

/*=====================================
 * 				Funciones
 *=====================================*/
void sensores_configuracion_fabrica(esSenConf *p);
extern int32_t ADC_Convertir_Valor (uint16_t dato, uint16_t ValorRef, int32_t ValorRefTemp);
extern void Mostrar_Valor_Temp (int32_t temp,uint8_t posY,uint8_t posX);
void MenuFunInit (void);

void MenuFunInit (void)
{
	esSenConf data[6];

	ColaConfigSens=xQueueCreate(1,sizeof(data));

	sensores_configuracion_fabrica(data);

	xQueueSend(ColaConfigSens,&data,portMAX_DELAY);
}

void sensores_configuracion_fabrica(esSenConf *p)
{
	uint8_t i;
	for(i=0;i<6;i++)
	{
		(p+i)->Alarma_Estado=SENSOR_OFF;
		(p+i)->Alarma_Value=(uint16_t)1405;
		(p+i)->Estado=SENSOR_OFF;
		(p+i)->ID=(uint8_t)i;
		(p+i)->ValorRefCuent=Valor_Ref_Cuentas;
		(p+i)->ValorRefTemp=Valor_Ref_Temp;
	}

	// Sensor 1 y 2 encendidos
	p->Estado=SENSOR_ON;
	(p+1)->Estado=SENSOR_ON;

}
/*************************************
 *
 */
void MenuFunOnOff  (void * pMenuItem,int IDSensor)
{
	uint8_t tecla,estado=200;
	esSenConf datos[6];

//	GLCD_Borrar_Display(Letras_Blancas);
//	GLCD_Escribir_Centrado((uint8_t*)((esMenu_Item *)pMenuItem)->ItemTexto,1,Letras_Blancas);

	//leer estado de los sensores de la cola e imprimirlos
	xQueuePeek(ColaConfigSens,&datos,portMAX_DELAY);
	estado=(datos+IDSensor)->Estado;

	if(estado==SENSOR_ON)
		GLCD_Escribir_Frase((uint8_t*)"SI",84,3+(IDSensor%3),Letras_Blancas);
	else
		GLCD_Escribir_Frase((uint8_t*)"NO",84,3+(IDSensor%3),Letras_Blancas);

	Menu_Mostrar_Menu_Botones(94,95);

	estado=(datos+IDSensor)->Estado;
	tecla=BotonFantasma;
	GLCD_Escribir_Frase((uint8_t*)" ",6,(3+(IDSensor%3)),Letras_Blancas);// Borro  flecha seleccion
	GLCD_Escribir_Caracter_Tabla(94,72,3+(IDSensor%3),Letras_Negras); // Pongo flecha seleccion
	tecla=Teclado_Esperar_Tecla(Menu_Tiempo_Espera);
	while((tecla==BotonUp) | (tecla==BotonDown))
	{
		if(estado==SENSOR_ON)
		{
			GLCD_Escribir_Frase((uint8_t*)"NO",84,3+(IDSensor%3),Letras_Blancas);
			estado=SENSOR_OFF;
		}
		else
		{
			GLCD_Escribir_Frase((uint8_t*)"SI",84,3+(IDSensor%3),Letras_Blancas);
			estado=SENSOR_ON;
		}
//		Menu_Mostrar_Menu_Botones();
		tecla=Teclado_Esperar_Tecla(Menu_Tiempo_Espera);
	}
	if(tecla==BotonOk)
	{
		(datos+IDSensor)->Estado=estado;
		Menu_Mensaje_Guardando(3+(IDSensor%3));
	}
		else
		{
			Menu_Mensaje_GuardandoNO(3+(IDSensor%3));

		}
	xQueueOverwrite(ColaConfigSens,&datos);
	vTaskDelay(Tiempo_Mensaje_Salida);
}

/*************************************
 *
 */

void MenuFunAlarmaOnOff (void * pMenuItem, int IDSensor)
{
	uint8_t tecla=BotonDown,estado=0;
//	uint8_t mensaje[]={"Sensor X   SI"};//X=[7] SI=[11,12]
	esSenConf datos[6];

//	GLCD_Borrar_Display(Letras_Blancas);
//	GLCD_Escribir_Centrado((uint8_t*)((esMenu_Item *)pMenuItem)->ItemTexto,1,Letras_Blancas);

	//leer estado de los sensores de la cola e imprimirlos
	//leer estado de los sensores de la cola e imprimirlos
	xQueuePeek(ColaConfigSens,&datos,portMAX_DELAY);
	estado=(datos+IDSensor)->Alarma_Estado;

	if(estado==SENSOR_ON)
		GLCD_Escribir_Frase((uint8_t*)"SI",84,3+(IDSensor%3),Letras_Blancas);
	else
		GLCD_Escribir_Frase((uint8_t*)"NO",84,3+(IDSensor%3),Letras_Blancas);

	Menu_Mostrar_Menu_Botones(94,95);

	estado=(datos+IDSensor)->Alarma_Estado;
	GLCD_Escribir_Frase((uint8_t*)" ",6,(3+(IDSensor%3)),Letras_Blancas);
	GLCD_Escribir_Caracter_Tabla(94,72,3+(IDSensor%3),Letras_Negras);
	tecla=Teclado_Esperar_Tecla(Menu_Tiempo_Espera);
	while((tecla==BotonUp) | (tecla==BotonDown))
	{
		if(estado)
		{
			GLCD_Escribir_Frase((uint8_t*)"NO",84,3+(IDSensor%3),Letras_Blancas);
			estado=SENSOR_OFF;
		}
		else
			{
				GLCD_Escribir_Frase((uint8_t*)"SI",84,3+(IDSensor%3),Letras_Blancas);
				estado=SENSOR_ON;
			}
//		Menu_Mostrar_Menu_Botones();
		tecla=Teclado_Esperar_Tecla(Menu_Tiempo_Espera);
	}

	if(tecla==BotonOk)
	{
		(datos+IDSensor)->Alarma_Estado=estado;
		Menu_Mensaje_Guardando(3+(IDSensor%3));
	}
		else
		{
			Menu_Mensaje_GuardandoNO(3+(IDSensor%3));

		}
	xQueueOverwrite(ColaConfigSens,&datos);
	vTaskDelay(Tiempo_Mensaje_Salida);
}
/*************************************
 *
 */

void MenuFunAjuste0 (void * pMenuItem, int IDSensor)
{
	esSenConf datos[6];
	uint16_t	ref;
	uint8_t tecla=BotonDown;

	GLCD_Borrar_Display(Letras_Blancas);
	GLCD_Escribir_Centrado((uint8_t*)"  AJUSTE DE 0  ",0,Letras_Negras);
	GLCD_Escribir_Centrado((uint8_t*)((esMenu_Item *)pMenuItem)->ItemTexto,2,Letras_Blancas);

	//busco el último valor leido desde el ADC para el sensor que quiero setear
//	xQueueReceive(colaUltimaLectura,&ultima_lectura,portMAX_DELAY);

	//leo el valor de la ultima configuarcion conocida
	xQueueReceive(ColaConfigSens,&datos,portMAX_DELAY);
	ref=(datos+IDSensor)->ValorRefCuent;

	GLCD_Escribir_Frase((uint8_t*)"Antes",AJ0PosTitulo3,3,Letras_Blancas);
	GLCD_Escribir_Frase((uint8_t*)"Nuevo",AJ0PosTitulo3+64,3,Letras_Blancas);

	GLCD_Escribir_Numero(((datos+IDSensor)->ValorRefCuent)/100,AJ0PosCuenta4,4,Letras_Blancas);
	GLCD_Escribir_Numero(((datos+IDSensor)->ValorRefCuent)%100,AJ0PosCuenta4+(6*2),4,Letras_Blancas);
	Mostrar_Valor_Temp(ADC_Convertir_Valor((datos+IDSensor)->UltimaLectura,(datos+IDSensor)->ValorRefCuent,(datos+IDSensor)->ValorRefTemp),AJ0PosTemper5,5);

	Menu_Mostrar_Menu_Botones(13,11);
	while((tecla==BotonUp) | (tecla==BotonDown))
	{
		GLCD_Escribir_Caracter_Tabla(94,64,4,Letras_Negras);
		GLCD_Escribir_Numero(((datos+IDSensor)->ValorRefCuent)/100,AJ0PosCuenta4+64,4,Letras_Blancas);
		GLCD_Escribir_Numero(((datos+IDSensor)->ValorRefCuent)%100,AJ0PosCuenta4+64+(2*6),4,Letras_Blancas);
		Mostrar_Valor_Temp(ADC_Convertir_Valor((datos+IDSensor)->UltimaLectura,(datos+IDSensor)->ValorRefCuent,(datos+IDSensor)->ValorRefTemp),AJ0PosTemper5+64,5);

		tecla=Teclado_Esperar_Tecla(Menu_Tiempo_Espera);

		if(tecla==BotonUp)
			{
				(datos+IDSensor)->ValorRefCuent+=Temp_Pasos_Ajuste0;
			}
		if(tecla==BotonDown)
			{
			(datos+IDSensor)->ValorRefCuent-=Temp_Pasos_Ajuste0;
			}
	}
	if(tecla!=BotonOk)
		{
			(datos+IDSensor)->ValorRefCuent=ref;
			Menu_Mensaje_GuardandoNO(4);
		}
	else Menu_Mensaje_Guardando(4);

	xQueueSend(ColaConfigSens,&datos,portMAX_DELAY);
	vTaskDelay(Tiempo_Mensaje_Salida);
}

/*************************************
 *
 */

void MenuFunAlarmas01 (void * pMenuItem, int IDSensor)
{
	esSenConf datos[6];
	uint8_t tecla=BotonDown;
	uint16_t	ref;

	GLCD_Borrar_Display(Letras_Blancas);
	GLCD_Escribir_Centrado((uint8_t*)"  ALARMA  ",0,Letras_Negras);
	GLCD_Escribir_Centrado((uint8_t*)((esMenu_Item *)pMenuItem)->ItemTexto,2,Letras_Blancas);

	xQueueReceive(ColaConfigSens,&datos,portMAX_DELAY);
	ref=(datos+IDSensor)->Alarma_Value;

	GLCD_Escribir_Frase((uint8_t*)"Actual:",6,4,Letras_Blancas);

	Mostrar_Valor_Temp(ADC_Convertir_Valor((datos+IDSensor)->Alarma_Value,(datos+IDSensor)->ValorRefCuent,(datos+IDSensor)->ValorRefTemp),70,4);

//	GLCD_Escribir_Numero(((datos+IDSensor)->Alarma_Value)/100,76,5,Letras_Blancas);
//	GLCD_Escribir_Numero(((datos+IDSensor)->Alarma_Value)%100,88,5,Letras_Blancas);

	GLCD_Escribir_Frase((uint8_t*)"Nuevo:",6,5,Letras_Blancas);

	Menu_Mostrar_Menu_Botones(13,11);
	while((tecla==BotonUp) | (tecla==BotonDown))
	{
		GLCD_Escribir_Caracter_Tabla(94,64,5,Letras_Negras);
//		GLCD_Escribir_Numero(((datos+IDSensor)->Alarma_Value)/100,76,4,Letras_Blancas);
//		GLCD_Escribir_Numero(((datos+IDSensor)->Alarma_Value)%100,88,4,Letras_Blancas);
		Mostrar_Valor_Temp(ADC_Convertir_Valor((datos+IDSensor)->Alarma_Value,(datos+IDSensor)->ValorRefCuent,(datos+IDSensor)->ValorRefTemp),70,5);

		tecla=Teclado_Esperar_Tecla(Menu_Tiempo_Espera);

		if(tecla==BotonUp)
			{
				(datos+IDSensor)->Alarma_Value+=Temp_Pasos_Alarma;
			}
		if(tecla==BotonDown)
			{
			(datos+IDSensor)->Alarma_Value-=Temp_Pasos_Alarma;
			}
	}

	if(tecla!=BotonOk)
		{
			(datos+IDSensor)->Alarma_Value=ref;
			Menu_Mensaje_GuardandoNO(5);
		}
	else Menu_Mensaje_Guardando(5);
	xQueueSend(ColaConfigSens,&datos,portMAX_DELAY);
	vTaskDelay(Tiempo_Mensaje_Salida);
}

#endif /* PROGPRINCIPAL_INC_MENUFUNCIONES_H_ */
