#include "entradasalida.h"

int main(int argc, char *argv[])
{
    log_entradasalida = log_create("./entradasalida.log", "ENTRADASALIDA", 1, LOG_LEVEL_TRACE);
    log_info(log_entradasalida, "INICIA EL MODULO DE ENTRADASALIDA");
    inicializar_registro();
    leer_config();
    procesar_todos_los_archivos_config(); // Nueva función para procesar todos los archivos .config
    generar_conexiones();

    // Esto está puesto para que el módulo no finalice y pueda seguir conectado a los otros una vez levantado
    while (1)
    {
    }

    log_info(log_entradasalida, "Finalizo conexion con servidores");
    finalizar_programa();
    return 0;
}

void leer_config()
{
    config_entradasalida = iniciar_config("/home/utnso/tp-2024-1c-GoC/entradasalida/config/entrada.config");

    if (config_entradasalida == NULL)
    {
        log_error(log_entradasalida, "No se pudo leer el archivo de configuración principal");
        exit(EXIT_FAILURE);
    }

    tipo_interfaz = config_get_string_value(config_entradasalida, "TIPO_INTERFAZ"); //creo q estos datos hay q borrarlos, entrada solo va a tener las ips y los puertos
    tiempo_unidad_trabajo = config_get_int_value(config_entradasalida, "TIEMPO_UNIDAD_TRABAJO"); //creo q estos datos hay q borrarlos, entrada solo va a tener las ips y los puertos
    ip_kernel = config_get_string_value(config_entradasalida, "IP_KERNEL");
    puerto_kernel = config_get_string_value(config_entradasalida, "PUERTO_KERNEL");
    ip_memoria = config_get_string_value(config_entradasalida, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_entradasalida, "PUERTO_MEMORIA");
    path_base_dialfs = config_get_string_value(config_entradasalida, "PATH_BASE_DIALFS"); //creo q estos datos hay q borrarlos, entrada solo va a tener las ips y los puertos
    block_size = config_get_int_value(config_entradasalida, "BLOCK_SIZE"); //creo q estos datos hay q borrarlos, entrada solo va a tener las ips y los puertos
    block_count = config_get_int_value(config_entradasalida, "BLOCK_COUNT"); //creo q estos datos hay q borrarlos, entrada solo va a tener las ips y los puertos
    retraso_compactacion = config_get_int_value(config_entradasalida, "RETRASO_COMPACTACION"); //creo q estos datos hay q borrarlos, entrada solo va a tener las ips y los puertos
}

void procesar_todos_los_archivos_config()
{
    const char *nombre_directorio = "/home/utnso/tp-2024-1c-GoC/entradasalida/config";
    struct dirent *entrada;
    DIR *directorio = opendir(nombre_directorio);

    if (directorio == NULL)
    {
        perror("No se puede abrir el directorio");
        exit(EXIT_FAILURE);
    }

    while ((entrada = readdir(directorio)) != NULL)
    {
        if (tiene_extension_config(entrada->d_name))
        {
            char ruta_archivo[1024];
            snprintf(ruta_archivo, sizeof(ruta_archivo), "%s/%s", nombre_directorio, entrada->d_name);
            procesar_archivo_config(ruta_archivo);
        }
    }

    closedir(directorio);
}

void procesar_archivo_config(const char *ruta_archivo)
{
    t_config *config = config_create((char *)ruta_archivo);

    if (config == NULL)
    {
        log_error(log_entradasalida, "No se pudo crear el config para el archivo %s", ruta_archivo);
        return;
    }

    char *tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");
    if (tipo_interfaz == NULL)
    {
        log_warning(log_entradasalida, "El archivo de configuración %s no tiene la clave TIPO_INTERFAZ", ruta_archivo);
        config_destroy(config);
        return;
    }

    // Obtener solo el nombre del archivo sin la ruta completa y sin la extensión .config
    const char *nombre_archivo = strrchr(ruta_archivo, '/');
    if (nombre_archivo == NULL) {
        log_error(log_entradasalida, "Ruta de archivo inválida: %s", ruta_archivo);
        config_destroy(config);
        return;
    }
    nombre_archivo++; // Saltar el caracter '/'

    char *nombre_interfaz = strdup(nombre_archivo);
    char *dot = strrchr(nombre_interfaz, '.');
    if (dot != NULL)
    {
        *dot = '\0'; // Eliminar la extensión .config
    }

    if (strcmp(tipo_interfaz, "GENERICA") == 0)
    {
        GENERICA *interfaz = malloc(sizeof(GENERICA));
        inicializar_interfaz_generica(config, interfaz, nombre_interfaz);
        conectar_interfaz(nombre_interfaz);
    }
    else if (strcmp(tipo_interfaz, "STDIN") == 0)
    {
        STDIN *interfaz = malloc(sizeof(STDIN));
        inicializar_interfaz_stdin(config, interfaz, nombre_interfaz);
        conectar_interfaz(nombre_interfaz);
    }
    else if (strcmp(tipo_interfaz, "STDOUT") == 0)
    {
        STDOUT *interfaz = malloc(sizeof(STDOUT));
        inicializar_interfaz_stdout(config, interfaz, nombre_interfaz);
        conectar_interfaz(nombre_interfaz);
    }
    else if (strcmp(tipo_interfaz, "DIALFS") == 0)
    {
        DIALFS *interfaz = malloc(sizeof(DIALFS));
        inicializar_interfaz_dialfs(config, interfaz, nombre_interfaz);
        conectar_interfaz(nombre_interfaz);
    }
    else
    {
        log_warning(log_entradasalida, "Tipo de interfaz desconocido: %s", tipo_interfaz);
    }

    free(nombre_interfaz);
    config_destroy(config);
}

