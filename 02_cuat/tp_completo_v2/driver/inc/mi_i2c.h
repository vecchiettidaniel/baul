/* ==============================
 * 			CM_PER
 * ============================== */
// Defines para el modulo del CLock
// salen del L4_WKUP register (pag 170)
// esto para activar el clock del periferico
// estan cableados... como le hago pa leerlos del periferico¿?
#define CM_PER_dir_base	0x44E00000
#define CM_PER_reg_tam	0x400		// 1k
#define CM_PER_I2C_dir	0x44
#define CM_PER_I2C_val	(1<<1)      //0x00000002
#define CM_PER_IDLE_MASK (3<<16)    // 00=Modulo PK
									// 01=Modulo en proceso de wakeup,sleep etc
									// 02=IDLE
									// 03=modulo desabilitado y no usable
/* ==============================
 *		PIN_MUX/CONTROL MODULE
 * ============================== */
// Defines para activar los pines
// SCL=19
// SDA=20
#define PIN_MUX_dir_base	0x44E10000
//#define PIN_MUX_reg_tam		0x20000		//128k = 2^17
#define PIN_MUX_reg_tam		0x1000		//4k... pa que mas si alcanzo al 0x900
#define PIN_I2C_SCL_dir		0x097c		//uart1_rtsn
#define PIN_I2C_SDA_dir		0x0978		//uart1_ctsn
#define PIN_MODE_FUNC3		(3<<0)	//00000xxx funcion/modo
#define PIN_MODE_PullDis	(1<<3)	//0000x000 1=Desabilita Pulls
#define PIN_MODE_PullUP		(1<<4)	//000x0000 1=PullUp 0=PDown
#define PIN_MODE_RxEn		(1<<5)	//00x00000 1=IN/OUT 0=Out
#define PIN_MODE_Slow		(1<<6)	//0x000000 0=fast 1=Slow
#define PIN_MODE_FUNC7		(7<<0)	//00000xxx funcion/modo

#define PIN_I2C_SCL_val		(PIN_MODE_FUNC3 | PIN_MODE_RxEn | PIN_MODE_PullUP)
#define PIN_I2C_SDA_val		(PIN_MODE_FUNC3 | PIN_MODE_RxEn | PIN_MODE_PullUP)

#define PIN_GPIO_PULL_UP	(PIN_MODE_FUNC7 | PIN_MODE_PullUP)
#define PIN_GPIO_PULL_DW	(PIN_MODE_FUNC7)

/* ==============================
 *			REGISTROS I2C
 * ============================== */
// Defines par alos registros del I2C
// la base esta en el device tree: of_iomap((op->dev).of_node,0);
// o para cablear directo usar este:
#define I2C_dir_base	0x4819c000

//Estos serian los valores a cargar en los registros
/* ==============================
	I2C_CON_val		0x00
	bit 15: 1=modulo habilitado 0=desabilitado para configurarlo
	12-13 :	00= I2C standart, otro svalores estan reservados
	11		0=normal 1=START Byte mode 	va 0
	10 		(1/0) Master/Slave para ver si SCL es OUT/IN
	9		(0/1) Reciver/Transmiter juega junto con el bit 10
	8		tamaño de direccion del esclavo (0=7bits 1=10bits)
	7-4		idem 8 para mis dirs (tiene 4 OWN ADDRES)
	3-2		reservados a 0
	1		Condicion de STOP 0 y que lo maneje el micro
	0		Condicion de START 0 y que lo maneje el micro

    START & STOP:
    STOP  1 : cuando dcount llegue a 0 se genear un STOP condition
          0 : No se genera STOP y SCL pasa a 0, despues puedo hacer otro START
              para realizar una nueva transferencia
    START 1 : para generar STARTs labura con stop y dcount....
*/
#define I2C_CON_dir		0xa4
#define I2C_CON_START   (1<<0)  // 0x0001 genera start condition Master
#define I2C_CON_STOT    (1<<1)  // 0x0002 genera STOP condition
#define I2C_CON_Tx      (1<<9)  // 0x0200 Modo Transmision
#define I2C_CON_Master  (1<<10) // 0x0400 Maestro
#define I2C_CON_EN      (1<<15) // 0x8000 Maestro
#define I2C_CON_DIS     0       // todo a 0...sof

