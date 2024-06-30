#include "entradasalida.h"

int main(int argc, char* argv[]) {
    log_entradasalida = log_create("./entradasalida.log", "ENTRADASALIDA", 1, LOG_LEVEL_TRACE);
    log_info(log_entradasalida, "INICIA EL MODULO DE ENTRADASALIDA");
    inicializar_registro();
    leer_config();
    generar_conexiones();
    log_info(log_entradasalida, "Finalizo conexion con servidores");
    finalizar_programa();
    return 0;
}

void leer_config(){
    config_entradasalida = iniciar_config("/home/utnso/tp-2024-1c-GoC/entradasalida/config/entrada.config");
    
    tipo_interfaz = config_get_string_value(config_entradasalida, "TIPO_INTERFAZ");
    tiempo_unidad_trabajo = config_get_int_value(config_entradasalida, "TIEMPO_UNIDAD_TRABAJO");
    ip_kernel = config_get_string_value(config_entradasalida, "IP_KERNEL");
    puerto_kernel = config_get_string_value(config_entradasalida, "PUERTO_KERNEL");
    ip_memoria = config_get_string_value(config_entradasalida, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_entradasalida, "PUERTO_MEMORIA");
    path_base_dialfs = config_get_string_value(config_entradasalida, "PATH_BASE_DIALFS");
    block_size = config_get_int_value(config_entradasalida, "BLOCK_SIZE");
    block_count = config_get_int_value(config_entradasalida, "BLOCK_COUNT");
    
}



//SIN TERMINAR
/*
void leer_config() {
    const char *nombre_directorio = "/home/utnso/tp-2024-1c-GoC/entradasalida/config";
    struct dirent *entrada;
    DIR *directorio = opendir(nombre_directorio);

    if (directorio == NULL) {
        perror("No se puede abrir el directorio");
        exit(EXIT_FAILURE);
    }

    while ((entrada = readdir(directorio)) != NULL) {
        if (tiene_extension_config(entrada->d_name)) {
            char ruta_archivo[1024];
            snprintf(ruta_archivo, sizeof(ruta_archivo), "%s/%s", nombre_directorio, entrada->d_name);
            procesar_archivo_config(ruta_archivo);
        }
    }

    closedir(directorio);
}


void procesar_archivo_config(const char* ruta_archivo) {
    t_config* config = config_create((char*)ruta_archivo);
    char* tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");
    if (tipo_interfaz == NULL) {
        printf("El archivo de configuración %s no tiene la clave TIPO_INTERFAZ\n", ruta_archivo);
        config_destroy(config);
        return;
    }

    // Validar las claves IP_KERNEL y PUERTO_KERNEL siempre
    char* ip_kernel_config = config_get_string_value(config, "IP_KERNEL");
    char* puerto_kernel_config = config_get_string_value(config, "PUERTO_KERNEL");
    char* ip_memoria_config = config_get_string_value(config, "IP_MEMORIA");
    char* puerto_memoria_config = config_get_string_value(config, "PUERTO_MEMORIA");

    if (ip_kernel_config == NULL || puerto_kernel_config == NULL || ip_memoria_config == NULL || puerto_memoria_config == NULL) {
        printf("El archivo de configuración %s no tiene las claves IP_KERNEL, PUERTO_KERNEL, IP_MEMORIA o PUERTO_MEMORIA\n", ruta_archivo);
        config_destroy(config);
        return;
    }

    if (strcmp(tipo_interfaz, "GENERICA") == 0) {
        InterfazGenerica* interfaz = malloc(sizeof(InterfazGenerica));
        inicializar_interfaz_generica(config, interfaz, "Interfaz Generica");
        conectar_interfaz("Interfaz Generica");
    } else if (strcmp(tipo_interfaz, "STDIN") == 0) {
        STDIN* interfaz = malloc(sizeof(STDIN));
        inicializar_interfaz_stdin(config, interfaz, "Interfaz Stdin");
        conectar_interfaz("Interfaz Stdin");
    } else if (strcmp(tipo_interfaz, "STDOUT") == 0) {
        STDOUT* interfaz = malloc(sizeof(STDOUT));
        inicializar_interfaz_stdout(config, interfaz, "Interfaz Stdout");
        conectar_interfaz("Interfaz Stdout");
    } else {
        printf("Tipo de interfaz desconocido: %s\n", tipo_interfaz);
    }

    config_destroy(config);
}
*/

