#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define LISTEN_SIMULTANEOS 2    // conexiones en espera (LISTEN)
#define HIJOS_CONCURRENTES 3  // Clientes atendidos (Nº Hijos)
#define	LE_PIFIE_FEO 1
#define TAMANO_BUFFER 200

/* =======================
 *
 * =======================*/
void manejadorSIGCHLD(int sig);
int codigo_pibes(int sfd,int mi_memoria,int mi_semaforo);

int limpiar_tuti(int memoria,const void *p,int semaforo);

/* =======================
 *
 * =======================*/

// sacado de man semctl... se necesita declarar esta estructutra pa operar.
union semun {
   int              val;    /* Value for SETVAL */
   struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
   unsigned short  *array;  /* Array for GETALL, SETALL */
   struct seminfo  *__buf;  /* Buffer for IPC_INFO */
};

struct dato_memoria{
    int temperatura;
    unsigned long N_lectura;
    // hora de lectura o algo asi
};

int hijos_vivos;
int hijos_Creados;
pid_t *lista_hijos_id;

/* =======================
 *
 * =======================*/
int desatachar_memoria (const void *p)
{

	if( (shmdt(p)) ==-1)
	{
		perror("error al desatachar la memoria\n");
		return (LE_PIFIE_FEO);
	}
    return 0;
}

int limpiar_tuti(int memoria,const void *p,int semaforo)
{
	int i=0;

    if(p!=NULL)
    {
        i=desatachar_memoria(p);
    }

	// borramos/eliminamos el semaforo
	if(semaforo!=0)
	{
		if( ( semctl(semaforo,0,IPC_RMID) ) ==-1)
		{
			perror("error al borra el semaforo\n");
			i=LE_PIFIE_FEO;
		}
	}

	if( (shmctl(memoria,IPC_RMID,0)) ==-1)
	{
		perror("error al eliminar la memoria\n");
		i=LE_PIFIE_FEO;
	}
	return i;
}

/* =======================
 *
 * =======================*/
void manejadorSIGCHLD(int sig)
{
	int i,valor_del_exit;
	pid_t pid;
	//lista hijos tiene todos los pid de los hijos que estan vivos
    //hijos_vivos es la cantidad de hijos que estan vivos
    // lista+vivos apunta a la siguiente posicion vacia de la lista
	while( ((pid=waitpid(-1,&valor_del_exit,WNOHANG)) > 0) && hijos_vivos )
	{	//espero por cualquier hijo sin bloquearme
		for(i=0;i<hijos_vivos;i++)
		{
            // recorro la lista para buscar el pid del que termino
            // cuando lo encuentro pongo en ese lugar el ultimo pid de la lista
            // y pongo un 0 en el ultimo lugar
			if(*(lista_hijos_id+i)==pid)
			{
				*(lista_hijos_id+i)=*(lista_hijos_id+hijos_vivos-1);
				*(lista_hijos_id+hijos_vivos-1)=0;
			}
		}
		hijos_vivos--;
		printf("Padre %d: Termino PID=%d y devolvio %d \n",getpid(),pid,WEXITSTATUS(valor_del_exit));
	}
}

/* =======================
 *
 * =======================*/
