#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#define LE_PIFIE_FEO 1

int main (int argc, char **argv)
{
	pid_t pid;
	int i,hijos=0;
    char *argv_exec[]={"./pc/bin/cliente","192.168.6.2","4545","1","2",NULL};

	if ((argc != 2) && (argc != 4) )
	{
		printf("USO: %d\n",argc);
		printf("	muchos-cliente cantidad_a_crear \n");
		printf("	muchos-cliente cantidad Ndepedidos tiempo_entre_Pedidos \n");
		return LE_PIFIE_FEO;
	}

	if(argc==4)
	{
		*(argv_exec+3)=*(argv+2);
		*(argv_exec+4)=*(argv+3);
	}

	for(i=0;i<atoi(*(argv+1));i++)
	{
		pid=fork();
		if(pid==0)
		{// soy el hijo,
			execv("./pc/bin/cliente",argv_exec);
	        perror("MUCHOS: ERROR al llamar cliente\n");
	        i=atoi(*(argv+1));
		}

		if(pid>0)
		{// soy el padre
			printf("MUCHOS: cree un hijo con pid %d\n",pid);
			hijos++;
		}
		else
		{// soy el padre error en el fork
			printf("MUCHOS: Error al crear un hijo para leertemps: hijs_vivos= %d\n",pid);
	        perror("MUCHOS: Error en fork");
	        i=atoi(*(argv+1));
	    }
	}
	printf("MUCHOS: saliendo, tengo %d hijos vivos\n",hijos);
	while( hijos )
	{
		while( ( (pid=waitpid(-1,NULL,WNOHANG)) > 0) )
		{
			if(pid>0)
			{
				printf("MUCHOS: termino %d\n",pid);
				hijos--;
			}
		}
		//sleep(1);
	}

	printf("MUCHOS: Chau, tengo %d hijos vivos\n",hijos);

    return 0;
}
