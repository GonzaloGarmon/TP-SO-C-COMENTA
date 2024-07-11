// entradasalida.h
#ifndef ENTRADASALIDA_H_
#define ENTRADASALIDA_H_

#include <utils/utils.h>

typedef struct {
    char* nombre;
    bool conectada;
} ListaIO;

// Estructura para representar un bloque de datos en DialFS
typedef struct {
    uint8_t *data;      // Datos del bloque
} Block;

// Estructura para representar el sistema de archivos DialFS
typedef struct {
    int num_blocks;     // Número total de bloques
    int *bitmap;        // Bitmap para rastrear bloques libres/ocupados
    Block *blocks;      // Arreglo de bloques
} DialFS;

typedef struct {
    ListaIO* interfaces;
    int cantidad;
    int capacidad;
} RegistroInterfaz;

RegistroInterfaz registro;
pthread_mutex_t mutex;
pthread_cond_t cond;

t_log* log_entradasalida;
t_config *config_entradasalida;

char* ruta_archivo;
char* ruta_completa;

char* nombre_interfaz;
char* tipo_interfaz_txt;
op_code tipo;  //Guarda el tipo de interfaz
int tiempo_unidad_trabajo;
char* ip_kernel;
char* puerto_kernel;
char* ip_memoria;
char* puerto_memoria;
char* path_base_dialfs;
int block_size;
int block_count;
int retraso_compactacion;
bool enUso;

int socket_servidor_entradasalida;
int conexion_entradasalida_kernel;
int conexion_entradasalida_memoria;

// funciones
void leer_config();
void crear_interfaz(char *nombre_interfaz, char *ruta_archivo);
void finalizar_programa();
void generar_conexiones();
void establecer_conexion_kernel(char* ip_kernel, char* puerto_kernel, t_config* config_entradasalida, t_log* logger);
void establecer_conexion_memoria(char* ip_memoria, char* puerto_memoria, t_config* config_entradasalida, t_log* logger);
void inicializar_interfaz_generica(t_config *config_entradasalida, const char *nombre);
void inicializar_interfaz_stdin(t_config *config_entradasalida, const char *nombre);
void inicializar_interfaz_stdout(t_config *config_entradasalida, const char *nombre);
void inicializar_interfaz_dialfs(t_config *config_entradasalida, const char *nombre);
bool es_operacion_compatible(op_code tipo, op_code operacion);

void esperar_interfaz_libre();
void dialfs_init(DialFS *dialfs, int num_blocks);
void dialfs_destroy(DialFS *fs);
int dialfs_allocate_block(DialFS *fs);
void dialfs_free_block(DialFS *fs, int block_index);
void dialfs_write_block(DialFS *fs, int block_index, const uint8_t *data, size_t size);
void dialfs_read_block(DialFS *fs, int block_index, uint8_t *buffer, size_t size);
int dialfs_crear_archivo(DialFS *fs, const char *nombre_archivo, const uint8_t *datos, size_t size);
void dialfs_redimensionar_archivo(DialFS *fs, int bloque_archivo, const uint8_t *nuevos_datos, size_t nuevo_size);
void dialfs_compactar_archivos(DialFS *fs);

void recibirOpKernel(int SOCKET_CLIENTE_KERNEL);
void recibirOpMemoria(int SOCKET_CLIENTE_MEMORIA);
void funcIoGenSleep(int unidades);
void funcIoStdRead(int direccion, int tamaño);
void funcIoStdWrite(int direccion, int tamaño);
void funcIoFsRead();
void funcIoFsWrite();
void funcIoFsCreate();
void funcIoFsDelete();
void funcIoFsTruncate();

#endif // ENTRADASALIDA_H_
