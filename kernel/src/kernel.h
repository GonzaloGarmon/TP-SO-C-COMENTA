#ifndef KERNEL_H_
#define KERNEL_H_

#include <utils/utils.h>


t_log* log_kernel;
t_config* config_kernel;
t_list* cola_new;

pthread_mutex_t mutex_cola_new;

char* puerto_escucha;
char* ip_memoria;
char* puerto_memoria;
char* ip_cpu;
char* puerto_cpu_interrupt;
char* puerto_cpu_dispatch;
char* algoritmo_planificacion;
char* quantum;
char* recursos;
char* instancias_recursos;
char* grado_multiprogramacion;


int socket_servidor_kernel_dispatch;
int socket_servidor_kernel_interrupt;
int socket_cliente_entradasalida;
int conexion_kernel;
int generador_pid;

void recibir_entradasalida(int SOCKET_CLIENTE_ENTRADASALIDA);
void establecer_conexion_memoria(char * ip_memoria, char* puerto_memoria, t_config* config, t_log* logger);
void establecer_conexion_cpu(char * ip_cpu, char* puerto_cpu, t_config* config, t_log* logger);
void iniciar_consola();
void ejecutar_script();
void iniciar_proceso();
void finalizar_proceso();
void iniciar_planificacion();
void detener_planificacion();
void listar_procesos_estado();
t_registros_cpu* inicializar_registros();
#endif