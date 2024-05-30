#include <memoria.h>

int main(int argc, char* argv[]) {
    
    log_memoria = log_create("./memoria.log", "MEMORIA", 1, LOG_LEVEL_TRACE);

    log_info(log_memoria, "INICIA EL MODULO DE MEMORIA");

    leer_config();

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
    pthread_join(atiende_cliente_entradasalida, NULL);

    log_info(log_memoria, "Finalizo conexion con clientes");
    finalizar_programa();
    return 0;
}

void leer_config(){
    config_memoria = iniciar_config("/home/utnso/tp-2024-1c-GoC/memoria/config/memoria.config");

    puerto_escucha = config_get_string_value(config_memoria, "PUERTO_ESCUCHA");
    tam_memoria = config_get_int_value(config_memoria, "TAM_MEMORIA");
    tam_pagina = config_get_int_value(config_memoria, "TAM_PAGINA");
    path_instrucciones = config_get_string_value(config_memoria, "PATH_INSTRUCCIONES");
    retardo_respuesta = config_get_int_value(config_memoria, "RETARDO_RESPUESTA");
}

void finalizar_programa(){
    liberar_conexion(socket_servidor_memoria_dispatch);
    liberar_conexion(socket_cliente_cpu);
    liberar_conexion(socket_cliente_entradasalida);
    liberar_conexion(socket_cliente_kernel);
    log_destroy(log_memoria);
    config_destroy(config_memoria);
}

void recibir_kernel(int SOCKET_CLIENTE_KERNEL){
    enviar_string(socket_cliente_kernel, "hola desde memoria", MENSAJE);
    int noFinalizar = 0;
    while(noFinalizar != -1){
        op_code codigo = recibir_operacion(SOCKET_CLIENTE_KERNEL);
        switch (codigo)
        {
        case INICIO_NUEVO_PROCESO:
            char* path = recibir_string(SOCKET_CLIENTE_KERNEL, log_memoria);
            
            int longitud = strlen(path) + strlen(path_instrucciones) + 1;
            char* path_completo = malloc(longitud*sizeof(char));
            strcpy(path_completo, path_instrucciones);
            strcat(path_completo, path);
            cargar_instrucciones_desde_archivo(path_completo, instrucciones);
            free(path_completo);
            //se deben crear las estructuras administrativas necesarias para el proceso al que corresponde ese path
            break;
        
        default:
            break;
        }
    }
}

void recibir_cpu(int SOCKET_CLIENTE_CPU){
    enviar_string(socket_cliente_cpu,"hola desde memoria", MENSAJE);
    int noFinalizar = 0;
    while(noFinalizar != -1){
        op_code codigo = recibir_operacion(SOCKET_CLIENTE_CPU);

        switch (codigo)
        {
        case PEDIR_INSTRUCCION_MEMORIA:

            t_list* enteros = recibir_doble_entero(SOCKET_CLIENTE_CPU);
            //RECIBO PID Y PC PRIMERO HAY QUE IDENTIFICAR QUE INSTRUCCIONES CORRESPONDE A PID
            //LUEGO HACER INSTRUCCIONES[PC], DEJO ESTO PARA PODER CONTINUAR CON LA EJECUCION
            //PERO TODAVIA FALTA VER COMO HACER LO DE IDENTIFICAR
            uint32_t ins = list_get(enteros,1);
            enviar_instruccion(SOCKET_CLIENTE_CPU, instrucciones[ins],READY);
            break;
        default:
            break;
        }
    }
}
 

void recibir_entradasalida(int SOCKET_CLIENTE_ENTRADASALIDA){
    enviar_string(socket_cliente_entradasalida, "hola desde memoria", MENSAJE);
    //enviar_string(SOCKET_CLIENTE_ENTRADASALIDA, "salame con patas", MENSAJE);
    int noFinalizar = 0;
    while(noFinalizar != -1){
        int op_code = recibir_operacion(SOCKET_CLIENTE_ENTRADASALIDA);
    }
}


//t_instruccion *instrucciones[instrucciones_maximas];
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