// entradasalida.h
#ifndef ENTRADASALIDA_H_
#define ENTRADASALIDA_H_

#include <utils/utils.h>

// Estructura para representar un bloque de datos en DialFS
typedef struct {
    uint8_t *data;      
} Block;

// Estructura para representar un archivo en DialFS
typedef struct {
    char *nombre_archivo;  
    int bloque_inicio;     
    size_t tamaño;         
} Archivo;

// Estructura para representar el sistema de archivos DialFS
typedef struct {
    int num_blocks; 
    int block_size;       
    int *bitmap;           
    Block *blocks;         
    t_list *archivos;      
    char *path_base;        // Agregado para almacenar el path base
} DialFS;


t_log* log_entradasalida;
t_config *config_entradasalida;

char* ruta_archivo;
char* ruta_completa;

//Datos recibidos de kernel
//Todas
char* nombreInterfazRecibido;
int pidRecibido;
//Generica
int unidadesRecibidas;
//Stdin Stdout
int tamañoRecibido;
int direccionRecibida;
//FS
char* nombreArchivoRecibido;
int registroPunteroArchivoRecibido;


op_code operacionActual;

DialFS fs;

t_list* lista_operaciones;
t_list* lista_pids;
t_list* lista_datos;

char* mensajeLeido;

//Caracteristicas de la interfaz
char* nombre_interfaz;
char* tipo_interfaz_txt;
op_code tipoInterfaz;  //Guarda el tipo de interfaz
int tiempo_unidad_trabajo;
char* ip_kernel;
char* puerto_kernel;
char* ip_memoria;
char* puerto_memoria;
char* path_base_dialfs;
int block_size;
int block_count;
int retraso_compactacion;

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

void dialfs_init(DialFS *dialfs, int block_size, int block_count, const char *path_base);
void dialfs_destroy(DialFS *fs);
int dialfs_allocate_block(DialFS *fs);
void dialfs_free_block(DialFS *fs, int block_index);
int dialfs_crear_archivo(DialFS *fs, const char *nombre_archivo, size_t tamaño);
void dialfs_eliminar_archivo(DialFS *fs, const char *nombre_archivo);
Archivo* buscar_archivo(DialFS *fs, const char *nombre_archivo);
void dialfs_escribir_archivo(DialFS *fs, const char *nombre_archivo, size_t offset, size_t size, const void *buffer);
void dialfs_leer_archivo(DialFS *fs, const char *nombre_archivo, int registro_direccion, int registro_tamaño, int registro_puntero_archivo);
void dialfs_truncar_archivo(DialFS *fs, const char *nombre_archivo, size_t nuevo_size);
void dialfs_compactar_archivos(DialFS *fs);

void recibirOpKernel(int SOCKET_CLIENTE_KERNEL);
void recibirOpMemoria(int SOCKET_CLIENTE_MEMORIA);
void conexionRecMem();
void funcIoGenSleep();
void funcIoStdRead();
void funcIoStdWrite();
void funcIoFsRead();
void funcIoFsWrite();
void funcIoFsCreate();
void funcIoFsDelete();
void funcIoFsTruncate();

void recibir_y_procesar_paquete(int socket_cliente);
void inicializar_listas();
void avanzar_a_siguiente_operacion();

#endif // ENTRADASALIDA_H_
