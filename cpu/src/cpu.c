#include <cpu.h>

int main(int argc, char* argv[]) {
    
    log_cpu = log_create("./cpu.log", "CPU", 1, LOG_LEVEL_TRACE);

    log_info(log_cpu, "INICIA EL MODULO DE CPU");

    config_cpu = iniciar_config("/home/utnso/tp-2024-1c-GoC/cpu/config/cpu.config");

    ip_memoria = config_get_string_value(config_cpu, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_cpu, "PUERTO_MEMORIA");
    puerto_escucha_dispatch = config_get_string_value(config_cpu, "PUERTO_ESCUCHA_DISPATCH");
    puerto_escucha_interrupt = config_get_string_value(config_cpu, "PUERTO_ESCUCHA_INTERRUPT");
    cantidad_entradas_tlb = config_get_string_value(config_cpu, "CANTIDAD_ENTRADAS_TLB");
    algoritmo_tlb = config_get_string_value(config_cpu, "ALGORITMO_TLB");

    log_info(log_cpu, "levanto la configuracion del cpu");

    socket_servidor_cpu_dispatch = iniciar_servidor(puerto_escucha_dispatch, log_cpu);

    log_info(log_cpu, "INICIO SERVIDOR");

    socket_cliente_kernel = esperar_cliente(socket_servidor_cpu_dispatch);

    pthread_t atiende_cliente_memoria, atiende_cliente_kernel;
    pthread_create(&atiende_cliente_kernel, NULL, (void *)recibir_kernel, (void *) socket_cliente_kernel);
    pthread_detach(atiende_cliente_kernel);
    
    log_info(log_cpu, "finalizo conexion con cliente");

    //comentado para que arranque el server y no tire error de conexion
    //establecer_conexion(ip_memoria,puerto_memoria, config_cpu, log_cpu);


    return 0;
}


void recibir_kernel(int SOCKET_CLIENTE_KERNEL){
    enviar_mensaje("recibido kernel", SOCKET_CLIENTE_KERNEL);
}

void establecer_conexion(char * ip_memoria, char* puerto_memoria, t_config* config, t_log* loggs){


    log_trace(loggs, "Inicio como cliente");

    log_trace(loggs,"Lei la IP %s , el Puerto Memoria %s ", ip_memoria, puerto_memoria);

    // Enviamos al servidor el valor de ip como mensaje si es que levanta el cliente
    if((conexion_cpu = crear_conexion(ip_memoria, puerto_memoria)) == -1){
        log_trace(loggs, "Error al conectar con Memoria. El servidor no esta activo");

        exit(2);
    }

    // recibir_operacion(conexion_cpu);
    recibir_mensaje(conexion_cpu, loggs);
}