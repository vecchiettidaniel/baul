#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/semaphore.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include "mi_i2c.c"

#define	FIRST_MINOR 0
#define	CANT_DISPO 1
#define	TAMANO_BUFFER 200
#define	LE_PIFIE_FEO 1


MODULE_LICENSE("Dual BSD/GPL");

int mi_func_open (struct inode *, struct file *);
int mi_func_close (struct inode *, struct file *);
ssize_t mi_func_write (struct file *, const char __user *, size_t, loff_t*);
ssize_t mi_func_read (struct file *, char __user *, size_t, loff_t *);

int mi_func_probe(struct platform_device *);
int mi_func_remove(struct platform_device *);
irqreturn_t mi_i2c_irq_handler(int irq,void *dev_id);

/*
 * Declaramos las variables y estructuras que vamos a usar para armar el driver
 *
 * */

// esto para el wait de la IRQ
DECLARE_WAIT_QUEUE_HEAD(mi_despertador);
DEFINE_SEMAPHORE(mi_semaforo);// arranca con valor 1

static dev_t mi_numero_dev;		//tiene Nº mayor y Nºs menores
static struct cdev* mi_disp;	// char device
static struct class* mi_clase;	// para crear una clase nueva en /sys y /proc
static struct device* mi_sysdev;// este es el device que aparecera en /sys/"mi_class"
static struct file_operations mi_file_OP ={
		.owner	= THIS_MODULE,
		.open	= mi_func_open,
		.release= mi_func_close,
		.write	= mi_func_write,
		.read	= mi_func_read,
	};

/*
	Con la siguiente estructura y con la llamada a MODULE_DEVICE_TABLE
	decimos que este driver responde a la entrada "compatible".
*/
static struct of_device_id mi_of_table[] ={
	{.compatible = "td3,mi_i2c_driver",},//esto tiene que estar en el device tree
	{},//indica fin de lista de compatibles
	};

MODULE_DEVICE_TABLE(of,mi_of_table);	//con esto hacemos "disponible" (global?) la tabla de of_device_ids


static struct platform_driver mi_plat_driver={
	.probe	=mi_func_probe,
	.remove	=mi_func_remove,
	.driver	= {		// cuando llamo a platform_driver_register() me llena el resto de esta estructura
				.owner = THIS_MODULE,
				.name="mi_i2c_driver",
				.of_match_table=mi_of_table,
				//.of_match_table=of_match_ptr(mi_of_table),
			}
	};

/*
 * Esta estructura es para tener todas las variables que voy a usar declaradas en
 * un solo puntero, asi para no pisar algun nombre que ya este usado por el kernel
 * el static es para asegurarme que no estoy usando algo que ya esta en uso
 * STATIC= solo vale para este programa
 * FALTA: Hacer lo mismo con todo lo que uso pal driver*/


/* si declaro la struct mi_i2c... como estatica me tira este warning:
warning: useless storage class specifier in empty declaration
  };
  ^
*/
struct mi_i2c_data_cfg
	{
		int vIRQ_N;			// virtual IRQ
		int Veces_Abierto;	// Cuantos open le hice al driver
		void *i2c_base;		// Base del mapeo de los registros a memoria
		unsigned int *pdato;// para guardar los datos leidos del I2C
		int irq_flag;		// Flag para sincronizar READ y Handler
	};

static struct mi_i2c_data_cfg *pmi_cfg;

/*========================
 *		PROBE()
 *========================*/
