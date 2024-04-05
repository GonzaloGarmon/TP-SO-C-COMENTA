#include <entradasalida.h>

int main(int argc, char* argv[]) {
    
    log_entradasalida = log_create("./entradasalida.log", "ENTRADASALIDA", 1, LOG_LEVEL_TRACE);

    log_info(log_entradasalida, "INICIA EL MODULO DE ENTRADASALIDA");

    config_entradasalida = iniciar_config("/home/utnso/tp-2024-1c-GoC/entradasalida/config/entrada.config");

    tipo_interfaz = config_get_string_value(config_entradasalida, "TIPO_INTERFAZ");
    tiempo_unidad_trabajo = config_get_string_value(config_entradasalida, "TIEMPO_UNIDAD_TRABAJO");
    ip_kernel = config_get_string_value(config_entradasalida, "IP_KERNEL");
    puerto_kernel = config_get_string_value(config_entradasalida, "PUERTO_KERNEL");
    ip_memoria = config_get_string_value(config_entradasalida, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_entradasalida, "PUERTO_MEMORIA");
    path_base_dialfs = config_get_string_value(config_entradasalida, "PATH_BASE_DIALFS");
    block_size = config_get_string_value(config_entradasalida, "BLOCK_SIZE");
    block_count = config_get_string_value(config_entradasalida, "BLOCK_COUNT");


    establecer_conexion(ip_kernel, puerto_kernel, config_entradasalida, log_entradasalida);


    return 0;
}


void establecer_conexion(char * ip_kernel, char* puerto_kernel, t_config* config, t_log* loggs){


    log_trace(loggs, "Inicio como cliente");

    log_trace(loggs,"Lei la IP %s , el Puerto Kernel %s ", ip_kernel, puerto_kernel);

    // Enviamos al servidor el valor de ip como mensaje si es que levanta el cliente
    if((conexion_entradasalida = crear_conexion(ip_kernel, puerto_kernel)) == -1){
        log_trace(loggs, "Error al conectar con Kernel. El servidor no esta activo");

        exit(2);
    }

    //recibir_operacion(conexion_entradasalida);
    recibir_mensaje(conexion_entradasalida,loggs);
}

