#ifndef ENTRADASALIDA_H_
#define ENTRADASALIDA_H_

#include <utils/utils.h>


t_log* log_entradasalida;
t_config* config_entradasalida;

char* tipo_interfaz;
char* tiempo_unidad_trabajo;
char* ip_kernel;
char* puerto_kernel;
char* ip_memoria;
char* puerto_memoria;
char* path_base_dialfs;
char* block_size;
char* block_count;

int conexion_entradasalida;

void establecer_conexion_kernel(char * ip_kernel, char* puerto_kernel, t_config* config, t_log* logger);
void establecer_conexion_memoria(char * ip_memoria, char* puerto_memoria, t_config* config, t_log* logger);


#endif