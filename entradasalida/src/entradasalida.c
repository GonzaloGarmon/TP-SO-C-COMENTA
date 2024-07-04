#include "entradasalida.h"

int main(int argc, char *argv[]){
    log_entradasalida = log_create("./entradasalida.log", "ENTRADASALIDA", 1, LOG_LEVEL_TRACE);
    log_info(log_entradasalida, "INICIA EL MODULO DE ENTRADASALIDA");
    inicializar_registro();
 
    //Crear Interfaz
    printf("Crear Interfaz \"Nombre\" \"Nombre.config\"\n");
    printf("Crear Interfaz ");

    if (scanf(" \"%255[^\"]\" \"%255[^\"]\"", nombre_interfaz, ruta_archivo) == 2) {
        sprintf(ruta_completa, "/home/utnso/tp-2024-1c-GoC/entradasalida/config/%s", ruta_archivo);
        crear_interfaz(nombre_interfaz, ruta_completa);
    } else {
        printf("Formato de entrada incorrecto. Uso: \"Nombre\" \"Nombre.config\"\n");
        printf("Reiniciar Interfaz\n");
    }

    while (1) {

    }

    generar_conexiones();

    log_info(log_entradasalida, "Finalizo conexion con servidores");
    finalizar_programa();
    return 0;
}

void crear_interfaz(char *nombre_interfaz,char *ruta_archivo) {
    
    config_entradasalida = iniciar_config(ruta_archivo);

    if (config_entradasalida == NULL) {
        log_error(log_entradasalida, "No se pudo crear el config_entradasalida para el archivo %s", ruta_archivo);
        return;
    }

    tipo_interfaz = config_get_string_value(config_entradasalida, "TIPO_INTERFAZ");
    
    if (tipo_interfaz == NULL) {
        log_warning(log_entradasalida, "El archivo de configuración %s no tiene la clave TIPO_INTERFAZ", ruta_archivo);
        config_destroy(config_entradasalida);
        return;
    }

    if (strcmp(tipo_interfaz, "GENERICA") == 0) {
        GENERICA *interfaz = malloc(sizeof(GENERICA));
        if (interfaz == NULL) {
            log_error(log_entradasalida, "Error al asignar memoria para GENERICA");
            config_destroy(config_entradasalida);
            return;
        }
        inicializar_interfaz_generica(config_entradasalida, interfaz, nombre_interfaz);
        conectar_interfaz((char *)nombre_interfaz);
    } else if (strcmp(tipo_interfaz, "STDIN") == 0) {
        STDIN *interfaz = malloc(sizeof(STDIN));
        if (interfaz == NULL) {
            log_error(log_entradasalida, "Error al asignar memoria para STDIN");
            config_destroy(config_entradasalida);
            return;
        }
        inicializar_interfaz_stdin(config_entradasalida, interfaz, nombre_interfaz);
        conectar_interfaz((char *)nombre_interfaz);
    } else if (strcmp(tipo_interfaz, "STDOUT") == 0) {
        STDOUT *interfaz = malloc(sizeof(STDOUT));
        if (interfaz == NULL) {
            log_error(log_entradasalida, "Error al asignar memoria para STDOUT");
            config_destroy(config_entradasalida);
            return;
        }
        inicializar_interfaz_stdout(config_entradasalida, interfaz, nombre_interfaz);
        conectar_interfaz((char *)nombre_interfaz);
    } else if (strcmp(tipo_interfaz, "DIALFS") == 0) {
        DIALFS *interfazDialFS = malloc(sizeof(DIALFS));
        if (interfazDialFS == NULL) {
            log_error(log_entradasalida, "Error al asignar memoria para DIALFS");
            config_destroy(config_entradasalida);
            return;
        }
        // Inicializar la interfaz DialFS
        inicializar_interfaz_dialfs(interfazDialFS, nombre_interfaz, block_size, block_count);
        conectar_interfaz(nombre_interfaz);
    } else {
        log_warning(log_entradasalida, "Tipo de interfaz desconocido: %s", tipo_interfaz);
    }
}

void finalizar_programa()
{
    log_destroy(log_entradasalida);
    config_destroy(config_entradasalida);
    liberar_registro();
}

void enviarOpKernel(){


}

void enviarOpMemoria(){


}

void funcIoGenSleep(){


}

