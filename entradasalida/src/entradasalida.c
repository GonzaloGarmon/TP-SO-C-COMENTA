#include <entradasalida.h>

int main(int argc, char* argv[]) {
    // Crear el logger para el módulo de entradasalida
    log_entradasalida = log_create("./entradasalida.log", "ENTRADASALIDA", 1, LOG_LEVEL_TRACE);

    // Registrar inicio del módulo en el log
    log_info(log_entradasalida, "INICIA EL MODULO DE ENTRADASALIDA");

    // Leer la configuración desde el archivo
    leer_config();

    // Generar conexiones con la memoria y el kernel
    generar_conexiones();

    // Registrar finalización de la conexión en el log
    log_info(log_entradasalida, "Finalizo conexion con servidores");

    // Liberar recursos utilizados por la interfaz y finalizar el programa
    liberar_interfaz(  , tipo_interfaz); //no va aca
    finalizar_programa();

    return 0;
}

/*
------------------------CONFIGS, INICIACION, COMUNICACIONES-------------------------------------
*/

// Función para leer la configuración desde el archivo
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

void procesar_archivo_config(const char *ruta_archivo) {
    t_config* config = config_create((char *)ruta_archivo);

    // Obtener el tipo de interfaz
    char *tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");
    if (tipo_interfaz == NULL) {
        printf("El archivo de configuración %s no tiene la clave TIPO_INTERFAZ\n", ruta_archivo);
        config_destroy(config);
        return;
    }

    // Inicializar la interfaz según el tipo
    if (strcmp(tipo_interfaz, "GENERICA") == 0) {
        InterfazGenerica *interfaz = malloc(sizeof(InterfazGenerica));
        inicializar_interfaz_generica(config, interfaz, "Interfaz Generica");
        conectar_interfaz("Interfaz Generica");

    } else if (strcmp(tipo_interfaz, "STDIN") == 0) {
        STDIN *interfaz = malloc(sizeof(STDIN));
        inicializar_interfaz_stdin(config, interfaz, "Interfaz Stdin");
        conectar_interfaz("Interfaz Stdin");

    } else if (strcmp(tipo_interfaz, "STDOUT") == 0) {
        STDOUT *interfaz = malloc(sizeof(STDOUT));
        inicializar_interfaz_stdout(config, interfaz, "Interfaz Stdout");
        conectar_interfaz("Interfaz Stdout");

    } else {
        printf("Tipo de interfaz desconocido: %s\n", tipo_interfaz);
    }

    // Liberar la memoria de t_config
    config_destroy(config);
}


void generar_conexiones() {
    establecer_conexion_memoria(ip_memoria, puerto_memoria, config_entradasalida, log_entradasalida);
    establecer_conexion_kernel(ip_kernel, puerto_kernel, config_entradasalida, log_entradasalida);
}

void finalizar_programa() {
    log_destroy(log_entradasalida);
    config_destroy(config_entradasalida);
    liberar_registro();
}

void establecer_conexion_kernel(char* ip_kernel, char* puerto_kernel, t_config* config, t_log* loggs) {
    log_trace(loggs, "Inicio como cliente");

    // Establecer conexión con el kernel
    if ((conexion_entradasalida_kernel = crear_conexion(ip_kernel, puerto_kernel)) == -1) {
        log_trace(loggs, "Error al conectar con Kernel. El servidor no está activo");
        exit(EXIT_FAILURE);
    }

    // Esperar y recibir una operación desde el kernel
    int operacion = recibir_operacion(conexion_entradasalida_kernel);

    // Verificar si se recibió una operación válida
    if (operacion == -1) {
        log_trace(loggs, "Error al recibir operación del Kernel. Cerrando conexión.");
        return;
    }

    // Procesar la operación recibida (completar según las operaciones disponibles)
    switch (operacion) {
        case IO_GEN_SLEEP:
            log_trace(loggs, "Operación requerida recibida");
            break;
        default:
            log_trace(loggs, "Operación no reconocida recibida desde Kernel");
    }
}

void establecer_conexion_memoria(char * ip_memoria, char* puerto_memoria, t_config* config, t_log* loggs){
    log_trace(loggs, "Inicio como cliente");

    // Establecer conexión con la memoria usando la IP y el puerto obtenidos de la configuración
    if ((conexion_entradasalida_memoria = crear_conexion(ip_memoria, puerto_memoria)) == -1){
        log_trace(loggs, "Error al conectar con Memoria. El servidor no está activo");
        exit(EXIT_FAILURE);
    }

    // Realizar operaciones de recepción de datos de la memoria si es necesario
    recibir_operacion(conexion_entradasalida_memoria);
    recibir_string(conexion_entradasalida_memoria, loggs);
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
            if (interfaces[i].conectada) {
                return true;
            } else {
                printf("La interfaz %s no está conectada.\n", nombre_solicitado);
                return false; // En una aplicación real, quizás sería adecuado terminar el programa en este punto
            }
        }
    }
    printf("La interfaz %s no existe.\n", nombre_solicitado);
    return false; // En una aplicación real, quizás sería adecuado terminar el programa en este punto
}

