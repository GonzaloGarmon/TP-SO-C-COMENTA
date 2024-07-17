#include "entradasalida.h"

//REVISAR TODOS LOS LOGS

int main(int argc, char *argv[]){
    log_entradasalida = log_create("./entradasalida.log", "ENTRADASALIDA", 1, LOG_LEVEL_TRACE);
    log_info(log_entradasalida, "INICIA EL MODULO DE ENTRADASALIDA");
    
    nombre_interfaz = malloc(100 * sizeof(char));
    ruta_archivo = malloc(100 * sizeof(char));

    //ruta_completa = "/home/utnso/so-deploy/tp-2024-1c-GoC/entradasalida/config/";
    ruta_completa = "/home/utnso/tp-2024-1c-GoC/entradasalida/config/";
    printf("ingresa nombre interfaz: ");
    scanf("%99s", nombre_interfaz);
    printf("\n ingresa el config: ");
    scanf("%99s", ruta_archivo);

    char* ruta_final = malloc(strlen(ruta_completa) + strlen(ruta_archivo) + 1);
    strcpy(ruta_final,ruta_completa);
    strcat(ruta_final,ruta_archivo);

    crear_interfaz(nombre_interfaz, ruta_final);

    generar_conexiones();
    
    pthread_t atiende_cliente_kernel;
    pthread_create(&atiende_cliente_kernel, NULL, (void *)recibirOpKernel, (void *) (intptr_t) conexion_entradasalida_kernel);
    pthread_join(atiende_cliente_kernel,NULL);
   
    log_info(log_entradasalida, "Finalizo conexion con servidores");
    finalizar_programa();
    return 0;
}

void crear_interfaz(char *nombre_interfaz, char *ruta_archivo){
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
    } else if (strcmp(tipo_interfaz_txt, "STDIN") == 0) {
        // Inicializar la interfaz Stdin
        inicializar_interfaz_stdin(config_entradasalida, nombre_interfaz);
    } else if (strcmp(tipo_interfaz_txt, "STDOUT") == 0) {
        // Inicializar la interfaz Stdout
        inicializar_interfaz_stdout(config_entradasalida,nombre_interfaz);
    } else if (strcmp(tipo_interfaz_txt, "DIALFS") == 0) {
        // Inicializar la interfaz DialFS
        inicializar_interfaz_dialfs(config_entradasalida, nombre_interfaz);
    } else {
        log_warning(log_entradasalida, "Tipo de interfaz desconocido: %s", tipo_interfaz_txt);
    }
}

void finalizar_programa(){
    log_destroy(log_entradasalida);
    config_destroy(config_entradasalida);
}

void conexionRecMem(){
    pthread_t atiende_cliente_memoria; 
    pthread_create(&atiende_cliente_memoria, NULL, (void *)recibirOpMemoria, (void *) (intptr_t) conexion_entradasalida_memoria);
    pthread_detach(atiende_cliente_memoria);
}

void recibirOpKernel(int SOCKET_CLIENTE_KERNEL){
    int noFinalizar = 0;
    while(noFinalizar != -1){
        op_code operacion = recibir_operacion(SOCKET_CLIENTE_KERNEL);
        char* nombre_recibido = ""; // nombre que recibo en el paquete con la

        if(es_operacion_compatible(tipo,operacion) && nombre_interfaz == nombre_recibido){
            switch (operacion){
                case IO_GEN_SLEEP:
                    int unidades = recibir_entero_uint32(SOCKET_CLIENTE_KERNEL,log_entradasalida);

                    pthread_t ejecutar_sleep; 
                    pthread_create(&ejecutar_sleep, NULL, (void *)funcIoGenSleep, (void *) (intptr_t) unidades);
                    pthread_detach(ejecutar_sleep);
                    //funcIoGenSleep(unidades);
                break;
                case IO_STDIN_READ:
                    funcIoStdRead(1,1);
                break;
                case IO_STDOUT_WRITE:
                    funcIoStdWrite(1,1);
                break;
                case IO_FS_READ:
                    funcIoFsRead();
                break;
                case IO_FS_WRITE:
                    funcIoFsWrite();
                break;
                case IO_FS_CREATE:
                    funcIoFsCreate();
                break;
                case IO_FS_DELETE:
                    funcIoFsDelete();
                case IO_FS_TRUNCATE:
                    funcIoFsTruncate();
                break;
                default:
                    log_warning(log_entradasalida, "Operacion no compatible");
                    noFinalizar = -1;
                break;
            }
        }
    }
}