void funcIoStdRead(){


}

void funcIoStdWrite(){


}

void funcIoFsRead(){


}

void funcIoFsWrite(){


}


void funcIoFsTruncate(){


}

void funcIoFsCreate(){


}

void funcIoFsDelete(){


}

void generar_conexiones() {   

    ip_kernel = config_get_string_value(config_entradasalida, "IP_KERNEL");
    puerto_kernel = config_get_string_value(config_entradasalida, "PUERTO_KERNEL");
    ip_memoria = config_get_string_value(config_entradasalida, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_entradasalida, "PUERTO_MEMORIA");

    if (ip_memoria && puerto_memoria)
    {
        establecer_conexion_memoria(ip_memoria, puerto_memoria, config_entradasalida, log_entradasalida);
    }
    else
    {
        log_error(log_entradasalida, "Faltan configuraciones de memoria.");
    }

    if(strcmp(tipo_interfaz, "GENERICA") != 0){
        if (ip_kernel && puerto_kernel){
            establecer_conexion_kernel(ip_kernel, puerto_kernel, config_entradasalida, log_entradasalida);
        }
    else {
        log_error(log_entradasalida, "Faltan configuraciones de kernel.");
        }
    }
}

void establecer_conexion_kernel(char *ip_kernel, char *puerto_kernel, t_config *config_entradasalida, t_log *loggs)
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

void establecer_conexion_memoria(char *ip_memoria, char *puerto_memoria, t_config *config_entradasalida, t_log *loggs)
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

