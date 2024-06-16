#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <utils/utils.h>

//creacion de estructuras para paginacion
typedef struct {
    uint32_t tam_memoria;
    uint32_t tam_pagina;
    uint32_t retardo_obtencion_instruccion;
}memoria_config_t;
typedef struct {
    uint32_t base;
    uint32_t limite;
}t_esp; // para marca un hueco en la memorua

typedef struct {
    uint32_t numero_pagina;
    uint32_t numero_marco;
} entrada_tabla_pagina_t;

typedef struct {
    uint32_t pid;
    entrada_tabla_pagina_t *tabla_paginas;
    uint32_t cantidad_paginas; 
}tabla_pagina_t;


t_log* log_memoria;
t_config* config_memoria;
memoria_config_t memoria_config;

char* puerto_escucha;
int tam_memoria;
int tam_pagina;
char* path_instrucciones;
int retardo_respuesta;
pthread_mutex_t mutex_memoria;

//Chequear los tamanios maximos de todo.
int longitud_maxima = 100;
int parametros_maximos = 6;
int instrucciones_maximas = 100;
t_instruccion* instrucciones[100];
int socket_servidor_memoria_dispatch;
int socket_servidor_memoria_interrupt;
int socket_cliente_kernel;
int socket_cliente_cpu;
int socket_cliente_entradasalida;
int socket_cliente;
uint32_t ESPACIO_LIBRE_TOTAL;
t_list *LISTA_ESPACIOS_LIBRES;
t_list *LISTA_TABLA_PAGINAS;

void *ESPACIO_USUARIO;

void leer_config();
void finalizar_programa();

void recibir_cpu(int SOCKET_CLIENTE_CPU);
void recibir_kernel(int SOCKET_CLIENTE_KERNEL);
void recibir_entradasalida(int SOCKET_CLIENTE_ENTRADASALIDA);


void recibir_cpu1(int SOCKET_CLIENTE_CPU);
void recibir_kernel1(int SOCKET_CLIENTE_KERNEL);
void recibir_entradasalida1(int SOCKET_CLIENTE_ENTRADASALIDA);
void finalizar_proceso(uint32_t proceso);
void ajustar_tamanio_proceso(uint32_t pid_ajuste, uint32_t nuevo_tam);
void crear_tabla_pagina(uint32_t pid_t, uint32_t cant_paginas);

void escribir(uint32_t dir_fisca, void* data, uint32_t size);
char* leer(uint32_t dir_fisca , uint32_t size);



int instrucciones_maximas;

void* cargar_instrucciones_desde_archivo(char* nombre_archivo, t_instruccion* instrucciones[instrucciones_maximas]);

#endif