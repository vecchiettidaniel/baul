#include <linux/delay.h>
#include "./inc/mi_i2c.h"

#define demora_ms 		10
#define Num_intentos 	100
#define I2C_DATA_tam    0x2	 // Tamño de la transferencia del sensor

/*========================
 *    mi_i2c_Clock_enable
 *========================*/
int mi_i2c_Clock_enable (void)
{
	void *p;
	uint aux;

	// Mapeamos a memoria los registros del contrlador de Clock
	p=ioremap((unsigned long)CM_PER_dir_base,(unsigned long)CM_PER_reg_tam);
	if(p==NULL)
	{
		printk(KERN_ALERT "Error al mapear CM_PER\n");
		return(1);
	}
	//printk(KERN_NOTICE "Mapeo CM_PER a %X \n",(uint) p);
	// habilitamos el clock en el I2C
	iowrite32(CM_PER_I2C_val,(p+CM_PER_I2C_dir));

	aux=Num_intentos;
	while( aux && (ioread32(p+CM_PER_I2C_dir) & CM_PER_IDLE_MASK) )
		{	//los bit IDLEs tienen que estar en 00
			msleep(demora_ms);
			aux--;
		}
	if(aux==0)
	{
		printk(KERN_ALERT "Nunca Termina de habilitar el clock\n");
		iounmap(p);
		return 1;
	}
	printk(KERN_NOTICE "Clock del periferico habilitado\n");
	iounmap(p);

	return 0;
}

int mi_i2c_Clock_disable (void)
{
	void *p;
	// Mapeamos a memoria los registros del contrlador de Clock
	p=ioremap((unsigned long)CM_PER_dir_base,(unsigned long)CM_PER_reg_tam);
	if(p==NULL)
	{
		printk(KERN_ALERT "Error al mapear CM_PER\n");
		return(1);
	}
	// Desbilita el clock en el I2C
	iowrite32(0,(p+CM_PER_I2C_dir));
	printk(KERN_NOTICE "Clock del periferico apagado\n");
	iounmap(p);
	return 0;
}
/*========================
 *    mi_i2c_Pin_config
 *========================*/

int mi_i2c_Pin_config (void)
{
	void *p;
	int aux;
	// Mapeamos a memoria los registros del pin mux
	p=ioremap((unsigned long)PIN_MUX_dir_base,(unsigned long)PIN_MUX_reg_tam);
	printk(KERN_NOTICE "Mapeo Pin_Mux a %X \n",(uint) p);
	if(p==NULL)
	{
		printk(KERN_ALERT "Error al mapear Pin_MUX\n");
		return(1);
	}

	aux=0;
	//seleccionamos Modo y Funcion para scl y sda
	iowrite32(PIN_I2C_SCL_val,(p+PIN_I2C_SCL_dir));
	iowrite32(PIN_I2C_SDA_val,(p+PIN_I2C_SDA_dir));
	iounmap(p);
	printk(KERN_WARNING "Pines Cnfigurados \n");
	return 0;
}

/*========================
 *   mi_i2c_reg_config
 *========================*/

int mi_i2c_reg_config (void *p)
{
	uint aux;
	//Reseteamos el I2C (hacemos un software reset pa que borre todo)
	iowrite32(I2C_SYSC_Reset,(p+I2C_SYSC_dir));	// resetear todo el modulo
	printk(KERN_WARNING "Periferico reseteado a valores por defecto\n");
	leer_registros_i2c(p);

	//Ahora tenemos que configurar el I2C
	//Configuramos el I2C
	// primero reseteamos o desactrivamos el modulo
	printk(KERN_WARNING "Cambiando configuracion\n");
	iowrite32(I2C_CON_DIS,(p+I2C_CON_dir));		//

	//Ok, ahora a configurar los registros
	iowrite32(I2C_PSC_val,(p+I2C_PSC_dir));		// preescaler
	iowrite32(I2C_SCLL_val,(p+I2C_SCLL_dir));	// div bajo
	iowrite32(I2C_SCLH_val,(p+I2C_SCLH_dir));	// div alto
	iowrite32(I2C_OA_val,(p+I2C_OA_dir));		// mi dir, solo para mod esclavo
	iowrite32(I2C_SYSC_val,(p+I2C_SYSC_dir));	// mi dir, solo para mod esclavo
	iowrite32(I2C_SA_val,(p+I2C_SA_dir));		// dir del otro dispo
	iowrite32(I2C_IRQDISABLE_all,(p+I2C_IRQDISABLE_dir));	// deshabilitamos
	iowrite32(I2C_FIFO_Rx_CLR|I2C_FIFO_Tx_CLR,(p+I2C_FIFO_CTR_dir));//vaciamos FIFOS y reprogramamos los niveles
	iowrite32(i2c_FIFO_Rx_level(I2C_DATA_tam-1),(p+I2C_FIFO_CTR_dir));	// nivel de disparo
	printk(KERN_WARNING "Seteando Cambios\n");
	iowrite32(I2C_CON_EN,(p+I2C_CON_dir));		// habilitamos

	// recien aca el bit de SYSS esta en 1 cuand termina el reset
	// o sea que tengo que tener habilitado el CON_EN para usar ese bit....

	aux=Num_intentos;
	while( aux && !(ioread32(p+I2C_SYSS_dir) & I2C_SYSS_mask))
		{	//si SYSS.bit0 esta en 1 termine el reset
			msleep(demora_ms);
			aux--;
		}
	if(aux==0)
	{
		printk(KERN_ALERT "Nunca termino el I2C_CON_Enable\n");
		iounmap(p);
		return 1;
	}

	printk(KERN_NOTICE "Nueva Configuracion del periferico\n");
	leer_registros_i2c(p);
	return 0;
}

