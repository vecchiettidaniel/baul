/*
 * ProgPrincipal.c
 *
 *  Created on: Nov 4, 2017
 *      Author: daniel
 */
#include "chip.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include <cr_section_macros.h>
#include "app_usbd_cfg.h"
#include "GLCD.h"
#include "Sensores.h"


#define Mi_Timer		LPC_TIMER0			// que timer Uso
#define Mi_Timer_PCL	SYSCTL_PCLK_TIMER0	// Cual PCLK uso... que coincida con el timer
#define Mi_Timer_ISR	TIMER0_IRQHandler
#define Mi_Timer_IRQ	TIMER0_IRQn
#define Mi_Timer_Div	SYSCTL_CLKDIV_8		// valor del divisor del SYSCTL_CLKDIV_[1,2,4,8]

#define PrescValor	((uint32_t) 0)		// valor del prescaler (Divisor que quiero -1)
#define Mi_Match_ON		((uint8_t) 0)		// match que voy a usar [0,1,2,3]

/* -----------------
 * valor del match
 * -----------------
 *
 * 	SystemCoreClock (SCC) = frec del micro;
 *  frec del timer (ft) es:
 * 	ft = SCC/Div y la frec de conteo sera fc=ft/(Pv + 1) Pv=valor del preescaler
 * 	Si quiero un frec dada (fx) o un Tiempo (Tx) el valor del match sera:
 * 	Vm = fc/fx = fc * Tx
 *
 * 	Vm = ft / [fx * (Pv + 1)] = SCC / [fx * Div * (Pv + 1)]
 * 	VM = (ft*Tx)/(Pv + 1) = SCC*Tx / [Div * (Pv + 1)]
 */

#define Match_frec2Val(frec) ( (SystemCoreClock/((frec) * Chip_Clock_GetPCLKDiv(Mi_Timer_PCL) * (1 + PrescValor))))
#define Match_time2Val(time) ( ( (SystemCoreClock * (time) )/( Chip_Clock_GetPCLKDiv(Mi_Timer_PCL) * (1 + PrescValor))))


//Salidas
#define LedSTICK_Port	((uint8_t) 0)
#define LedSTICK_Pin	((uint8_t) 22)

/*=====================================
 *		Funciones Declaraciones
 *=====================================*/
void Setear_Frec_Micro(void);
void USB_Recibir_Dato(USBD_HANDLE_T hUsb, void *data);
uint32_t USB_Analizar_Reporte(uint8_t *p);
void USB_Enviar_Dato(USBD_HANDLE_T hUsb, void *data);
extern void usb_main_config(void);
void entero2string(int32_t valor,uint8_t *p);
/*=====================================
 *		Handlers Globales
 *=====================================*/
xQueueHandle cola_datos_usb;
xQueueHandle cola_datos_sensores;
/*=====================================
 *				Funciones
 *=====================================*/
/*************************************
 * 		config_hard
 * 	configura los perifericos a utilizar
 */
void config_hard (void)
{

	Setear_Frec_Micro();
	SystemCoreClockUpdate();
//	Teclado_Config_Hard();

}

/*************************************
 * 		USB_funcion_principal
 * 	Esta funcion es llamada desde "HID Interrupt endpoint event handler"
 * 	tenemos que ller el rporte recivido y responder algo si corresponde
 * 	El tamaño y direccion de los enpoints y reportes esta definido en
 * 	app_usbd_cfg.h
 * 	#define HID_EP_IN       0x81
 * 	#define HID_EP_OUT      0x01
 * 	#define HID_INPUT_REPORT_BYTES		16	size of report in Bytes
 * 	#define HID_OUTPUT_REPORT_BYTES		16	size of report in Bytes
 * 	#define HID_FEATURE_REPORT_BYTES	1	size of report in Bytes
 */
void USB_Recibir_Dato(USBD_HANDLE_T hUsb, void *data)
{
	USB_HID_CTRL_T *pHidCtrl = (USB_HID_CTRL_T *) data;
	uint32_t leidos=0;
	uint8_t bufer[HID_INPUT_REPORT_BYTES];

	/* Leemos lo que nos enviaron */
	leidos = USBD_API->hw->ReadEP(hUsb, pHidCtrl->epout_adr, bufer);
	if(leidos)
		leidos=USB_Analizar_Reporte(bufer);
	if(leidos)
		//con esto ponemos lo que esta en el buffer en la "cola" de salida, ponerlo donde corresponda
		USBD_API->hw->WriteEP(hUsb, pHidCtrl->epin_adr, bufer, leidos);
}