int mi_func_probe(struct platform_device *op)
{
	const struct of_device_id *match;
	printk(KERN_CRIT "PROBE: estoy en el probe\n");

	// me aseguro que mi struct device coincida con alguna entrada en la tabla de of_device_id
	// seria chequear que la estrcutura que me pasa el kernel coinsida con alguna entrada y
	// asi me aseguro que estoy atendiendo al dispositivo correcto
	match = of_match_device(mi_of_table, &op->dev);
	if (!match)
	{
		printk(KERN_ALERT "Dispositivo no coinside con l alista de OF\n");
	    return -EINVAL;
	}

	// Habilitamo el clock del periferico, hay que hacerlo antes
	// de poder acceder a los registros o se rompe
	if(mi_i2c_Clock_enable())
	{
		printk(KERN_ALERT "Fallo configuracion del Clock\n");
		return LE_PIFIE_FEO;
	}

	if(mi_i2c_Pin_config())
	{
		printk(KERN_ALERT "Fallo configuracion de pines\n");
		return LE_PIFIE_FEO;
	}

	// Mapeo los registros del I2C a memoria pa usarlos despues
	// lo hago en el prove asi pued usar este codigo para otros I2Cs
	pmi_cfg->i2c_base=of_iomap((op->dev).of_node,0);
	if (pmi_cfg->i2c_base==NULL)
	{
		printk(KERN_ALERT "error al mapear el i2C\n");
		return LE_PIFIE_FEO;
	}

	// Preparo la parte de Interrupciones
	pmi_cfg->vIRQ_N=platform_get_irq(op,0);
	if(pmi_cfg->vIRQ_N <= 0)
	{
		printk(KERN_NOTICE "ERROR platform_get_irq devolvio %d\n",pmi_cfg->vIRQ_N);
		return LE_PIFIE_FEO;
	}

	if (request_irq(pmi_cfg->vIRQ_N,mi_i2c_irq_handler,
		IRQF_TRIGGER_RISING,op->name,(void *) pmi_cfg))
	{//devuelve 0 si todo OK
		printk(KERN_ALERT "Error en platform_get_IRQ\n");
		return LE_PIFIE_FEO;
	}

	printk(KERN_CRIT "PROBE: FIN funcion probe\n");
	return(0);
}

/*========================
 *		REMOVE()
 *========================*/
int mi_func_remove(struct platform_device *p)
{
	printk(KERN_CRIT "REMOVE: estoy en el remove\n");
	mi_i2c_Clock_disable();

	free_irq (pmi_cfg->vIRQ_N,pmi_cfg);
	if(pmi_cfg->i2c_base!=NULL)
	{
		iounmap(pmi_cfg->i2c_base);
	}

	return 0;
}

/*========================
 *		CLOSE()
 *========================*/
int mi_func_close (struct inode *a, struct file *b)
{
	printk(KERN_CRIT "CLOSE: toy en el close\n");
	if(down_interruptible(&mi_semaforo)==0)
	{// tome el semaforo
		pmi_cfg->Veces_Abierto-=1;
		up(&mi_semaforo);//libera el semaforo
	}

	if(pmi_cfg->pdato != NULL)
	{
		kfree(pmi_cfg->pdato);
	}
	return 0;
}
/*========================
 *		OPEN()
 *========================*/

int mi_func_open (struct inode *a, struct file *b)
{
	// toma el semaforo
	//	0 		si lo pudo tomar
	//	-EINTR 	si una señal interrumpio
	//	-ETIME 	si se cumplio el tiempo
	//if(down_timeout(&mi_semaforo,2000)==0)//2000 = tiempo de espera en ¿jiffies?
	if(down_interruptible(&mi_semaforo)==0)
	{// tome el semaforo

	}
	if(pmi_cfg->Veces_Abierto)
	{
		// le quiero decir al que hiso el open que ya hay uno abierto
		// NO se como hacerlo
		//printk(KERN_EMERG "OPEN: YA existe una conexion\n");
		printk(KERN_ALERT "OPEN: YA existe una conexion\n");
		up(&mi_semaforo);//libera el semaforo
		return(-1);
	}
	else
	{
		pmi_cfg->Veces_Abierto+=1;
	}
	up(&mi_semaforo);//libera el semaforo

	printk(KERN_CRIT "OPEN: toy en el openn\n");
	//tomar un semaforo o algo pa que solo 1 procesos pueda tener acces al driver

	if(mi_i2c_reg_config (pmi_cfg->i2c_base))
	{// devuelve 0 si todo ok
		printk(KERN_ALERT "OPEN: ERROR en reg_config\n");
		return(LE_PIFIE_FEO);
	}

	pmi_cfg->pdato=(uint *) kmalloc (I2C_DATA_tam * sizeof(uint), GFP_KERNEL);
	if(pmi_cfg->pdato == NULL)
	{
		printk(KERN_ALERT "OPEN: ERROR kmalloc\n");
		return (LE_PIFIE_FEO);
	}

	printk(KERN_CRIT "OPEN: Fin Open\n");
	return 0;
}

