/*
===============================================================================
 Name        : $(ProjName).c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/
#include "chip.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "GLCD.h"
#include "Teclado.h"
#include <cr_section_macros.h>

/* Ojo que la definicion de IOCON_MODE_PULLDOWN del LPCOpen  esta Mal...
 * Dice (0x3<<2) y tiene que ser solo 0x3
 *
 * En pinmux para establecer el modo hace :
 *
 * 		pIOCON->PINMODE[reg] = temp | (mode << bitPos); bitPos=(pin%16)*2
 *
 *	por lo que el define tiene que ser 0x3 (son 2bits por cada pin)
 *		ej.: pin 5 seria bitPos= (5)*2=10
 *			(mode << bitPos) = (0x3 << 10) = 110000000000 = 0xc00
 *			0xC00 correponde a los bits 10 y 11 del registro esto esta OK
 *
 *		pero mode queda como (0x3 << 2)=(0xC)
 *			(mode << bitPos) = (0xc << 10) = 11000000000000 = 0x3000
 *			0x3000 correponde a los bits 12 y 13 del registro, que corresponden al pin 6!!!
 *
 *	Corregir el define de IOCON_MODE_PULLDOWN o armar uno propio
 */
#define Usar_PullUp 	0x0
#define Usar_PullDown 	0x3
#define Usar_Ninguno 	0x2


#define Letras_Color	Letras_Negras

//Salidas
#define LedSTICK_Port	((uint8_t) 0)
#define LedSTICK_Pin	((uint8_t) 22)

#define Demora_Rebote	100						// en mseg
#define frec_trabajo	(1000/Demora_Rebote)	// en Hz

#define Mi_Timer		LPC_TIMER0			// que timer Uso
#define Mi_Timer_PCL	SYSCTL_PCLK_TIMER0	// Cual PCLK uso... que coincida con el timer
#define Mi_Timer_ISR	TIMER0_IRQHandler
#define Mi_Timer_IRQ	TIMER0_IRQn
#define Mi_Timer_Div	SYSCTL_CLKDIV_8		// valor del divisor del SYSCTL_CLKDIV_[1,2,4,8]

#define PrescValor	((uint32_t) 9)		// valor del prescaler (Divisor que quiero -1)
#define Mi_Match_ON		((uint8_t) 0)		// match que voy a usar [0,1,2,3]

/* -----------------
 * valor del match
 * -----------------
 *
 * SystemCoreClock (SCC) = frec del micro; y frec del timer (ft) es:
 * 	ft = SCC/Div y la frec de conteo sera fc=ft/(Pv + 1) Pv=valor del preescaler
 * 	Si quiero un frec dada (fx) el valor del match sera:
 * 	Vm = fc/fx
 *
 * 	Vm = ft / [fx * (Pv + 1)] = SCC / [fx * Div * (Pv + 1)]
 */

#define Mi_Match_Val(frec) ( (SystemCoreClock/((frec) * Chip_Clock_GetPCLKDiv(Mi_Timer_PCL) * (1 + PrescValor))))


/*=====================================
 *===			Funciones			===
 *=====================================*/
void Setear_Frec_Micro(void);
void config_hard(void);

/*=====================================
 *		Handlers Globales
 *=====================================*/
//Semaforos
SemaphoreHandle_t	SemDemoraTeclado;

// Colas
QueueHandle_t		ColaTeclado;

/*=====================================
 *===			Funciones			===
 *=====================================*/
/*************************************
 * 		config_hard
 * 	configura los perifericos a utilizar
 */
void config_hard(void)
{
/* Frec Del Micro */
	Setear_Frec_Micro();
	SystemCoreClockUpdate();

/* =GPIO= */
	//Configuramos los pines de salida
	Chip_IOCON_PinMux(LPC_IOCON , LedSTICK_Port,LedSTICK_Pin, Usar_Ninguno, IOCON_FUNC0);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, LedSTICK_Port,LedSTICK_Pin);

	Teclado_Config_Hard();
	GLCD_Config_Pines();

/* =TIMERS= */
	//Habilitamos el timer y ponemos las cuentas en 0
	NVIC_DisableIRQ(Mi_Timer_IRQ);
	Chip_TIMER_Init(Mi_Timer);
	Chip_TIMER_Reset(Mi_Timer);

	// Configuramos el Clock del Timer y el preescaler
	Chip_Clock_SetPCLKDiv(Mi_Timer_PCL,Mi_Timer_Div);
	Chip_TIMER_PrescaleSet(Mi_Timer,PrescValor);

	// Cargamos el Valor del Match y configuramos las opciones
	// Match 0 es el de la frec de trabajo (pone en 1 la salida)
	Chip_TIMER_SetMatch(Mi_Timer, Mi_Match_ON, Mi_Match_Val(frec_trabajo));
	Chip_TIMER_ExtMatchControlSet(Mi_Timer,0,TIMER_EXTMATCH_DO_NOTHING,Mi_Match_ON);
	Chip_TIMER_ResetOnMatchEnable(Mi_Timer,Mi_Match_ON);
	Chip_TIMER_StopOnMatchEnable(Mi_Timer,Mi_Match_ON);
	Chip_TIMER_MatchEnableInt(Mi_Timer,Mi_Match_ON);
}

/*************************************
 * 		Teclado_Demora_Rebotes
 * 	Requerida por el manejo de teclado para evitar reentrancia por rebotes
 */
void Teclado_Demora_Rebotes (void)
{

	//vTaskDelay(Demora_Rebote/portTICK_PERIOD_MS);
	Chip_TIMER_Enable(Mi_Timer);
	xSemaphoreTake(SemDemoraTeclado,portMAX_DELAY);
}