uint32_t USB_Analizar_Reporte(uint8_t *pBufUSB)
{
	//dependiendo del contenido de lo que recibi, llama a una funcion u a otra
	//por ahora solo un funcion pa probar
	esSenConf sensores[Cantidad_Sensores_Max];
	int32_t temp=0;
	int i,tamano=0;

	xQueuePeek(cola_datos_sensores,&sensores,portMAX_DELAY);
	//armo respuesta
	for(i=0;i<Cantidad_Sensores_Max;i++)
	{
		temp=Sensor_Convertir_Valor((sensores+i)->Sensor.UltimaLectura,(sensores+i)->ValorRefCuent,(sensores+i)->ValorRefTemp);
		*pBufUSB=i;
		tamano++;
		entero2string(temp,(pBufUSB+tamano));
		tamano+=4;
	}
	return tamano;

}

void entero2string(int32_t valor,uint8_t *p)
{
	*p=( valor >> 24) & 0xff;
	*(p+1)=( valor >> 16) & 0xff;
	*(p+2)=( valor >> 8) & 0xff;
	*(p+3)=( valor ) & 0xff;
	/*
	 * Para "rearmar" el valor del entero hay que hacer alguno de los siguiente:
	 * i= ((*(p+0)<< 24) | (*(p+1)<<16) | (*(p+2) << 8) | (*(p+3)));
	 * i= aux[1] * 16777216 + aux[2] * 65536 + aux[3] * 256 + aux[4];
	 */
}

/*************************************
 * 		Setear_Frec_Micro
 * 	Establece la frecuencia del micro, es copia de "void Chip_SetupXtalClocking(void)"
 * 	Configura los divisores y el pll
 */
void Setear_Frec_Micro(void)
{
	/* Disconnect the Main PLL if it is connected already */
	if (Chip_Clock_IsMainPLLConnected()) {
		Chip_Clock_DisablePLL(SYSCTL_MAIN_PLL, SYSCTL_PLL_CONNECT);
	}

	/* Disable the PLL if it is enabled */
	if (Chip_Clock_IsMainPLLEnabled()) {
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
	 *	 96 [MHz] => M=(15+1) N=(0+1)	D=(3+1)
	 *	100 [MHz] => M=(24+1) N=(1+1)	D=(2+1) <--
	 *	120 [MHz] => M=(14+1) N=(0+1)	D=(2+1)
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
 * 		Tareas
 *=====================================*/
static void Tarea_Control(void *pvParameters)
{
	/*	Tarda aprox 38.000*4 pasos en hacer todo el recorrido que tiene disponible (los 70mm del stub)
	 * 	en tiempo son 5 minutos y 10 segundos
	 */
	esSenConf Sensores[Cantidad_Sensores_Max];

	int i;
	for(i=0;i<Cantidad_Sensores_Max;i++)
	{
		(Sensores+i)->Sensor.ID=i;
		(Sensores+i)->Sensor.Estado=SENSOR_ON;
		(Sensores+i)->Sensor.UltimaLectura=(i/2)?(300):(4000);
		(Sensores+i)->Alarma_Estado=SENSOR_ON;
		(Sensores+i)->Alarma_Value=(i>2)?(2222):(5555);;
		(Sensores+i)->ValorRefCuent=2000;
		(Sensores+i)->ValorRefTemp=25000;
	}


	while (1)
	{
		for(i=0;i<Cantidad_Sensores_Max;i++)
		{
//			(Sensores+i)->Sensor->Estado=SENSOR_ON;
			((Sensores+i)->Sensor.UltimaLectura)+=(i/2)?(20):(-10);
//			(Sensores+i)->Alarma_Estado=SENSOR_ON;
//			(Sensores+i)->Alarma_Value=(i>2)?(2222):(5555);;
//			(Sensores+i)->ValorRefCuent=2000;
//			(Sensores+i)->ValorRefTemp=25000;
			vTaskDelay(500/portTICK_PERIOD_MS);
			xQueueOverwrite(cola_datos_sensores,&Sensores);
		}

		vTaskDelay(2000/portTICK_PERIOD_MS);
	}
}


/*=====================================
 * 				MAIN
 *=====================================*/

int main(void)
{

	config_hard();
	usb_main_config();

	cola_datos_usb=xQueueCreate(1,HID_OUTPUT_REPORT_BYTES*sizeof(uint8_t));
	cola_datos_sensores=xQueueCreate(1,Cantidad_Sensores_Max*sizeof(esSenConf));

	xTaskCreate(Tarea_Control, (char *) "Auto",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(xTaskHandle *) NULL);


	/* Start the scheduler */
	vTaskStartScheduler();

	/* Nunca debería arribar aquí */

    return 0;
}