/*========================
 *   mi_i2c_irq_handler
 *========================*/
int mi_i2c_ISR (void *p,unsigned int *aca)
{
	int i=0;
	// leo mientras quede algo en la FIFO
	while(ioread32(p+I2C_FIFO_STATUS_dir) & I2C_FIFO_Rx_count)
	{
		*(aca+i)=ioread32(p+I2C_DATA_dir);
		i++;
	}

	iowrite32(I2C_IRQ_RRDY,(p+I2C_IRQ_STATUS_dir));	// Limpia el flag
	return i;
}

/*==============*/
int mi_i2c_ISR_debug (void *p,unsigned int *aca)
{
	int i=0;
	int debugaux;
	// leo mientras quede algo en la FIFO

	debugaux=(ioread32(p+I2C_FIFO_STATUS_dir) & I2C_FIFO_Rx_count);
	printk(KERN_NOTICE "ISR: quedan %d datos en la FIFO",debugaux>>I2C_FIFO_Rx_pos);
	while(debugaux)

	while(ioread32(p+I2C_FIFO_STATUS_dir) & I2C_FIFO_Rx_count)
	{
		*(aca+i)=ioread32(p+I2C_DATA_dir);
		i++;
		debugaux=(ioread32(p+I2C_FIFO_STATUS_dir) & I2C_FIFO_Rx_count);
		printk(KERN_NOTICE "ISR: quedan %d datos en la FIFO",debugaux>>I2C_FIFO_Rx_pos);
	}

	iowrite32(I2C_IRQ_RRDY,(p+I2C_IRQ_STATUS_dir));	// Limpia el flag
	return i;
}

/*========================
 *   Funciones pa maquillar cosas
 *========================
 * Recive un buffer con los dos valores leidos
 * y devuelve un entero que corresponde a la temperatura * 10
 * [tttttttt txxxxxxx]
 * x = bits basura
 * bit7 a bit 15 = datos de la temperatura en cmplemento a 2
 * 1 bit = 0,5 [ºC]
 */
int i2c_data_a_temp(unsigned int *p)
{
	volatile int dato;
	dato= ( *(p+1) & 0x000000ff );
	dato = dato + ( ( *(p) & 0x000000ff ) <<8 );
	dato = (dato >> 7);
	if( dato > 0x100)
	{//es negativo
		dato=(~dato)-1;
	}
	dato=dato*5;	//0,5ºC de resolucion asique dividir por 10 despues
	return((int)dato);
}

/*========================
 *   Funciones pa probar csas
 *========================*/

void leer_registros_i2c (void *p)
{
	printk(KERN_NOTICE "I2C_OA_dir tiene %x \n",ioread32(p+I2C_OA_dir));
	printk(KERN_NOTICE "I2C_SYSC_dir tiene %x \n",ioread32(p+I2C_SYSC_dir));
	printk(KERN_NOTICE "I2C_SCLH_dir tiene %x \n",ioread32(p+I2C_SCLH_dir));
	printk(KERN_NOTICE "I2C_SCLL_dir tiene %x \n",ioread32(p+I2C_SCLL_dir));
	printk(KERN_NOTICE "I2C_PSC_dir tiene %x \n",ioread32(p+I2C_PSC_dir));
	printk(KERN_NOTICE "I2C_CON_dir tiene %x \n",ioread32(p+I2C_CON_dir));
	printk(KERN_NOTICE "I2C_SYSS_dir tiene %x \n",ioread32(p+I2C_SYSS_dir));
	printk(KERN_NOTICE "I2C_FIFO_CTR_dir tiene %x \n",ioread32(p+I2C_FIFO_CTR_dir));
	printk(KERN_NOTICE "I2C_IRQRAW_dir tiene %x \n",ioread32(p+I2C_IRQ_RAW_dir));
	printk(KERN_NOTICE "I2C_IRQSTATUS_dir tiene %x \n",ioread32(p+I2C_IRQ_STATUS_dir));
	printk(KERN_NOTICE "I2C_IRQENABLE_dir tiene %x \n",ioread32(p+I2C_IRQENABLE_dir));
}

/* PRINTK:
0 	KERN_EMERG 	Emergency condition, system is probably dead
1 	KERN_ALERT 	Some problem has occurred, immediate attention is needed
2 	KERN_CRIT 	A critical condition
3 	KERN_ERR 	An error has occurred
4 	KERN_WARNING 	A warning
5 	KERN_NOTICE 	Normal message to take note of
6 	KERN_INFO 	Some information
7 	KERN_DEBUG 	Debug information related to the program
*/
void distintos_printk(void)
{

	// el "KERN_EMERG" lo apago porque hace escandalo y tira derecho a la consola
	//printk(KERN_EMERG "Emergency condition, system is probably dead\n");
	//KERN_ALERT RPJO SOBRE ROJO muy molesto
	printk(KERN_ALERT "Some problem has occurred, immediate attention is needed\n");
	//KERN_crit RoJO SOBRE Negro safa ... KERN_ERR es parecido pero sin negrita
	printk(KERN_CRIT "A critical condition\n");
	printk(KERN_ERR "An error has occurred\n");
	// los que siguen son blanco sobre negor osea que camina ok
	printk(KERN_WARNING "A warning\n");	// este es en negrita
	printk(KERN_NOTICE "Normal message to take note of\n");
	printk(KERN_INFO "Some information\n");
	printk(KERN_DEBUG "Debug information related to the program\n");
}