void finalizar_programa()
{
    log_destroy(log_entradasalida);
    config_destroy(config_entradasalida);
    liberar_registro();
}

void generar_conexiones()
{
    if (ip_memoria && puerto_memoria)
    {
        establecer_conexion_memoria(ip_memoria, puerto_memoria, config_entradasalida, log_entradasalida);
    }
    else
    {
        log_error(log_entradasalida, "Faltan configuraciones de memoria.");
    }

    if (ip_kernel && puerto_kernel)
    {
        establecer_conexion_kernel(ip_kernel, puerto_kernel, config_entradasalida, log_entradasalida);
    }
    else
    {
        log_error(log_entradasalida, "Faltan configuraciones de kernel.");
    }
}

void establecer_conexion_kernel(char *ip_kernel, char *puerto_kernel, t_config *config, t_log *loggs)
{
    log_trace(loggs, "Inicio como cliente Kernel");

    if ((conexion_entradasalida_kernel = crear_conexion(ip_kernel, puerto_kernel)) == -1)
    {
        log_error(loggs, "Error al conectar con Kernel. El servidor no está activo");
        return;
    }

    int operacion = recibir_operacion(conexion_entradasalida_kernel);

    if (operacion == -1)
    {
        log_error(loggs, "Error al recibir operación del Kernel");
        return;
    }

    recibir_string(conexion_entradasalida_kernel, loggs);
}

void establecer_conexion_memoria(char *ip_memoria, char *puerto_memoria, t_config *config, t_log *loggs)
{
    log_trace(loggs, "Inicio como cliente Memoria");

    if ((conexion_entradasalida_memoria = crear_conexion(ip_memoria, puerto_memoria)) == -1)
    {
        log_error(loggs, "Error al conectar con Memoria. El servidor no está activo");
        return;
    }

    int operacion = recibir_operacion(conexion_entradasalida_memoria);

    if (operacion == -1)
    {
        log_error(loggs, "Error al recibir operación de Memoria");
        return;
    }

    recibir_string(conexion_entradasalida_memoria, loggs);
}

void inicializar_interfaz_generica(t_config *config, GENERICA *interfazGen, const char *nombre)
{
    if (interfazGen == NULL)
    {
        fprintf(stderr, "Interfaz vacia\n");
        return;
    }
    // Inicializar atributos de la interfaz genérica
    interfazGen->nombre = strdup(nombre);
    interfazGen->tipo = TIPO_GENERICA;
    interfazGen->tiempo_unidad_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    interfazGen->ip_kernel = ip_kernel;
    interfazGen->puerto_kernel = puerto_kernel;
    interfazGen->enUso = false;

    //Ver si es necesario una funcion para mostrar interfaces, esto es para ver si las crea bien
    printf("Interfaz Generica creada:\n");
    printf("  Nombre: %s\n", interfazGen->nombre);
    printf("  Tipo de interfaz: GENERICA\n");
    printf("  Tiempo de unidad de trabajo: %d\n", interfazGen->tiempo_unidad_trabajo);
    printf("  IP Kernel: %s\n", interfazGen->ip_kernel);
    printf("  Puerto Kernel: %s\n", interfazGen->puerto_kernel);
    printf("  En uso: %s\n", interfazGen->enUso ? "true" : "false");
}

