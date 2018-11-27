#ifndef INC_ipcs_H_
#define INC_ipcs_H_

#include "includes.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>	// perror
#include <errno.h>


#define PERMISOS 00660	// NO ANDA con 660... lei man 2 open ....

int crear_ipcs_a_usar(int *mi_memoria,int *mi_semaforo);
int desatachar_memoria (const void *p);
int borrar_semaforos(int semaforo);
int borrar_memoria(int memoria);
int limpiar_tuti(int memoria,int semaforo);

int escribir_dato (int semaforos, struct dato_memoria *origen,struct dato_memoria *destino);
int leer_dato (int semaforos, struct dato_memoria *origen,struct dato_memoria *destino);

/* =======================
 *	Creamos las que vamso ausar
 * =======================*/
int crear_ipcs_a_usar(int *mi_memoria,int *mi_semaforo)
{
	key_t mi_key;

	// creamo sla clave para generar IPCs
	mi_key=ftok("/home/debian/td3/leertemps",1);
	if(mi_key==-1)
	{
		perror("Erro al crear el key\n");
		return(LE_PIFIE_FEO);
	}

	// Creamos la memoria compartida SOLO LECTURA de este lado
	*mi_memoria=shmget(mi_key,sizeof(struct dato_memoria),IPC_CREAT|IPC_EXCL|PERMISOS);
	if(*mi_memoria==-1)
	{
		if(errno!=EEXIST)
		{
			perror("Erro al crear memria compartida\n");
			return(LE_PIFIE_FEO);
		}
		else
		{
			*mi_memoria=shmget(mi_key,sizeof(struct dato_memoria),PERMISOS);
		}
	}

	// ahora creamos los semaforos (1 set con 2 semaforos) solo lectura
	if( (*mi_semaforo=semget(mi_key,2,IPC_CREAT|PERMISOS)) == -1)
	{
		if(errno!=EEXIST)
		{
			perror("Erro al crear los semaforos\n");
		    borrar_memoria(*mi_memoria);
			return(LE_PIFIE_FEO);
		}
		else
		{
			*mi_semaforo=semget(mi_key,2,PERMISOS);
		}
	}
	return 0;
}

/* =======================
 *	Para borrar algunas cosas
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

int borrar_semaforos(int semaforo)
{
	if(semaforo!=0)
	{
		// borramos/eliminamos el semaforo
		if( ( semctl(semaforo,0,IPC_RMID) ) ==-1)
		{
			perror("error al borra el semaforo\n");
			return LE_PIFIE_FEO;
		}
	}
	return 0;

}

int borrar_memoria(int memoria)
{
	if(memoria!=0)
	{
		if( (shmctl(memoria,IPC_RMID,0)) ==-1)
		{
			perror("error al eliminar la memoria\n");
			return LE_PIFIE_FEO;
		}
	}
		return 0;
}

int limpiar_tuti(int memoria,int semaforo)
{
	int i=0;
	i=borrar_memoria(memoria);
	i=borrar_semaforos(semaforo);
	return i;
}

/* =======================
 *	Para Usar los IPCS
 * =======================*/

/* =================================
 *  leer_dato
 * ----------------------
 * Para leer:
 *  1º) Esperar a que el sem0 este en 0
 *  2º) Tomar sem1 (sumarle 1) para indicar 1 lector más
 *  3º) Leer valor
 *  4º) Liberar sem1 (restarle 1) para indicar 1 lector menos
*/
int leer_dato (int semaforos, struct dato_memoria *origen,struct dato_memoria *destino)
{
	struct sembuf mi_semop[2];

    (mi_semop+0)->sem_num=0;    // semaforo Nº0 dentro del set
	(mi_semop+0)->sem_op=0;     // esperar por 0
    (mi_semop+0)->sem_flg=0;
    (mi_semop+1)->sem_num=1;	// semaforo Nº1 dentro del set
    (mi_semop+1)->sem_op=1;	    // Sumarle 1 (tomar semaforo)
	(mi_semop+1)->sem_flg=0;

	// Intentamos las dos operaciones para poder leer (espero por 0 y sumo 1)
	if( (semop(semaforos,mi_semop,2)) == -1)	// dos operaciones
	{
		perror("Erroe en la lectura al tomar el semaforo\n");
        return(LE_PIFIE_FEO);
	}

	// listo, leemos o copiamos desde la memoria hacia donde me digeron
	*destino=*origen;

	// ok ahora resto uno del metaforo para indicar fin de lectura
	(mi_semop+1)->sem_op=-1;	    // resto 1 (libero semaforo)
	if( (semop(semaforos,mi_semop+1,1)) == -1)	// dos operaciones
	{
		perror("Erroe en la lectura al liberar el semaforo\n");
        return(LE_PIFIE_FEO);
	}

	return 0;
}

/* =================================
 * escribir_dato
 * ----------------------
 * par aescribir;
 * 1º) Tomar sem0 (pongo a 1) indica que no se haceptan mas lectores
 * 2º) espero a que sem1 sea 0 (espero a que todos terminen de leer)
 * 3º) cambio lo que haiga que cambiar o actualizar
 * 4º) Libero sem0 (pongo 0) indica que ya pueden leer
*/
int escribir_dato (int semaforos, struct dato_memoria *origen,struct dato_memoria *destino)
{
	struct sembuf mi_semop[2];

    (mi_semop+0)->sem_num=0;    // semaforo Nº0 dentro del set
	(mi_semop+0)->sem_op=1;     // Sumarle 1 (tomar semaforo)
    (mi_semop+0)->sem_flg=0;
    (mi_semop+1)->sem_num=1;	// semaforo Nº1 dentro del set
    (mi_semop+1)->sem_op=0;	    // esperar por 0
	(mi_semop+1)->sem_flg=0;

	// Intentamos las dos operaciones para poder leer (espero por 0 y sumo 1)
	if( (semop(semaforos,mi_semop,2)) == -1)	// dos operaciones
	{
		perror("Error en escritura, al tomar los semaforo \n");
        return(LE_PIFIE_FEO);
	}

	// listo, escribimos o copiamos en la memoria
	*destino=*origen;

    // Ya escribi lo que queria ahora a liberar semaforo
	(mi_semop+0)->sem_op=-1;     // restarle 1 (liberar semaforo)
	if(semop(semaforos,mi_semop,1)) // usa sola operacion
	{
		perror("Error en escritura, al liberar los semaforo \n");
        return(LE_PIFIE_FEO);
	}

	return 0;
}


#endif //#ifndef INC_includes_H_