void inicializar_interfaz_generica(t_config *config_entradasalida, GENERICA *interfazGen, const char *nombre)
{
    if (interfazGen == NULL)
    {
        fprintf(stderr, "Interfaz vacia\n");
        return;
    }
    // Inicializar atributos de la interfaz genérica
    interfazGen->nombre = strdup(nombre);
    interfazGen->tipo = TIPO_GENERICA;
    interfazGen->tiempo_unidad_trabajo = config_get_int_value(config_entradasalida, "TIEMPO_UNIDAD_TRABAJO");
    interfazGen->ip_kernel = config_get_string_value(config_entradasalida, "IP_KERNEL");
    interfazGen->puerto_kernel = config_get_string_value(config_entradasalida, "PUERTO_KERNEL");
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

void inicializar_interfaz_stdin(t_config *config_entradasalida, STDIN *interfazStdin, const char *nombre)
{
    if (interfazStdin == NULL)
    {
        fprintf(stderr, "Interfaz vacia\n");
        return;
    }
    // Inicializar atributos de la interfaz stdin
    interfazStdin->nombre = strdup(nombre);
    interfazStdin->tipo = TIPO_STDIN;
    interfazStdin->tiempo_unidad_trabajo = config_get_int_value(config_entradasalida, "TIEMPO_UNIDAD_TRABAJO");
    interfazStdin->ip_kernel = config_get_string_value(config_entradasalida, "IP_KERNEL");
    interfazStdin->puerto_kernel = config_get_string_value(config_entradasalida, "PUERTO_KERNEL");
    interfazStdin->ip_memoria = config_get_string_value(config_entradasalida, "IP_MEMORIA");
    interfazStdin->puerto_memoria = config_get_string_value(config_entradasalida, "PUERTO_MEMORIA");
    interfazStdin->enUso = false;

    printf("Interfaz StdIn creada:\n");
    printf("  Nombre: %s\n", interfazStdin->nombre);
    printf("  Tipo de interfaz: STDIN\n");
    printf("  Tiempo de unidad de trabajo: %d\n", interfazStdin->tiempo_unidad_trabajo);
    printf("  IP Kernel: %s\n", interfazStdin->ip_kernel);
    printf("  Puerto Kernel: %s\n", interfazStdin->puerto_kernel);
    printf("  IP Memoria: %s\n", interfazStdin->ip_memoria);
    printf("  Puerto Memoria: %s\n", interfazStdin->puerto_memoria);
    printf("  En uso: %s\n", interfazStdin->enUso ? "true" : "false");
}

void inicializar_interfaz_stdout(t_config *config_entradasalida, STDOUT *interfazStdout, const char *nombre)
{
    if (interfazStdout == NULL)
    {
        fprintf(stderr, "Interfaz vacia\n");
        return;
    }
    // Inicializar atributos de la interfaz stdout
    interfazStdout->nombre = strdup(nombre);
    interfazStdout->tipo = TIPO_STDOUT;
    interfazStdout->tiempo_unidad_trabajo = config_get_int_value(config_entradasalida, "TIEMPO_UNIDAD_TRABAJO");
    interfazStdout->ip_kernel = config_get_string_value(config_entradasalida, "IP_KERNEL");
    interfazStdout->puerto_kernel = config_get_string_value(config_entradasalida, "PUERTO_KERNEL");
    interfazStdout->ip_memoria = config_get_string_value(config_entradasalida, "IP_MEMORIA");
    interfazStdout->puerto_memoria = config_get_string_value(config_entradasalida, "PUERTO_MEMORIA");
    interfazStdout->enUso = false;

    printf("Interfaz StdOut creada:\n");
    printf("  Nombre: %s\n", interfazStdout->nombre);
    printf("  Tipo de interfaz: STDOUT\n");
    printf("  Tiempo de unidad de trabajo: %d\n", interfazStdout->tiempo_unidad_trabajo);
    printf("  IP Kernel: %s\n", interfazStdout->ip_kernel);
    printf("  Puerto Kernel: %s\n", interfazStdout->puerto_kernel);
    printf("  IP Memoria: %s\n", interfazStdout->ip_memoria);
    printf("  Puerto Memoria: %s\n", interfazStdout->puerto_memoria);
    printf("  En uso: %s\n", interfazStdout->enUso ? "true" : "false");
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

bool es_operacion_compatible(TipoInterfaz tipo, op_code operacion)
{
    switch (tipo)
    {
    case TIPO_GENERICA:
        return operacion == IO_GEN_SLEEP;
    case TIPO_STDIN:
        return operacion == IO_STDIN_READ;
    case TIPO_STDOUT:
        return operacion == IO_STDOUT_WRITE;
    case TIPO_DIALFS:
        return operacion == IO_FS_CREATE || operacion == IO_FS_DELETE || operacion == IO_FS_TRUNCATE || operacion == IO_FS_WRITE || operacion == IO_FS_READ;
    default:
        return false;
    }
}

void validar_operacion_io(void *interfaz, op_code operacion)
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

void solicitar_operacion_io(void *interfaz, op_code operacion)
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

void conectar_interfaz(char *nombre_interfaz) {
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

void liberar_interfaz(void *interfaz, TipoInterfaz tipo) {
    if (interfaz != NULL) {
        switch (tipo) {
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

// Ver como recibo datos
void inicializar_interfaz_dialfs(DIALFS *interfazDialFS, const char *nombre, int block_size, int block_count) {
    interfazDialFS->nombre = strdup(nombre);
    interfazDialFS->tipo = TIPO_DIALFS;
    interfazDialFS->tiempo_unidad_trabajo = config_get_int_value(config_entradasalida, "TIEMPO_UNIDAD_TRABAJO");

    interfazDialFS->ip_kernel = config_get_string_value(config_entradasalida, "IP_KERNEL");
    interfazDialFS->puerto_kernel = config_get_string_value(config_entradasalida, "PUERTO_KERNEL");
    interfazDialFS->ip_memoria = config_get_string_value(config_entradasalida, "IP_MEMORIA");
    interfazDialFS->puerto_memoria = config_get_string_value(config_entradasalida, "PUERTO_MEMORIA");

    //modificar para poner ruta de archivo o algo
    interfazDialFS->path_base_dialfs = config_get_string_value(config_entradasalida, "PATH_BASE_DIALFS");
    interfazDialFS->block_size = config_get_int_value(config_entradasalida, "BLOCK_SIZE");
    interfazDialFS->block_count = config_get_int_value(config_entradasalida, "BLOCK_COUNT");
    interfazDialFS->retraso_compactacion = config_get_int_value(config_entradasalida, "RETRASO_COMPACTACION");
    interfazDialFS->enUso = false;
}

// Función para inicializar el sistema de archivos DialFS
void dialfs_init(DialFS *dialfs, int num_blocks) {
    dialfs->num_blocks = num_blocks;

    // Inicialización del bitmap de bloques
    dialfs->bitmap = (int *)malloc(num_blocks * sizeof(int));
    if (dialfs->bitmap == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria para el bitmap de bloques\n");
        exit(EXIT_FAILURE);
    }
    memset(dialfs->bitmap, 0, num_blocks * sizeof(int));

    // Inicialización de los bloques de datos
    dialfs->blocks = (Block *)malloc(num_blocks * sizeof(Block));
    if (dialfs->blocks == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria para los bloques de datos\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < num_blocks; ++i) {
        dialfs->blocks[i].data = NULL; // Inicialmente los datos están vacíos
    }
}

// Función para destruir el sistema de archivos DialFS y liberar memoria
void dialfs_destroy(DialFS *dialfs) {
    free(dialfs->bitmap);
    for (int i = 0; i < dialfs->num_blocks; ++i) {
        free(dialfs->blocks[i].data);
    }
    free(dialfs->blocks);
}

// Función para reservar un bloque
int dialfs_allocate_block(DialFS *fs) {
    for (int i = 0; i < fs->num_blocks; i++) {
        if (fs->bitmap[i] == 0) {
            fs->bitmap[i] = 1; // Marcar como ocupado
            return i;
        }
    }
    return -1; // No hay bloques libres
}

// Función para liberar un bloque
void dialfs_free_block(DialFS *fs, int block_index) {
    if (block_index >= 0 && block_index < fs->num_blocks) {
        fs->bitmap[block_index] = 0; // Marcar como libre
    }
}

// Función para escribir en un bloque
void dialfs_write_block(DialFS *fs, int block_index, const uint8_t *data, size_t size) {
    if (block_index >= 0 && block_index < fs->num_blocks) {
        memcpy(fs->blocks[block_index].data, data, size);
    }
}

// Función para leer desde un bloque
void dialfs_read_block(DialFS *fs, int block_index, uint8_t *buffer, size_t size) {
    if (block_index >= 0 && block_index < fs->num_blocks) {
        memcpy(buffer, fs->blocks[block_index].data, size);
    }
}

// Función para crear un archivo en DialFS
int dialfs_crear_archivo(DialFS *fs, const char *nombre_archivo, const uint8_t *datos, size_t size) {
    // Buscar un bloque libre para el archivo
    int bloque = dialfs_allocate_block(fs);
    if (bloque == -1) {
        fprintf(stderr, "No hay suficientes bloques libres para crear el archivo.\n");
        return -1;
    }

    // Escribir datos en el bloque asignado
    dialfs_write_block(fs, bloque, datos, size);

    // Aquí podrías implementar la lógica para mantener un registro de archivos creados, por ejemplo:
    printf("Archivo '%s' creado exitosamente en el bloque %d.\n", nombre_archivo, bloque);

    return bloque;
}

// Función para redimensionar un archivo en DialFS
void dialfs_redimensionar_archivo(DialFS *fs, int bloque_archivo, const uint8_t *nuevos_datos, size_t nuevo_size) {
    if (bloque_archivo < 0 || bloque_archivo >= fs->num_blocks) {
        fprintf(stderr, "El bloque de archivo proporcionado (%d) no es válido.\n", bloque_archivo);
        return;
    }

    // Escribir los nuevos datos en el bloque existente
    dialfs_write_block(fs, bloque_archivo, nuevos_datos, nuevo_size);

    printf("Archivo en el bloque %d redimensionado exitosamente.\n", bloque_archivo);
}

// Función para compactar archivos en DialFS
void dialfs_compactar_archivos(DialFS *fs) {
    int next_free_block = 0; // Siguiente bloque libre disponible

    // Recorrer todos los bloques del sistema de archivos
    for (int i = 0; i < fs->num_blocks; ++i) {
        if (fs->bitmap[i] == 1) {
            // Este bloque está ocupado, necesitamos copiar su contenido
            // al próximo bloque libre disponible
            if (i != next_free_block) {
                // Copiar datos al próximo bloque libre
                memcpy(fs->blocks[next_free_block].data, fs->blocks[i].data, sizeof(Block));
                fs->bitmap[next_free_block] = 1; // Marcar el nuevo bloque como ocupado
                fs->bitmap[i] = 0; // Marcar el bloque original como libre
            }
            ++next_free_block; // Mover al siguiente bloque libre
        }
    }

    // Actualizar el número total de bloques ocupados
    fs->num_blocks = next_free_block;

    printf("Compactación de archivos realizada exitosamente.\n");
}