void inicializar_interfaz_stdin(t_config *config, STDIN *interfazStdin, const char *nombre)
{
    if (interfazStdin == NULL)
    {
        fprintf(stderr, "Interfaz vacia\n");
        return;
    }
    // Inicializar atributos de la interfaz stdin
    interfazStdin->nombre = strdup(nombre);
    interfazStdin->tipo = TIPO_STDIN;
    interfazStdin->tiempo_unidad_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    interfazStdin->ip_kernel = ip_kernel;
    interfazStdin->puerto_kernel = puerto_kernel;
    interfazStdin->ip_memoria = ip_memoria;
    interfazStdin->puerto_memoria = puerto_memoria;
    interfazStdin->enUso = false;

    printf("Interfaz StdIn creada:\n");
    printf("  Nombre: %s\n", interfazStdin->nombre);
    printf("  Tipo de interfaz: STDIN\n");
    printf("  Tiempo de unidad de trabajo: %d\n", interfazStdin->tiempo_unidad_trabajo);
    printf("  IP Kernel: %s\n", interfazStdin->ip_kernel);
    printf("  Puerto Kernel: %s\n", interfazStdin->puerto_kernel);
    printf("  IP Memoria: %s\n", interfazStdin->ip_kernel);
    printf("  Puerto Memoria: %s\n", interfazStdin->puerto_kernel);
    printf("  En uso: %s\n", interfazStdin->enUso ? "true" : "false");
}

void inicializar_interfaz_stdout(t_config *config, STDOUT *interfazStdout, const char *nombre)
{
    if (interfazStdout == NULL)
    {
        fprintf(stderr, "Interfaz vacia\n");
        return;
    }
    // Inicializar atributos de la interfaz stdout
    interfazStdout->nombre = strdup(nombre);
    interfazStdout->tipo = TIPO_STDOUT;
    interfazStdout->tiempo_unidad_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    interfazStdout->ip_kernel = ip_kernel;
    interfazStdout->puerto_kernel = puerto_kernel;
    interfazStdout->ip_memoria = ip_memoria;
    interfazStdout->puerto_memoria = puerto_memoria;
    interfazStdout->enUso = false;

    printf("Interfaz StdOut creada:\n");
    printf("  Nombre: %s\n", interfazStdout->nombre);
    printf("  Tipo de interfaz: STDOUT\n");
    printf("  Tiempo de unidad de trabajo: %d\n", interfazStdout->tiempo_unidad_trabajo);
    printf("  IP Kernel: %s\n", interfazStdout->ip_kernel);
    printf("  Puerto Kernel: %s\n", interfazStdout->puerto_kernel);
    printf("  IP Memoria: %s\n", interfazStdout->ip_kernel);
    printf("  Puerto Memoria: %s\n", interfazStdout->puerto_kernel);
    printf("  En uso: %s\n", interfazStdout->enUso ? "true" : "false");
}

//SIN TERMINAR, Ver ejemplo de archivo de configuracion para hacer bien
void inicializar_interfaz_dialfs(t_config *config, DIALFS *interfazDialFS, const char *nombre)
{
    if (interfazDialFS == NULL)
    {
        fprintf(stderr, "Interfaz vacia\n");
        return;
    }
    // Inicializar atributos de la interfaz dialFS
    interfazDialFS->nombre = strdup(nombre);
    interfazDialFS->tipo = TIPO_DIALFS;
    interfazDialFS->tiempo_unidad_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    interfazDialFS->ip_kernel = ip_kernel;
    interfazDialFS->puerto_kernel = puerto_kernel;
    interfazDialFS->ip_memoria = ip_memoria;
    interfazDialFS->puerto_memoria = puerto_memoria;
    interfazDialFS->path_base_dialfs = path_base_dialfs;
    interfazDialFS->block_size = block_size;
    interfazDialFS->block_count = block_count;
    interfazDialFS->retraso_compactacion = retraso_compactacion;
    interfazDialFS->enUso = false;
}

bool validar_interfaz(ListaIO *interfaces, int num_interfaces, char *nombre_solicitado)
{
    for (int i = 0; i < num_interfaces; i++)
    {
        if (strcmp(interfaces[i].nombre, nombre_solicitado) == 0)
        {
            return true;
        }
    }
    return false;
}

bool es_operacion_compatible(TipoInterfaz tipo, OperacionIO operacion)
{
    switch (tipo)
    {
    case TIPO_GENERICA:
        return operacion == IOGENSLEEP;
    case TIPO_STDIN:
        return operacion == STDINREAD;
    case TIPO_STDOUT:
        return operacion == STDOUTWRITE;
    case TIPO_DIALFS:
        return operacion == FSCREATE || operacion == FSDELETE || operacion == FSTRUNCATE || operacion == FSREAD || operacion == FSWRITE;
    default:
        return false;
    }
}

