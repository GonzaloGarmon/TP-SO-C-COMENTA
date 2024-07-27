#ifndef CPU_H_
#define CPU_H_
#include <utils/utils.h>

//estructura tlb
typedef struct {
    uint32_t pid;
    uint32_t numero_de_pagina;
    uint32_t marco;
    uint32_t contador_reciente; // LRU
}tlb;

tlb entrada_tlb[]; //verificar que funcione
t_log* log_cpu;
t_config* config_cpu;
t_contexto *contexto;
uint32_t pid_interrupt;
int hay_interrupcion;
int es_por_usuario;

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
int indice_frente;

void leer_config(char* path);

void generar_conexiones();

void terminar_programa();

void recibir_kernel_dispatch(int SOCKET_CLIENTE_KERNEL);
void recibir_kernel_interrupt(int SOCKET_CLIENTE_KERNEL_INTERRUPT);
void establecer_conexion(char * ip_memoria, char* puerto_memoria, t_config* config, t_log* logger);

void funcSet(t_instruccion *instruccion);
void funcSum(t_instruccion *instruccion);
void funcSub(t_instruccion *instruccion);
void funcJnz(t_instruccion *instruccion);
void funcWait(t_instruccion *instruccion);
void funcExit(t_instruccion *instruccion);
void funcResize(t_instruccion *instruccion);
void funcCopyString(t_instruccion *instruccion);
void funcMovIn(t_instruccion *instruccion);
void funcMovOut(t_instruccion* instruccion);
void funcSignal(t_instruccion *instruccion);
void funcIoGenSleep(t_instruccion *instruccion);
void funcIoStdinRead(t_instruccion *instruccion);
void funcIoStdOutWrite(t_instruccion *instruccion);
void funcIoFsCreate(t_instruccion *instruccion);
void funcIoFsDelete(t_instruccion *instruccion);
void funcIoFsTruncate(t_instruccion *instruccion);
void funcIoFsRead(t_instruccion *instruccion);
void funcIoFsWrite(t_instruccion *instruccion);


op_code decode(t_instruccion *instruccion);
void agregar_valor_a_registro(char *reg, char *val);
void agregar_entrada_tlb(uint32_t pid, uint32_t marco, uint32_t pagina);
char *leer_valor_de_registro(char *registro) ;
void guardar_valor_en_registro(char *valor, char *registro);
char * leer_valor_de_memoria(uint32_t direccionFisica, uint32_t tamanio);
void escribir_valor_en_memoria(uint32_t direccionFisica, char *valor, int tamanio);
uint32_t tamanio_registro(char *registro);
void pedir_instruccion_memoria(uint32_t pid, uint32_t pc, t_log *logg); 
void execute(op_code instruccion_nombre, t_instruccion* instruccion);
uint8_t obtener_valor_registro_XX(char* parametro);
uint32_t obtener_valor_registro_XXX(char* parametro);
t_instruccion* fetch(uint32_t pid, uint32_t pc);
void checkInturrupt(uint32_t pid);
void esperar_devolucion_pcb();
uint32_t obtener_valor_registro(char* registro);
void ejecutar_ciclo_de_instruccion(t_log *loggs);
void reemplazarXLRU(uint32_t pid, uint32_t marco, uint32_t pagina);
void reemplazarXFIFO(uint32_t pid, uint32_t marco, uint32_t pagina);
uint32_t traducirDireccion(uint32_t dirLogica, uint32_t tamanio_pagina);

#endif