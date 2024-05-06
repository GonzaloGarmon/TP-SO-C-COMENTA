#ifndef CPU_H_
#define CPU_H_

#include <utils/utils.h>


t_log* log_cpu;
t_config* config_cpu;
t_pcb *contexto;

char* ip_memoria;
char* puerto_memoria; 
char* puerto_escucha_dispatch;
char* puerto_escucha_interrupt;
int cantidad_entradas_tlb;
char* algoritmo_tlb;


int socket_servidor_cpu_dispatch;
int socket_servidor_cpu_interrupt;
int socket_cliente_kernel;
int conexion_cpu;



void recibir_kernel(int SOCKET_CLIENTE_KERNEL);
void establecer_conexion(char * ip_memoria, char* puerto_memoria, t_config* config, t_log* logger);
// void funcSet(t_instruccion *&instruccion);
// void funcSum(t_instruccion *&instruccion);
// void funcSub(t_instruccion *&instruccion);
// void funcJnz(t_instruccion *&instruccion);
// void funcIoGenSleep(t_instruccion *&instruccion);
// t_nombre_instruccion decode(t_instruccion *instruccion, t_pcb *contexto);

typedef enum{ //solo estan las instrucciones necesarias para el checkpoint 2
    SET,
    SUB,
    SUM,
    JNZ,
    IO_GEN_SLEEP,
    MOV_IN,
    MOV_OUT,
    RESIZE,
    COPY_STRING,
    WAIT,
    SIGNAL,
    IO_STDIN_READ,
    IO_STDOUT_WRITE,
    IO_FS_CREATE,
    IO_FS_DELETE,
    IO_FS_TRUNCATE,
    IO_FS_WRITE,
    IO_FS_READ,
    EXIT_
}t_nombre_instruccion;

typedef struct{
    t_nombre_instruccion nombre;
    char parametro1[20];
    char parametro2[20];
}t_instruccion;


#endif