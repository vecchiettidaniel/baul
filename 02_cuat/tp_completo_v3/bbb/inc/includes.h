#ifndef INC_includes_H_
#define INC_includes_H_

#include <sys/types.h>
#include <sys/sem.h>

#define	TAMANO_BUFFER 200
#define	LE_PIFIE_FEO 1
// sacado de man semctl... se necesita declarar esta estructutra pa operar.
union semun {
   int              val;    /* Value for SETVAL */
   struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
   unsigned short  *array;  /* Array for GETALL, SETALL */
   struct seminfo  *__buf;  /* Buffer for IPC_INFO */
};

struct dato_memoria{
    int temperatura;
    unsigned long N_lectura;
    // hora de lectura o algo asi
};

struct mem_control{
    int mi_memoria;
    int mi_semaforo;
    union semun mi_semun[2];
    struct sembuf mi_semop[2];
};

struct mi_server_cfg{
    pid_t pid_jefe;         // pid del proceso padre
    pid_t pid_temp;    // pid del proceso que lee temperaturas
    pid_t *lista_hijos;        // pids de los hijos que atienden a los clientes
    int hijos_vivos;        // Cantidad de clientes activos
    int hijos_Creados;      // cantidad de clientes creados hasta el momente
};

#endif
