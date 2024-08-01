// entradasalida.h
#ifndef ENTRADASALIDA_H_
#define ENTRADASALIDA_H_

#include <utils/utils.h>

// Estructura para representar un archivo en DialFS
typedef struct {
    char *nombre_archivo;
    int bloque_inicio;
    size_t tamaño;
} Archivo;

// Estructura para representar el sistema de archivos DialFS
typedef struct {
    int block_count;
    int block_size;
    t_bitarray *bitmap;
    void *blocks;
    t_list *archivos;
    char *path_base;
} DialFS;

DialFS fs;

t_log* log_entradasalida;
t_config *config_entradasalida;
sem_t sem_termino;
char* ruta_archivo;
char* ruta_completa;

pthread_mutex_t mutex_dialfs = PTHREAD_MUTEX_INITIALIZER;

//Datos recibidos de kernel
//Todas
char* nombreInterfazRecibido;
uint32_t pidRecibido;
//Generica
int unidadesRecibidas;
//Stdin Stdout
int tamañoRecibido;
int direccionRecibida;
//FS
char* nombreArchivoRecibido;
int registroPunteroArchivoRecibido;

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

// Prototipos de funciones de fs
void dialfs_init(DialFS *dialfs, int block_size, int block_count, const char *path_base);
void dialfs_destroy(DialFS *fs);
int dialfs_allocate_block(DialFS *fs);
void dialfs_allocate_specific_block(DialFS *fs, int block_index);
void dialfs_free_block(DialFS *fs, int block_index);
int dialfs_crear_archivo(DialFS *fs, const char *nombre_archivo);
void dialfs_eliminar_archivo(DialFS *fs, const char *nombre_archivo);
void dialfs_truncar_archivo(DialFS *fs, const char *nombre_archivo, size_t nuevo_size);
void dialfs_escribir_archivo(DialFS *fs, const char *nombre_archivo, size_t offset, size_t size, const void *buffer);
Archivo* buscar_archivo(DialFS *fs, const char *nombre_archivo);
void dialfs_leer_archivo(DialFS *fs, const char *nombre_archivo, void *registro_direccion, int registro_tamaño, int registro_puntero_archivo);
void dialfs_compactar_archivos(DialFS *fs);
bool espacioContiguoDisponible(DialFS *fs, size_t bloques_necesarios);
int comparar_archivos_por_bloque_inicio(const void *a, const void *b);
void limpiar_buffer_entrada();

void recibirOpKernel(int SOCKET_CLIENTE_KERNEL);
void recibirOpMemoria(int SOCKET_CLIENTE_MEMORIA);
void conexionRecMem();
void funcIoGenSleep(t_entero_bool** ejecucion);
void funcIoStdRead(t_entero_bool** ejecucion);
void funcIoStdWrite(t_entero_bool** ejecucion);
void funcIoFsRead(t_entero_bool** ejecucion);
void funcIoFsWrite(t_entero_bool** ejecucion);
void funcIoFsCreate(t_entero_bool** ejecucion);
void funcIoFsDelete(t_entero_bool** ejecucion);
void funcIoFsTruncate(t_entero_bool** ejecucion);

void recibir_y_procesar_paquete(int socket_cliente);
void inicializar_listas();
void liberar_listas();
void avanzar_a_siguiente_operacion();

uint32_t* malloc_copiar_uint32(uint32_t valor);

#endif // ENTRADASALIDA_H_
