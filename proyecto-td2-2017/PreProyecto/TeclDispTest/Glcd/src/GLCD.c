
#include "GLCD.h"

/*****************************************************************************
 * Private functions
 ****************************************************************************/
/*=====================================
 *  Demora necesaria en la linea Enable para tomoar los datos
 *=====================================*/
void GLCD_Demora_500ns(void)
{
	int i;
	//demora mayor a 450 n seg, el u corre a 100-120 Mhz (10-8 nseg/Inst)
	for(i=0;i<Demora_Enable;i++);
	//vTaskDelay(500);
}
/*=====================================
 * configura los pines correspondientes al GLCD,
 * pone todos como salidas (datos y control)
 *=====================================*/
void GLCD_Config_Pines (void)
{
	uint8_t i=0;

	//Bus de Control como salidas
	Chip_IOCON_PinMux(LPC_IOCON,GLCD_Control_Port,GLCD_DI_pin,IOCON_MODE_INACT,IOCON_FUNC0);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO,GLCD_Control_Port,GLCD_DI_pin);

	Chip_IOCON_PinMux(LPC_IOCON,GLCD_Control_Port,GLCD_EN_pin,IOCON_MODE_INACT,IOCON_FUNC0);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO,GLCD_Control_Port,GLCD_EN_pin);

//RW y Reset conectados directamente a GND
//	Chip_IOCON_PinMux(LPC_IOCON,GLCD_Control_Port,GLCD_RESET_pin,IOCON_MODE_INACT,IOCON_FUNC0);
//	Chip_GPIO_SetPinDIROutput(LPC_GPIO,GLCD_Control_Port,GLCD_RESET_pin);

//	Chip_IOCON_PinMux(LPC_IOCON,GLCD_Control_Port,GLCD_RW_pin,IOCON_MODE_INACT,IOCON_FUNC0);
//	Chip_GPIO_SetPinDIROutput(LPC_GPIO,GLCD_Control_Port,GLCD_RW_pin);

	Chip_IOCON_PinMux(LPC_IOCON,GLCD_ChipSel_port,GLCD_Izq_pin,IOCON_MODE_INACT,IOCON_FUNC0);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO,GLCD_ChipSel_port,GLCD_Izq_pin);

	Chip_IOCON_PinMux(LPC_IOCON,GLCD_ChipSel_port,GLCD_Der_pin,IOCON_MODE_INACT,IOCON_FUNC0);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO,GLCD_ChipSel_port,GLCD_Der_pin);

	//bus de datos como salida
	for(i=0;i<8;i++)
	{
		Chip_IOCON_PinMux(LPC_IOCON,GLCD_Data_Port,(GLCD_Data_bit0+i),IOCON_MODE_INACT,IOCON_FUNC0);
		Chip_GPIO_SetPinDIROutput(LPC_GPIO,GLCD_Data_Port,(GLCD_Data_bit0+i));
	}

}

/*====================================
 * Deja la pantalla lista para usar
 *=====================================*/

void GLCD_Inicializar (void)
{
//	int i;

//	Si Reset no esta puesta a 1 por Hard => descomentar lo siguiente
/*
	Chip_GPIO_SetPinOutLow(LPC_GPIO,GLCD_Control_Port,GLCD_RESET);
	for(i=0;i<1000000;i++);

	Chip_GPIO_SetPinOutHigh(LPC_GPIO,GLCD_Control_Port,GLCD_RESET);
	Chip_IOCON_PinMux(LPC_IOCON,GLCD_Control_Port,GLCD_RESET_pin,IOCON_MODE_PULLUP,IOCON_FUNC0);
	for(i=0;i<1000000;i++);
*/
//sin esta demora no funciona... por debajo de este valor 200.000 tampoco
//	for (i = 0; i < 2000000; ++i);

	Chip_GPIO_SetPinOutHigh(LPC_GPIO,GLCD_Control_Port,GLCD_ENABLE);
	Chip_GPIO_SetPinOutLow(LPC_GPIO,GLCD_ChipSel_port,GLCD_Izq);
	Chip_GPIO_SetPinOutLow(LPC_GPIO,GLCD_ChipSel_port,GLCD_Der);

	//Borro la pantalla
	GLCD_Borrar_Display(Letras_Negras);
}

