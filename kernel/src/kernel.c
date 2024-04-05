#include <kernel.h>

int main(int argc, char* argv[]) {


log_kernel = log_create("./kernel.log", "KERNEL", 1, LOG_LEVEL_INFO);

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


    establecer_conexion(ip_cpu, puerto_cpu_dispatch, config_kernel, log_kernel);
    

    return 0;
}


void establecer_conexion(char * ip_cpu, char* puerto_cpu_dispatch, t_config* config, t_log* logger){


    log_trace(logger, "Inicio como cliente");

    log_trace(logger,"Lei la IP %s , el Puerto Memoria %s ", ip_cpu, puerto_cpu_dispatch);

    // Enviamos al servidor el valor de ip como mensaje si es que levanta el cliente
    if((conexion_kernel = crear_conexion(ip_cpu, puerto_cpu_dispatch)) == -1){
        log_trace(logger, "Error al conectar con Memoria. El servidor no esta activo");

        exit(2);
    }

    recibir_operacion(conexion_kernel);
    recibir_mensaje(conexion_kernel);
}

