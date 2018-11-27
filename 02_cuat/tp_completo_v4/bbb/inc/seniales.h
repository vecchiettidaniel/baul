#ifndef INC_SENIALES_H_
#define INC_SENIALES_H_

#include "globales.h"
#include <sys/types.h>
#include <stdlib.h> //exit
#include <stdio.h>	// perror
#include <unistd.h>//getpid
#include <sys/wait.h> //waitpid


void manejadorSIGCHLD(int sig);
void manejadorSIGINT(int sig);
/* =======================
 *
 * =======================*/
void manejadorSIGCHLD(int sig)
{
	int i,valor_del_exit;
	pid_t pid;
    //printf("Padre : estoy en sigchild\n");
	//lista hijos tiene todos los pid de los hijos que estan vivos
    //hijos_vivos es la cantidad de hijos que estan vivos
    // lista+vivos apunta a la siguiente posicion vacia de la lista
	while( ((pid=waitpid(-1,&valor_del_exit,WNOHANG)) > 0))
	{

		if(pid>0)
		{
			if(pid==(pSCfg->pid_temp))
			{	// termino leertemps
				(pSCfg->pid_temp)=0;
				if(WIFEXITED(valor_del_exit))
				{	// salio por exit
					if( WEXITSTATUS(valor_del_exit) == EXIT_FAILURE)
					{// error grave en leertemps
						//printf("Papa: Se rompio Leertemps\n");
						fin_programa=0;
					}
				}
			}
			else
			{	// termino algunos de los hijos-que atienden a clientes
		        // recorro la lista para buscar el pid del que termino
		        // cuando lo encuentro pongo en ese lugar el ultimo pid de la lista
		        // y pongo un 0 en el ultimo lugar
				for(i=0;i<(pSCfg->hijos_vivos);i++)
				{
					if(*((pSCfg->lista_hijos)+i)==pid)
					{
						*((pSCfg->lista_hijos)+i)=*((pSCfg->lista_hijos)+(pSCfg->hijos_vivos)-1);
						*((pSCfg->lista_hijos)+(pSCfg->hijos_vivos)-1)=0;
					}
				}
				(pSCfg->hijos_vivos)--;
				//printf("Padre %d: Termino PID=%d y devolvio %d \n",getpid(),pid,WEXITSTATUS(valor_del_exit));
		        //printf("Padre %d: quedan %d hijos vivos\n",getpid(),(pSCfg->hijos_vivos));
			}//else if(pid==(pSCfg->pid_temp))
		}//if(pid>0)
		else
		{// pid -1
			perror("Padre: error en el waitpid\n");
		}
	}
}

/* =======================
 *
 * =======================*/
void manejadorSIGINT(int sig)
{
	int i;
	//wpid_t pid;
	//lista hijos tiene todos los pid de los hijos que estan vivos
    //(pSCfg->hijos_vivos) es la cantidad de hijos que estan vivos
    // lista+vivos apunta a la siguiente posicion vacia de la lista
    if((pSCfg->pid_jefe)==getpid())
    {
        //printf("PAPA: Recibi SIGTERM cerrando procesos...\n");
        // soy el padre y me hicieron SIGTERM
        // ahora tendria que matar a todos los hijos que me quedan vivos pa
        // poder salir sin dejar huellas
    	for(i=0;i<(pSCfg->hijos_vivos);i++)
    	{
            //Mando SIGTERM a todos los hijos que quedan vivos
            //printf("PAPA: SIGINT a %d\n",*((pSCfg->lista_hijos)+i));
            if((kill(*((pSCfg->lista_hijos)+i) , SIGINT))==-1)
            {
                printf("Error al enviar SIGINT a %d\n",*((pSCfg->lista_hijos)+i));
                perror("\n");
            }
    	}
        // ya ta ya me quede sin herederos, ahora a decir sali del loop principal
    }//if((pSCfg->pid_jefe)==getpid())
    //mando fin de loop al programa principal para que haga el exit
	fin_programa=0;
    //printf("AMBOS: chau SIGINT %d...\n",getpid());
}

#endif //#ifndef INC_SENIALES_H_