void recibirOpMemoria(int SOCKET_CLIENTE_MEMORIA){
    op_code operacion = recibir_operacion(SOCKET_CLIENTE_MEMORIA);
    switch (operacion){
        case IO_STDOUT_WRITE_OK:
            log_info(log_entradasalida, "Mensaje escrito correctamente");
        break;
        case IO_STDIN_READ_OK:
            char* mensaje = recibir_string(SOCKET_CLIENTE_MEMORIA,log_entradasalida);
            log_info(log_entradasalida, "Mensaje escrito en esa direccion de memoria: %s",mensaje);
        break;
        case IO_FS_READ_OK:
            log_info(log_entradasalida, "Mensaje escrito correctamente");
        break;
        case IO_FS_WRITE_OK:
            log_info(log_entradasalida, "Mensaje escrito correctamente");
        break;
        default:
            log_warning(log_entradasalida, "Operacion no compatible");
        break;
    }
}

void funcIoGenSleep(int unidades){
    enUso = true;
    log_info(log_entradasalida, "Ejecutando operacion IO_GEN_Sleep...");
    sleep(unidades*tiempo_unidad_trabajo);
    log_info(log_entradasalida, "Operacion completada");
    enUso = false;
}

void funcIoStdRead(int direccion, int tamaño){
    enUso = true;
    log_info(log_entradasalida, "PID: <PID> - Operacion: <OPERACION_A_REALIZAR>");

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
    enviar_string(conexion_entradasalida_memoria, buffer, IO_STDIN_READ_OK);
    
    free(buffer);
    enUso = false;
}

void funcIoStdWrite(int direccion, int tamaño){
    enUso = true;
    enviar_entero(conexion_entradasalida_memoria,direccion,IO_STDOUT_WRITE_OK);
    conexionRecMem();
    enUso = false;
}

void funcIoFsRead(){
    enUso = true;
    conexionRecMem();
    enUso = false;
}

void funcIoFsWrite(){
    enUso = true;
    conexionRecMem();
    enUso = false;
}

void funcIoFsTruncate(){
    enUso = true;
    enUso = false;
}

void funcIoFsCreate(){
    enUso = true;
    enUso = false;
}