void finalizar_programa() {
    log_destroy(log_entradasalida);
    config_destroy(config_entradasalida);
    liberar_registro();
}

void generar_conexiones() {
    if (strcmp(tipo_interfaz, "GENERICA") != 0) {
        establecer_conexion_memoria(ip_memoria, puerto_memoria, config_entradasalida, log_entradasalida);
        establecer_conexion_kernel(ip_kernel, puerto_kernel, config_entradasalida, log_entradasalida);
    }
}

void establecer_conexion_kernel(char* ip_kernel, char* puerto_kernel, t_config* config, t_log* loggs) {
    log_trace(loggs, "Inicio como cliente");

    if ((conexion_entradasalida_kernel = crear_conexion(ip_kernel, puerto_kernel)) == -1) {
        log_trace(loggs, "Error al conectar con Kernel. El servidor no está activo");
        exit(EXIT_FAILURE);
    }

    int operacion = recibir_operacion(conexion_entradasalida_kernel);

    if (operacion == -1) {
        log_trace(loggs, "Error al recibir operación");
        exit(EXIT_FAILURE);
    }
}

void establecer_conexion_memoria(char* ip_memoria, char* puerto_memoria, t_config* config, t_log* loggs) {
    log_trace(loggs, "Inicio como cliente");

    if ((conexion_entradasalida_memoria = crear_conexion(ip_memoria, puerto_memoria)) == -1) {
        log_trace(loggs, "Error al conectar con Memoria. El servidor no está activo");
        exit(EXIT_FAILURE);
    }

    int operacion = recibir_operacion(conexion_entradasalida_memoria);

    if (operacion == -1) {
        log_trace(loggs, "Error al recibir operación");
        exit(EXIT_FAILURE);
    }
}

void inicializar_interfaz_generica(t_config *config, InterfazGenerica *interfazGen, const char *nombre) {
    if (interfazGen == NULL) {
        fprintf(stderr, "Interfaz vacia\n");
        return;
    }
    // Inicializar atributos de la interfaz genérica
    interfazGen->nombre = strdup(nombre);
    interfazGen->tiempo_unidad_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    interfazGen->ip_kernel = config_get_string_value(config, "IP_KERNEL");
    interfazGen->puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    interfazGen->enUso = false;
}

void inicializar_interfaz_stdin(t_config *config, STDIN *interfazStdin, const char *nombre) {
    if (interfazStdin == NULL) {
        fprintf(stderr, "Interfaz vacia\n");
        return;
    }
    // Inicializar atributos de la interfaz stdin
    interfazStdin->nombre = strdup(nombre);
    interfazStdin->tiempo_unidad_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    interfazStdin->ip_kernel = config_get_string_value(config, "IP_KERNEL");
    interfazStdin->puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    interfazStdin->ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    interfazStdin->puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    interfazStdin->enUso = false;
}

void inicializar_interfaz_stdout(t_config *config, STDOUT *interfazStdout, const char *nombre) {
    if (interfazStdout == NULL) {
        fprintf(stderr, "Interfaz vacia\n");
        return;
    }
    // Inicializar atributos de la interfaz stdout
    interfazStdout->nombre = strdup(nombre);
    interfazStdout->tiempo_unidad_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    interfazStdout->ip_kernel = config_get_string_value(config, "IP_KERNEL");
    interfazStdout->puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    interfazStdout->ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    interfazStdout->puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    interfazStdout->enUso = false;
}

bool validar_interfaz(ListaIO* interfaces, int num_interfaces, char* nombre_solicitado) {
    for (int i = 0; i < num_interfaces; i++) {
        if (strcmp(interfaces[i].nombre, nombre_solicitado) == 0) {
            return true;
        }
    }
    return false;
}

void validar_operacion_io(void *interfaz, OperacionIO operacion) {
    if (interfaz == NULL) {
        log_trace(log_entradasalida, "Interfaz no válida");
        return;
    }

    InterfazGenerica *interfazGen = (InterfazGenerica *)interfaz;
    if (!es_operacion_compatible(interfazGen->tipo, operacion)) {
        log_trace(log_entradasalida, "Operación no compatible con la interfaz");
    } else {
        log_trace(log_entradasalida, "Operación válida para la interfaz");
    }
}

bool es_operacion_compatible(TipoInterfaz tipo, OperacionIO operacion) {
    return true; // Simplificado para este ejemplo
}

