#ifndef CPU_H_
#define CPU_H_

#include <utils/utils.h>


t_log* log_cpu;
t_config* config_cpu;

char* ip_memoria;
char* puerto_memoria; 
char* puerto_escucha_dispatch;
char* puerto_escucha_interrupt;
char* cantidad_entradas_tlb;
char* algoritmo_tlb;


int socket_servidor_cpu_dispatch;
int socket_servidor_cpu_interrupt;
int socket_cliente_kernel;
int conexion_cpu;



void recibir_kernel(int SOCKET_CLIENTE_KERNEL);
void establecer_conexion(char * ip_memoria, char* puerto_memoria, t_config* config, t_log* logger);

#endif