/*=====================================
 * Borra el contenido de la Ram del Display,
 * Resetea las direcciones al inicio de la patalla
 *=====================================*/

void GLCD_Borrar_Display (uint8_t color)
{
volatile int i,j;

	//Pongo en 0 las direcciones X,Y,Z
	GLCD_Escribir(GLCDin_Set_Ydir,GLCDmodo_Ins,GLCD_Izq);
	GLCD_Escribir(GLCDin_Set_Zdir,GLCDmodo_Ins,GLCD_Izq);

	GLCD_Escribir(GLCDin_Set_Ydir,GLCDmodo_Ins,GLCD_Der);
	GLCD_Escribir(GLCDin_Set_Zdir,GLCDmodo_Ins,GLCD_Der);

	GLCD_Escribir(GLCDin_Display_off,GLCDmodo_Ins,GLCD_Izq);
	GLCD_Escribir(GLCDin_Display_off,GLCDmodo_Ins,GLCD_Der);

	//"Borro" el display escribiendo "0" en toda la memoria de la pantalla
	for(i=0;i<8;i++)
	{//barro las 8 filas
		GLCD_Escribir((GLCDin_Set_Xdir+i),GLCDmodo_Ins,GLCD_Izq);
		GLCD_Escribir((GLCDin_Set_Xdir+i),GLCDmodo_Ins,GLCD_Der);
		for(j=0;j<64;j++)
		{
			GLCD_Escribir((uint8_t) (0xff*color),GLCDmodo_Dato,GLCD_Izq);
			GLCD_Escribir((uint8_t) (0xff*color),GLCDmodo_Dato,GLCD_Der);
		}
	}

	//vuelvo a posisionar en la direccion (0,0); Z sigue siendo '0', no se modificó
	GLCD_Escribir(GLCDin_Set_Xdir,GLCDmodo_Ins,GLCD_Izq);
	GLCD_Escribir(GLCDin_Set_Ydir,GLCDmodo_Ins,GLCD_Izq);

	GLCD_Escribir(GLCDin_Set_Xdir,GLCDmodo_Ins,GLCD_Der);
	GLCD_Escribir(GLCDin_Set_Ydir,GLCDmodo_Ins,GLCD_Der);

	//Habilito las pantalla
	GLCD_Escribir(GLCDin_Display_on,GLCDmodo_Ins,GLCD_Izq);
	GLCD_Escribir(GLCDin_Display_on,GLCDmodo_Ins,GLCD_Der);

}

/*=====================================
 * Borra el contenido de una fila de la pantalla,
 * Resetea las direcciones al inicio de la patalla
 *=====================================*/

void GLCD_Borrar_Fila (uint8_t fila, uint8_t color)
{
	volatile int j;

	//Pongo en 0 las direcciones Y,Z
	GLCD_Escribir(GLCDin_Set_Ydir,GLCDmodo_Ins,GLCD_Izq);
	GLCD_Escribir(GLCDin_Set_Zdir,GLCDmodo_Ins,GLCD_Izq);

	GLCD_Escribir(GLCDin_Set_Ydir,GLCDmodo_Ins,GLCD_Der);
	GLCD_Escribir(GLCDin_Set_Zdir,GLCDmodo_Ins,GLCD_Der);

	//"Borro" el display escribiendo "0" en toda la pantalla

	GLCD_Escribir((GLCDin_Set_Xdir+fila),GLCDmodo_Ins,GLCD_Izq);
	GLCD_Escribir((GLCDin_Set_Xdir+fila),GLCDmodo_Ins,GLCD_Der);

	for(j=0;j<64;j++)
	{
		GLCD_Escribir((uint8_t) (0xff*color),GLCDmodo_Dato,GLCD_Izq);
		GLCD_Escribir((uint8_t) (0xff*color),GLCDmodo_Dato,GLCD_Der);
	}

}