/*************************************
 * 		Setear_Frec_Micro
 * 	Establece la frecuencia del micro,
 * 	es copia exacta de "Chip_SetupXtalClocking()"
 * 	Configura los divisores y el pll
 */
void Setear_Frec_Micro(void)
{
	/* Disconnect the Main PLL if it is connected already */
	if (Chip_Clock_IsMainPLLConnected())
	{
		Chip_Clock_DisablePLL(SYSCTL_MAIN_PLL, SYSCTL_PLL_CONNECT);
	}

	/* Disable the PLL if it is enabled */
	if (Chip_Clock_IsMainPLLEnabled())
	{
		Chip_Clock_DisablePLL(SYSCTL_MAIN_PLL, SYSCTL_PLL_ENABLE);
	}

	/* Enable the crystal */
	if (!Chip_Clock_IsCrystalEnabled())
		Chip_Clock_EnableCrystal();
	while(!Chip_Clock_IsCrystalEnabled()) {}

	/* Set PLL0 Source to Crystal Oscillator */
	Chip_Clock_SetCPUClockDiv(0);
	Chip_Clock_SetMainPLLSource(SYSCTL_PLLCLKSRC_MAINOSC);

	/*	Distintas Frecuencias:
	 *	 96 [MHz] => M=(15+1)	N=(0+1)	D=(3+1)
	 *	100 [MHz] => M=(24+1)	N=(1+1)	D=(2+1)
	 *	120 [MHz] => M=(14+1)	N=(0+1)	D=(2+1)
	 */

	//M y N OJO que es el valor -1... ACORDARCE DEL +1!!!!!
	Chip_Clock_SetupPLL(SYSCTL_MAIN_PLL, 24, 1);
	Chip_Clock_EnablePLL(SYSCTL_MAIN_PLL, SYSCTL_PLL_ENABLE);
	// D
	Chip_Clock_SetCPUClockDiv(2);
	while (!Chip_Clock_IsMainPLLLocked()) {} /* Wait for the PLL to Lock */

	Chip_Clock_EnablePLL(SYSCTL_MAIN_PLL, SYSCTL_PLL_CONNECT);
}

/*=====================================
 *==	Rutinas de Interrupciones	 ==
 *=====================================*/

/*************************************
 * 		Interrupcion del Timer
 */
void Mi_Timer_ISR (void)
{
	BaseType_t cambio;

	if (Chip_TIMER_MatchPending(Mi_Timer,Mi_Match_ON))
	{
		Chip_TIMER_ClearMatch(Mi_Timer,Mi_Match_ON);
		xSemaphoreGiveFromISR(SemDemoraTeclado,&cambio);
	}

	portEND_SWITCHING_ISR(cambio);
}

/*************************************
 * 		Interrupcion De los Pines
 */
void EINT3_IRQHandler(void)
{
	static BaseType_t cambiamos,aux;

	cambiamos=pdFALSE;
	aux=pdFALSE;

	//identificar int, cual fue?,
	if(Chip_GPIOINT_IsIntPending(LPC_GPIOINT,Teclado_Port))
	{
		if( Chip_GPIOINT_GetStatusFalling(LPC_GPIOINT,Teclado_Port) & TecladoMascara )
		{
			Teclado_IRQHandler(&aux);
			cambiamos = cambiamos | aux;
		}

	}

	//llamamos al scheulder??
	portEND_SWITCHING_ISR(cambiamos);
}

/*=====================================
 * 				TAREAS
 *=====================================*/

static void Tarea_Parpa (void *p)
{
	uint8_t tecla;

	GLCD_Inicializar();
	GLCD_Borrar_Display(Letras_Color);
	NVIC_EnableIRQ(EINT3_IRQn);	// habilitamos las interrupciones en el NVIC
	NVIC_EnableIRQ(TIMER0_IRQn);	// habilitamos las interrupciones en el NVIC

	while(1)
	{
		Chip_GPIO_SetPinOutLow(LPC_GPIO,LedSTICK_Port,LedSTICK_Pin);
		tecla=Teclado_Esperar_Tecla(portMAX_DELAY);
		switch (tecla)
		{
			case BotonMenu:
				{
					GLCD_Borrar_Fila(0,Letras_Color);
					GLCD_Escribir_Frase((uint8_t*)"Boton Menu",34,2,Letras_Color);
				}
				break;

			case BotonOk:
				{
					GLCD_Borrar_Fila(2,Letras_Color);
					GLCD_Escribir_Frase((uint8_t*)"Boton OK",40,2,Letras_Color);
				}
				break;

			case BotonUp:
				{
					GLCD_Borrar_Fila(4,Letras_Color);
					GLCD_Escribir_Frase((uint8_t*)"Boton UP",40,4,Letras_Color);
				}
				break;

			case BotonDown:
				{
					GLCD_Borrar_Fila(6,Letras_Color);
					GLCD_Escribir_Frase((uint8_t*)"Boton Down",34,6,Letras_Color);
				}
				break;

			default:
				break;
		}
	}

}

/*=====================================
 * 				MAIN
 *=====================================*/

int main(void)
{
	config_hard();

	vSemaphoreCreateBinary(SemDemoraTeclado);
	xSemaphoreTake(SemDemoraTeclado,portMAX_DELAY);

	xTaskCreate(Tarea_Parpa, (char *) "vTaskLed2",
					configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
					(xTaskHandle *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Nunca debería arribar aquí */

    return 0;
}

