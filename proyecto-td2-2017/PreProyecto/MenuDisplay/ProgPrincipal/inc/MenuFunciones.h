/*
 * MenuFunciones.h
 *
 *  Created on: Sep 8, 2017
 *      Author: daniel
 */

#ifndef PROGPRINCIPAL_INC_MENUFUNCIONES_H_
#define PROGPRINCIPAL_INC_MENUFUNCIONES_H_

#include <menues.h>

#define SENSOR_OFF 0
#define SENSOR_ON 	(~SENSOR_OFF)

#define Temp_Pasos_Alarma	5 // Pasos para el seteo de la alarma en ºC
#define Temp_Pasos_Ajuste0	8 // Pasos para el seteo del 0 en N cuentas (deberia coincidir con bits_descarte)

/*=====================================
 *		Handlers Globales
 *=====================================*/

QueueHandle_t		ColaConfigSens;

typedef struct SenConf
{
	uint8_t ID;			//Nº de sensor [0:5]
	uint8_t Estado;		// ON/OFF
	uint16_t Valor0C;	// Valor de 0[ºC]
	uint16_t Alarma_Value;	// Temperatura de Alarma
	uint8_t Alarma_Estado;	// Alarma Habilitada ON/OFF
}SenConf;

/*=====================================
 * 				Funciones
 *=====================================*/
void MenuFunInit (void)
{
	SenConf data[6];
	int i;

	ColaConfigSens=xQueueCreate(1,sizeof(data));

	for(i=0;i<6;i++)
	{
		data[i].ID=i;
		data[i].Alarma_Value=3000;
		data[i].Estado=SENSOR_OFF;
		data[i].Valor0C=200;
	}

	(data+2)->Estado=SENSOR_ON;
	data[3].Estado=SENSOR_ON;
	xQueueSend(ColaConfigSens,&data,portMAX_DELAY);
}
/*************************************
 *
 */
void MenuFunOnOff  (void * pMenuItem,int IDSensor)
{
	uint8_t tecla=BotonDown,estado=0;
	uint8_t mensaje[]={"Sensor X   SI"};//X=[7] SI=[11,12]
	SenConf datos[6];

//	GLCD_Borrar_Display(Letras_Blancas);
//	GLCD_Escribir_Centrado((uint8_t*)((Menu_Item *)pMenuItem)->ItemTexto,1,Letras_Blancas);

	//leer estado de los sensores de la cola e imprimirlos
	xQueuePeek(ColaConfigSens,&datos,portMAX_DELAY);

	for(tecla=0;tecla<3;tecla++)
	{//Mostramos el Estado de los sensores
		mensaje[7]=49+tecla+(3*(IDSensor/3));
		estado=(datos+tecla)->Estado;
		if(estado)
		{
			mensaje[11]='S';
			mensaje[12]='I';
		}
		else
			{
				mensaje[11]='N';
				mensaje[12]='O';
			}
		GLCD_Escribir_Frase(mensaje ,18,(3+tecla),Letras_Blancas);
	}


	Menu_Mostrar_Menu_Botones(94,95);

	estado=(datos+IDSensor)->Estado;
	GLCD_Escribir_Frase((uint8_t*)" ",6,(3+(IDSensor%3)),Letras_Blancas);
	GLCD_Escribir_Caracter_Tabla(94,72,3+(IDSensor%3),Letras_Negras);
	while((tecla==BotonUp) | (tecla==BotonDown))
	{
		if(estado)
			GLCD_Escribir_Frase((uint8_t*)"SI",84,3+(IDSensor%3),Letras_Blancas);
		else
			GLCD_Escribir_Frase((uint8_t*)"NO",84,3+(IDSensor%3),Letras_Blancas);
//		Menu_Mostrar_Menu_Botones();
		tecla=Teclado_Esperar_Tecla(Menu_Tiempo_Espera);
		estado=~estado;
	}
	estado=~estado;
	if(tecla==BotonOk)
	{
		(datos+IDSensor)->Estado=estado;
		xQueueOverwrite(ColaConfigSens,&datos);
	}
}

/*************************************
 *
 */

