/*
 * Teclado.c
 *
 *  Created on: Aug 28, 2017
 *      Author: daniel
 */

#ifndef TECLADO_INC_TECLADO_H_
#define TECLADO_INC_TECLADO_H_

#include <FreeRTOS.h>
#include <semphr.h>
#include <chip.h>

#define Usar_PullUp 	0x0
#define Usar_PullDown 	0x3
#define Usar_Ninguno 	0x2

//Entradas
#define BotonMenu_Port	((uint8_t) 0)
#define BotonMenu_Pin	((uint8_t) 28)

#define BotonOk_Port	((uint8_t) 0)
#define BotonOk_Pin		((uint8_t) 27)

#define BotonUp_Port	((uint8_t) 0)
#define BotonUp_Pin		((uint8_t) 3)
#define BotonDown_Port	((uint8_t) 0)
#define BotonDown_Pin	((uint8_t) 21)

#define Teclado_Port	((uint8_t) 0)
#define TecladoMascara	((uint32_t) ((1<<BotonMenu_Pin)|(1<<BotonOk_Pin) |(1<<BotonDown_Pin)|(1<<BotonUp_Pin)))


//Control de Teclado
#define BotonMenu	BotonMenu_Pin
#define BotonOk		BotonOk_Pin
#define BotonUp		BotonUp_Pin
#define BotonDown	BotonDown_Pin
#define BotonFantasma	100	//Id de boton/tecla inexistente

/*=====================================
 *===		Funciones Dec			===
 *=====================================*/
void Teclado_Config_Hard (void);
void Teclado_IRQHandler(BaseType_t *cambio);
uint8_t Teclado_Esperar_Tecla(TickType_t TiempoBloqueo);

extern void Teclado_Demora_Rebotes (void);

void TecladoDesabilitarINT(void);
void TecladoHabilitarINT(void);

/*=====================================
 *===	Handlers Globales			===
 *=====================================*/
//Semaforos
SemaphoreHandle_t	SemBotonPresionado;

/*=====================================
 *===		Funciones Ins			===
 *=====================================*/
/*************************************
 * 		Teclado_Config_Hard
 * 	configura los pines que usara el teclado
 * 	Los pone como salidas con PULL_UP/DOWN segun corresponda
 */
void Teclado_Config_Hard (void)
{
/* =GPIO= */
	//Prendemos el GPIO
	Chip_GPIO_Init (LPC_GPIO);

	//Configuramos los pines de Entrada
	Chip_IOCON_PinMux(LPC_IOCON, BotonMenu_Port, BotonMenu_Pin, Usar_PullUp, IOCON_FUNC0);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO,BotonMenu_Port, BotonMenu_Pin);

	Chip_IOCON_PinMux(LPC_IOCON, BotonOk_Port, BotonOk_Pin, Usar_PullUp, IOCON_FUNC0);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO,BotonOk_Port, BotonOk_Pin);

	Chip_IOCON_PinMux(LPC_IOCON, BotonDown_Port, BotonDown_Pin, Usar_PullUp, IOCON_FUNC0);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO,BotonDown_Port, BotonDown_Pin);

	Chip_IOCON_PinMux(LPC_IOCON, BotonUp_Port, BotonUp_Pin, Usar_PullUp, IOCON_FUNC0);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO,BotonUp_Port, BotonUp_Pin);

	//Semforos
	SemBotonPresionado=xSemaphoreCreateBinary();
}

/*************************************
 * 		Teclado_IRQHandler
 * 	Llamar desde dentro de la EINT3_IRQn
 */
void Teclado_IRQHandler(BaseType_t *cambio)
{
	TecladoDesabilitarINT();
	Chip_GPIOINT_ClearIntStatus(LPC_GPIOINT,Teclado_Port,TecladoMascara);
	xSemaphoreGiveFromISR(SemBotonPresionado,cambio);
}

/*************************************
 * 		Teclado_Esperar_Tecla
 *	Funcion que espera por hasta que se presione una tecla.
 *	#Recibe: Tiempo de espera máximo en ticks
 *	#Retorna:
 *			ID del boton presionado si OK
 *			BotonFantasma si no se presiono nada
 */
uint8_t Teclado_Esperar_Tecla(TickType_t TiempoBloqueo)
{
	uint8_t botonId=BotonFantasma;
	uint8_t presione=1;

	while ((presione)&(botonId==BotonFantasma))
	{
		TecladoHabilitarINT();
		//espero a que se presione una tecla
		if(pdFALSE==xSemaphoreTake(SemBotonPresionado , TiempoBloqueo ))
		{// Paso el tiempo de espera
			presione=0;
		}
		else
			Teclado_Demora_Rebotes(); // Funcion externa para realizar una demora pequeña

		/* si presiono mas de una tecla y una de estas es MENU => "gana" Menu
		 * si presiono mas de una tecla y ninguna es MENU => "gana" Abajo
		 * OK siempre pierde... solamente si presiono ese boton me valida el OK
		 * 	Esto para que Menu sea Atras o Salir y que OK no haga cosas que no queriamos hacer...
		 */
		// Los botones son activos en bajo... siempre leo 1 salvo al presionarlos


		if(!Chip_GPIO_GetPinState(LPC_GPIO,BotonOk_Port,BotonOk_Pin))
			botonId=BotonOk;

		if(!Chip_GPIO_GetPinState(LPC_GPIO,BotonUp_Port,BotonUp_Pin))
			botonId=BotonUp;

		if(!Chip_GPIO_GetPinState(LPC_GPIO,BotonDown_Port,BotonDown_Pin))
			botonId=BotonDown;

		if(!Chip_GPIO_GetPinState(LPC_GPIO,BotonMenu_Port,BotonMenu_Pin))
			botonId=BotonMenu;
	}
	return(botonId);
}

/*************************************
 * 		Teclado_Desabilitar_INT
 * 	Desabilita las interrupciones asociadas al teclado
 */
void TecladoDesabilitarINT(void)
{
	Chip_GPIOINT_SetIntFalling(LPC_GPIOINT,Teclado_Port,(Chip_GPIOINT_GetIntFalling(LPC_GPIOINT,Teclado_Port) & ~TecladoMascara));
}

/*************************************
 * 		Teclado_Habilitar_INT
 * 	Habilita las interrupciones asociadas al teclado
 */
void TecladoHabilitarINT(void)
{
	Chip_GPIOINT_ClearIntStatus(LPC_GPIOINT,Teclado_Port,TecladoMascara);
	Chip_GPIOINT_SetIntFalling(LPC_GPIOINT,Teclado_Port,(Chip_GPIOINT_GetIntFalling(LPC_GPIOINT,Teclado_Port) | TecladoMascara));
}

#endif /* TECLADO_INC_TECLADO_H_ */
