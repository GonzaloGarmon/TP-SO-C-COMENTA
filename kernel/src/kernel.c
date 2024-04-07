#include <kernel.h>

int main(int argc, char* argv[]) {

    log_kernel = log_create("./kernel.log", "KERNEL", 1, LOG_LEVEL_TRACE);

    log_info(log_kernel, "INICIA EL MODULO DE KERNEL");

    config_kernel = iniciar_config("./config/kernel.config");
        
    puerto_escucha = config_get_string_value(config_kernel, "PUERTO_ESCUCHA");
    ip_memoria = config_get_string_value(config_kernel, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_kernel, "PUERTO_MEMORIA");
    ip_cpu = config_get_string_value(config_kernel, "IP_CPU");
    puerto_cpu_interrupt = config_get_string_value(config_kernel, "PUERTO_CPU_INTERRUPT");
    puerto_cpu_dispatch = config_get_string_value(config_kernel, "PUERTO_CPU_DISPATCH");
    algoritmo_planificacion = config_get_string_value(config_kernel, "ALGORITMO_PLANIFICACION");
    quantum = config_get_string_value(config_kernel, "QUANTUM");
    recursos = config_get_string_value(config_kernel, "RECURSOS");
    instancias_recursos = config_get_string_value(config_kernel, "INSTANCIAS_RECURSOS");
    grado_multiprogramacion = config_get_string_value(config_kernel, "GRADO_MULTIPROGRAMACION");

    log_info(log_kernel, "levanto la configuracion del kernel");

    establecer_conexion_memoria(ip_memoria, puerto_memoria, config_kernel, log_kernel);

    establecer_conexion_cpu(ip_cpu, puerto_cpu_dispatch, config_kernel, log_kernel);

    socket_servidor_kernel_dispatch = iniciar_servidor(puerto_escucha, log_kernel);

    log_info(log_kernel, "INICIO SERVIDOR");

    log_info(log_kernel, "Listo para recibir a EntradaSalida");

    socket_cliente_entradasalida = esperar_cliente(socket_servidor_kernel_dispatch);

    pthread_t atiende_cliente_entradasalida;
    pthread_create(&atiende_cliente_entradasalida, NULL, (void *)recibir_entradasalida, (void *) socket_cliente_entradasalida);
    pthread_detach(atiende_cliente_entradasalida);
    
    log_info(log_kernel, "finalizo conexion con cliente");


    
    return 0;
}

void recibir_entradasalida(int SOCKET_CLIENTE_ENTRADASALIDA){
    enviar_mensaje("recibido entradasalida", SOCKET_CLIENTE_ENTRADASALIDA);
}

void establecer_conexion_cpu(char * ip_cpu, char* puerto_cpu_dispatch, t_config* config, t_log* loggs){

    log_trace(loggs, "Inicio como cliente");

    log_trace(loggs,"Lei la IP %s , el Puerto CPU %s ", ip_cpu, puerto_cpu_dispatch);

    // Enviamos al servidor el valor de ip como mensaje si es que levanta el cliente
    if((conexion_kernel = crear_conexion(ip_cpu, puerto_cpu_dispatch)) == -1){
        log_trace(loggs, "Error al conectar con CPU. El servidor no esta activo");

        exit(2);
    }

    //recibir_operacion(conexion_kernel);
    recibir_mensaje(conexion_kernel,loggs);
}

void establecer_conexion_memoria(char * ip_memoria, char* puerto_memoria_dispatch, t_config* config, t_log* loggs){


    log_trace(loggs, "Inicio como cliente");

    log_trace(loggs,"Lei la IP %s , el Puerto Memoria %s ", ip_memoria, puerto_memoria_dispatch);

    // Enviamos al servidor el valor de ip como mensaje si es que levanta el cliente
    if((conexion_kernel = crear_conexion(ip_memoria, puerto_memoria_dispatch)) == -1){
        log_trace(loggs, "Error al conectar con Memoria. El servidor no esta activo");

        exit(2);
    }

    //recibir_operacion(conexion_kernel);
    recibir_mensaje(conexion_kernel,loggs);
}