void MenuFunAlarmaOnOff (void * pMenuItem, int IDSensor)
{
	uint8_t tecla=BotonDown,estado=0;
	uint8_t mensaje[]={"Sensor X   SI"};//X=[7] SI=[11,12]
	SenConf datos[6];

//	GLCD_Borrar_Display(Letras_Blancas);
//	GLCD_Escribir_Centrado((uint8_t*)((Menu_Item *)pMenuItem)->ItemTexto,1,Letras_Blancas);

	//leer estado de los sensores de la cola e imprimirlos
	xQueuePeek(ColaConfigSens,&datos,portMAX_DELAY);

	for(tecla=0;tecla<3;tecla++)
	{//Mostramos el Estado de los sensores
		mensaje[7]=49+tecla+(3*(IDSensor/3));
		estado=(datos+tecla)->Alarma_Estado;
		if(estado)
		{
			mensaje[11]='S';
			mensaje[12]='I';
		}
		else
			{
				mensaje[11]='N';
				mensaje[12]='O';
			}
		GLCD_Escribir_Frase(mensaje ,18,(3+tecla),Letras_Blancas);
	}


	Menu_Mostrar_Menu_Botones(94,95);

	estado=(datos+IDSensor)->Alarma_Estado;
	GLCD_Escribir_Frase((uint8_t*)" ",6,(3+(IDSensor%3)),Letras_Blancas);
	GLCD_Escribir_Caracter_Tabla(94,72,3+(IDSensor%3),Letras_Negras);
	while((tecla==BotonUp) | (tecla==BotonDown))
	{
		if(estado)
			GLCD_Escribir_Frase((uint8_t*)"SI",84,3+(IDSensor%3),Letras_Blancas);
		else
			GLCD_Escribir_Frase((uint8_t*)"NO",84,3+(IDSensor%3),Letras_Blancas);
//		Menu_Mostrar_Menu_Botones();
		tecla=Teclado_Esperar_Tecla(Menu_Tiempo_Espera);
		estado=~estado;
	}
	estado=~estado;
	if(tecla==BotonOk)
	{
		(datos+IDSensor)->Alarma_Estado=estado;
		xQueueOverwrite(ColaConfigSens,&datos);
	}
}
/*************************************
 *
 */

void MenuFunAjuste0 (void * pMenuItem, int IDSensor)
{
	SenConf datos[6];
	uint8_t tecla=BotonDown;

	GLCD_Borrar_Display(Letras_Blancas);
	GLCD_Escribir_Centrado((uint8_t*)"  AJUSTE DE 0  ",0,Letras_Negras);
	GLCD_Escribir_Centrado((uint8_t*)((Menu_Item *)pMenuItem)->ItemTexto,2,Letras_Blancas);
	xQueuePeek(ColaConfigSens,&datos,portMAX_DELAY);

	GLCD_Escribir_Frase((uint8_t*)"Actual:",6,5,Letras_Blancas);
	GLCD_Escribir_Numero(((datos+IDSensor)->Valor0C)/100,76,5,Letras_Blancas);
	GLCD_Escribir_Numero(((datos+IDSensor)->Valor0C)%100,88,5,Letras_Blancas);

	GLCD_Escribir_Frase((uint8_t*)"Nuevo:",6,4,Letras_Blancas);

	Menu_Mostrar_Menu_Botones(13,11);
	while((tecla==BotonUp) | (tecla==BotonDown))
	{
		GLCD_Escribir_Caracter_Tabla(94,64,4,Letras_Negras);
		GLCD_Escribir_Numero(((datos+IDSensor)->Valor0C)/100,76,4,Letras_Blancas);
		GLCD_Escribir_Numero(((datos+IDSensor)->Valor0C)%100,88,4,Letras_Blancas);

		tecla=Teclado_Esperar_Tecla(Menu_Tiempo_Espera);

		if(tecla==BotonUp)
			{
				(datos+IDSensor)->Valor0C+=Temp_Pasos_Ajuste0;
			}
		if(tecla==BotonDown)
			{
			(datos+IDSensor)->Valor0C-=Temp_Pasos_Ajuste0;
			}
	}
	if(tecla==BotonOk) xQueueOverwrite(ColaConfigSens,&datos);
}

/*************************************
 *
 */

void MenuFunAlarmas01 (void * pMenuItem, int IDSensor)
{
	SenConf datos[6];
	uint8_t tecla=BotonDown;

	GLCD_Borrar_Display(Letras_Blancas);
	GLCD_Escribir_Centrado((uint8_t*)"  ALARMA  ",0,Letras_Negras);
	GLCD_Escribir_Centrado((uint8_t*)((Menu_Item *)pMenuItem)->ItemTexto,2,Letras_Blancas);
	xQueuePeek(ColaConfigSens,&datos,portMAX_DELAY);

	GLCD_Escribir_Frase((uint8_t*)"Actual:",6,5,Letras_Blancas);
	GLCD_Escribir_Numero(((datos+IDSensor)->Alarma_Value)/100,76,5,Letras_Blancas);
	GLCD_Escribir_Numero(((datos+IDSensor)->Alarma_Value)%100,88,5,Letras_Blancas);

	GLCD_Escribir_Frase((uint8_t*)"Nuevo:",6,4,Letras_Blancas);

	Menu_Mostrar_Menu_Botones(13,11);
	while((tecla==BotonUp) | (tecla==BotonDown))
	{
		GLCD_Escribir_Caracter_Tabla(94,64,4,Letras_Negras);
		GLCD_Escribir_Numero(((datos+IDSensor)->Alarma_Value)/100,76,4,Letras_Blancas);
		GLCD_Escribir_Numero(((datos+IDSensor)->Alarma_Value)%100,88,4,Letras_Blancas);

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
	if(tecla==BotonOk) xQueueOverwrite(ColaConfigSens,&datos);
}

#endif /* PROGPRINCIPAL_INC_MENUFUNCIONES_H_ */
