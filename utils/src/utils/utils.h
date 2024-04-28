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

typedef enum{
	NEW,
	READY,
	EXEC,
	BLOCK,
	EXIT,
}t_estado_proceso;
typedef struct {
	uint32_t pid;
	uint32_t pc;
	//FALTA LO DEL QUANTUM
	t_registros_cpu* registros;
	t_estado_proceso estado;
}t_pcb;

typedef enum
{
	MENSAJE,
	PAQUETE
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
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void liberar_conexion(int socket_cliente);
t_config* iniciar_config(char *ruta);



// Serializacion

void agregar_entero_a_paquete(t_paquete *paquete, uint32_t numero);
void agregar_entero_uint8_a_paquete(t_paquete *paquete, uint8_t numero);
void agregar_entero_int_a_paquete(t_paquete *paquete, t_estado_proceso numero);
void agregar_string_a_paquete(t_paquete *paquete, char* palabra);
void agregar_pcb_a_paquete(t_paquete *paquete, t_pcb * pcb);
void agregar_registros_a_paquete(t_paquete * paquete, t_registros_cpu * registros);
void agregar_estado_a_paquete(t_paquete* paquete, t_pcb *pcb);
void enviar_entero (int conexion, uint32_t numero, int codop);
void enviar_string (int conexion, char* palabra, int codop);
t_paquete* crear_paquete_op(op_code codop);


// Una vez serializado -> recibimos y leemos estas variables

int leer_entero(char *buffer, int * desplazamiento);
uint32_t leer_entero_uint32(char *buffer, int * desplazamiento);
char* leer_string(char *buffer, int * desplazamiento);

uint32_t recibir_entero_uint32(int socket, t_log* loggs);
char* recibir_string(int socket, t_log* loggs);



#endif
