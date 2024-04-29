#ifndef KERNEL_H_
#define KERNEL_H_

#include <utils/utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum{
    FIFO,
    RR,
    VRR
} t_algoritmo;

t_log* log_kernel;
t_config* config_kernel;
t_list* cola_new;
t_list* cola_ready;
t_list* cola_exec;

pthread_mutex_t mutex_cola_new;
pthread_mutex_t mutex_cola_ready;
pthread_mutex_t mutex_cola_exec;
sem_t sem_listos_para_ready;
sem_t sem_multiprogamacion;
sem_t sem_listos_para_exec;

char* puerto_escucha;
char* ip_memoria;
char* puerto_memoria;
char* ip_cpu;
char* puerto_cpu_interrupt;
char* puerto_cpu_dispatch;

char* algoritmo;
t_algoritmo algoritmo_planificacion;
int quantum;

char* recursos;
char* instancias_recursos;
int grado_multiprogramacion;


int socket_servidor_kernel_dispatch;
int socket_servidor_kernel_interrupt;
int socket_cliente_entradasalida;
int conexion_kernel;
int generador_pid;

void iniciar_semaforos();
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
t_pcb* elegir_pcb_segun_algoritmo();
void pcb_ready();
t_pcb* remover_pcb_de_lista(t_list *list, pthread_mutex_t *mutex);
void planificar();
void planificar_largo_plazo();
void planificar_corto_plazo();
void planificar_rr();
void contador_quantum_RR();
void exec_pcb();
#endif