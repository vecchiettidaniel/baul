#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void manejadorSIGCHLD(int sig);
int codigo_pibes(int i);

int hijos_vivos;
int hijos_Creados;
pid_t *lista_hijos_id;

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
int codigo_pibes(int i)
{
	printf("Hijo %d: mi pid %d el de papa: %d\n",i,getpid(),getppid());
	//sleep(1+2*i);
	sleep(1);
	printf("Hijo %d: chau!!!! mi pid es %d\n",i,getpid());
	exit(i);
}

/* =======================
 *
 * =======================*/
int main (int argc, char **argv)
{
	pid_t pid=0;
	int i;

    if (argc != 2) {
		printf("Uso: ./fork Nº_Hijos a crear \n");
		exit(1);
    }

	if(SIG_ERR == signal(SIGCHLD, manejadorSIGCHLD) )
	{
		printf("Error al implantar el handler de SIGCHLD\n");
		exit(1);
	}

	lista_hijos_id=(int *) malloc( (atoi(*(argv+1)))*sizeof(pid_t));

	printf("Padre antes de fork(): pid %d ppid %d\n", getpid(), getppid());

	for (i=0;i<atoi(*(argv+1));i++)
	{
		pid = fork();//aca intento crear un nuevo hijo
		if(pid) {//Parent
			printf("Padre: Cree un hijo %d\n",pid);
			*(lista_hijos_id+i)=pid;// lo agrego a la lista
			hijos_vivos++;
			hijos_Creados++;
			}
		else {//1-else
			if (pid==0)
			{//hijo
				codigo_pibes(i);
			}//if (pid>0)
			else {
				//error
				printf("PADRE: Error del fork(%d) errno(%d) %s",i,errno,strerror(errno));
				}
			}
	}//for

	if(pid)
	{// soy PAPA
		printf("Padre: Esperando a los pibes\n");
		while(hijos_vivos)
		{
			pause();
			printf("Padre: quedan %d hijos vivos \n",hijos_vivos);
		}
		free(lista_hijos_id);
		printf("Padre: termine\n");
	}

	return 0;
}
/*
 * pid_t waitpid(pid_t pid, int *wstatus, int options);
 * PID:
 *		pid = 0; espera por cualquier hijo que tenga el mismo groupID
 * 		pid > 0; espera por hijo en particular (le paso a cual quiero esperar)
 *		pid = -1; espera por cualquier hijo
 *
 * OPTIONS:
 * 		WNOHANG, no se bloquea esperando hasta que algun hijo haga exit
 *
 * DEVUELVE:
 *		>0 : pid del proceso que tuvo un cambio de estado
 *		=0 : (si WNOHANG) si todavia quedan hijos vivos que no terminaron
 *		-1 : si rompimo algo o no hay mas hijos que espetrar
 */