void validar_operacion_io(void *interfaz, OperacionIO operacion)
{
    if (interfaz == NULL)
    {
        log_trace(log_entradasalida, "Interfaz no válida");
        return;
    }

    GENERICA *interfazGen = (GENERICA *)interfaz;
    if (!es_operacion_compatible(interfazGen->tipo, operacion))
    {
        log_error(log_entradasalida, "Operación no compatible con la interfaz");
    }
    else
    {
        log_trace(log_entradasalida, "Operación válida para la interfaz");
    }
}

void solicitar_operacion_io(void *interfaz, OperacionIO operacion)
{
    pthread_mutex_lock(&mutex);
    validar_operacion_io(interfaz, operacion);
    pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
}

void operacion_io_finalizada()
{
    pthread_mutex_lock(&mutex);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

void inicializar_registro()
{
    registro.capacidad = 10;
    registro.cantidad = 0;
    registro.interfaces = malloc(registro.capacidad * sizeof(ListaIO));
    if (registro.interfaces == NULL)
    {
        perror("Error al asignar memoria para registro.interfaces");
        exit(EXIT_FAILURE);
    }
}

void liberar_registro()
{
    if (registro.interfaces == NULL)
    {
        fprintf(stderr, "Error: registro.interfaces no ha sido inicializado correctamente\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < registro.cantidad; i++)
    {
        free(registro.interfaces[i].nombre);
    }
    free(registro.interfaces);
}

void conectar_interfaz(char *nombre_interfaz)
{
    if (registro.interfaces == NULL)
    {
        fprintf(stderr, "Error: registro.interfaces no ha sido inicializado correctamente\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < registro.cantidad; i++)
    {
        if (registro.interfaces[i].nombre != NULL && strcmp(registro.interfaces[i].nombre, nombre_interfaz) == 0)
        {
            registro.interfaces[i].conectada = true;
            break; // Salir del bucle al encontrar la interfaz
        }
    }
}
void desconectar_interfaz(char *nombre_interfaz)
{
    for (int i = 0; i < registro.cantidad; i++)
    {
        if (strcmp(registro.interfaces[i].nombre, nombre_interfaz) == 0)
        {
            registro.interfaces[i].conectada = false;
        }
    }
}

bool interfaz_conectada(char *nombre_interfaz)
{
    for (int i = 0; i < registro.cantidad; i++)
    {
        if (strcmp(registro.interfaces[i].nombre, nombre_interfaz) == 0)
        {
            return registro.interfaces[i].conectada;
        }
    }
    return false;
}

void esperar_interfaz_libre()
{
    pthread_mutex_lock(&mutex);
    while (true)
    {
        bool interfaz_libre = false;
        for (int i = 0; i < registro.cantidad; i++)
        {
            if (!registro.interfaces[i].conectada)
            {
                interfaz_libre = true;
                break;
            }
        }

        if (interfaz_libre)
        {
            break;
        }
        else
        {
            pthread_cond_wait(&cond, &mutex);
        }
    }
    pthread_mutex_unlock(&mutex);
}

void liberar_interfaz(void *interfaz, TipoInterfaz tipo)
{
    if (interfaz != NULL)
    {
        switch (tipo)
        {
        case TIPO_GENERICA:
        {
            GENERICA *interfazGen = (GENERICA *)interfaz;
            free(interfazGen->nombre);
            free(interfazGen->ip_kernel);
            free(interfazGen->puerto_kernel);
            free(interfaz);
            break;
        }
        case TIPO_STDIN:
        {
            STDIN *interfazStdin = (STDIN *)interfaz;
            free(interfazStdin->nombre);
            free(interfazStdin->ip_kernel);
            free(interfazStdin->puerto_kernel);
            free(interfazStdin->ip_memoria);
            free(interfazStdin->puerto_memoria);
            free(interfaz);
            break;
        }
        case TIPO_STDOUT:
        {
            STDOUT *interfazStdout = (STDOUT *)interfaz;
            free(interfazStdout->nombre);
            free(interfazStdout->ip_kernel);
            free(interfazStdout->puerto_kernel);
            free(interfazStdout->ip_memoria);
            free(interfazStdout->puerto_memoria);
            free(interfaz);
            break;
        }
        case TIPO_DIALFS:
        {
            DIALFS *interfazDialFS = (DIALFS *)interfaz;
            free(interfazDialFS->nombre);
            free(interfazDialFS->ip_kernel);
            free(interfazDialFS->puerto_kernel);
            free(interfazDialFS->ip_memoria);
            free(interfazDialFS->puerto_memoria);
            free(interfazDialFS->path_base_dialfs);
            free(interfazDialFS);
            break;
        }
        default:
            break;
        }
    }
}

bool tiene_extension_config(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    return (dot && strcmp(dot, ".config") == 0);
}