void solicitar_operacion_io(void *interfaz, OperacionIO operacion) {
    pthread_mutex_lock(&mutex);
    validar_operacion_io(interfaz, operacion);
    pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
}

void operacion_io_finalizada() {
    pthread_mutex_lock(&mutex);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

void inicializar_registro() {
    registro.capacidad = 10;
    registro.cantidad = 0;
    registro.interfaces = malloc(registro.capacidad * sizeof(ListaIO));
    if (registro.interfaces == NULL) {
        perror("Error al asignar memoria para registro.interfaces");
        exit(EXIT_FAILURE);
    }
}

void liberar_registro() {
    if (registro.interfaces == NULL) {
        fprintf(stderr, "Error: registro.interfaces no ha sido inicializado correctamente\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < registro.cantidad; i++) {
        free(registro.interfaces[i].nombre);
    }
    free(registro.interfaces);
}

void conectar_interfaz(char* nombre_interfaz) {
    if (registro.interfaces == NULL) {
        fprintf(stderr, "Error: registro.interfaces no ha sido inicializado correctamente\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < registro.cantidad; i++) {
        if (registro.interfaces[i].nombre != NULL && strcmp(registro.interfaces[i].nombre, nombre_interfaz) == 0) {
            registro.interfaces[i].conectada = true;
            break; // Salir del bucle al encontrar la interfaz
        }
    }
}
void desconectar_interfaz(char* nombre_interfaz) {
    for (int i = 0; i < registro.cantidad; i++) {
        if (strcmp(registro.interfaces[i].nombre, nombre_interfaz) == 0) {
            registro.interfaces[i].conectada = false;
        }
    }
}

bool interfaz_conectada(char* nombre_interfaz) {
    for (int i = 0; i < registro.cantidad; i++) {
        if (strcmp(registro.interfaces[i].nombre, nombre_interfaz) == 0) {
            return registro.interfaces[i].conectada;
        }
    }
    return false;
}

void esperar_interfaz_libre() {
    pthread_mutex_lock(&mutex);
    while (true) {
        bool interfaz_libre = false;
        for (int i = 0; i < registro.cantidad; i++) {
            if (!registro.interfaces[i].conectada) {
                interfaz_libre = true;
                break;
            }
        }

        if (interfaz_libre) {
            break;
        } else {
            pthread_cond_wait(&cond, &mutex);
        }
    }
    pthread_mutex_unlock(&mutex);
}

void* crear_interfaz(TipoInterfaz tipo) {
    void* interfaz = NULL;
    switch (tipo) {
        case TIPO_GENERICA:
            interfaz = malloc(sizeof(InterfazGenerica));
            inicializar_interfaz_generica(config_entradasalida, (InterfazGenerica*)interfaz, "Interfaz Generica");
            break;
        case TIPO_STDIN:
            interfaz = malloc(sizeof(STDIN));
            inicializar_interfaz_stdin(config_entradasalida, (STDIN*)interfaz, "Interfaz Stdin");
            break;
        case TIPO_STDOUT:
            interfaz = malloc(sizeof(STDOUT));
            inicializar_interfaz_stdout(config_entradasalida, (STDOUT*)interfaz, "Interfaz Stdout");
            break;
        default:
            break;
    }
    return interfaz;
}

void liberar_interfaz(void* interfaz, TipoInterfaz tipo) {
    if (interfaz != NULL) {
        switch (tipo) {
            case TIPO_GENERICA:
                free(((InterfazGenerica*)interfaz)->nombre);
                free(((InterfazGenerica*)interfaz)->ip_kernel);
                free(((InterfazGenerica*)interfaz)->puerto_kernel);
                free(interfaz);
                break;
            case TIPO_STDIN:
                free(((STDIN*)interfaz)->nombre);
                free(((STDIN*)interfaz)->ip_kernel);
                free(((STDIN*)interfaz)->puerto_kernel);
                free(interfaz);
                break;
            case TIPO_STDOUT:
                free(((STDOUT*)interfaz)->nombre);
                free(((STDOUT*)interfaz)->ip_kernel);
                free(((STDOUT*)interfaz)->puerto_kernel);
                free(interfaz);
                break;
            default:
                break;
        }
    }
}

bool tiene_extension_config(const char* filename) {
    const char* dot = strrchr(filename, '.');
    return (dot && strcmp(dot, ".config") == 0);
}