/*========================
 *		WRITE()
 *========================*/
ssize_t mi_func_write (struct file *fp, const char __user *ubuffer, size_t tam_pedido, loff_t* uoffset)
{
	printk(KERN_CRIT "WRITE: estoy en el write\n");
	return 0;
}

/*========================
 *		READ()
 *========================*/
ssize_t mi_func_read (struct file * fp, char __user * ubuffer, size_t cant_a_copiar, loff_t * offset)
{
	int i=0;

	if(cant_a_copiar < sizeof(int))
	{// solo voy a copiar un entero
		printk(KERN_CRIT "READ: Lugar insusficiente\n");
		return (-1);
	}

//	printk(KERN_CRIT "READ: estoy en el read\n");

	pmi_cfg->irq_flag=0;
//	printk(KERN_NOTICE "READ: A dormir\n");

	iowrite32(I2C_IRQ_RRDY,((pmi_cfg->i2c_base)+I2C_IRQENABLE_dir));	// Habilito Interrupciones
	iowrite32(I2C_DATA_tam,((pmi_cfg->i2c_base)+I2C_COUNT_dir));		// Nº de bytes
	iowrite32(I2C_CON_leer,((pmi_cfg->i2c_base)+I2C_CON_dir));			// Habilito la lectura

	wait_event_interruptible(mi_despertador,pmi_cfg->irq_flag);
	iowrite32(I2C_IRQ_RRDY,((pmi_cfg->i2c_base)+I2C_IRQDISABLE_dir));	// Deshabilito Interrupciones

//	printk(KERN_NOTICE "READ: me desperte, condicion=%d\n",pmi_cfg->irq_flag);
	if(!(pmi_cfg->irq_flag))
	{
		printk(KERN_ALERT "READ: Acurrio un error\n");
		return 0;
	}

	i=i2c_data_a_temp(pmi_cfg->pdato);
//	printk(KERN_WARNING "READ: finalizando TEMP = %d",i);
	// quiero mandarle al usuario un int, copytouser me devuelve lo que le
	// falto copiar (0 si copio todo)...yo le mando al usuario la cantidad de
	// bytes que copie
	return (sizeof(int) - copy_to_user(ubuffer , &i , sizeof(int)));
}

/*========================
 *		ISR Handler
 *========================*/

irqreturn_t mi_i2c_irq_handler(int irq,void *dev_id)
{
//	printk(KERN_CRIT "ISR: estoy en interrupcion\n");
	// chequeo cual fue la interrupcion
	//printk(KERN_CRIT "ISR: IRQSTATUS=%x\n",ioread32(pmi_cfg->i2c_base+I2C_IRQ_STATUS_dir));
	if ((ioread32(pmi_cfg->i2c_base+I2C_IRQ_STATUS_dir) & I2C_IRQ_RRDY))
	{//si tengo datos listos para leer, los leo y limpio el flag
		//pmi_cfg->irq_flag=mi_i2c_ISR_debug(pmi_cfg->i2c_base,pmi_cfg->pdato);
		pmi_cfg->irq_flag=mi_i2c_ISR(pmi_cfg->i2c_base,pmi_cfg->pdato);
		wake_up_interruptible(&mi_despertador);
//		printk(KERN_CRIT "ISR: Finalizando, condicion=%d\n",pmi_cfg->irq_flag);
	}
	return (IRQ_HANDLED);
}

/*========================
 *		MODULE_INIT()
 *========================*/