#define I2C_CON_read	0x8401	// Maestro, recibiendo y condicion de start
#define I2C_CON_leer	(I2C_CON_Master|I2C_CON_START|I2C_CON_EN|I2C_CON_STOT)
//#define I2C_CON_val		0x8202	//Valor original

/* ==============================
	Configuracion del reloj o clck que usa el I2C
	1º toma el reloj del sistema y lo divide por un preescaler
		ClockI2C= ClockSystema / (preescaler+1)

	2º para "crear" el clock que sale por SCL pone el pin en 1 y 0 segun
		SCL=1 dutante ( SCLH + 5)*(Clock_I2C)=thigh
		SCL=0 dutante ( SCLL + 7)*(Clock_I2C)=tlow

	SCLL/SCLH/Preescaler son valores de 8 bits (0 a 255) pero registros de 32
	Al final lo que hice fue leer los valores origuinales, esto es sin modificar
	el device tree, y eso es lo que pongo aca... me da una frec de 100[kH]
    => I2c_clok = CLCKSYS/(PSC+1)=> CLOKSYS=I2c*(PSC+1)= 100k*11 =1M¿?
    => SCL_1=(f+5)*Clock_i2c = 20 * i2c_clok
    => SCL_0=(d+7)*Clock_i2c = 20 * i2c_clok
*/
#define I2C_PSC_dir 	0xb0
#define I2C_SCLL_dir	0xb4
#define I2C_SCLH_dir	0xb8

#define I2C_PSC_val 	0x0000000b	//ICLK=SCLK/(I2C_PSC_val+1)
#define I2C_SCLL_val	0x0000000d	//Tiempo del 0
#define I2C_SCLH_val	0x0000000f	//Tiempo del 1

/* ==============================
	I2C_SYSC_dir	0x10
	IDLE state y wake up enabled,
	I2C_SYSC_val este registro se usa pa configurar el iddle state
	Tamien tiene un reset de software... por ahora va todo a 0
	0 Autoidle 	0=desabilitado
	1 SoftReset	1=Reseteamos tod el modulo
	2 ENWAKE	0=Mecanismo de wake up desabilitado
   3-4 IDLEMODE 01 = No idle mode, 00=forzar idle¿? 10=smartIdle
   5-7 reservados 0
   8-9 CLOCKIDLE alg que ver con la psibilidad de apagar los clock
				 cuando estoy en modo idle...
				11 es no romper y dejar los closck activos
===========

	registro I2C_SYSS_dir 0x90
	0 	este bit me indica si termin de resetearce en caso que le haya mandado
		  un I2C_SYSC_Reset, un I2C_CON_EN o reset de hardware
	el resto esta reservado
		1=finaliz el reset
		0= sigo resetenadome
*/

#define I2C_SYSC_dir        0x10
#define I2C_SYSC_Reset      (1<<1) // (0x00000002) borra todos los registros
#define I2C_SYSC_IDLEOFF	(1<<3) // (0x00000008) 0x08=no idle
#define I2C_SYSC_AUTIDLE	(1<<0) // (0x00000001) autoidel on/off
#define I2C_SYSC_WKUPEN		(1<<2) // (0x00000004) habilitar wape up
#define I2C_SYSC_CLKACTV    (1<<8) // (0x00000300) 3=> dejar activos ambos clocks
#define I2C_SYSC_val	(I2C_SYSC_IDLEOFF|I2C_SYSC_CLKACTV) //0x308

#define I2C_SYSS_dir	0x90	//0=esta reseteandose 1=termino el reset
#define I2C_SYSS_mask	0x00000001	//0=esta reseteandose 1=termino el reset
//#define I2C_SYSC_val	0x0000021d	//Valor original

/* ==============================
	Direccion del esclavo
	sera de 7 bits y el LM75 tiene asi 01001XXX
*/
#define I2C_SA_dir		0xac
#define I2C_SA_val		0x00000049	//direccion del esclavo

#define I2C_OA_dir		0xa8
#define I2C_OA_val		0x0000000f

