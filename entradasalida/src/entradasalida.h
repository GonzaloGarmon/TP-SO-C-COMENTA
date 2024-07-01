// entradasalida.h
#ifndef ENTRADASALIDA_H_
#define ENTRADASALIDA_H_

#include <utils/utils.h>

typedef struct {
    char* nombre;
    bool conectada;
} ListaIO;

typedef enum {
    TIPO_GENERICA,
    TIPO_STDIN,
    TIPO_STDOUT,
    TIPO_DIALFS
} TipoInterfaz;

typedef struct {
    char* nombre;
    TipoInterfaz tipo;
    int tiempo_unidad_trabajo;
    char* ip_kernel;
    char* puerto_kernel;
    bool enUso;
} GENERICA;

typedef struct {
    char* nombre;
    TipoInterfaz tipo;
    int tiempo_unidad_trabajo;
    char* ip_kernel;
    char* puerto_kernel;
    char* ip_memoria;
    char* puerto_memoria;
    bool enUso;
} STDIN;

typedef struct {
    char* nombre;
    TipoInterfaz tipo;
    int tiempo_unidad_trabajo;
    char* ip_kernel;
    char* puerto_kernel;
    char* ip_memoria;
    char* puerto_memoria;
    bool enUso;
} STDOUT;

typedef struct {
    char* nombre;
    TipoInterfaz tipo;
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
} DIALFS;

typedef enum {
    IOGENSLEEP,
    STDINREAD,
    STDOUTWRITE,
    FSCREATE,
    FSDELETE,
    FSTRUNCATE,
    FSREAD,
    FSWRITE
} OperacionIO;

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
char* tipo_interfaz;
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
void procesar_todos_los_archivos_config();
void procesar_archivo_config(const char* ruta_archivo);
void finalizar_programa();

void generar_conexiones();
void establecer_conexion_kernel(char* ip_kernel, char* puerto_kernel, t_config* config, t_log* logger);
void establecer_conexion_memoria(char* ip_memoria, char* puerto_memoria, t_config* config, t_log* logger);

void inicializar_interfaz_generica(t_config *config, GENERICA *interfazGen, const char *nombre);
void inicializar_interfaz_stdin(t_config *config, STDIN *interfazStdin, const char *nombre);
void inicializar_interfaz_stdout(t_config *config, STDOUT *interfazStdout, const char *nombre);
void inicializar_interfaz_dialfs(t_config *config, DIALFS *interfazDialFS, const char *nombre);

bool validar_interfaz(ListaIO* interfaces, int num_interfaces, char* nombre_solicitado);
void validar_operacion_io(void *interfaz, OperacionIO operacion);
bool es_operacion_compatible(TipoInterfaz tipo, OperacionIO operacion);
void solicitar_operacion_io(void *interfaz, OperacionIO operacion);
void operacion_io_finalizada();
void inicializar_registro();
void liberar_registro();
void conectar_interfaz(char* nombre_interfaz);
void desconectar_interfaz(char* nombre_interfaz);
bool interfaz_conectada(char* nombre_interfaz);
void esperar_interfaz_libre();
void liberar_interfaz(void* interfaz, TipoInterfaz tipo);
bool tiene_extension_config(const char* filename);

#endif // ENTRADASALIDA_H_
