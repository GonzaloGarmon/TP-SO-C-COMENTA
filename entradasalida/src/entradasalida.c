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

    // Crear interfaz según el tipo especificado en la configuración
    void *interfaz = crear_interfaz(tipo_interfaz);

    // Establecer conexión con el kernel usando la interfaz creada
    establecer_conexion_kernel(interfaz, config_entradasalida, log_entradasalida);

    // Registrar finalización de la conexión en el log
    log_info(log_entradasalida, "Finalizo conexion con servidores");

    // Liberar recursos utilizados por la interfaz y finalizar el programa
    liberar_interfaz(interfaz, tipo_interfaz);
    finalizar_programa();

    return 0;
}

/*
------------------------CONFIGS, INICIACION, COMUNICACIONES-------------------------------------
*/

// Función para leer la configuración desde el archivo
void leer_config(){
    config_entradasalida = iniciar_config("/home/utnso/tp-2024-1c-GoC/entradasalida/config/entrada.config");

    // Leer valores de configuración
    tipo_interfaz = config_get_string_value(config_entradasalida, "TIPO_INTERFAZ");
    tiempo_unidad_trabajo = config_get_int_value(config_entradasalida, "TIEMPO_UNIDAD_TRABAJO");
    ip_kernel = config_get_string_value(config_entradasalida, "IP_KERNEL");
    puerto_kernel = config_get_string_value(config_entradasalida, "PUERTO_KERNEL");
    ip_memoria = config_get_string_value(config_entradasalida, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_entradasalida, "PUERTO_MEMORIA");
    path_base_dialfs = config_get_string_value(config_entradasalida, "PATH_BASE_DIALFS");
    block_size = config_get_int_value(config_entradasalida, "BLOCK_SIZE");
    block_count = config_get_int_value(config_entradasalida, "BLOCK_COUNT");
    retraso_compactacion = config_get_int_value(config_entradasalida, "RETRASO_COMPACTACION");

    // Crear interfaz según el tipo especificado en la configuración


    config_GENERICA= iniciar_config("/home/utnso/tp-2024-1c-GoC/entradasalida/config/GENERICA.config");
    
    config_TECLADO= iniciar_config("/home/utnso/tp-2024-1c-GoC/entradasalida/config/TECLADO.config");

    config_MONITOR= iniciar_config("/home/utnso/tp-2024-1c-GoC/entradasalida/config/MONITOR.config");
    
    char *tipo_interfaz = config_get_string_value(config_TECLADO, "TIPO_INTERFAZ");
    void *interfaz = crear_interfaz(tipo_interfaz);
    // Inicializar la interfaz creada según su tipo
        if (strcmp(tipo_interfaz, "GENERICA") == 0) {
            inicializar_interfaz_generica(interfaz, "Interfaz Generica", tiempo_unidad_trabajo);
        } else if (strcmp(tipo_interfaz, "STDIN") == 0) {
            inicializar_interfaz_stdin(interfaz, "Interfaz Stdin");
        } else if (strcmp(tipo_interfaz, "STDOUT") == 0) {
            inicializar_interfaz_stdout(interfaz, "Interfaz Stdout");
        } else if (strcmp(tipo_interfaz, "DIALFS") == 0) {
            // Aquí se podría agregar la inicialización para DIALFS si es necesario
        }

        // Liberar recursos utilizados por la interfaz
        liberar_interfaz(interfaz, tipo_interfaz);
}

// Función para generar conexiones con la memoria y el kernel
void generar_conexiones(){
    establecer_conexion_memoria(ip_memoria, puerto_memoria, config_entradasalida, log_entradasalida);
    establecer_conexion_kernel(ip_kernel, puerto_kernel, config_entradasalida, log_entradasalida);
}

