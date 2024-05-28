#ifndef ENTRADASALIDA_H_
#define ENTRADASALIDA_H_

#include <utils/utils.h>

typedef struct {
    char nombre[25];
    int tiempo_unidad_trabajo;
} InterfazGenerica;

InterfazGenerica interfazGen;

t_log* log_entradasalida;
t_config* config_entradasalida;

char* tipo_interfaz;
int tiempo_unidad_trabajo;
char* ip_kernel;
char* puerto_kernel;
char* ip_memoria;
char* puerto_memoria;
char* path_base_dialfs;
int block_size;
int block_count;

int conexion_entradasalida;

void leer_config();
void generar_conexiones();
void finalizar_programa();

void establecer_conexion_kernel(char * ip_kernel, char* puerto_kernel, t_config* config, t_log* logger);
void establecer_conexion_memoria(char * ip_memoria, char* puerto_memoria, t_config* config, t_log* logger);

void inicializar_interfaz_generica(InterfazGenerica *interfazGen, const char *nombre, int tiempo);

#endif