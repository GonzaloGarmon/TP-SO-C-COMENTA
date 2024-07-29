#ifndef KERNEL_H_
#define KERNEL_H_

#include <utils/utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 256

typedef struct{
 int socket_servidor_kernel_dispatch;
 t_list* conexiones_io;
 t_list* conexiones_io_nombres;

}t_conexiones_kernel_io;
typedef enum{
    FIFO,
    RR,
    VRR
} t_algoritmo;

t_log* log_kernel;
t_config* config_kernel;
t_list* cola_new;
t_list* cola_ready;
t_list* cola_ready_aux;
t_list* cola_exec;
t_list* cola_exit;
t_list* cola_block;

pthread_mutex_t mutex_cola_new;
pthread_mutex_t mutex_cola_ready;
pthread_mutex_t mutex_cola_ready_aux;
pthread_mutex_t mutex_cola_exec;
pthread_mutex_t mutex_cola_exit;
pthread_mutex_t mutex_cola_block;
pthread_mutex_t conexiones_io_mutex;
pthread_mutex_t mutex_recurso;
pthread_mutex_t conexion;

sem_t sem_listos_para_ready;
sem_t sem_multiprogamacion;
sem_t sem_listos_para_exec;
sem_t sem_listos_para_exit;
sem_t sem_empezar_quantum;
sem_t sem_iniciar_consola;
sem_t sem_eliminar_quantum;
sem_t esta_ejecutando;
sem_t sem_chequear_validacion;

char* puerto_escucha;
char* ip_memoria;
char* puerto_memoria;
char* ip_cpu;
char* puerto_cpu_interrupt;
char* puerto_cpu_dispatch;

char* algoritmo;
t_algoritmo algoritmo_planificacion;
int quantum;

int corto_VRR;
uint32_t validacion;
t_conexiones_kernel_io conexiones_io;
char** recursos;
int* instancias_recursos;
int cantidad_recursos;
int grado_multiprogramacion;
t_list** lista_recurso;


int socket_servidor_kernel_dispatch;
int socket_servidor_kernel_interrupt;
int socket_cliente_entradasalida;
int conexion_kernel_cpu_dispatch;
int conexion_kernel_cpu_interrupt;
int conexion_kernel_memoria;
int apagar_planificacion;
uint32_t generador_pid;

/*
------------------------CONFIGS, INICIACION, COMUNICACIONES-------------------------------------
*/
void leer_config(char* path);
void generar_conexiones();
void iniciar_semaforos();
void finalizar_programa();
int* convertirArrayDeNumeros(char** caracteres);
void esperar_cliente_especial(int socket_cliente_entradasalida);
/*
------------------------CONFIGS, INICIACION, COMUNICACIONES-------------------------------------
*/
void mostrar_prioridad_ready();
void recibir_entradasalida(int SOCKET_CLIENTE_ENTRADASALIDA);
void establecer_conexion_memoria(char * ip_memoria, char* puerto_memoria, t_config* config, t_log* logger);
void establecer_conexion_cpu_dispatch(char * ip_cpu, char* puerto_cpu, t_config* config, t_log* logger);
void establecer_conexion_cpu_interrupt(char * ip_cpu, char* puerto_cpu, t_config* config, t_log* logger);
void iniciar_consola();
int ejecutar_script(char* path);
void iniciar_proceso(char* path);
void finalizar_proceso(uint32_t pid);
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
void planificar_vrr();
void manejar_VRR();
void contador_quantum_RR(uint32_t verificacion);
void exec_pcb();
void pcb_exit();
void dispatch(t_pcb* pcb_enviar);
void recibir_cpu_dispatch(int conexion_kernel_cpu_dispatch);
void recibir_cpu_interrupt(int conexion_kernel_cpu_interrupt);
int existe_recurso(char* recurso);
void actualizar_contexto(t_contexto* pcb_wait);
void actualizar_pcb_con_cambiar_lista(t_contexto* pcb_wait, t_list* lista_bloq_recurso);
void desbloquear_proceso(t_list* lista_recurso_liberar);
void actualizar_pcb_envia_exit(t_contexto* pcb_wait, motivo_exit codigo);
void actualizar_pcb_envia_ready(t_contexto* pcb_wait);
bool esta_en_esta_lista(t_list* lista, uint32_t pid_encontrar);
void sacar_de_lista_mover_exit(t_list* lista,pthread_mutex_t mutex_lista, uint32_t pid);
void sacar_de_lista_mover_exit_recurso(t_list* lista, uint32_t pid);
void enviar_interrupcion(uint32_t pid);
void enviar_interrupcion_fin_proceso();
int existe_interfaz_conectada(char* nombre_interfaz);
int admite_operacion_con_u32(char* nombre_interfaz, op_code codigo, uint32_t entero32, uint32_t pid);
int admite_operacion_con_2u32(char* nombre_interfaz, op_code codigo, uint32_t primer_entero32, uint32_t segundo_entero32, uint32_t pid);
int admite_operacion_con_string(char* nombre_interfaz, op_code codigo, char* palabra, uint32_t pid);
int admite_operacion_con_string_u32(char* nombre_interfaz, op_code codigo, char* palabra, uint32_t primer_entero32, uint32_t pid);
int admite_operacion_con_string_3u32(char* nombre_interfaz, op_code codigo,char* palabra, uint32_t primer_entero32, uint32_t segundo_entero32, uint32_t tercer_entero32, uint32_t pid);
void bloquear_pcb(t_contexto* contexto);
void desbloquear_proceso_block(uint32_t pid);
char* motivo_exit_to_string(motivo_exit motivo);
void cambio_estado(uint32_t pid, char* estado_anterior, char* estado_nuevo);
void mostrar_motivo_block(uint32_t pid, char* motivo_block);
int proceso_activos();

#endif