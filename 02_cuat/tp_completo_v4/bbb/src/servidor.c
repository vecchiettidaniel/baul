#include "../inc/includes.h"
#include "../inc/globales.h"
#include "../inc/seniales.h"
#include "../inc/codigo_hijos.h"
#include "../inc/ipcs.h"


#define LISTEN_SIMULTANEOS 2	// conexiones en espera (LISTEN)
#define HIJOS_CONCURRENTES 5	// Clientes atendidos (Nº Hijos)
#define NO_MATEN_A_LEER 5		// tiempo de changui que le doy a leertemps
/* =======================
 *
 * =======================*/

int crear_hijos(int aux_fd,int mi_memoria,int mi_semaforo);
int llamar_a_leertemps(void);
int matar_a_leertemps(void);

/*=============================
 *
 *=============================*/
int matar_a_leertemps(void)
{
	// por si las dudas chequeo que el pid no sea 0
	// es redundante porque nunca llamo a esta funcion cuando el pid=0 perooo
	// como hay tiemos que no manejo masvale prevenir
	if((pSCfg->pid_temp))
	{
		//printf("\n<---------------------------------->");
		//printf("PAPA: SIGINT a LeerTemps en %d \n",(pSCfg->pid_temp));
	    if( (kill((pSCfg->pid_temp) , SIGUSR1)) ==-1 )
	    {
	        printf("PAPA: Error al enviar SIGINT a LeerTemps\n");
	        perror("\n");
			return LE_PIFIE_FEO;
	    }

		while((pSCfg->pid_temp))
		{
			// esto no se si esta alpedo... aca quedo esperando a que
			// m ellegue el SIGCHLD de este proceso, pero en realidad ya
			// estaria finiquitado y tendria tiempos muerros alpedo..
			// asi por lo menos me aseguro que este programa ya termino
			//sleep(1); // espero a que leertemps se muera
		}
		printf("\n<---------------------------------->");
		printf("PAPA: suicide a LeerTemps\n");
	}
	else
	{
		printf("\n<---------------------------------->");
		printf("PAPA: OjO!! leertemps NO esta corriendo\n");
	}

	return 0;
}

/*=============================
 *
 *=============================*/
int llamar_a_leertemps(void)
{
    char *argv[]={"./leertemps","1",NULL};

	if(pSCfg->pid_temp)
	{
		printf("Padre: Leertemps ya esta corriendo %d<-------------\n",pSCfg->pid_temp);
		return(0);
	}

	pSCfg->pid_temp=fork();
	if((pSCfg->pid_temp)==0)
	{// soy el hijo,
		execv("./leertemps",argv);
        perror("Padre: ERROR al llamar a leertemps <-------------\n");
        return(LE_PIFIE_FEO);
	}

	if((pSCfg->pid_temp)>0)
	{// soy el padre
		printf("Padre: LLame a leertemps con pid %d\n",(pSCfg->pid_temp));
		(pSCfg->hijos_Creados)++;// cantidad de hijos desde el inicio de los tiempos
	}
	else
	{// soy el padre error en el fork
		printf("Error al crear un hijo para leertemps: hijs_vivos= %d\n",pSCfg->hijos_vivos);
        perror("Error en fork");
        return LE_PIFIE_FEO;
    }
	sleep(1); // pequeña demora pa que leertemps se inicialice
    return 0;
}

/*=============================
 *
 *=============================*/