/*====================================
 * escribe un dato (8bits) en el glcd,
 * hay que indicarle el dato a escribir
 * Si sera un ainstruccion o un dato a mostrar en pantalla
 * cual de los dos chips escribo (0-63 Izquierda) o (64-127 derecha)
 *
 * Despues de algunas pruebas, lo que funciono fue hacer un "pulso" de Enable
 * poniendolo a 1 esperando y despues a cero, de otra forma me tomaba el dato
 * varias veces y escribia "de mas" en el display:
 * => sería así
 * Pongo RW/DI/Chip segun corresponda
 * Pongo el dato en el bus
 *
 * Poner enable =1
 * demora > 450 ns
 * Poner enable =0 (aca se toma el dato)
 *
 * resetear el bus de control
 *====================================*/

void GLCD_Escribir (uint8_t dato,uint8_t modo,uint8_t lado)
{

	/*Seleccion del Controlador Izquierdo o Derecho, son activos en alto*/
	Chip_GPIO_SetPinOutHigh(LPC_GPIO,GLCD_ChipSel_port,lado);

	// Enable=0; RW=0 (escritura); DI; Seleccion de chip

	//Escribo un dato o una instruccion??
	if (modo) {//Es una Instruccion (DI=1)
		Chip_GPIO_SetPinOutHigh(LPC_GPIO,GLCD_Control_Port,GLCD_DI);
		} else {//es un dato (DI=0)
			Chip_GPIO_SetPinOutLow(LPC_GPIO,GLCD_Control_Port,GLCD_DI);
		}

	//Poner los datos en el puerto
	Chip_GPIO_SetPortValue(LPC_GPIO,GLCD_Data_Port,GLCD_Data_Bus(dato));

	//"Pulso" de Enable
	Chip_GPIO_SetPinOutHigh(LPC_GPIO,GLCD_Control_Port,GLCD_ENABLE);
	GLCD_Demora_500ns();//demora requerida en la linea de enable
	Chip_GPIO_SetPinOutLow(LPC_GPIO,GLCD_Control_Port,GLCD_ENABLE);

	Chip_GPIO_SetPinOutLow(LPC_GPIO,GLCD_ChipSel_port,lado);
}

/*=====================================
 * Escribe un Caracter en la pantalla
 * para esto utiliza la tabla de fuentes,
 * cada caracter utiliza 5 bytes
 *=====================================*/
void GLCD_Escribir_Caracter (uint8_t caracter,uint8_t lado,uint8_t color)
{
	int i;
	/* la tabla solo tiene los valores de los asciis "imprimibles" (desde 32 en adelante)
	 * o sea, el ascii de "A"=65 dec. y en la tabla "A"=33 que es 65-32=33, por lo tanto
	 * para obtener la representacion del ascii que busco le restamos 32 al mismo.
	 * cada caracter ocupa 5 columnas y ademas agrego una columna vacia despues para mejor
	 * lectura
	 */
	caracter-=32;
	for(i=0;i<Glcd_Char_Ancho;i++)
	{
		if(color==Letras_Blancas)
			GLCD_Escribir( (uint8_t) (*(font5x8+(caracter*Glcd_Char_Ancho)+i)),GLCDmodo_Dato,lado);
		if(color==Letras_Negras)
			GLCD_Escribir( (uint8_t) ~(*(font5x8+(caracter*Glcd_Char_Ancho)+i)),GLCDmodo_Dato,lado);
	}
}

/*=====================================
 * Escribe un Caracter en el medio de la pantalla
 * o sea parte en el iz y parte en la derecha
 *=====================================*/
