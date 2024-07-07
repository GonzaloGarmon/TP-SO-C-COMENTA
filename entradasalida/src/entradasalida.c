#include "entradasalida.h"

int main(int argc, char *argv[]){
    log_entradasalida = log_create("./entradasalida.log", "ENTRADASALIDA", 1, LOG_LEVEL_TRACE);
    log_info(log_entradasalida, "INICIA EL MODULO DE ENTRADASALIDA");
    inicializar_registro();

    // Crear Interfaz
    printf("Crear Interfaz \"Nombre\" \"Nombre.config\"\n");
    printf("Crear Interfaz ");

    if (scanf(" \"%255[^\"]\" \"%255[^\"]\"", nombre_interfaz, ruta_archivo) == 2) {
        sprintf(ruta_completa, "/home/utnso/tp-2024-1c-GoC/entradasalida/config/%s", ruta_archivo);
        crear_interfaz(nombre_interfaz, ruta_completa);
    } else {
        printf("Formato de entrada incorrecto. Uso: \"Nombre\" \"Nombre.config\"\n");
        printf("Reiniciar Interfaz\n");
    }

    generar_conexiones();

    //Conexion Kernel
    socket_servidor_entradasalida = iniciar_servidor(puerto_kernel, log_entradasalida);
    log_info(log_entradasalida, "INICIO SERVIDOR con Kernel");

    pthread_t atiende_cliente_kernel;
    log_info(log_entradasalida, "Listo para recibir a kernel");
    conexion_entradasalida_kernel = esperar_cliente(socket_servidor_entradasalida);
   
    pthread_create(&atiende_cliente_kernel, NULL, (void *)recibirOpKernel, (void *) (intptr_t) conexion_entradasalida_kernel);
    pthread_join(atiende_cliente_kernel, NULL);

    //Conexion Memoria
    if(tipo != GENERICA_I){
        socket_servidor_entradasalida = iniciar_servidor(puerto_memoria, log_entradasalida);
        log_info(log_entradasalida, "INICIO SERVIDOR con Memoria");

        pthread_t atiende_cliente_memoria;
        log_info(log_entradasalida, "Listo para recibir a memoria");
        conexion_entradasalida_memoria = esperar_cliente(socket_servidor_entradasalida);
   
        pthread_create(&atiende_cliente_memoria, NULL, (void *)recibirOpKernel, (void *) (intptr_t) conexion_entradasalida_memoria);
        pthread_join(atiende_cliente_memoria, NULL);
    }

    enviar_string(conexion_entradasalida_kernel, tipo_interfaz_txt, IDENTIFICACION);
    log_trace(log_entradasalida, "mande un mensaje");
    enviar_string(conexion_entradasalida_kernel, "hola papito", tipo);
    log_trace(log_entradasalida, "mande un mensaje");

    enviar_string(conexion_entradasalida_memoria, "hola", GENERICA_I);

    while (1) {
        // Bucle principal
    }

    log_info(log_entradasalida, "Finalizo conexion con servidores");
    finalizar_programa();
    return 0;
}

void crear_interfaz(char *nombre_interfaz, char *ruta_archivo) {
    if (nombre_interfaz == NULL || ruta_archivo == NULL) {
        log_error(log_entradasalida, "El nombre de la interfaz o la ruta del archivo es NULL");
        return;
    }

    config_entradasalida = iniciar_config(ruta_archivo);

    if (config_entradasalida == NULL) {
        log_error(log_entradasalida, "No se pudo crear el config_entradasalida para el archivo %s", ruta_archivo);
        return;
    }

    tipo_interfaz_txt = config_get_string_value(config_entradasalida, "TIPO_INTERFAZ");

    if (tipo_interfaz_txt == NULL) {
        log_warning(log_entradasalida, "El archivo de configuración %s no tiene la clave TIPO_INTERFAZ", ruta_archivo);
        config_destroy(config_entradasalida);
        return;
    }
    if (strcmp(tipo_interfaz_txt, "GENERICA") == 0) {
        // Inicializar la interfaz Generica
        inicializar_interfaz_generica(config_entradasalida, nombre_interfaz);
        conectar_interfaz((char *)nombre_interfaz);
    } else if (strcmp(tipo_interfaz_txt, "STDIN") == 0) {
        // Inicializar la interfaz Stdin
        inicializar_interfaz_stdin(config_entradasalida, nombre_interfaz);
        conectar_interfaz((char *)nombre_interfaz);
    } else if (strcmp(tipo_interfaz_txt, "STDOUT") == 0) {
        // Inicializar la interfaz Stdout
        inicializar_interfaz_stdout(config_entradasalida,nombre_interfaz);
        conectar_interfaz((char *)nombre_interfaz);
    } else if (strcmp(tipo_interfaz_txt, "DIALFS") == 0) {
        // Inicializar la interfaz DialFS
        inicializar_interfaz_dialfs(config_entradasalida, nombre_interfaz);
        conectar_interfaz((char *)nombre_interfaz);
    } else {
        log_warning(log_entradasalida, "Tipo de interfaz desconocido: %s", tipo_interfaz_txt);
    }
}

