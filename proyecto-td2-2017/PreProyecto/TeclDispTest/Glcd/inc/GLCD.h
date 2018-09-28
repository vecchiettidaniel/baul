/*
 * GLCD.h
 *
 *  Created on: Nov 28, 2016
 *      Author: daniel
 */

#ifndef INC_GLCD_H_
#define INC_GLCD_H_

/*****************************************************************************
 * Includes de librerias
 ****************************************************************************/
#include "chip.h"

//GLCD
#include "GLCD_font5x8.h"

/*****************************************************************************
 * Declaraciones de Valores, Ports y Pines a utilizar
 ****************************************************************************/
#define Glcd_Char_Ancho	6
#define Glcd_Char_Alto		7

// Defines
#define SALIDA	1
#define ENTRADA	0

/*======*/
/* GLCD */
/*======*/
#define Demora_Enable 50 //para implementar la demora en linea Enable
#define Letras_Negras	1
#define Letras_Blancas	0

//Defines del GLCD
//6 pines de control y 8 de datos
#define GLCD_Control_Port ((uint8_t) 0) //todas las linea de control al mismo puerto
#define GLCD_DI_pin		((uint8_t) 10)	//0=Dato 1=Intruccion 	pin4 GLCD
//#define GLCD_RW_pin		((uint8_t) 7)	//1=Lectura 0=Escritura o poner a GND pin5 GLCD
#define GLCD_EN_pin		((uint8_t) 11) 	//Linea Enable			//pin6 GLCD
//#define GLCD_RESET_pin	((uint8_t) 9) 	//Linea Reset (o conectar a GND)  pin17 GLCD

#define GLCD_ChipSel_port	((uint8_t) 0)
#define GLCD_Izq_pin	((uint8_t) 5)	//Chip Izquiero, (columnas 0 a 63) activo=1   pin15 GLCD
#define GLCD_Der_pin	((uint8_t) 4)	//Chip derecho, (columnas 64 a 128) activo=1  pin16 GLCD

#define GLCD_ENABLE		GLCD_EN_pin
//#define GLCD_RESET	GLCD_RESET_pin
#define GLCD_DI		GLCD_DI_pin
//#define GLCD_RW		GLCD_RW_pin
#define GLCD_Izq	GLCD_Izq_pin
#define GLCD_Der	GLCD_Der_pin

//P2.0 = pin 42 LPC1769 = pin 7 GLCD
//P2.7 = pin 49 LPC1769 = pin 14 GLCD
#define GLCD_Data_Port ((uint8_t) 2) 	//8 lineas para bus de datos
#define GLCD_Data_bit0 ((uint8_t) 0) 	//primer bit del bus de datos

#define GLCD_Data_Bus(dato) ((uint32_t) (dato << GLCD_Data_bit0))

//instrucciones Display
#define GLCDin_Display_on		0x3F
#define GLCDin_Display_off	0x3E

#define GLCDin_Set_Xdir	0xB8
#define GLCDin_Set_Ydir	0x40
#define GLCDin_Set_Zdir	0xC0

#define GLCDmodo_Ins	0
#define GLCDmodo_Dato	1

void GLCD_Config_Pines(void);
void GLCD_Demora_500ns(void);
void GLCD_Inicializar(void);
void GLCD_Borrar_Display(uint8_t color);
void GLCD_Borrar_Fila (uint8_t fila, uint8_t color);
void GLCD_Escribir(uint8_t dato, uint8_t modo, uint8_t lado);
void GLCD_Escribir_Caracter(uint8_t caracter,uint8_t lado, uint8_t color);
uint8_t GLCD_Escribir_Caracter_Medio (uint8_t caracter,uint8_t posRel,uint8_t color);
void GLCD_Escribir_Frase (uint8_t * frase, uint8_t posY, uint8_t posX,uint8_t color);
void GLCD_Escribir_Numero (uint32_t valor,uint8_t posY, uint8_t posX,uint8_t color);


#endif /* INC_GLCD_H_ */