uint8_t GLCD_Escribir_Caracter_Medio (uint8_t caracter,uint8_t posRel,uint8_t color)
{
	int i;
	/* la tabla solo tiene los valores de los asciis "imprimibles" (desde 32 en adelante)
	 * o sea, el ascii de "A"=65 dec. y en la tabla "A"=33 que es 65-32=33, por lo tanto
	 * para obtener la representacion del ascii que busco le restamos 32 al mismo.
	 * cada caracter ocupa 5 columnas y ademas agrego una columna vacia despues para mejor
	 * lectura
	 */
	caracter-=32;
	posRel=64-posRel; // posRel toma valores entre 1 y 5
	i=0;
	while(posRel)
	{
		if(color==Letras_Blancas)
			GLCD_Escribir( (uint8_t) (*(font5x8+(caracter*Glcd_Char_Ancho)+i)),GLCDmodo_Dato,GLCD_Izq);
		if(color==Letras_Negras)
			GLCD_Escribir( (uint8_t) ~(*(font5x8+(caracter*Glcd_Char_Ancho)+i)),GLCDmodo_Dato,GLCD_Izq);

		posRel--;
		i++;
	}
	GLCD_Escribir(GLCDin_Set_Ydir,GLCDmodo_Ins,GLCD_Der);

	while(i<Glcd_Char_Ancho)
	{

		if(color==Letras_Blancas)
			GLCD_Escribir( (uint8_t) (*(font5x8+(caracter*Glcd_Char_Ancho)+i)),GLCDmodo_Dato,GLCD_Der);
		if(color==Letras_Negras)
			GLCD_Escribir( (uint8_t) ~(*(font5x8+(caracter*Glcd_Char_Ancho)+i)),GLCDmodo_Dato,GLCD_Der);
		posRel++;
		i++;
	}

	return(posRel);
}

/*=====================================
 * Toma un valor decimal y lo muestra en
 * la pantalla, pasandolo a ascii antes.
 * Hasta 2 digitos (00 a 99)
 *=====================================*/
void GLCD_Escribir_Numero (uint32_t valor,uint8_t posY, uint8_t posX, uint8_t color)
{
	uint8_t lado;

	if(posY<64)
		{
			lado=GLCD_Izq;
			GLCD_Escribir((GLCDin_Set_Ydir+posY),GLCDmodo_Ins,lado);
		}
		else{
			lado=GLCD_Der;
			GLCD_Escribir((GLCDin_Set_Ydir+(posY-64)),GLCDmodo_Ins,lado);
		}

	GLCD_Escribir((GLCDin_Set_Xdir|posX),GLCDmodo_Ins,GLCD_Izq);
	GLCD_Escribir((GLCDin_Set_Xdir|posX),GLCDmodo_Ins,GLCD_Der);

	GLCD_Escribir_Caracter ((48+(valor/10)),lado,color);
	GLCD_Escribir_Caracter ((48+(valor%10)),lado,color);


}

/*=====================================
 * Escribe un afrase en la pantalla
 *=====================================*/
void GLCD_Escribir_Frase (uint8_t * frase,uint8_t posY, uint8_t posX, uint8_t color)
{
	uint8_t lado;

	if(posY<64)
		{
			lado=GLCD_Izq;
			GLCD_Escribir((GLCDin_Set_Ydir+posY),GLCDmodo_Ins,lado);
		}
		else{
			lado=GLCD_Der;
			GLCD_Escribir((GLCDin_Set_Ydir+(posY-64)),GLCDmodo_Ins,lado);
		}

	GLCD_Escribir((GLCDin_Set_Xdir|posX),GLCDmodo_Ins,GLCD_Izq);
	GLCD_Escribir((GLCDin_Set_Xdir|posX),GLCDmodo_Ins,GLCD_Der);


	while(*frase)
	{

		if((posY>(64-Glcd_Char_Ancho))&(lado==GLCD_Izq))
		{
			posY=GLCD_Escribir_Caracter_Medio(*frase++,posY,color);
			posY+=64;
			lado=GLCD_Der;
		}


		if((posY>(128-Glcd_Char_Ancho)))
		{
			lado=GLCD_Izq;
			posX++;
			GLCD_Escribir((GLCDin_Set_Xdir|posX),GLCDmodo_Ins,GLCD_Der);
			GLCD_Escribir((GLCDin_Set_Xdir|posX),GLCDmodo_Ins,GLCD_Izq);
			GLCD_Escribir(GLCDin_Set_Ydir,GLCDmodo_Ins,GLCD_Izq);
			posY=0;
		}

		GLCD_Escribir_Caracter(*frase++,lado,color);
		posY+=Glcd_Char_Ancho;
	}
}
