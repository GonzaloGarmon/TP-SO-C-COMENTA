#include <memoria.h>

int main(int argc, char* argv[]) {
    
    log_memoria = log_create("./memoria.log", "MEMORIA", 1, LOG_LEVEL_TRACE);

    log_info(log_memoria, "INICIA EL MODULO DE MEMORIA");

    config_memoria = iniciar_config("/home/utnso/tp-2024-1c-GoC/memoria/config/memoria.config");

    puerto_escucha = config_get_string_value(config_memoria, "PUERTO_ESCUCHA");
    tam_memoria = config_get_string_value(config_memoria, "TAM_MEMORIA");
    tam_pagina = config_get_string_value(config_memoria, "TAM_PAGINA");
    path_instrucciones = config_get_string_value(config_memoria, "PATH_INSTRUCCIONES");
    retardo_respuesta = config_get_string_value(config_memoria, "RETARDO_RESPUESTA");
    
    log_info(log_memoria, "levanto la configuracion de memoria");

    socket_servidor_memoria_dispatch = iniciar_servidor(puerto_escucha, log_memoria);

    log_info(log_memoria, "Inicia el servidor de memoria");

    pthread_t atiende_cliente_cpu, atiende_cliente_kernel, atiende_cliente_entradasalida;

    log_info(log_memoria, "Listo para recibir a CPU");
    socket_cliente_cpu = esperar_cliente(socket_servidor_memoria_dispatch);
    pthread_create(&atiende_cliente_cpu, NULL, (void *)recibir_cpu, (void *) socket_cliente_cpu);
    pthread_detach(atiende_cliente_cpu);
    
    log_info(log_memoria, "Listo para recibir a Kernel");
    socket_cliente_kernel = esperar_cliente(socket_servidor_memoria_dispatch);
    pthread_create(&atiende_cliente_kernel, NULL, (void *)recibir_kernel, (void *) socket_cliente_kernel);
    pthread_detach(atiende_cliente_kernel);

    log_info(log_memoria, "Listo para recibir a EntradaSalida");
    socket_cliente_entradasalida = esperar_cliente(socket_servidor_memoria_dispatch);
    pthread_create(&atiende_cliente_entradasalida, NULL, (void *)recibir_entradasalida, (void *) socket_cliente_entradasalida);
    pthread_detach(atiende_cliente_entradasalida);

    log_info(log_memoria, "Finalizo conexion con clientes");

    return 0;
}


void recibir_kernel(int SOCKET_CLIENTE_KERNEL){
    enviar_mensaje("recibido kernel", SOCKET_CLIENTE_KERNEL);
}

void recibir_cpu(int SOCKET_CLIENTE_CPU){
    enviar_mensaje("recibido cpu", SOCKET_CLIENTE_CPU);
}

void recibir_entradasalida(int SOCKET_CLIENTE_ENTRADASALIDA){
    enviar_mensaje("recibido entrada salida", SOCKET_CLIENTE_ENTRADASALIDA);
}