void finalizar_programa(){
    log_destroy(log_entradasalida);
    config_destroy(config_entradasalida);
    liberar_registro();
}

void recibirOpKernel(int SOCKET_CLIENTE_KERNEL){
    int noFinalizar = 0;
    while(noFinalizar != -1){
        char* nombreRecibido = recibir_string(SOCKET_CLIENTE_KERNEL, log_entradasalida); //preguntar como usar funcion
        if(nombreRecibido == nombre_interfaz){
            op_code operacion = recibir_operacion(SOCKET_CLIENTE_KERNEL);
            switch (operacion){
            case IO_GEN_SLEEP:
                if(es_operacion_compatible(tipo,operacion)){
                    //ejemplo
                    int unidades = recibir_entero_uint32(SOCKET_CLIENTE_KERNEL,log_entradasalida);
                    funcIoGenSleep(unidades);//ver como recibir los componentes
                }
            break;
            case IO_STDIN_READ:
                if(es_operacion_compatible(tipo,operacion)){
                    funcIoStdRead(1,1);
                }
            break;
            case IO_STDOUT_WRITE:
                if(es_operacion_compatible(tipo,operacion)){
                    funcIoStdWrite(1,1);
                }
            break;
            case IO_FS_READ:
                if(es_operacion_compatible(tipo,operacion)){
                    funcIoFsRead();
                }
            break;
            case IO_FS_WRITE:
                if(es_operacion_compatible(tipo,operacion)){
                    funcIoFsWrite();
                }
            break;
            case IO_FS_CREATE:
                if(es_operacion_compatible(tipo,operacion)){
                    funcIoFsCreate();
                }
            break;
            case IO_FS_DELETE:
                if(es_operacion_compatible(tipo,operacion)){
                    funcIoFsDelete();
                }
            case IO_FS_TRUNCATE:
                if(es_operacion_compatible(tipo,operacion)){
                    funcIoFsTruncate();
                }
            break;
            default:
                log_warning(log_entradasalida, "Operacion no compatible");
            break;
            }
        }
    }

}

void recibirOpMemoria(int SOCKET_CLIENTE_MEMORIA){
    int noFinalizar = 0;
    while(noFinalizar != -1){
        op_code operacion = recibir_operacion(SOCKET_CLIENTE_MEMORIA);
        switch (operacion){
            case IO_STDOUT_WRITE_OK:
            break;
            case IO_STDIN_READ_OK:
            break;
            default:
                log_warning(log_entradasalida, "Operacion no compatible");
            break;
        }
    }
}

void funcIoGenSleep(int unidades){
    enUso = true;
    log_trace(log_entradasalida, "Esperando");
    sleep(unidades*tiempo_unidad_trabajo);
    enUso = false;
}

void funcIoStdRead(int direccion, int tamaño){
    enUso = true;
    log_trace(log_entradasalida, "Leyendo desde STDIN");

    char *buffer = (char *)malloc(tamaño + 1);

    // Leer entrada del usuario
    if (fgets(buffer, tamaño + 1, stdin) == NULL) {
        log_error(log_entradasalida, "Error al leer desde STDIN");
        free(buffer);
        enUso = false;
        return;
    }

    // Validar que el tamaño de la entrada no exceda el tamaño especificado
    if (strlen(buffer) > tamaño) {
        log_warning(log_entradasalida, "El texto ingresado es mayor al tamaño permitido. Se truncará.");
        buffer[tamaño] = '\0';
    }

    // Escribir el valor en la memoria. se encarga memoria yo solo le envio lo que escribio el usuario
    enviar_string(conexion_entradasalida_memoria, buffer, GENERICA_I);//Ver 3er elemento de la funcion

    free(buffer);
    enUso = false;
}

void funcIoStdWrite(int direccion, int tamaño){
    enUso = true;
    log_trace(log_entradasalida, "Leyendo desde memoria");
    enviar_entero(conexion_entradasalida_memoria,direccion,GENERICA_I);//Ver 3er elemento de la funcion
    enviar_entero(conexion_entradasalida_memoria,tamaño,GENERICA_I);//Ver 3er elemento de la funcion
    recibirOpMemoria(conexion_entradasalida_memoria);//Ver si poner puerto o conexion
    enUso = false;
}

void funcIoFsRead(){
    enUso = true;
    log_trace(log_entradasalida, "a");

    enUso = false;
}

