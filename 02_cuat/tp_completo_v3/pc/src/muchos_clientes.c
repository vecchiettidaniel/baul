#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#define LE_PIFIE_FEO 1

int main (int argc, char **argv)
{
	pid_t pid;
	int i,hijos=0;
    char *argv_exec[]={"./pc/bin/cliente","192.168.6.2","4545","1","1",NULL};

	if (argc != 2)
	{
		printf("USO: \n");
		printf("	muchos-cliente Cantidad a crear\n");
		return LE_PIFIE_FEO;
	}

	for(i=0;i<atoi(*(argv+1));i++)
	{
		pid=fork();
		if(pid==0)
		{// soy el hijo,
			execv("./pc/bin/cliente",argv_exec);
	        perror("Padre: ERROR al llamar cliente\n");
	        i=atoi(*(argv+1));
		}

		if(pid>0)
		{// soy el padre
			printf("Padre: cree un hijo con pid %d\n",pid);
			hijos++;
		}
		else
		{// soy el padre error en el fork
			printf("Error al crear un hijo para leertemps: hijs_vivos= %d\n",pid);
	        perror("Error en fork");
	        i=atoi(*(argv+1));
	    }
	}
	printf("Padre: fin, cree %d\n",hijos);
	while( hijos )
	{
		while( ( (pid=waitpid(-1,NULL,WNOHANG)) > 0) )
		{
			if(pid>0)
			{
				printf("Padre: termino %d\n",pid);
				hijos--;
			}
		}
		//sleep(1);
	}

	printf("Padre: Chau, tengo %d\n",hijos);

    return 0;
}
