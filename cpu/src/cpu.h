#include <stdlib.h>
#include <stdio.h>
#include <commons/log.h> 
#include <commons/config.h>
#include <pthread.h>




char* ip_memoria;
int* puerto_memoria; 
int* puerto_escucha_dispatch;
int* puerto_escucha_interrupt;
int* cantidad_entradas_tlb;
char* algoritmo_tlb;


int socket_servidor_cpu_dispatch;
int socket_servidor_cpu_interrupt;
int socket_cliente_kernel;


t_log* log_cpu;
t_config* config_cpu;
