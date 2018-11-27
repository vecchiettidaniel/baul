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
	int aux,error_encontrado=0;
	int mi_memoria,mi_semaforo;
	union semun mi_semun[2];

    struct dato_memoria temp_leida, *pGuardarTempsAca;

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
	// int semctl(int semid, int semnum, int cmd, ...);
	// despues con semop intento restar o sumar 1 al semaforo
	// para tomarlo -1 y para liberarlo +1 => lo arranco en 1;
	// semop -1 funca si .val=1 si es 0 no anda
	//(mi_semun+0)->val=1;	// semaforo0 a 1 (No LEER)
    (mi_semun+0)->val=1;	// Lo inicializo en 1 hasta que lea algo
    (mi_semun+1)->val=0;	// semaforo1 a 1 (Nº lectores=0)
	aux=semctl(mi_semaforo,0,SETALL,mi_semun);
	if(aux==-1)
	{
		perror("LEERTEMPS: Erro del semctl\n");
        limpiar_tuti(mi_memoria,mi_semaforo);
		exit(EXIT_FAILURE);
	}
    printf("LEERTEMPS: Semaforos inicializados\n");

    // com inicie el semaforo en 1 el resto esta esperand a que lo ponga en 0
	// Atachamos la memria compartida
	pGuardarTempsAca=(struct dato_memoria *)shmat(mi_memoria,NULL,0);
	if(pGuardarTempsAca== ((struct dato_memoria *)-1) )
	{
		perror("LEERTEMPS: Erro al Atachear la memoria\n");
        limpiar_tuti(mi_memoria,mi_semaforo);
		exit(EXIT_FAILURE);
	}
    printf("LEERTEMPS: Memoria atacheada\n");

    // hago un aprimera lectura pa inicializar la memoria
    // esto pa que el resto no mande 0 o cualquier cosa
	if( (fd = open ("/dev/mi_i2c", O_RDWR)) < 0 )
	{
	    perror("LEERTEMPS: Error en Open;\n");
        limpiar_tuti(mi_memoria,mi_semaforo);
        exit(EXIT_FAILURE);
	}

	if( read(fd,&temp_leida.temperatura,sizeof(int)) == 0 )
	{
		printf("LEERTEMPS: read devolvio error\n");
        limpiar_tuti(mi_memoria,mi_semaforo);
        close(fd);
        exit(EXIT_FAILURE);
	}

    // Ok, lei bien, tengo que guardar en la mem compartida
    temp_leida.N_lectura=0;    // id de lectura + 1
    if(escribir_primer_dato (mi_semaforo,&temp_leida,pGuardarTempsAca))
    {
        printf("LEERTEMPS: error al escribir el primer dato\n");
        limpiar_tuti(mi_memoria,mi_semaforo);
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("LEERTEMPS: Leyendo del sensor cada %d segundos\n",atoi(*(argv+1)));
    fin_programa=1;

	while(fin_programa)
	{
		// leo un valor de tempèratura del sensor
		if( read(fd,&temp_leida.temperatura,sizeof(int)) == 0 )
		{
			printf("LEERTEMPS: read devolvio error\n");
            fin_programa=0;
			error_encontrado=1;
		}
        else
        {// Ok, lei bien, tengo que guardar en la mem compartida
            temp_leida.N_lectura+=1;    // id de lectura + 1
            error_encontrado=escribir_dato (mi_semaforo,&temp_leida,pGuardarTempsAca);
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