int codigo_pibes(int sfd,int mi_memoria,int mi_semaforo)
{
	char mensaje[TAMANO_BUFFER];
    const char patron[]={"Hola, soy PID"};
	int i,aux,totalenviados=0;
	struct sembuf mi_semop[2];


    struct dato_memoria DatoaEnviar,*pLeerTempsAca;

	// Atachamos la memria compartida
	pLeerTempsAca=(struct dato_memoria *)shmat(mi_memoria,NULL,0);
	if(pLeerTempsAca==((struct dato_memoria *)-1))
	{
		perror("Erro al Atachear la memoria\n");
        close(sfd);
		exit(LE_PIFIE_FEO);
	}

	// Para leer: (los semaforos se inicializan en el otro programa)
    //  1º) Esperar a que el sem0 este en 0
    //  2º) Tomar sem1 (sumarle 1) para indicar 1 lector más
    //  3º) Leer valor
    //  4º) Liberar sem1 (restarle 1) para indicar 1 lector menos
    (mi_semop+0)->sem_num=0;    // semaforo Nº0 dentro del set
	(mi_semop+0)->sem_op=0;     // esperar por 0
    (mi_semop+0)->sem_flg=0;
    (mi_semop+1)->sem_num=1;	// semaforo Nº1 dentro del set
    (mi_semop+1)->sem_op=1;	    // Sumarle 1 (tomar semaforo)
	(mi_semop+1)->sem_flg=0;

	printf("Hijo: mi pid %d el de papa: %d\n",getpid(),getppid());

	// el cliente me manda "sigo aca", si no recivo esto lo mato
	while((aux=recv(sfd, mensaje, sizeof(mensaje),0)) > 0)
	{
		printf("\nHijo %d Recibi: %s\n",getpid(),mensaje);

        aux=0;
        for(i=0;i<sizeof(patron)-1;i++)
        {
            aux=(*(mensaje+i)==*(patron+i))?0:1;
            //printf("\naux %d : mensaje %c patron %c\n",aux,*(mensaje+i),*(patron+i));
        }

        if(aux)
        {
            printf("Hijo %d: Error en mensaje cerrando conexion\n",getpid());
        	desatachar_memoria((const void *) pLeerTempsAca);
        	close(sfd);
        	exit(LE_PIFIE_FEO);
           //¿esto rompe con el while?, es para poner en vez de todo eso
            break;
        }

		// Ahora tengo que mandarle la temperatura que leyeron del sensor
		// Si el semaforo esta en 0 leo, si no me bloqueo hasta que este en 0
        // despues sumo 1 al otro semaforo
//        (mi_semop+0)->sem_op=0;     // esperar por 0
        (mi_semop+1)->sem_op=1;	    // Sumarle 1 (tomar semaforo)
		if( (semop(mi_semaforo,mi_semop,2)) == -1)	// una sola operacion
		{
			perror("Erro semop al tomar\n");
			desatachar_memoria((const void *) pLeerTempsAca);
			close(sfd);
			exit(LE_PIFIE_FEO);
		}
		// Ok ahora le mando la temperatura guardada

		printf("enviando: %d tamano %d \n",(pLeerTempsAca->temperatura),sizeof(struct dato_memoria));
		DatoaEnviar.temperatura=htonl(pLeerTempsAca->temperatura);
        DatoaEnviar.N_lectura=htonl(pLeerTempsAca->N_lectura);

		if (send(sfd, &DatoaEnviar,sizeof(struct dato_memoria), 0) == -1)
		{
			perror("Error en send\n");
			desatachar_memoria((const void *) pLeerTempsAca);
			close(sfd);
			exit(LE_PIFIE_FEO);
		}

        // libero el semaforo cntador
        (mi_semop+1)->sem_op=-1;    // restale 1 (liberar 1 lugar del semaforo)
		if( (semop(mi_semaforo,(mi_semop+1),1)) == -1)	// una sola operacion
		{
			perror("Erro semop al liberar\n");
			desatachar_memoria((const void *) pLeerTempsAca);
			close(sfd);
			exit(LE_PIFIE_FEO);
		}

		totalenviados++;	// veces que mande algo
		// despues de esto me quedo bloquead en el recv hasta que me escriban
		// de guelta... si se corta la comunicacion => recv devuelve 0
	}

    if(aux==-1)
    {//recv dio error (que no devuelva -1)
		perror("Erro en el recv\n");
		desatachar_memoria((const void *) pLeerTempsAca);
		close(sfd);
		exit(LE_PIFIE_FEO);
    }

	printf("Hijo %d: todo OK cerrando conexion, envie %d veces \n",getpid(),totalenviados);
	desatachar_memoria((const void *) pLeerTempsAca);
	close(sfd);
	exit(0);
}