void funcIoFsWrite(){
    enUso = true;
    log_trace(log_entradasalida, "a");

    enUso = false;
}

void funcIoFsTruncate(){
    enUso = true;
    log_trace(log_entradasalida, "a");

    enUso = false;
}

void funcIoFsCreate(){
    enUso = true;
    log_trace(log_entradasalida, "a");

    enUso = false;
}

void funcIoFsDelete(){
    enUso = true;
    log_trace(log_entradasalida, "a");

    enUso = false;
}

void generar_conexiones(){   
    if(tipo != GENERICA_I){
        if (ip_memoria && puerto_memoria) {
            establecer_conexion_memoria(ip_memoria, puerto_memoria, config_entradasalida, log_entradasalida);
        }
        else {
            log_error(log_entradasalida, "Faltan configuraciones de memoria.");
        }
    }

    if (ip_kernel && puerto_kernel){
        establecer_conexion_kernel(ip_kernel, puerto_kernel, config_entradasalida, log_entradasalida);
    }
    else {
        log_error(log_entradasalida, "Faltan configuraciones de kernel.");
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
    /*
    int operacion = recibir_operacion(conexion_entradasalida_kernel);

    if (operacion == -1) {
        log_trace(loggs, "Error al recibir operación");
        exit(EXIT_FAILURE);
    }
    recibir_string(conexion_entradasalida_kernel, log_entradasalida);
    */
}

void establecer_conexion_memoria(char *ip_memoria, char *puerto_memoria, t_config *config_entradasalida, t_log *loggs)
{
    log_trace(loggs, "Inicio como cliente Memoria");

    if ((conexion_entradasalida_memoria = crear_conexion(ip_memoria, puerto_memoria)) == -1)
    {
        log_error(loggs, "Error al conectar con Memoria. El servidor no está activo");
        return;
    }
    /*
    int operacion = recibir_operacion(conexion_entradasalida_memoria);

    if (operacion == -1) {
        log_trace(loggs, "Error al recibir operación");
        exit(EXIT_FAILURE);
    }

    recibir_string(conexion_entradasalida_memoria, log_entradasalida);
    */
}

void inicializar_interfaz_generica(t_config *config_entradasalida, const char *nombre){
    // Inicializar atributos de la interfaz genérica
    nombre = strdup(nombre);
    tipo = GENERICA_I;
    tiempo_unidad_trabajo = config_get_int_value(config_entradasalida, "TIEMPO_UNIDAD_TRABAJO");
    ip_kernel = config_get_string_value(config_entradasalida, "IP_KERNEL");
    puerto_kernel = config_get_string_value(config_entradasalida, "PUERTO_KERNEL");
    enUso = false;

    //Ver si es necesario una funcion para mostrar interfaces, esto es para ver si las crea bien
    printf("Interfaz Generica creada:\n");
    printf("  Nombre: %s\n", nombre);
    printf("  Tipo de interfaz: GENERICA\n");
    printf("  Tiempo de unidad de trabajo: %d\n", tiempo_unidad_trabajo);
    printf("  IP Kernel: %s\n", ip_kernel);
    printf("  Puerto Kernel: %s\n", puerto_kernel);
    printf("  En uso: %s\n", enUso ? "true" : "false");
}

void inicializar_interfaz_stdin(t_config *config_entradasalida, const char *nombre){
    // Inicializar atributos de la interfaz stdin
    nombre = strdup(nombre);
    tipo = STDIN_I;
    tiempo_unidad_trabajo = config_get_int_value(config_entradasalida, "TIEMPO_UNIDAD_TRABAJO");
    ip_kernel = config_get_string_value(config_entradasalida, "IP_KERNEL");
    puerto_kernel = config_get_string_value(config_entradasalida, "PUERTO_KERNEL");
    ip_memoria = config_get_string_value(config_entradasalida, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_entradasalida, "PUERTO_MEMORIA");
    enUso = false;

    printf("Interfaz StdIn creada:\n");
    printf("  Nombre: %s\n", nombre);
    printf("  Tipo de interfaz: STDIN\n");
    printf("  Tiempo de unidad de trabajo: %d\n", tiempo_unidad_trabajo);
    printf("  IP Kernel: %s\n", ip_kernel);
    printf("  Puerto Kernel: %s\n", puerto_kernel);
    printf("  IP Memoria: %s\n", ip_memoria);
    printf("  Puerto Memoria: %s\n", puerto_memoria);
    printf("  En uso: %s\n", enUso ? "true" : "false");
}

void inicializar_interfaz_stdout(t_config *config_entradasalida, const char *nombre){
    // Inicializar atributos de la interfaz stdout
    tipo = STDOUT_I;
    nombre = strdup(nombre);
    tiempo_unidad_trabajo = config_get_int_value(config_entradasalida, "TIEMPO_UNIDAD_TRABAJO");
    ip_kernel = config_get_string_value(config_entradasalida, "IP_KERNEL");
    puerto_kernel = config_get_string_value(config_entradasalida, "PUERTO_KERNEL");
    ip_memoria = config_get_string_value(config_entradasalida, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_entradasalida, "PUERTO_MEMORIA");
    enUso = false;

    printf("Interfaz StdOut creada:\n");
    printf("  Nombre: %s\n", nombre);
    printf("  Tipo de interfaz: STDOUT\n");
    printf("  Tiempo de unidad de trabajo: %d\n", tiempo_unidad_trabajo);
    printf("  IP Kernel: %s\n", ip_kernel);
    printf("  Puerto Kernel: %s\n", puerto_kernel);
    printf("  IP Memoria: %s\n", ip_memoria);
    printf("  Puerto Memoria: %s\n", puerto_memoria);
    printf("  En uso: %s\n", enUso ? "true" : "false");
}

void inicializar_interfaz_dialfs(t_config *config_entradasalida, const char *nombre){
    // Inicializar atributos de la interfaz stdout
    nombre = strdup(nombre);
    tipo = DIALFS_I;
    tiempo_unidad_trabajo = config_get_int_value(config_entradasalida, "TIEMPO_UNIDAD_TRABAJO");
    ip_kernel = config_get_string_value(config_entradasalida, "IP_KERNEL");
    puerto_kernel = config_get_string_value(config_entradasalida, "PUERTO_KERNEL");
    ip_memoria = config_get_string_value(config_entradasalida, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_entradasalida, "PUERTO_MEMORIA");
    path_base_dialfs = config_get_string_value(config_entradasalida, "PATH_BASE_DIALFS");
    block_size = config_get_int_value(config_entradasalida, "BLOCK_SIZE");
    block_count = config_get_int_value(config_entradasalida, "BLOCK_COUNT");
    retraso_compactacion = config_get_int_value(config_entradasalida, "RETRASO_COMPACTACION");
    enUso = false;

    printf("Interfaz DialFS creada:\n");
    printf("  Nombre: %s\n", nombre);
    printf("  Tipo de interfaz: STDOUT\n");
    printf("  Tiempo de unidad de trabajo: %d\n", tiempo_unidad_trabajo);
    printf("  IP Kernel: %s\n", ip_kernel);
    printf("  Puerto Kernel: %s\n", puerto_kernel);
    printf("  IP Memoria: %s\n", ip_memoria);
    printf("  Puerto Memoria: %s\n",puerto_memoria);
    printf("  Path Base: %s\n", path_base_dialfs);
    printf("  Block Size: %d\n", block_size);
    printf("  Block Count: %d\n",block_count);
    printf("  Retraso Compactacion: %d\n", retraso_compactacion);
    printf("  En uso: %s\n", enUso ? "true" : "false");
}

bool validar_interfaz(ListaIO *interfaces, int num_interfaces, char *nombre_solicitado){
    for (int i = 0; i < num_interfaces; i++)
    {
        if (strcmp(interfaces[i].nombre, nombre_solicitado) == 0)
        {
            return true;
        }
    }
    return false;
}

bool es_operacion_compatible(op_code tipo, op_code operacion){
    switch (tipo)
    {
    case GENERICA_I:
        return operacion == IO_GEN_SLEEP;
    case STDIN_I:
        return operacion == IO_STDIN_READ;
    case STDOUT_I:
        return operacion == IO_STDOUT_WRITE;
    case DIALFS_I:
        return operacion == IO_FS_CREATE || operacion == IO_FS_DELETE || operacion == IO_FS_TRUNCATE || operacion == IO_FS_WRITE || operacion == IO_FS_READ;
    default:
        return false;
    }
}

void inicializar_registro(){
    registro.capacidad = 10;
    registro.cantidad = 0;
    registro.interfaces = malloc(registro.capacidad * sizeof(ListaIO));
    if (registro.interfaces == NULL)
    {
        perror("Error al asignar memoria para registro.interfaces");
        exit(EXIT_FAILURE);
    }
}

void liberar_registro(){
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

void conectar_interfaz(char *nombre_interfaz){
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

void desconectar_interfaz(char *nombre_interfaz){
    for (int i = 0; i < registro.cantidad; i++)
    {
        if (strcmp(registro.interfaces[i].nombre, nombre_interfaz) == 0)
        {
            registro.interfaces[i].conectada = false;
        }
    }
}

bool interfaz_conectada(char *nombre_interfaz){
    for (int i = 0; i < registro.cantidad; i++)
    {
        if (strcmp(registro.interfaces[i].nombre, nombre_interfaz) == 0)
        {
            return registro.interfaces[i].conectada;
        }
    }
    return false;
}

void esperar_interfaz_libre(){
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
