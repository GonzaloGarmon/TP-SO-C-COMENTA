#include <memoria.h>

int main(int argc, char* argv[]) {
    
    log_memoria = log_create("./memoria.log", "MEMORIA", 1, LOG_LEVEL_TRACE);

    log_info(log_memoria, "INICIA EL MODULO DE MEMORIA");

    config_memoria = iniciar_config("/home/utnso/tp-2024-1c-GoC/memoria/config/memoria.config");

    puerto_escucha = config_get_string_value(config_memoria, "PUERTO_ESCUCHA");
    tam_memoria = config_get_int_value(config_memoria, "TAM_MEMORIA");
    tam_pagina = config_get_int_value(config_memoria, "TAM_PAGINA");
    path_instrucciones = config_get_string_value(config_memoria, "PATH_INSTRUCCIONES");
    retardo_respuesta = config_get_int_value(config_memoria, "RETARDO_RESPUESTA");
    
    log_info(log_memoria, "levanto la configuracion de memoria");

    socket_servidor_memoria_dispatch = iniciar_servidor(puerto_escucha, log_memoria);

    log_info(log_memoria, "Inicia el servidor de memoria");
    
    pthread_t atiende_cliente_cpu, atiende_cliente_kernel, atiende_cliente_entradasalida;

    log_info(log_memoria, "Listo para recibir a CPU");
    socket_cliente_cpu = esperar_cliente(socket_servidor_memoria_dispatch);
    pthread_create(&atiende_cliente_cpu, NULL, (void *)recibir_cpu, (void *) (intptr_t) socket_cliente_cpu);
    pthread_detach(atiende_cliente_cpu);
    
    log_info(log_memoria, "Listo para recibir a Kernel");
    socket_cliente_kernel = esperar_cliente(socket_servidor_memoria_dispatch);
    pthread_create(&atiende_cliente_kernel, NULL, (void *)recibir_kernel, (void *) (intptr_t) socket_cliente_kernel);
    pthread_detach(atiende_cliente_kernel);

    log_info(log_memoria, "Listo para recibir a EntradaSalida");
    socket_cliente_entradasalida = esperar_cliente(socket_servidor_memoria_dispatch);
    pthread_create(&atiende_cliente_entradasalida, NULL, (void *)recibir_entradasalida, (void *) (intptr_t) socket_cliente_entradasalida);
    pthread_detach(atiende_cliente_entradasalida);

    log_info(log_memoria, "Finalizo conexion con clientes");

    return 0;
}


void recibir_kernel(int SOCKET_CLIENTE_KERNEL){
    enviar_string(SOCKET_CLIENTE_KERNEL, "salame con patas", MENSAJE);
}

void recibir_cpu(int SOCKET_CLIENTE_CPU){
    t_instruccion* instrucciones[instrucciones_maximas];
    cargar_instrucciones_desde_archivo(path_instrucciones, instrucciones);

    //PRIMERO DEBERIA OBTENER EL PROGRAM COUNTER Y EL PROCESS ID PARA SABER QUE INSTRUCCION DEVOLVER

    //enviar_instruccion(socket_cliente_cpu, instrucciones[pc],)
}

void recibir_entradasalida(int SOCKET_CLIENTE_ENTRADASALIDA){
    enviar_string(SOCKET_CLIENTE_ENTRADASALIDA, "salame con patas", MENSAJE);
}

//Chequear los tamanios maximos de todo.
int longitud_maxima = 100;
int parametros_maximos = 10;
int instrucciones_maximas = 100;

t_instruccion *instrucciones[instrucciones_maximas];
//t_instruccion ins a enviar = instrucciones[pid];

void* cargar_instrucciones_desde_archivo(char* nombre_archivo, t_instruccion* instrucciones[instrucciones_maximas]) {
    FILE* archivo = fopen(nombre_archivo, "r");
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    int indice_instruccion = 0;
    char linea[longitud_maxima];

    while (fgets(linea, longitud_maxima, archivo) != NULL && indice_instruccion < instrucciones_maximas) {
        t_instruccion* instruccion = malloc(sizeof(t_instruccion));
        char* token = strtok(linea, " \t\n");
        int param_count = 0;

        while (token != NULL && param_count < parametros_maximos) {
            switch (param_count) {
                case 0:
                    instruccion->parametros1 = strdup(token);
                    break;
                case 1:
                    instruccion->parametros2 = strdup(token);
                    break;
                case 2:
                    instruccion->parametros3 = strdup(token);
                    break;
                case 3:
                    instruccion->parametros4 = strdup(token);
                    break;
                case 4:
                    instruccion->parametros5 = strdup(token);
                    break;
                case 5:
                    instruccion->parametros6 = strdup(token);
                    break;
                default:
                    break;
            }
            token = strtok(NULL, " \t\n");
            param_count++;
        }
        instrucciones[indice_instruccion] = instruccion;
        indice_instruccion++;
    }

    fclose(archivo);
    return instrucciones;
}