// Función para establecer conexión con el kernel
void establecer_conexion_kernel(t_config* config, t_log* loggs) {
    log_trace(loggs, "Inicio como cliente");

    // Obtener la IP y el puerto del kernel desde la configuración
    char *ip_kernel = config_get_string_value(config, "IP_KERNEL");
    char *puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    log_trace(loggs, "Lei la IP %s , el Puerto Kernel %s ", ip_kernel, puerto_kernel);

    // Establecer conexión con el kernel
    if ((conexion_entradasalida_kernel = crear_conexion(ip_kernel, puerto_kernel)) == -1) {
        log_trace(loggs, "Error al conectar con Kernel. El servidor no esta activo");
        exit(EXIT_FAILURE);
    }

    // Esperar y recibir una operación desde el kernel
    int operacion = recibir_operacion(conexion_entradasalida_kernel);

    // Verificar si se recibió una operación válida
    if (operacion == -1) {
        log_trace(loggs, "Error al recibir operacion del Kernel. Cerrando conexion.");
        return;
    }

    // Procesar la operación recibida
    if (operacion == ) {
        log_trace(loggs, "Operacion requerida recibida");



    } else {
        log_trace(loggs, "Operacion no reconocida recibida desde Kernel");
    }
}

// Función para establecer conexión con la memoria
void establecer_conexion_memoria(char * ip_memoria, char* puerto_memoria, t_config* config, t_log* loggs){
    log_trace(loggs, "Inicio como cliente");

    log_trace(loggs,"Lei la IP %s , el Puerto Memoria %s ", ip_memoria, puerto_memoria);

    // Establecer conexión con la memoria usando la IP y el puerto obtenidos de la configuración
    if((conexion_entradasalida_memoria = crear_conexion(ip_memoria, puerto_memoria)) == -1){
        log_trace(loggs, "Error al conectar con Memoria. El servidor no esta activo");

        exit(EXIT_FAILURE);
    }

    // Realizar operaciones de recepción de datos de la memoria si es necesario
    recibir_operacion(conexion_entradasalida_memoria);
    recibir_string(conexion_entradasalida_memoria, loggs);
}

// Función para liberar recursos al finalizar el programa
void finalizar_programa(){
    log_destroy(log_entradasalida);
    config_destroy(config_entradasalida);
}

// Función para inicializar una interfaz genérica
void inicializar_interfaz_generica(InterfazGenerica *interfazGen, const char *nombre, int tiempo) {
    if (interfazGen == NULL) {
        fprintf(stderr, "Interfaz vacia\n");
    }
    // Inicializar atributos de la interfaz genérica
    interfazGen->nombre = strdup(nombre);
    interfazGen->tiempo_unidad_trabajo = config_get_int_value(config_GENERICA, "TIEMPO_UNIDAD_TRABAJO");
    interfazGen->ip_kernel = config_get_string_value(config_entradasalida, "IP_KERNEL");
    interfazGen->puerto_kernel = config_get_string_value(config_entradasalida, "PUERTO_KERNEL");
}

// Función para inicializar una interfaz stdin
void inicializar_interfaz_stdin(STDIN *interfazStdin, const char *nombre) {
    if (interfazStdin == NULL) {
        fprintf(stderr, "Interfaz vacia\n");
    }
    // Inicializar atributos de la interfaz stdin
    interfazStdin->nombre = strdup(nombre);
    interfazStdin->tiempo_unidad_trabajo = config_get_int_value(config_TECLADO, "TIEMPO_UNIDAD_TRABAJO");
    interfazStdin->ip_kernel = config_get_string_value(config_entradasalida, "IP_KERNEL");
    interfazStdin->puerto_kernel = config_get_string_value(config_entradasalida, "PUERTO_KERNEL");
    interfazStdin->ip_memoria = config_get_string_value(config_entradasalida, "IP_MEMORIA");
    interfazStdin->puerto_memoria = config_get_string_value(config_entradasalida, "PUERTO_MEMORIA");
}