void validar_operacion_io(void *interfaz, OperacionIO operacion) {
    // Aquí deberías validar la operación según el tipo de interfaz y la operación solicitada
    // Esto es una estructura básica, adaptar según sea necesario
    TipoInterfaz tipo = TIPO_GENERICA;  // Cambiar esto por la función que obtiene el tipo de interfaz
    if (!es_operacion_compatible(tipo, operacion)) {
        fprintf(stderr, "La operación solicitada no es compatible con el tipo de interfaz.\n");
        exit(EXIT_FAILURE);
    }
}

bool es_operacion_compatible(TipoInterfaz tipo, OperacionIO operacion) {
    // Aquí implementar la lógica para verificar si la operación es compatible con el tipo de interfaz
    switch (tipo) {
        case TIPO_GENERICA:
            return operacion == IO_GEN_SLEEP;
        case TIPO_STDIN:
            return operacion == STDIN_READ;
        case TIPO_STDOUT:
            return operacion == STDOUT_WRITE;
        default:
            return false;
    }
}

void solicitar_operacion_io(void *interfaz, OperacionIO operacion) {
    // Implementación básica, adaptar según sea necesario
    if (interfaz == NULL) {
        fprintf(stderr, "Interfaz no válida.\n");
        exit(EXIT_FAILURE);
    }

    // Aquí deberías implementar la lógica para realizar la operación de entrada/salida según el tipo de interfaz
    // Esto es un ejemplo básico
    if (!validar_interfaz(registro.interfaces, registro.cantidad, "nombre_interfaz")) {
        fprintf(stderr, "La interfaz no está conectada o no existe.\n");
        exit(EXIT_FAILURE);
    }

    // Realizar operación de entrada/salida
    // ...

    // Marcar la operación como finalizada
    operacion_io_finalizada();
}

void operacion_io_finalizada() {
    // Aquí deberías marcar que la operación de entrada/salida ha finalizado
    // Esto es un ejemplo básico
    registro.interfaces[0].enUso = false;  // Cambiar por la interfaz correcta
}

// Función para inicializar el registro de interfaces
void inicializar_registro() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    registro.capacidad = 10;
    registro.cantidad = 0;
    registro.interfaces = malloc(registro.capacidad * sizeof(ListaIO));  // Cambiar por el tipo correcto
}

void liberar_registro() {
    for (int i = 0; i < registro.cantidad; i++) {
        free(registro.interfaces[i].nombre);
    }
    free(registro.interfaces);
}

void conectar_interfaz(char* nombre_interfaz) {
    if (registro.cantidad == registro.capacidad) {
        // Si el registro está lleno, agrandarlo
        registro.capacidad *= 2;
        registro.interfaces = realloc(registro.interfaces, registro.capacidad * sizeof(ListaIO));  // Cambiar por el tipo correcto
    }
    ListaIO nueva_interfaz;
    nueva_interfaz.nombre = strdup(nombre_interfaz);
    nueva_interfaz.conectada = true;
     registro.interfaces[registro.cantidad++] = nueva_interfaz;
}

void desconectar_interfaz(char* nombre_interfaz) {
    for (int i = 0; i < registro.cantidad; i++) {
        if (strcmp(registro.interfaces[i].nombre, nombre_interfaz) == 0) {
            free(registro.interfaces[i].nombre);
            // Mover la última interfaz al lugar de la interfaz a eliminar
            registro.interfaces[i] = registro.interfaces[--registro.cantidad];
            return;
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
    while (registro.interfaces[0].enUso == true) {  // Cambiar por la interfaz correcta
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);
}

int tiene_extension_config(const char *nombre_archivo) {
    const char *extension = strrchr(nombre_archivo, '.');
    return extension && strcmp(extension, ".config") == 0;
}

void* crear_interfaz(TipoInterfaz tipo) {
    void* interfaz = NULL;
    switch (tipo) {
        case TIPO_GENERICA:
            interfaz = malloc(sizeof(InterfazGenerica));
            break;
        case TIPO_STDIN:
            interfaz = malloc(sizeof(STDIN));
            break;
        case TIPO_STDOUT:
            interfaz = malloc(sizeof(STDOUT));
            break;
        default:
            fprintf(stderr, "Tipo de interfaz no válido\n");
            break;
    }
    return interfaz;
}

void liberar_interfaz(void* interfaz, TipoInterfaz tipo) {
    if (interfaz == NULL) {
        fprintf(stderr, "Interfaz no válida.\n");
        return;
    }

    switch (tipo) {
        case TIPO_GENERICA:
            free(((InterfazGenerica*)interfaz)->nombre);
            break;
        case TIPO_STDIN:
            free(((STDIN*)interfaz)->nombre);
            break;
        case TIPO_STDOUT:
            free(((STDOUT*)interfaz)->nombre);
            break;
        default:
            fprintf(stderr, "Tipo de interfaz no válido\n");
            break;
    }

    free(interfaz);
}