int crear_hijos(int aux_fd,int mi_memoria,int mi_semaforo)
{
    pid_t pid;

	if((pSCfg->pid_temp)==0)
	{	// si no esta corriendo leertemps => pa que voy a aceptar clientes y eso
		// ni me gasto en crear hijos tampoco, cierro tdo y chau
		printf("Papa: Se rompio Leertemps\n");
		return LE_PIFIE_FEO;
	}

	pid=fork();
	if(pid==0)
	{// soy el hijo, no vuelvo y hago exit en la funcion esa
		codigo_pibes(aux_fd,mi_memoria,mi_semaforo);
	}
	if(pid>0)
	{// soy el padre
		printf("Padre: Cree un hijo %d\n",pid);
		*((pSCfg->lista_hijos)+(pSCfg->hijos_vivos))=pid;// lo agrego a la lista
		(pSCfg->hijos_vivos)++;	// Hijos activos
		(pSCfg->hijos_Creados)++;// cantidad de hijos desde el inicio de los tiempos
        // cerramos el socket, que se encargue el pibe de esto
		close(aux_fd);
		//printf("Padre: Cerre el socket %d\n",pid);
	}
	else
	{// error en el fork
		printf("Error al crear un nuevo hijo, vivos: %d\n",(pSCfg->hijos_vivos));
        perror("Error en fork");
        return LE_PIFIE_FEO;
    }
    return 0;
}

/* =======================
 *
 * =======================*/