/* =======================
 *
 * =======================*/

int main(int argc, char *argv[])
{
	int sfd,aux_fd;
	pid_t pid;
	struct sockaddr_in mi_dir,cliente;
	socklen_t addrlen;

	key_t mi_key;
	int mi_memoria,mi_semaforo;

	if(argc!=2)
	{
		printf("Uso: servidor puerto\n");
		return LE_PIFIE_FEO;
	}

    if(SIG_ERR == signal(SIGCHLD, manejadorSIGCHLD) )
	{
		printf("Error al implantar el handler de SIGCHLD\n");
		return(LE_PIFIE_FEO);
	}
    /*------------------------------------*/
    /*	PARA COMUNICARME CON EL LEERTEMPS */
    /*------------------------------------*/
	// creamo sla clave para generar IPCs
	mi_key=ftok("/home/debian/td3/leertemps",1);
	if(mi_key==-1)
	{
		perror("Erro al crear el key\n");
		exit(LE_PIFIE_FEO);
	}

	// Creamos la memoria compartida SOLO LECTURA de este lado
	mi_memoria=shmget(mi_key,sizeof(struct dato_memoria),IPC_CREAT|440);
	if(mi_memoria==-1)
	{
		perror("Erro al crear memria compartida\n");
		exit(LE_PIFIE_FEO);
	}

	// ahora creamos los semaforos (1 set con 2 semaforos) solo lectura
	if( (mi_semaforo=semget(mi_key,2,IPC_CREAT|660)) == -1)
	{
		perror("Erro al crear los semaforos\n");
        limpiar_tuti(mi_memoria,(const void *) NULL,0);
		exit(LE_PIFIE_FEO);
	}
	// semafors creados, ahora hay que inicializarlos
	// la inicializacion la hace el otro programa (leertemps.c)

    /*------------------------------------*/
    /*	PARA ARMAR LA CUMUNICACION TCPIP  */
    /*------------------------------------*/

	// creamos el socket donde voy a esperar conexines nuevas:
	sfd=socket(AF_INET, SOCK_STREAM,0);
	if(sfd<0)
	{
		perror("Error al crear el socket");
        limpiar_tuti(mi_memoria,(const void *) NULL,mi_semaforo);
		return LE_PIFIE_FEO;
	}

	// inicializamos a 0 la estructura
	memset(&mi_dir,0,sizeof(struct sockaddr_in));

	// Armamos la estructura con las direcciones de mi maquina
	mi_dir.sin_family = AF_INET;	// ipv4
	mi_dir.sin_port = htons( atoi( *(argv+1) ) );	// puerto donde escucho
	mi_dir.sin_addr.s_addr = htonl(INADDR_ANY);	// Dir IP de esta maquina

	//conectams el socket al puerto y a este programa
	if(bind(sfd, (struct sockaddr*)&mi_dir, sizeof(mi_dir)) < 0 )
	{
		perror("Error al hacer el bind\n");
        limpiar_tuti(mi_memoria,(const void *) NULL,mi_semaforo);
		close(sfd);
		return LE_PIFIE_FEO;
	}

	// hacemso el listen para encolar pedidos
	if (listen(sfd,LISTEN_SIMULTANEOS)<0)
	{
		perror("Error en el listen\n");
        limpiar_tuti(mi_memoria,(const void *) NULL,mi_semaforo);
		close(sfd);
		return LE_PIFIE_FEO;
	}

	/* de aca pa abajo tenemos que esperar a que venga un pedido de conexion
	 * cuando este llega hay que hacer un accept (que me hace el connect pero en un
	 * nuevo socket copia del riginal => aca hago el read/write)
	 *
	 * Ver como funciona eso del select y los fd_sets....
	*/
    lista_hijos_id=(int *) malloc( HIJOS_CONCURRENTES * sizeof(pid_t));

    hijos_vivos=0;
    hijos_Creados=0;
	while(1)
	{
		printf("Padre: Esperando conexiones\n");
		addrlen=sizeof(cliente);
		//addrlen=sizeof(struct sockaddr);
		aux_fd = accept (sfd, (struct sockaddr*) &cliente, &addrlen);
		if(aux_fd<0)
		{
			perror("Error en accept");
            limpiar_tuti(mi_memoria,(const void *) NULL,mi_semaforo);
			return LE_PIFIE_FEO;
		}
		printf("Padre: Recibi pedido de un cliente\n");
		// ahora en cliente tengo la direccion y el puerto del qu eme quiere hablar
		// tengo que crear un hijo que se encargue de esto, y cerrar el socket en el
		// padre y desùes seguir escuchando

		pid=fork();
		if(pid==0)
		{// soy el hijo, no vuelvo y hago exit en la funcion esa
			codigo_pibes(aux_fd,mi_memoria,mi_semaforo);
		}
		if(pid>0)
		{// soy el padre
			printf("Padre: Cree un hijo %d\n",pid);
			*(lista_hijos_id+hijos_vivos)=pid;// lo agrego a la lista
			hijos_vivos++;	// Hijos activos
			hijos_Creados++;// cantidad de hijos desde el inicio de los tiempos
            // cerramos el socket, que se encargue el pibe de esto
			close(aux_fd);
			//printf("Padre: Cerre el socket %d\n",pid);
		}
		else
		{// error en el fork
			printf("Error al crear un nuevo hijo, vivos: %d",hijos_vivos);
		}

        // si ya tengo la maxima cantidad de clientes vemos que hacer
		while(hijos_vivos >= HIJOS_CONCURRENTES)
		{
            printf("Padre: Maximas Concurrencias Alcanzadas, hay %d clientes activos\n",hijos_vivos);
            printf("Padre: Esperando a que se libere algun lugar...\n");
			// si uso pause nunca sale del pasue... se queda hay pa siempre
            // ops, no habia puesto el signal pa implantar la señal
            pause(); // espero a que venga una señal a despertarme
		}

	}//while(1) nunca salgo
    limpiar_tuti(mi_memoria,(const void *) NULL,mi_semaforo);
    close(sfd);
    return 0;
}

