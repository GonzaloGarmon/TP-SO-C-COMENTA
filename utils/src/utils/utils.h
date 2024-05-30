#ifndef UTILS_H_
#define UTILS_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <commons/log.h> 
#include <commons/config.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <commons/collections/list.h>
#include <assert.h>

typedef struct{
	uint8_t AX;
	uint8_t BX;
	uint8_t CX;
	uint8_t DX;
	uint32_t EAX;
	uint32_t EBX;
	uint32_t ECX;
	uint32_t EDX;
}t_registros_cpu;

// typedef enum{
// 	NEW,
// 	READY,
// 	EXEC,
// 	BLOCK,
// 	EXIT,
// }t_estado_proceso;
typedef struct {
	uint32_t pid;
	uint32_t pc;
	int qq;
	t_registros_cpu* registros;

}t_pcb;

typedef enum
{
	//ESTADOS
	NEW,
	READY,
	EXEC,
	BLOCK,
	EXIT_,
	//MENSAJES GENERICOS
	MENSAJE,
	PAQUETE,
	//INSTRUCCIONES
	SET,
    SUB,
    SUM,
    JNZ,
    IO_GEN_SLEEP,
    MOV_IN,
    MOV_OUT,
    RESIZE,
    COPY_STRING,
    WAIT,
    SIGNAL,
    IO_STDIN_READ,
    IO_STDOUT_WRITE,
    IO_FS_CREATE,
    IO_FS_DELETE,
    IO_FS_TRUNCATE,
    IO_FS_WRITE,
    IO_FS_READ,
    EXIT,
	//SOLICITUDES DE CPU A OTROS
	PEDIR_INSTRUCCION_MEMORIA,
	EJECUTAR_IO_GEN_SLEEP,
	EJECUTAR_WAIT,
	EJECUTAR_SIGNAL,
	//SOLICITUDES DE KERNEL A OTROS
	INICIO_NUEVO_PROCESO,
	//MOTIVOS DE DESALOJO
	TERMINO_PROCESO,
	INTERRUPCION,
	ERROR,
}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;


typedef struct{
    char* parametros1;
	char* parametros2;
	char* parametros3;
	char* parametros4;
	char* parametros5;
	char* parametros6;
}t_instruccion;

/**
* @fn    decir_hola
* @brief Imprime un saludo al nombre que se pase por parámetro por consola.
*/

void decir_hola(char* quien);
int iniciar_servidor(char *puerto, t_log* loggs);
int esperar_cliente(int socket_servidor);
int recibir_operacion(int socket_cliente);
void* recibir_buffer(int* size, int socket_cliente);
void recibir_mensaje(int socket_cliente, t_log* loggs);
t_list* recibir_paquete(int socket_cliente);
void* serializar_paquete(t_paquete* paquete, int bytes);
int crear_conexion(char *ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
void crear_buffer(t_paquete* paquete);
t_paquete* crear_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, uint32_t tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void liberar_conexion(int socket_cliente);
t_config* iniciar_config(char *ruta);



// Serializacion

void agregar_entero_a_paquete(t_paquete *paquete, uint32_t numero);
void agregar_entero_uint8_a_paquete(t_paquete *paquete, uint8_t numero);
void agregar_entero_int_a_paquete(t_paquete *paquete, int numero);
void agregar_string_a_paquete(t_paquete *paquete, char* palabra);
void agregar_pcb_a_paquete(t_paquete *paquete, t_pcb * pcb);
void agregar_registros_a_paquete(t_paquete * paquete, t_registros_cpu * registros);
void agregar_instruccion_a_paquete(t_paquete *paquete, t_instruccion * instruccion_nueva);
void enviar_entero (int conexion, uint32_t numero, int codop);
void enviar_string (int conexion, char* palabra, int codop);
void enviar_pcb (int conexion, t_pcb* pcb, int codop);
void enviar_instruccion (int conexion, t_instruccion* nueva_instruccion, int codop);
t_paquete* crear_paquete_op(op_code codop);


// Una vez serializado -> recibimos y leemos estas variables

int leer_entero(char *buffer, int * desplazamiento);
uint8_t leer_entero_uint8(char *buffer, int * desplazamiento);
uint32_t leer_entero_uint32(char *buffer, int * desplazamiento);
char* leer_string(char *buffer, int * desplazamiento);
t_registros_cpu * leer_registros(char* buffer, int* desp);

uint32_t recibir_entero_uint32(int socket, t_log* loggs);
char* recibir_string(int socket, t_log* loggs);
t_pcb* recibir_pcb(int socket);
t_instruccion* recibir_instruccion(int socket);
t_list* recibir_doble_entero(int socket);

#endif
