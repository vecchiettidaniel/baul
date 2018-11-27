// Defines para activar los pines
// SCL=19
// SDA=20

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
//#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define	LE_PIFIE_FEO 1
#define	TAMANO_BUFFER 200

/* =======================
 *
 * =======================*/
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
int main (int argc, char **argv)
{
	int fd,*pGuardarTempsAca;
	int temperatura,aux;
	int mi_memoria,mi_semaforo;
	union semun mi_semun[2];
	struct sembuf mi_semop[2];
	key_t mi_key;

    if (argc != 2)
	{
		printf("uso: leertem TiempoEntreLecturas\n");
		return(1);
    }

	if( (fd = open ("/dev/mi_i2c", O_RDWR)) < 0 )
	{
	    printf("Error en Open; devolvio : %d\n",fd);
	}

	// creamo sla clave para generar IPCs
	mi_key=ftok("/home/debian/td3/leertemps",1);
	if(mi_key==-1)
	{
		perror("Erro al crear el key\n");
		return LE_PIFIE_FEO;
	}

	// Creamos la memoria compartida
	mi_memoria=shmget(mi_key,TAMANO_BUFFER*sizeof(int),IPC_CREAT|660);
	if(mi_memoria==-1)
	{
		perror("Erro al crear memria compartida\n");
		return LE_PIFIE_FEO;
	}
	// Atachamos la memria compartida
	pGuardarTempsAca=(int *)shmat(mi_memoria,NULL,0);
	if(pGuardarTempsAca==((int*)-1))
	{
		perror("Erro al Atachear la memoria\n");
		limpiar_tuti(mi_memoria,(const void *) NULL,0);
		return LE_PIFIE_FEO;
	}

	// ahora creamos los semaforos (1 set con 2s semaforos)
	if( (mi_semaforo=semget(mi_key,2,IPC_CREAT|660)) == -1)
	{
		perror("Erro al crear los semaforos\n");
		limpiar_tuti(mi_memoria,(const void *) pGuardarTempsAca,0);
		return LE_PIFIE_FEO;
	}
	// semafors creados, ahora hay que inicializarlos
	// int semctl(int semid, int semnum, int cmd, ...);
	// despues con semop intento restar o sumar 1 al semaforo
	// para tomarlo -1 y para liberarlo +1 => lo arranco en 1;
	// semop -1 funca si .val=1 si es 0 no anda
	//(mi_semun+0)->val=1;	// semaforo0 a 1 (No LEER)
    (mi_semun+0)->val=0;	// Lo pongo en 0 porque lo primero que hago es +1
    (mi_semun+0)->val=0;	// semaforo1 a 1 (Nº lectores=0)
	aux=semctl(mi_semaforo,0,SETALL,mi_semun);
	if(aux==-1)
	{
		perror("Erro del semctl\n");
		limpiar_tuti(mi_memoria,(const void *) pGuardarTempsAca,mi_semaforo);
		return LE_PIFIE_FEO;
	}

	// par aescribir;
    // 1º) Tomar sem0 (pongo a 1) indica que no se haceptan mas lectores
    // 2º) espero a que sem1 sea 0 (espero a que todos terminen de leer)
    // 3º) cambio lo que haiga que cambiar o actualizar
    // 4º) Libero sem0 (pongo 0) indica que ya pueden leer

    (mi_semop+0)->sem_num=0;    // semaforo Nº0 dentro del set
	(mi_semop+0)->sem_op=1;     // Sumarle 1 (tomar semaforo)
    (mi_semop+0)->sem_flg=0;
    (mi_semop+1)->sem_num=1;	// semaforo Nº1 dentro del set
    (mi_semop+1)->sem_op=0;	    // esperar por 0
	(mi_semop+1)->sem_flg=0;


	while(1)
	{
		// leo un valor de tempèratura del sensor
		if( read(fd,&temperatura,sizeof(int)) == 0 )
		{
			printf(" read devolvio error\n");
			limpiar_tuti(mi_memoria,(const void *) pGuardarTempsAca,mi_semaforo);
			close(fd);
			return 1;
		}
//		printf("Temperatura leida %d,%d ºC\n",temperatura/10,temperatura%10);
		// Escritura indico NO LEAN
		(mi_semop+0)->sem_op=1;     // Sumarle 1 (tomar semaforo)
//        (mi_semop+1)->sem_op=0;     // esperar por 0 (que todos terminende leer)
		aux=semop(mi_semaforo,mi_semop,2); // dos operaciones
		if(aux==-1)
		{
			perror("Erro del semop +1\n");
			limpiar_tuti(mi_memoria,(const void *) pGuardarTempsAca,mi_semaforo);
			close(fd);
			return LE_PIFIE_FEO;
		}

		// OK, tengo el semaforo ahora a escribir en la memoria
		*pGuardarTempsAca=temperatura;
		// Ya escribi lo que queria ahora a liberar semaforo
		(mi_semop+0)->sem_op=-1;     // restarle 1 (liberar semaforo)
		aux=semop(mi_semaforo,mi_semop,1); // usa sola operacion
		if(aux==-1)
		{
			perror("Erro del semop -1\n");
			limpiar_tuti(mi_memoria,(const void *) pGuardarTempsAca,mi_semaforo);
			close(fd);
			return LE_PIFIE_FEO;
		}

		sleep(atoi(*(argv+1)));
	}

	close(fd);
	return 0;
}
