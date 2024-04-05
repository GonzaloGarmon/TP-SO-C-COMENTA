#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <utils/utils.h>


t_log* log_memoria;
t_config* config_memoria;

char* puerto_escucha;
char* tam_memoria;
char* tam_pagina;
char* path_instrucciones;
char* retardo_respuesta;


int socket_servidor_memoria_dispatch;
int socket_servidor_memoria_interrupt;
int socket_cliente_kernel;
int socket_cliente_cpu;
int socket_cliente_entradasalida;


void recibir_cpu(int SOCKET_CLIENTE_CPU);
void recibir_kernel(int SOCKET_CLIENTE_KERNEL);
void recibir_entradasalida(int SOCKET_CLIENTE_ENTRADASALIDA);

#endif