// Función para inicializar una interfaz stdout
void inicializar_interfaz_stdout(STDOUT *interfazStdout, const char *nombre) {
    if (interfazStdout == NULL) {
        fprintf(stderr, "Interfaz vacia\n");
    }
    // Inicializar atributos de la interfaz stdout
    interfazStdout->nombre = strdup(nombre);
    interfazStdout->tiempo_unidad_trabajo = config_get_int_value(config_MONITOR, "TIEMPO_UNIDAD_TRABAJO");
    interfazStdout->ip_kernel = config_get_string_value(config_entradasalida, "IP_KERNEL");
    interfazStdout->puerto_kernel = config_get_string_value(config_entradasalida, "PUERTO_KERNEL");
    interfazStdout->ip_memoria = config_get_string_value(config_entradasalida, "IP_MEMORIA");
    interfazStdout->puerto_memoria = config_get_string_value(config_entradasalida, "PUERTO_MEMORIA");
}

// Función para validar si una interfaz está conectada y disponible para operaciones
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

// Función para validar si una operación es compatible con el tipo de interfaz
void validar_operacion_io(void *interfaz, OperacionIO operacion) {
    TipoInterfaz tipo = obtener_tipo_interfaz(interfaz);

    if (!es_operacion_compatible(tipo, operacion)) {
        log_error(log_entradasalida, "La operación solicitada no es compatible con el tipo de interfaz.");
        exit(EXIT_FAILURE); // En una aplicación real, quizás sería adecuado manejar este error de manera más elegante
    }
}

// Función para verificar si una operación es compatible con el tipo de interfaz
bool es_operacion_compatible(TipoInterfaz tipo, OperacionIO operacion) {
    switch (tipo) {
        case GENERICA:
            return operacion == IO_GEN_SLEEP;
        case STDIN:
            return operacion == STDIN_READ;
        case STDOUT:
            return operacion == STDOUT_WRITE;
        default:
            return false;
    }
}

// Función para solicitar una operación de entrada/salida
void solicitar_operacion_io(InterfazIO interfaz, OperacionIO operacion) {
    if (estado_interfaz == LIBRE) {
        estado_interfaz = OCUPADA;
        realizar_operacion_io(interfaz, operacion);
    } else {
        esperar_interfaz_libre();
        estado_interfaz = OCUPADA;
        realizar_operacion_io(interfaz, operacion);
    }
}

// Función para marcar que una operación de entrada/salida ha finalizado
void operacion_io_finalizada() {
    estado_interfaz = LIBRE;
    desbloquear_proceso();
}

// Función para inicializar el registro de interfaces
void inicializar_registro() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    registro.capacidad = 10; // Se puede ajustar este valor según sea necesario
    registro.cantidad = 0;
    registro.interfaces = malloc(registro.capacidad * sizeof(Interfaz));
}

// Función para liberar memoria utilizada por el registro de interfaces
void liberar_registro() {
    for (int i = 0; i < registro.cantidad; i++) {
        free(registro.interfaces[i].nombre);
    }
    free(registro.interfaces);
}

// Función para conectar una interfaz al registro
void conectar_interfaz(char* nombre_interfaz) {
    if (registro.cantidad == registro.capacidad) {
        // Si el registro está lleno, agrandarlo
        registro.capacidad *= 2;
        registro.interfaces = realloc(registro.interfaces, registro.capacidad * sizeof(Interfaz));
    }
    Interfaz nueva_interfaz;
    nueva_interfaz.nombre = strdup(nombre_interfaz);
    nueva_interfaz.conectada = true;
    registro.interfaces[registro.cantidad++] = nueva_interfaz;
}

// Función para desconectar una interfaz del registro
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

// Función para verificar si una interfaz está conecta
bool interfaz_conectada(char* nombre_interfaz) {
    for (int i = 0; i < registro.cantidad; i++) {
        if (strcmp(registro.interfaces[i].nombre, nombre_interfaz) == 0) {
            return registro.interfaces[i].conectada;
        }
    }
    return false;
}

// Variables y funciones para la sincronización de hilos
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// Función para esperar a que una interfaz esté libre para su uso
void esperar_interfaz_libre() {
    pthread_mutex_lock(&mutex);
    while (estado_interfaz == OCUPADA) {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);
}