/*
 * Los semaforos y la memoria compartida las creo emn el padre y despues lso hijos
 * las heredan... esto esta OK, o es preferible crearlas en cada hijo¿?*/

/* SEMAFOROS y como esta sincronizada l amemoria compartida
 *  1) tengo uno que escribe y varios que leen
 *  - Si uso un solo semaforo :
 *      Lector espera a que el semaforo sea 0 y lee
 *      escritor, pone a 1 el semaforo cuando quiere escribir y a 0 cuando termina
 *  en este escenario, si hay alguien leyendo, el escritor puede tomar el
 *  semaforo y escribir sin dramas, por lo que solamente los nuevos lectores
 *  seran bloqueados, y el que estaba leyendo puede leer basura
 *  => tendria que buscar que el escritor espere a que todos los que esten leyendo
 *  terminen de leer antes de escribir algo nuevo....
 *  si cada lector suma 1 al semafor => mientras sigan entrando lectores, el escrito
 *  nunca va a escribir...=> pongo otro semaforo para cantidad de lectores.
 *
 *  => voy a tener 2 semaforos, uno para indicar que el escritor quiere modificar algo
 *  y los lectores no podran leer hasta que el escritor lo modifique
 *  y otro semaforo que me diga cuantos estan leyendo... asi el escritor puede avisar
 *  "che quiero modificar", despues esperar a que los que estan leyendo terminen
 *  y una vez que quede solo el escritor, modificar los datos y despues "listo, lean"
 *  m
 */
