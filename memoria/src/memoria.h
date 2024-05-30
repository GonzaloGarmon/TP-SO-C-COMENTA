#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <utils/utils.h>


t_log* log_memoria;
t_config* config_memoria;

char* puerto_escucha;
int tam_memoria;
int tam_pagina;
char* path_instrucciones;
int retardo_respuesta;

//Chequear los tamanios maximos de todo.
int longitud_maxima = 100;
int parametros_maximos = 10;
int instrucciones_maximas = 100;
t_instruccion* instrucciones[100];
int socket_servidor_memoria_dispatch;
int socket_servidor_memoria_interrupt;
int socket_cliente_kernel;
int socket_cliente_cpu;
int socket_cliente_entradasalida;
int socket_cliente;

void leer_config();
void finalizar_programa();

void recibir_cpu(int SOCKET_CLIENTE_CPU);
void recibir_kernel(int SOCKET_CLIENTE_KERNEL);
void recibir_entradasalida(int SOCKET_CLIENTE_ENTRADASALIDA);

int instrucciones_maximas;

void* cargar_instrucciones_desde_archivo(char* nombre_archivo, t_instruccion* instrucciones[instrucciones_maximas]);

#endif