/* ==============================
	Direccion del esclavo
	sera de 7 bits y el LM75 tiene asi 01001XXX
*/
#define I2C_COUNT_dir	0x98		// Cantidad de bytes a enviar/recibir
#define I2C_DATA_dir	0x9c	// Aca se escribe lee lo que se quiere mandar

/* ==============================
 *			FIFOs -DMA
 * ============================== */

// FIFOs Status
#define I2C_FIFO_STATUS_dir	0xC0
// FIFO control
#define I2C_FIFO_CTR_dir	0x94
// para el nivel de la fifo, el nivel posta es lo que escribo - 1
// si Pongo un 2 => me interrumpe cuando hay 3 bytes o 3 datos disponibles

#define I2C_FIFO_Tx_count   (0x3f<<0)   // bits 0-5 =FIFO_TX level
#define I2C_FIFO_Tx_CLR		(1<<6)	    // borramos el contenido de la TX FIFO
#define I2C_FIFO_TxDMA_E	(1<<7)	    // habilta DMA en transmicin
#define I2C_FIFO_Rx_count   (0x3f<<8)   // bits 8-13=FIFO_RX level
#define I2C_FIFO_Rx_CLR		(1<<14)	   // borramos el contenido de la RX FIFO
#define I2C_FIFO_RxDMA_E	(1<<15)	   // habilta DMA en recepcion

#define I2C_FIFO_Tx_pos     (0)         // posicion del nivel de disparo
#define I2C_FIFO_Rx_pos     (8)         // posicion del nivel de disparo
#define i2c_FIFO_Rx_level(x) ((x) << I2C_FIFO_Rx_pos )
#define i2c_FIFO_Tx_level(x) ((x) << I2C_FIFO_Tx_pos )
/* ==============================
 *		Interrumpcines
 * ============================== */

#define I2C_IRQ_STATUS_dir	0x28
#define I2C_IRQ_RAW_dir		0x24
#define I2C_IRQENABLE_dir	0x2c
#define I2C_IRQDISABLE_dir	0x30

#define I2C_IRQ_AL		(1<<0)	// Arbitration Lose
#define I2C_IRQ_NACK	(1<<1)	// no recibi el acknowledge
#define I2C_IRQ_ARDY	(1<<2)	// Registros listos para acceso
#define I2C_IRQ_RRDY	(1<<3)	// Datos disponibles pa lectura (nivel de Rxfifo)
#define I2C_IRQ_XRDY	(1<<4)	// nivel de Txfifo alcanzado,
#define I2C_IRQ_GC		(1<<5)	// MODO SCLAVO: detecte direccion 00
#define I2C_IRQ_STC		(1<<6)	// detecte START... para iddle y wake up
#define I2C_IRQ_AERR	(1<<7)	// Error de acceso (FIFO TX llena o Rx vacia)
#define I2C_IRQ_BF		(1<<8)	// Bus libre (recibi un STOP condition)
#define I2C_IRQ_AAS		(1<<9)	// Me quieren hablar (slave addres detectec)
#define I2C_IRQ_XUDF	(1<<10)	// Master: Dcount!=0 y tx vacio
								// SLAVE: Recivo read request pero Tx vacio
#define I2C_IRQ_ROVR	(1<<11)	// Rx apinto de overrun (shift y rxfifo llenas)
#define I2C_IRQ_BB		(1<<12)	// Bus cupado MASTER: lo maneja... SLAVE: recibi start
#define I2C_IRQ_RDR		(1<<13)	// Cuando recibo menos que el fifo level
#define I2C_IRQ_XDR		(1<<14) // Cuando quedan menos datos pa enviar que el fifo level

#define I2C_IRQDISABLE_all	0x00006fff	//Desabilito todas
#define I2C_IRQENABLE_val	0x00000008	//Recive data ready


/* ==============================
 *		Funciones
 * ============================== */
int mi_i2c_ISR (void *p,unsigned int *aca);
int mi_i2c_Clock_enable (void);
int mi_i2c_Pin_config (void);
int mi_i2c_reg_config (void *p);
int i2c_data_a_temp(unsigned int *p);
void leer_registros_i2c (void *p);
void distintos_printk(void);