void funcIoFsDelete(){
    enUso = true;
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

void establecer_conexion_kernel(char *ip_kernel, char *puerto_kernel, t_config *config_entradasalida, t_log *loggs){
    log_trace(loggs, "Inicio como cliente Kernel");

    if ((conexion_entradasalida_kernel = crear_conexion(ip_kernel, puerto_kernel)) == -1)
    {
        log_error(loggs, "Error al conectar con Kernel. El servidor no está activo");
        return;
    }
    
    //BORRAR
    enviar_string(conexion_entradasalida_kernel, "hola papito", MENSAJE);
    log_trace(log_entradasalida, "mande un mensaje");

    sleep(3);
    enviar_string(conexion_entradasalida_kernel, tipo_interfaz_txt, IDENTIFICACION);
    log_trace(log_entradasalida, "mande un mensaje");
}

void establecer_conexion_memoria(char *ip_memoria, char *puerto_memoria, t_config *config_entradasalida, t_log *loggs){
    log_trace(loggs, "Inicio como cliente Memoria");

    if ((conexion_entradasalida_memoria = crear_conexion(ip_memoria, puerto_memoria)) == -1)
    {
        log_error(loggs, "Error al conectar con Memoria. El servidor no está activo");
        return;
    }

    //BORRAR
    sleep(3);
    int operacion = recibir_operacion(conexion_entradasalida_memoria);
    log_info(loggs, "Recibi operacion 1 %i", operacion);
    char* palabra = recibir_string(conexion_entradasalida_memoria, log_entradasalida);
    log_info(loggs, "Recibi operacion 2 %s", palabra);

    enviar_string(conexion_entradasalida_memoria,"mando saludos", MENSAJE);  
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

bool es_operacion_compatible(op_code tipo, op_code operacion){
    switch (tipo){
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

//Ver si se necesita
void esperar_interfaz_libre(){
    pthread_mutex_lock(&mutex);
    while (true){
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
    log_info(log_entradasalida, "DialFS - Inicio Compactación: PID: <PID> - Inicio Compactación.");
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
    log_info(log_entradasalida, "DialFS - Inicio Compactación: PID: <PID> - Inicio Compactación.");
}

// Función para destruir el sistema de archivos DialFS y liberar memoria
void dialfs_destroy(DialFS *dialfs) {
    log_info(log_entradasalida, "DialFS - Inicio Compactación: PID: <PID> - Inicio Compactación.");
    free(dialfs->bitmap);
    for (int i = 0; i < dialfs->num_blocks; ++i) {
        free(dialfs->blocks[i].data);
    }
    free(dialfs->blocks);
    log_info(log_entradasalida, "DialFS - Inicio Compactación: PID: <PID> - Inicio Compactación.");
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
    log_info(log_entradasalida, "DialFS - Inicio Compactación: PID: <PID> - Inicio Compactación.");
    if (block_index >= 0 && block_index < fs->num_blocks) {
        fs->bitmap[block_index] = 0; // Marcar como libre
    }
    log_info(log_entradasalida, "DialFS - Inicio Compactación: PID: <PID> - Inicio Compactación.");
}

// Función para escribir en un bloque
void dialfs_write_block(DialFS *fs, int block_index, const uint8_t *data, size_t size) {
    log_info(log_entradasalida, "DialFS - Inicio Compactación: PID: <PID> - Inicio Compactación.");
    if (block_index >= 0 && block_index < fs->num_blocks) {
        memcpy(fs->blocks[block_index].data, data, size);
    }
    log_info(log_entradasalida, "DialFS - Inicio Compactación: PID: <PID> - Inicio Compactación.");
}

// Función para leer desde un bloque
void dialfs_read_block(DialFS *fs, int block_index, uint8_t *buffer, size_t size) {
    log_info(log_entradasalida, "DialFS - Inicio Compactación: PID: <PID> - Inicio Compactación.");
    if (block_index >= 0 && block_index < fs->num_blocks) {
        memcpy(buffer, fs->blocks[block_index].data, size);
    }
    log_info(log_entradasalida, "DialFS - Inicio Compactación: PID: <PID> - Inicio Compactación.");
}

// Función para crear un archivo en DialFS
int dialfs_crear_archivo(DialFS *fs, const char *nombre_archivo, const uint8_t *datos, size_t size) {
    log_info(log_entradasalida, "DialFS - Inicio Compactación: PID: <PID> - Inicio Compactación.");
    // Buscar un bloque libre para el archivo
    int bloque = dialfs_allocate_block(fs);
    if (bloque == -1) {
        fprintf(stderr, "No hay suficientes bloques libres para crear el archivo.\n");
        return -1;
    }

    // Escribir datos en el bloque asignado
    dialfs_write_block(fs, bloque, datos, size);

    // Aquí podrías implementar la lógica para mantener un registro de archivos creados, por ejemplo:
    log_info(log_entradasalida, "DialFS - Inicio Compactación: PID: <PID> - Inicio Compactación.");
    return bloque;
}

// Función para redimensionar un archivo en DialFS
void dialfs_redimensionar_archivo(DialFS *fs, int bloque_archivo, const uint8_t *nuevos_datos, size_t nuevo_size) {
    log_info(log_entradasalida, "DialFS - Inicio Compactación: PID: <PID> - Inicio Compactación.");
    if (bloque_archivo < 0 || bloque_archivo >= fs->num_blocks) {
        fprintf(stderr, "El bloque de archivo proporcionado (%d) no es válido.\n", bloque_archivo);
        return;
    }

    // Escribir los nuevos datos en el bloque existente
    dialfs_write_block(fs, bloque_archivo, nuevos_datos, nuevo_size);

    log_info(log_entradasalida, "DialFS - Inicio Compactación: PID: <PID> - Inicio Compactación.");
}

// Función para compactar archivos en DialFS
void dialfs_compactar_archivos(DialFS *fs) {
    log_info(log_entradasalida, "DialFS - Inicio Compactación: PID: <PID> - Inicio Compactación.");
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

    log_info(log_entradasalida, "DialFS - Fin Compactación: PID: <PID> - Fin Compactación.");
}

