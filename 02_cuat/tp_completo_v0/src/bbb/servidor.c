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

#define CONEXIONES_MAXIMAS 10 //Nro maximo de conexiones en espera
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

int hijos_vivos;
int hijos_Creados;
pid_t *lista_hijos_id;

/* =======================
 *
 * =======================*/

int limpiar_tuti(int memoria,const void *p,int semaforo)
{
	int i=0;
	// borramos/eliminamos el semaforo
	if(semaforo!=0)
	{
		if( ( semctl(semaforo,0,IPC_RMID) ) ==-1)
		{
			perror("error al borra el semaforo\n");
			i=LE_PIFIE_FEO;
		}
	}

	if(p!=NULL)
	{
		if( (shmdt(p)) ==-1)
		{
			perror("error al desatachar la memoria\n");
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
	//ahora a esperar por los hijos, despues mver al handler de señal
	while( ((pid=waitpid(-1,&valor_del_exit,WNOHANG)) > 0) && hijos_vivos )
	{	//espero por cualquier hijo sin bloquearme
		for(i=0;i<hijos_vivos;i++)
		{
			if(*(lista_hijos_id+i)==pid)
			{
				*(lista_hijos_id+i)=*(lista_hijos_id+hijos_vivos-1);
				*(lista_hijos_id+hijos_vivos-1)=0;
			}
		}
		hijos_vivos--;
		printf("Padre: Termino PID=%d y devolvio %d \n",pid,WEXITSTATUS(valor_del_exit));
	}
}

/* =======================
 *
 * =======================*/
int codigo_pibes(int sfd,int mi_memoria,int mi_semaforo)
{
	char mensaje[TAMANO_BUFFER];
	int i,*pLeerTempsAca,temperatura;
	struct sembuf mi_semop[2];

	// Atachamos la memria compartida
	pLeerTempsAca=(int *)shmat(mi_memoria,NULL,0);
	if(pLeerTempsAca==((int*)-1))
	{
		perror("Erro al Atachear la memoria\n");
		limpiar_tuti(mi_memoria,(const void *) NULL,0);
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
	while(recv(sfd, mensaje, sizeof(mensaje),0) > 0)
	{
		printf("\nHijo %d Recibi: %s\n",getpid(),mensaje);

		// Ahora tengo que mandarle la temperatura que leyeron del sensor
		// Si el semaforo esta en 0 leo, si no me bloqueo hasta que este en 0
        // despues sumo 1 al otro semaforo
//        (mi_semop+0)->sem_op=0;     // esperar por 0
        (mi_semop+1)->sem_op=1;	    // Sumarle 1 (tomar semaforo)
		if( (semop(mi_semaforo,mi_semop,2)) == -1)	// una sola operacion
		{
			perror("Erro semop al tomar\n");
			limpiar_tuti(mi_memoria,(const void *) pLeerTempsAca,mi_semaforo);
			close(sfd);
			return LE_PIFIE_FEO;
		}
		// Ok ahora le mando la temperatura guardada
/*
		mensaje[0]=(char) ( ((*pLeerTempsAca)>>0) & 0xff );
		mensaje[1]=(char) ( ((*pLeerTempsAca)>>8) & 0xff );
		mensaje[2]=(char) ( ((*pLeerTempsAca)>>16) & 0xff );
		mensaje[3]=(char) ( ((*pLeerTempsAca)>>24) & 0xff );
		mensaje[4]=(char) '\0';
		printf("enviando: %s tamano %d \n",mensaje,sizeof(int));
*/
		printf("enviando: %d tamano %d \n",*pLeerTempsAca,sizeof(int));
		temperatura=htonl(*pLeerTempsAca);
		if (send(sfd, &temperatura,sizeof(int), 0) == -1)
		{
			perror("Error en send\n");
			close(sfd);
		    exit(1);
		}

        // libero el semaforo cntador
        (mi_semop+1)->sem_op=-1;    // restale 1 (liberar 1 lugar del semaforo)
		if( (semop(mi_semaforo,(mi_semop+1),1)) == -1)	// una sola operacion
		{
			perror("Erro semop al liberar\n");
			limpiar_tuti(mi_memoria,(const void *) pLeerTempsAca,mi_semaforo);
			close(sfd);
			return LE_PIFIE_FEO;
		}

		i++;	// veces que mande algo
		// despues de esto me quedo bloquead en el recv hasta que me escriban
		// de guelta... si se corta la comunicacion => recv devuelve 0
	}

	//falta verificar que recv no me de error (que no devuelva -1)
	printf("Hijo %d: todo OK cerrando conexion, envie %d veces \n",getpid(),i);
	limpiar_tuti(mi_memoria,(const void *) pLeerTempsAca,mi_semaforo);
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

	int mi_memoria,mi_semaforo;
	key_t mi_key;


	if(argc!=2)
	{
		printf("Uso: servidor puerto\n");
		return LE_PIFIE_FEO;
	}

/*------------------------------------*/
/*	PARA COMUNICARME CON EL LEERTEMPS */
/*------------------------------------*/
	// creamo sla clave para generar IPCs
	mi_key=ftok("/home/debian/td3/leertemps",1);
	if(mi_key==-1)
	{
		perror("Erro al crear el key\n");
		return LE_PIFIE_FEO;
	}

	// nos conectamos a la memoria compartida SOLO LECTURA de este lado
	mi_memoria=shmget(mi_key,TAMANO_BUFFER*sizeof(int),IPC_CREAT|440);
	if(mi_memoria==-1)
	{
		perror("Erro al crear memria compartida\n");
		return LE_PIFIE_FEO;
	}

	// ahora creamos los semaforos (1 set con 1 slo semaforo) solo lectura
	if( (mi_semaforo=semget(mi_key,1,IPC_CREAT|440)) == -1)
	{
		perror("Erro al crear los semaforos\n");
		limpiar_tuti(mi_memoria,NULL,0);
		return LE_PIFIE_FEO;
	}
	// semafors creados, ahora hay que inicializarlos
	// la inicializacion la hace el otro programa (leertemps.c)


	// ya ta ahora a armar la cumunicacion
/*------------------------------------*/
/*	PARA ARMAR LA CUMUNICACION TCPIP  */
/*------------------------------------*/

	lista_hijos_id=(int *) malloc( CONEXIONES_MAXIMAS * sizeof(pid_t));
	// creamos el socket donde voy a esperar conexines nuevas:
	sfd=socket(AF_INET, SOCK_STREAM,0);
	if(sfd<0)
	{
		perror("Error al crear el socket");
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
		close(sfd);
		return LE_PIFIE_FEO;
	}

	// hacemso el listen para encolar pedidos
	if (listen(sfd,CONEXIONES_MAXIMAS)<0)
	{
		perror("Error en el listen\n");
		close(sfd);
		return LE_PIFIE_FEO;
	}

	/* de aca pa abajo tenemos que esperar a que venga un pedido de conexion
	 * cuando este llega hay que hacer un accept (que me hace el connect pero en un
	 * nuevo socket copia del riginal => aca hago el read/write)
	 *
	 * Ver como funciona eso del select y los fd_sets....
	*/
	while(1)
	{
		if(hijos_vivos >= CONEXIONES_MAXIMAS)
		{
			//alcance el numero de conexiones... ver de que me disfrazo
		}

		printf("Padre: Esperando conexiones\n");
		addrlen=sizeof(cliente);
		//addrlen=sizeof(struct sockaddr);
		aux_fd = accept (sfd, (struct sockaddr*) &cliente, &addrlen);
		if(aux_fd<0)
		{
			perror("Error en accept");
			return LE_PIFIE_FEO;
		}
		printf("Padre: Recibi pedido de un cliente\n");
		// ahora en cliente tengo la direccion y el puerto del qu eme quiere hablar
		// tengo que crear un hijo que se encargue de esto, y cerrar el socket en el
		// padre y desùes seguir escuchando

		pid=fork();
		if(pid==0)
		{// soy el hijo
			codigo_pibes(aux_fd,mi_memoria,mi_semaforo);
		}
		if(pid>0)
		{// soy el padre
			printf("Padre: Cree un hijo %d\n",pid);
			hijos_vivos++;	// Hijos activos
			hijos_Creados++;// cantidad de hijos desde el inicio de los tiempos
			*(lista_hijos_id+hijos_vivos)=pid;// lo agrego a la lista
			close(aux_fd);
			printf("Padre: Cerre el socket %d\n",pid);
		}
		else
		{// error en el fork
			printf("Error al crear un nuevo hijo, vivos: %d",hijos_vivos);
		}
	}
}
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
