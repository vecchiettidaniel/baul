// Defines para activar los pines
// SCL=19
// SDA=20

#include "../inc/includes.h"
#include "../inc/ipcs.h"
#include <sys/types.h>
#include <stdlib.h>         //exit
#include <signal.h>
#include <fcntl.h>          //open
#include <unistd.h>         //getpid

void manejadorSIGUSER1(int sig);

// ver que onda esta variable que esta con el mismo nombre en elpadre
int fin_programa;

void manejadorSIGUSER1(int sig)
{
    fin_programa=0;
}
/* =======================
 *
 * =======================*/
int main (int argc, char **argv)
{
	int fd;
	int temperatura,aux,error_encontrado=0;
	int mi_memoria,mi_semaforo;
	union semun mi_semun[2];
	struct sembuf mi_semop[2];

    struct dato_memoria *pGuardarTempsAca;

    if (argc != 2)
	{
		printf("LEERTEMPS uso: leertem TiempoEntreLecturas\n");
		exit(EXIT_FAILURE);
    }

    if(SIG_ERR == signal(SIGUSR1, manejadorSIGUSER1) )
	{
		printf("LEERTEMPS: Error al implantar el handler de SIGCHLD errno=%d\n",errno);
		exit(EXIT_FAILURE);
	}

/*
    if(SIG_ERR == signal(SIGINT, SIG_IGN) )
    {
        printf("LEERTEMPS: Error al implantar el handler de SIGCHLD errno=%d\n",errno);
        return(LE_PIFIE_FEO);
    }
*/

    /*--------------------------------------*/
    /*	PARA COMUNICARME CON LOS HIJO-server*/
    /*--------------------------------------*/
    if(crear_ipcs_a_usar(&mi_memoria,&mi_semaforo))
    {
        perror("LEERTEMPS: Erro al crear ipcs\n");
		exit(EXIT_FAILURE);
    }
	// semafors creados, ahora hay que inicializarlos
	// semafors creados, ahora hay que inicializarlos
	// int semctl(int semid, int semnum, int cmd, ...);
	// despues con semop intento restar o sumar 1 al semaforo
	// para tomarlo -1 y para liberarlo +1 => lo arranco en 1;
	// semop -1 funca si .val=1 si es 0 no anda
	//(mi_semun+0)->val=1;	// semaforo0 a 1 (No LEER)
    (mi_semun+0)->val=1;	// Lo inicializo en 1 hasta que lea algo
    (mi_semun+0)->val=0;	// semaforo1 a 1 (Nº lectores=0)
	aux=semctl(mi_semaforo,0,SETALL,mi_semun);
	if(aux==-1)
	{
		perror("LEERTEMPS: Erro del semctl\n");
        limpiar_tuti(mi_memoria,mi_semaforo);
		exit(EXIT_FAILURE);
	}
    printf("LEERTEMPS: Semaforos inicializados\n");

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


	if( (fd = open ("/dev/mi_i2c", O_RDWR)) < 0 )
	{
	    perror("LEERTEMPS: Error en Open;\n");
        limpiar_tuti(mi_memoria,mi_semaforo);
        exit(EXIT_FAILURE);
	}


	// Atachamos la memria compartida
	pGuardarTempsAca=(struct dato_memoria *)shmat(mi_memoria,NULL,0);
	if(pGuardarTempsAca== ((struct dato_memoria *)-1) )
	{
		perror("LEERTEMPS: Erro al Atachear la memoria\n");
        limpiar_tuti(mi_memoria,mi_semaforo);
        close(fd);
		exit(EXIT_FAILURE);
	}
    printf("LEERTEMPS: Memoria atacheada\n");


    // inicializamos el contador de Lectura en 0
    pGuardarTempsAca->N_lectura=0;
    printf("LEERTEMPS: Leyendo del sensor cada %d segundos\n",atoi(*(argv+1)));
    fin_programa=1;
	while(fin_programa)
	{
		// leo un valor de tempèratura del sensor
		if( read(fd,&temperatura,sizeof(int)) == 0 )
		{
			printf("LEERTEMPS: read devolvio error\n");
            fin_programa=0;
			error_encontrado=1;
		}
        else
        {//read
    //		printf("Temperatura leida %d,%d ºC\n",temperatura/10,temperatura%10);
    		// Escritura indico NO LEAN
    		(mi_semop+0)->sem_op=1;     // Sumarle 1 (tomar semaforo)
    //        (mi_semop+1)->sem_op=0;     // esperar por 0 (que todos terminende leer)
    		aux=semop(mi_semaforo,mi_semop,2); // dos operaciones
    		if(aux==-1)
    		{
    			perror("LEERTEMPS: Erro del semop +1\n");
                error_encontrado=1;
                fin_programa=0;
    		}
            else
            {
        		// OK, tengo el semaforo ahora a escribir en la memoria
        		pGuardarTempsAca->temperatura=temperatura;
        		(pGuardarTempsAca->N_lectura)+=1;
                // Ya escribi lo que queria ahora a liberar semaforo
        		(mi_semop+0)->sem_op=-1;     // restarle 1 (liberar semaforo)
        		aux=semop(mi_semaforo,mi_semop,1); // usa sola operacion
        		if(aux==-1)
        		{
        			perror("LEERTEMPS: Erro del semop -1\n");
                    error_encontrado=1;
                    fin_programa=0;
        		}
            }
        }//else{//read(sfd)
        if(!error_encontrado)
        {
            sleep(atoi(*(argv+1)));
        }
	}

    if(error_encontrado)
    {// ocurrio algun error en algun punto del programa
        printf("LEERTEMPS: Error detectado\n");
        exit(EXIT_FAILURE);
    }
    printf("LEERTEMPS: finalizando\n");

	desatachar_memoria((const void *) pGuardarTempsAca);
	close(fd);
	exit(EXIT_SUCCESS);
}