static int mi_func_init(void)
{
	printk(KERN_CRIT "INIT: TOY en el INIT\n");
	// 1ro Pido un Numero Mayor al jefe Linux
	//devuelve 0 o un codigo de error negativo
	if( (alloc_chrdev_region(&mi_numero_dev, FIRST_MINOR,CANT_DISPO,"DanielVec")) < 0)
	{
		printk(KERN_ALERT "Error al pedir Nº Mayor\n");
		return LE_PIFIE_FEO;
	}
	printk(KERN_NOTICE "INIT: Tengo Nº Mayor %d\n",MAJOR(mi_numero_dev));

	// Ya tengo un Nº Mayor, ahora a armar la esctructura del dispositivo (la cdev)
	//1º pido memoria para la estructura (ya me inicializa algunos campos de la estructura)
	if ( (mi_disp=cdev_alloc()) == NULL)
	{//devuelve NULL si hubo error
		printk(KERN_ALERT "Error con cdev_alloc\n");
		unregister_chrdev_region(mi_numero_dev,CANT_DISPO);
		return LE_PIFIE_FEO;
	}

	//printk(KERN_NOTICE "Tengo cdev vacia\n");
	//2º Ya tengo la estructura pero ta vacia, a inicializar los campos necesarios:
	//cdev_init(mi_disp,&mi_file_OP);
	mi_disp->ops=&mi_file_OP;
	mi_disp->dev=mi_numero_dev;
	mi_disp->owner=THIS_MODULE;

	//3º estructura inicializada con los campos que necesito, falta registrarla
	if((cdev_add(mi_disp,mi_numero_dev,CANT_DISPO)) < 0)
	{// retorna <0 si hubo error
		printk(KERN_ALERT "Error con cdev_add\n");
		cdev_del(mi_disp);
		unregister_chrdev_region(mi_numero_dev,CANT_DISPO);
		return LE_PIFIE_FEO;
	}
	//printk(KERN_NOTICE "Tengo cdev llena\n");

	// Ya tenemos el driver registrado, falta crear el "punto de acceso" (entrada en el directorio /sys y /dev)
	// para eso en vez de registrarlo dentro de alguna de las clases que ya tiene linux, nos creamos un anueva
	// 1º- Creamos l anueva clase:
	mi_clase=class_create(THIS_MODULE, "mi_i2c_Class");
	mi_sysdev=device_create(mi_clase,NULL,mi_numero_dev,NULL,"mi_i2c");
	//Ya esta!!! supuestamente ahor atenemos todo OK... faltaria armar las funciones open y eso
	// Acordarce que en el exit hay qu edesacer todo esto y en orden
	//printk(KERN_NOTICE "Dispositivo y entrada en /sys creadas!!!\n");


	//Pedimos memoria para nuestras variables, estas son en memoria kernel
	pmi_cfg=(struct mi_i2c_data_cfg *) kmalloc(sizeof(struct mi_i2c_data_cfg),GFP_KERNEL);
	if( pmi_cfg == NULL)
	{
		printk(KERN_ALERT "Error Kmalloc pmi_cfg\n");
		return LE_PIFIE_FEO;
	}

	//con esto linkeamos el device tree con la estructura plataform driver
	platform_driver_register(&mi_plat_driver);
	pmi_cfg->Veces_Abierto=0; // Cuantos open me hicieron
	printk(KERN_CRIT "Fin funcion INIT \n");
	return 0;
}

/*========================
 *		module_EXIT()
 *========================*/
static void mi_func_exit(void)
{
	printk(KERN_CRIT "EXIT: estoy en el EXIT\n");

	platform_driver_unregister(&mi_plat_driver);
	kfree(pmi_cfg);
	device_destroy(mi_clase,mi_numero_dev);
	class_destroy(mi_clase);
	cdev_del(mi_disp);
	unregister_chrdev_region(mi_numero_dev,CANT_DISPO);
	printk(KERN_NOTICE "EXIT: ya limpie todo...Chau\n");

}

module_init(mi_func_init);
module_exit(mi_func_exit);

/* exec{[v|l][p|e]}
 * execl: accept a variable number of arguments. necesita <stdarg.h>
 * execv: the arguments are passed using an array of pointers to strings
 *			where the last entry is NULL. como *argv[]={"arg1","arg2",NULL}
 * exec-p: usa la environment variable PATH
 * exec-e: le paso array of strings con environment variables, null terminated
			char *envp[] = {"USER=dwharder", "HOME=/home/dwharder", NULL};
*/