int main(int argc, char *argv[])
{
	int sfd,aux_fd,error_detectado=0;
	int cliente_en_espera=0;
    fd_set readfds;
	int basta_de_hijos=0;
	struct sockaddr_in mi_dir,cliente;
	socklen_t addrlen;

	int mi_memoria,mi_semaforo;
	struct timeval timeout;

	if(argc!=2)
	{
		printf("Uso: servidor puerto\n");
		return LE_PIFIE_FEO;
	}

	// esto es para comprobar que el driver esta instalad y que tenemos los
	// permisos para accder al mismo!!!!
	if( (aux_fd = open ("/dev/mi_i2c", O_RDWR)) < 0 )
	{
	    if(errno==EACCES)
		{
			printf("PAPA: driver instalado pero no tenemos permisos\n tenes que hacer: make\n");
		}
		if(errno==ENOENT)
		{
			printf("PAPA: driver no instalado\n dale a make install que hace magia\n");
		}
        return LE_PIFIE_FEO;
	}
	close(aux_fd);

    if(SIG_ERR == signal(SIGCHLD, manejadorSIGCHLD) )
	{
		printf("PAPA: Error al implantar el handler de SIGCHLD errno=%d\n",errno);
		return(LE_PIFIE_FEO);
	}

    if(SIG_ERR == signal(SIGINT, manejadorSIGINT) )
	{
		printf("PAPA: Error al implantar el handler de SIGINT errno=%d\n",errno);
		return(LE_PIFIE_FEO);
	}

    /*------------------------------------*/
    /*	PARA COMUNICARME CON EL LEERTEMPS */
    /*------------------------------------*/
    if(crear_ipcs_a_usar(&mi_memoria,&mi_semaforo))
    {
		return(LE_PIFIE_FEO);
    }
	// semafors creados, ahora hay que inicializarlos

    /*------------------------------------*/
    /*	PARA CONTROL GENERAL DEL SERVIDOR */
    /*------------------------------------*/
	/* Toda la configuracion y conytrol del servidor esta en una estructura
	 * como esta, teng un puntero declarado com global pa manejar esto;
	struct mi_server_cfg{
	    pid_t pid_jefe;         // pid del proceso padre
	    pid_t pid_temp;    // pid del proceso que lee temperaturas
	    pid_t *hijos_ids;        // pids de los hijos que atienden a los clientes
	    int hijos_vivos;        // Cantidad de clientes activos
	    int hijos_Creados;      // cantidad de clientes creados hasta el momente
	};
	*/
    pSCfg=(struct mi_server_cfg  *) malloc(sizeof(struct mi_server_cfg));
	if(pSCfg==NULL)
	{
		printf("PAPA: Error malloc de la estructura de control\n");
        limpiar_tuti(mi_memoria,mi_semaforo);
		return LE_PIFIE_FEO;
	}

	pSCfg->lista_hijos=(int *) malloc( HIJOS_CONCURRENTES * sizeof(pid_t));
	if(pSCfg->lista_hijos==NULL)
	{
		printf("PAPA: Error malloc del array de pids\n");
		free(pSCfg);
        limpiar_tuti(mi_memoria,mi_semaforo);
		return LE_PIFIE_FEO;
	}


    /*------------------------------------*/
    /*	PARA ARMAR LA CUMUNICACION TCPIP  */
    /*------------------------------------*/

	// creamos el socket donde voy a esperar conexines nuevas:
	sfd=socket(AF_INET, SOCK_STREAM,0);
	if(sfd<0)
	{
		perror("Error al crear el socket");
        limpiar_tuti(mi_memoria,mi_semaforo);
		free(pSCfg);
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
        limpiar_tuti(mi_memoria,mi_semaforo);
		close(sfd);
		free(pSCfg);
		return LE_PIFIE_FEO;
	}

	// hacemso el listen para encolar pedidos
	if (listen(sfd,LISTEN_SIMULTANEOS)<0)
	{
		perror("Error en el listen\n");
        limpiar_tuti(mi_memoria,mi_semaforo);
		close(sfd);
		free(pSCfg);
		return LE_PIFIE_FEO;
	}

	/* de aca pa abajo tenemos que esperar a que venga un pedido de conexion
	 * cuando este llega hay que hacer un accept (que me hace el connect pero en un
	 * nuevo socket copia del riginal => aca hago el read/write)
	*/

	pSCfg->pid_jefe=getpid();//guardo el pid del padre por si los uso despues

	(pSCfg->pid_temp)=0;
    pSCfg->hijos_vivos=0;
    pSCfg->hijos_Creados=0;
    fin_programa=1; // si es 0 hay que salir

/*=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=*/
/*=0=													=0=*/
/*=0=					LOOOP PRINCIPAL					=0=*/


	while(fin_programa)
	{
		if(basta_de_hijos)
		{	// no puedo haceptar mas clientes
			if(((pSCfg->hijos_vivos) < HIJOS_CONCURRENTES))
			{// se libero un hijo => puedo haceptar una connexion
				//sleep(1);
				printf("Padre: Se libero un lugar\n");
				basta_de_hijos=0;
			}
		}
		else
		{	// el mesnaje esta puesto aca para que no se imprima cada vez
			// que el select salga por señales o esas cosas
			printf("Padre: Esperando conexiones\n");
		}
        FD_ZERO(&readfds);		// pongo a 0 los flags de file descriptors
        FD_SET(sfd,&readfds);	// cargo sfd a la lista de file descriptors reads
        // select devuelve -1 si error y me dice cual fue el error
        // si errno = EINTR => fue una señal
        // si no me devuelve la cantidad de fd que se setearon en la lista de readfds
		if( (pSCfg->hijos_vivos==0) && (pSCfg->pid_temp))
		{
			// aca si ya me fui... esto es para que si no tengp mas hijos, antes de
			//matar a leertemps espero un rato a que venga algun cliente, si no vinene
			// recien ahi lo mato... sin esto el muchos_clientes hace llamar a leertemps
			// como 8 veces con 50 clientes... si pongo 1 segundo llama 2 veces con 3 solo una
	        timeout.tv_sec=NO_MATEN_A_LEER;
	        timeout.tv_usec=0;
			aux_fd=select(sfd+1, &readfds, NULL, NULL, &timeout);
			// sale por timeout/señal/cliente/error
			if(aux_fd==0)
			{// termino el tiempo
				if(matar_a_leertemps())
				{	// si falla es porque no pude mandar SIGINT...
					//error_detectado=1;
				}
			}
		}
		else
		{
        	aux_fd=select(sfd+1, &readfds, NULL, NULL, NULL);
			// sale por señal/cliente/error
		}

        if( (aux_fd<0) && (errno!=EINTR) )
        {// erro en el select, si huvo error aca salimos y cerramos todo
			perror("Error en SELCT");
            error_detectado=1;
            fin_programa=0;
        }

        if( (aux_fd<0) && (errno==EINTR) )
        {// el select salio por una señal
			// alguien apreto CTR+C o un hijo hizo exit y llego SIGCHLD!!!!
			// aca tendria que revisar si tengo que matar o no a leertemps
			// tambien si se libero un lugar (basta_de_hijos)
			// pero en realidad el handler de sugchld no se ejecuto todavia
			// asique no se decremento la cantidad de hijos
        }

        if( (aux_fd>0) || cliente_en_espera )// me llego un pedido de cliente
        {
			if(cliente_en_espera==0)
				printf("Padre: Recibi pedido de un cliente\n");
			if(!basta_de_hijos)
			{
				cliente_en_espera=0;
	            addrlen=sizeof(cliente);
	            aux_fd = accept (sfd, (struct sockaddr*) &cliente, &addrlen);
	    		if( (aux_fd==-1))
	    		{// error en el accept
	                printf("Padre: accept devolvio -1\n");
	    			perror("Error en accept");
	                error_detectado=1;
	                fin_programa=0;
	    		}

		        if(aux_fd>0)
		        {// accept ok
		    		// ahora en cliente tengo la direccion y el puerto del qu eme quiere hablar
		    		// tengo que crear un hijo que se encargue de esto, y cerrar el socket en el
		    		// padre y desùes seguir escuchando
		            printf("Padre: Acepte pedido de un cliente\n");

					// reviso que leertemps este corriendo,
					// si no lo esta => la llamo
					if(pSCfg->pid_temp==0)
					{
				        error_detectado=llamar_a_leertemps();
					}

					// creo el hijo que se encargara de la comunicacion
		            error_detectado=crear_hijos(aux_fd,mi_memoria,mi_semaforo);
		            if(error_detectado)
		            {
		                fin_programa=0;
		            }

					if( (pSCfg->hijos_vivos) >= HIJOS_CONCURRENTES )
					{// ya ta, alcance la maxima cantidad de clentes permitidos
						printf("Padre: Maximas Concurrencias Alcanzadas, hay %d clientes activos\n",pSCfg->hijos_vivos);
						basta_de_hijos=1;
					}
		        }//if(aux_fd>0) <--del aux_fd=accept(...)
			}//if(!basta_de_hijos)

			else //if(!basta_de_hijos)
			{ 	// llego pedido de un cliente, pero no tengo mas lugar, ¿que hago?
				/* el listen/select los encola supuestamente, asique no hago nada,
				el select me va a avisar si llego otro pedido, o sea a este lo enmascare
				por eso pongo un "cliente en espera", para que ni bien se libere un lugar
				le puedo hacer el accep, podria hacer un ++ y encolar hasta listen peroo*/
				cliente_en_espera=1;
			}
		}//if( (aux_fd>0) || cliente_en_espera )// me llego un pedido de cliente
	}//while(fin_programa)


/*=0=				FIN	LOOOP PRINCIPAL					=0=*/
/*=0=													=0=*/
/*=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=0=*/


    if(error_detectado)
    {// ocurrio algun error en algun punto del programa
        printf("Padre: Error detectado, cerrando conexiones y saliendo\n");
    }
    else
    {
        printf("Padre: Finalizando OK, Cerrando conexione y saliendo\n");
    }

    if(pSCfg->hijos_vivos)
    {	// si queda algun hijo vivo fuerzo su muerte, para eso hago SIGINT a mi mismo
        if((kill(getpid() , SIGINT))==-1)
        {
            printf("Error al enviar SIGINT\n");
            perror("\n");
        }
    }
    // listo espero a que los hijos terminen de cerrar y me voy
	printf("Padre: Quedan %d hijos vivos\n",pSCfg->hijos_vivos);
    while(pSCfg->hijos_vivos)
    {
        //pause();
        sleep(1);// no va pause, es mejor este
    }

	// OK, hasta aca mate a todos los hijos que atendian a los clientes
	// todavia queda vivo leertemps, asique le mando sigint tambien a ese
	if(pSCfg->pid_temp)
	{
		matar_a_leertemps();
	}

    printf("Padre: ya no quedan hijos vivos, limpiando huellas\n");
    limpiar_tuti(mi_memoria,mi_semaforo);
    close(sfd);
    printf("Padre: termine, en total cree %d hijos\n",pSCfg->hijos_Creados);
    return 0;
}
