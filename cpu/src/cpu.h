#ifndef CPU_H_
#define CPU_H_

#include <utils/utils.h>


t_log* log_cpu;
t_config* config_cpu;
t_pcb *contexto;

char* ip_memoria;
char* puerto_memoria; 
char* puerto_escucha_dispatch;
char* puerto_escucha_interrupt;
int cantidad_entradas_tlb;
char* algoritmo_tlb;


int socket_servidor_cpu_dispatch;
int socket_servidor_cpu_interrupt;
int socket_cliente_kernel_dispatch;
int socket_cliente_kernel_interrupt;
int conexion_memoria;
int seguir_ejecutando;

void leer_config();

void generar_conexiones();

void terminar_programa();

void recibir_kernel_dispatch(int SOCKET_CLIENTE_KERNEL);
void recibir_kernel_interrupt(int SOCKET_CLIENTE_KERNEL_INTERRUPT);
void establecer_conexion(char * ip_memoria, char* puerto_memoria, t_config* config, t_log* logger);

void funcSet(t_instruccion *instruccion);
void funcSum(t_instruccion *instruccion);
void funcSub(t_instruccion *instruccion);
void funcJnz(t_instruccion *instruccion);
void funcIoGenSleep(t_instruccion *instruccion);

op_code decode(t_instruccion *instruccion);
t_instruccion* pedir_instruccion_memoria(uint32_t pid, uint32_t pc); 
void execute(op_code instruccion_nombre, t_instruccion* instruccion);
uint8_t obtener_valor_registro_XX(char* parametro);
uint32_t obtener_valor_registro_XXX(char* parametro);
t_instruccion* fetch(uint32_t pid, uint32_t pc);
#endif