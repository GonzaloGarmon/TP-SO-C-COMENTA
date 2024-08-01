#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <utils/utils.h>

// Creación de estructuras para paginación
typedef struct {
    uint32_t tam_memoria;
    uint32_t tam_pagina;
    uint32_t retardo_obtencion_instruccion;
}memoria_config_t;
typedef struct {
    uint32_t base;
    uint32_t limite;
} t_esp; // Para marcar un hueco en la memoria

typedef struct {
    uint32_t numero_marco;
} entrada_tabla_pagina_t;

typedef struct {
    uint32_t pid;
    t_list *tabla_paginas;
} tabla_pagina_t;

t_log* log_memoria;
t_config* config_memoria;
memoria_config_t memoria_config;

char* puerto_escucha;
int tam_memoria;
int tam_pagina;
char* path_instrucciones;
int retardo_respuesta;
pthread_mutex_t mutex_memoria;
pthread_mutex_t mutex_de_tabla_Paginas;
sem_t sem;

t_list* pids_ejecucion;
t_list** listas_instrucciones;

int longitud_maxima=200;
int parametros_maximos=6;
int instrucciones_maximas=200;
t_instruccion* instrucciones[200];
int socket_servidor_memoria_dispatch;
int socket_servidor_memoria_interrupt;
int socket_cliente_kernel;
int socket_cliente_cpu;
int socket_cliente_entradasalida;
int socket_cliente;
uint32_t ESPACIO_LIBRE_TOTAL;
t_list *LISTA_ESPACIOS_LIBRES;
t_list *LISTA_TABLA_PAGINAS;
uint32_t marcos_libres;
uint32_t marcos_totales;
uint32_t *marcos_asignados;

void *ESPACIO_USUARIO;

// Declaraciones de funciones
void inicializar_memoria();
void levantar_estructuras_administrativas();
void leer_config(char* path);
void finalizar_programa();
void recibir_cpu(int SOCKET_CLIENTE_CPU);
void recibir_kernel(int SOCKET_CLIENTE_KERNEL);
void recibir_entradasalida(int SOCKET_CLIENTE_ENTRADASALIDA);
void esperar_cliente_especial(int socket_servidor_memoria_dispatch);
void finalizar_proceso(uint32_t proceso);
op_code ajustar_tamanio_proceso(uint32_t nuevo_tam, uint32_t pid);
void crear_tabla_pagina(uint32_t pid_t, uint32_t cant_paginas);
void copiarBytes(uint32_t tamanio, t_contexto *contexto);
void escribir(uint32_t dir_fisca, void* data, uint32_t size);
char* leer(uint32_t dir_fisca, uint32_t size);
uint32_t obtener_marco_pagina(uint32_t pid, uint32_t num_pagina);
void cargar_instrucciones_desde_archivo(char* nombre_archivo, uint32_t pid);
uint32_t buscar_marco_libre();
void marco_ocupado(uint32_t numero_marco);
void marco_libre(uint32_t numero_marco);
uint32_t cantidad_marcos_vacios();
op_code ampliar(tabla_pagina_t *tabla, uint32_t nuevo_tam);
void reducir(tabla_pagina_t* tabla, uint32_t nuevo_tam);


#endif // MEMORIA_H_