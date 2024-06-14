#ifndef ENTRADASALIDA_H_
#define ENTRADASALIDA_H_

#include <utils/utils.h> // Incluir los headers necesarios

// Definir las estructuras para las diferentes interfaces
typedef struct {
    char* nombre;
    int tiempo_unidad_trabajo;
    char* ip_kernel;
    char* puerto_kernel;
} InterfazGenerica;

typedef struct {
    char* nombre;
    int tiempo_unidad_trabajo;
    char* ip_kernel;
    char* puerto_kernel;
    char* ip_memoria;
    char* puerto_memoria;
} STDIN;

typedef struct {
    char* nombre;
    int tiempo_unidad_trabajo;
    char* ip_kernel;
    char* puerto_kernel;
    char* ip_memoria;
    char* puerto_memoria;
} STDOUT;

// Definir un enum para el tipo de interfaz
typedef enum {
    GENERICA,
    STDIN,
    STDOUT,
    DIALFS
} TipoInterfaz;

// Definir una estructura para una interfaz en el registro
typedef struct {
    char* nombre;
    bool conectada;
} Interfaz;

// Definir el estado de una interfaz
typedef enum {
    LIBRE,
    OCUPADA
} EstadoInterfaz;

// Definir el registro de interfaces
typedef struct {
    Interfaz* interfaces;
    int cantidad;
    int capacidad;
} RegistroInterfaz;

// Declarar variables globales
RegistroInterfaz registro;
EstadoInterfaz estado_interfaz = LIBRE;

// Declarar las operaciones de entrada/salida
typedef enum {
    IO_GEN_SLEEP,
    STDIN_READ,
    STDOUT_WRITE
} OperacionIO;

void* crear_interfaz(TipoInterfaz tipo) {
    void* interfaz = NULL;
    switch (tipo) {
        case GENERICA:
            return malloc(sizeof(InterfazGenerica));
        case STDIN:
            return malloc(sizeof(STDIN));
        case STDOUT:
            return malloc(sizeof(STDOUT));
        default:
            fprintf(stderr, "Tipo de interfaz no válido\n");
            return NULL;
    }
    return interfaz;
}


void liberar_interfaz(void* interfaz, TipoInterfaz tipo) {
    switch (tipo) {
        case GENERICA:
            free(((InterfazGenerica*)interfaz)->nombre);
            free(interfaz);
            break;
        case STDIN:
            free(((STDIN*)interfaz)->nombre);
            free(interfaz);
            break;
        case STDOUT:
            free(((STDOUT*)interfaz)->nombre);
            free(interfaz);
            break;
        default:
            fprintf(stderr, "Tipo de interfaz no válido\n");
    }
}


t_log* log_entradasalida;
t_config* config_entradasalida;
t_config* config_TECLADO;
t_config* config_MONITOR;
t_config* config_GENERICA;

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

// Prototipos de funciones
void* crear_interfaz(TipoInterfaz tipo);
void liberar_interfaz(void* interfaz, TipoInterfaz tipo);
void leer_config();
void generar_conexiones();
void finalizar_programa();
void establecer_conexion_kernel(char* ip_kernel, char* puerto_kernel, t_config* config, t_log* logger);
void establecer_conexion_memoria(char* ip_memoria, char* puerto_memoria, t_config* config, t_log* logger);
void inicializar_interfaz_generica(Generica *interfazGen, const char *nombre, int tiempo);
void inicializar_interfaz_stdin(STDIN *interfazStdin, const char *nombre);
void inicializar_interfaz_stdout(STDOUT *interfazStdout, const char *nombre);
bool validar_interfaz(ListaIO* interfaces, int num_interfaces, char* nombre_solicitado);
void validar_operacion_io(void *interfaz, OperacionIO operacion);
bool es_operacion_compatible(TipoInterfaz tipo, OperacionIO operacion);
void solicitar_operacion_io(InterfazIO interfaz, OperacionIO operacion);
void operacion_io_finalizada();
void inicializar_registro();
void liberar_registro();
void conectar_interfaz(char* nombre_interfaz);
void desconectar_interfaz(char* nombre_interfaz);
bool interfaz_conectada(char* nombre_interfaz);
void esperar_interfaz_libre();

#endif
