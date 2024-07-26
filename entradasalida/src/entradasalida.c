#include "entradasalida.h"

//REVISAR TODOS LOS LOGS

int main(int argc, char *argv[]){
    log_entradasalida = log_create("./entradasalida.log", "ENTRADASALIDA", 1, LOG_LEVEL_TRACE);
    log_info(log_entradasalida, "INICIA EL MODULO DE ENTRADASALIDA");
    
    nombre_interfaz = malloc(100 * sizeof(char));
    ruta_archivo = malloc(100 * sizeof(char));

    //ruta_completa = "/home/utnso/so-deploy/tp-2024-1c-GoC/entradasalida/config/";
    ruta_completa = "/home/utnso/tp-2024-1c-GoC/entradasalida/config/";
    printf("Ingresa nombre de interfaz: ");
    scanf("%99s", nombre_interfaz);
    printf("\nIngresa el config: ");
    scanf("%99s", ruta_archivo);

    char* ruta_final = malloc(strlen(ruta_completa) + strlen(ruta_archivo) + 1);
    strcpy(ruta_final,ruta_completa);
    strcat(ruta_final,ruta_archivo);

    crear_interfaz(nombre_interfaz, ruta_final);

    generar_conexiones();
    inicializar_listas();
    
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
    free(nombre_interfaz);
    free(ruta_archivo);         

    log_destroy(log_entradasalida);
    config_destroy(config_entradasalida);
    if(tipoInterfaz == DIALFS_I){
        dialfs_destroy(&fs);
    }
}

void conexionRecMem(){
    pthread_t atiende_cliente_memoria; 
    pthread_create(&atiende_cliente_memoria, NULL, (void *)recibirOpMemoria, (void *) (intptr_t) conexion_entradasalida_memoria);
    pthread_detach(atiende_cliente_memoria);
}

void recibirOpKernel(int SOCKET_CLIENTE_KERNEL) {
    int noFinalizar = 0;
    while (noFinalizar != -1) {

        recibir_y_procesar_paquete(SOCKET_CLIENTE_KERNEL);

        int *operacionActualPtr = (int *)list_get(lista_operaciones, 0);
        int operacionActual = *operacionActualPtr;
        
        bool operacionRealizada = false;

        switch (operacionActual) {
            case IO_GEN_SLEEP:
                if (strcmp(nombreInterfazRecibido, nombre_interfaz) == 0 && es_operacion_compatible(tipoInterfaz, operacionActual)) {
                    pthread_t ejecutar_sleep; 
                    log_info(log_entradasalida, "entre a gen sleep");
                    pthread_create(&ejecutar_sleep,NULL,(void*) funcIoGenSleep, (void*) (intptr_t) &operacionRealizada);
                    pthread_detach(ejecutar_sleep);
                    enviar_entero(SOCKET_CLIENTE_KERNEL,pidRecibido,TERMINO_INTERFAZ);
                    operacionRealizada = true;
                }
                break;
            case IO_STDIN_READ:
                if (strcmp(nombreInterfazRecibido, nombre_interfaz) == 0 && es_operacion_compatible(tipoInterfaz, operacionActual)) {
                    pthread_t ejecutar_StdRead; 
                    pthread_create(&ejecutar_StdRead, NULL, (void *)funcIoStdRead, NULL);
                    pthread_detach(ejecutar_StdRead);
                    enviar_entero(SOCKET_CLIENTE_KERNEL,pidRecibido,TERMINO_INTERFAZ);
                    operacionRealizada = true;
                }
                break;
            case IO_STDOUT_WRITE:
                if (strcmp(nombreInterfazRecibido, nombre_interfaz) == 0 && es_operacion_compatible(tipoInterfaz, operacionActual)) {
                    pthread_t ejecutar_StdWrite; 
                    pthread_create(&ejecutar_StdWrite, NULL, (void *)funcIoStdWrite, NULL);
                    pthread_detach(ejecutar_StdWrite);
                    enviar_entero(SOCKET_CLIENTE_KERNEL,pidRecibido,TERMINO_INTERFAZ);
                    operacionRealizada = true;
                }
                break;
            case IO_FS_READ:
                if (strcmp(nombreInterfazRecibido, nombre_interfaz) == 0 && es_operacion_compatible(tipoInterfaz, operacionActual)) {
                    pthread_t ejecutar_FsRead; 
                    pthread_create(&ejecutar_FsRead, NULL, (void *)funcIoFsRead, NULL);
                    pthread_detach(ejecutar_FsRead);
                    enviar_entero(SOCKET_CLIENTE_KERNEL,pidRecibido,TERMINO_INTERFAZ);
                    operacionRealizada = true;
                }
                break;
            case IO_FS_WRITE:
                if (strcmp(nombreInterfazRecibido, nombre_interfaz) == 0 && es_operacion_compatible(tipoInterfaz, operacionActual)) {
                    pthread_t ejecutar_FsWrite; 
                    pthread_create(&ejecutar_FsWrite, NULL, (void *)funcIoFsWrite, NULL);
                    pthread_detach(ejecutar_FsWrite);
                    enviar_entero(SOCKET_CLIENTE_KERNEL,pidRecibido,TERMINO_INTERFAZ);
                    operacionRealizada = true;
                }
                break;
            case IO_FS_CREATE:
                if (strcmp(nombreInterfazRecibido, nombre_interfaz) == 0 && es_operacion_compatible(tipoInterfaz, operacionActual)) {
                    pthread_t ejecutar_FsCreate; 
                    pthread_create(&ejecutar_FsCreate, NULL, (void *)funcIoFsCreate, NULL);
                    pthread_detach(ejecutar_FsCreate);
                    enviar_entero(SOCKET_CLIENTE_KERNEL,pidRecibido,TERMINO_INTERFAZ);
                    operacionRealizada = true;
                }
                break;
            case IO_FS_DELETE:
                if (strcmp(nombreInterfazRecibido, nombre_interfaz) == 0 && es_operacion_compatible(tipoInterfaz, operacionActual)) {
                    pthread_t ejecutar_FsDelete; 
                    pthread_create(&ejecutar_FsDelete, NULL, (void *)funcIoFsDelete, NULL);
                    pthread_detach(ejecutar_FsDelete);
                    enviar_entero(SOCKET_CLIENTE_KERNEL,pidRecibido,TERMINO_INTERFAZ);
                    operacionRealizada = true;
                }
                break;
            case IO_FS_TRUNCATE:
                if (strcmp(nombreInterfazRecibido, nombre_interfaz) == 0 && es_operacion_compatible(tipoInterfaz, operacionActual)) {
                    pthread_t ejecutar_FsTruncate; 
                    pthread_create(&ejecutar_FsTruncate, NULL, (void *)funcIoFsTruncate, NULL);
                    pthread_detach(ejecutar_FsTruncate);
                    enviar_entero(SOCKET_CLIENTE_KERNEL,pidRecibido,TERMINO_INTERFAZ);
                    operacionRealizada = true;
                }
                break;
            case -1:
                noFinalizar =operacionActual;
                break;
            default:
                break;
        }

        if (operacionRealizada) {
            avanzar_a_siguiente_operacion();
        }
    }
}

void recibir_y_procesar_paquete(int socket_cliente) {
    int size = 0;
    int desplazamiento = 0;
    char* buffer;

    // Recibir operación
    int operacion = recibir_operacion(socket_cliente);
    log_info(log_entradasalida, "recibi codigo: %d", operacion);

    int *operacionPtr = malloc(sizeof(int));
    *operacionPtr = operacion;
    list_add(lista_operaciones, operacionPtr);


    // Recibir el buffer
    buffer = recibir_buffer(&size, socket_cliente);

    // Leer pidRecibido
    /*
    if (desplazamiento + sizeof(uint32_t) > size) {
        printf("Error: Desplazamiento fuera de rango al leer pidRecibido\n");
        free(buffer);
        return;
    }
    */
    pidRecibido = leer_entero_uint32(buffer, &desplazamiento);
    int *pidPtr = malloc(sizeof(int));
    *pidPtr = pidRecibido;
    list_add(lista_pids, pidPtr);

    // Procesar según operación
    switch (operacion) {
        case IO_GEN_SLEEP:
            //if (desplazamiento < size) {
                nombreInterfazRecibido = leer_string(buffer, &desplazamiento);
                log_info(log_entradasalida, "nombnre interfaz: %s", nombreInterfazRecibido);
                list_add(lista_datos, nombreInterfazRecibido);

            
            //} else {
              //  printf("Error: No se puede leer nombreInterfazRecibido\n");
            //}
            //if (desplazamiento + sizeof(uint32_t) <= size) {
                unidadesRecibidas = leer_entero_uint32(buffer, &desplazamiento);
                log_info(log_entradasalida, "unidades: %d", unidadesRecibidas);
                list_add(lista_datos, malloc_copiar_uint32(unidadesRecibidas));
            //}
            break;
        case IO_STDIN_READ:
        case IO_STDOUT_WRITE:
            if (desplazamiento < size) {
                nombreInterfazRecibido = leer_string(buffer, &desplazamiento);
                list_add(lista_datos, nombreInterfazRecibido);
            } else {
                printf("Error: No se puede leer nombreInterfazRecibido\n");
            }
            if (desplazamiento + sizeof(uint32_t) * 2 <= size) {
                direccionRecibida = leer_entero_uint32(buffer, &desplazamiento);
                tamañoRecibido = leer_entero_uint32(buffer, &desplazamiento);
                list_add(lista_datos, malloc_copiar_uint32(direccionRecibida));
                list_add(lista_datos, malloc_copiar_uint32(tamañoRecibido));
            }
            break;
        case IO_FS_CREATE:
        case IO_FS_DELETE:
            if (desplazamiento < size) {
                nombreInterfazRecibido = leer_string(buffer, &desplazamiento);
                list_add(lista_datos, nombreInterfazRecibido);
            } else {
                printf("Error: No se puede leer nombreInterfazRecibido\n");
            }
            if (desplazamiento < size) {
                nombreArchivoRecibido = leer_string(buffer, &desplazamiento);
                list_add(lista_datos, nombreArchivoRecibido);
            }
            break;
        case IO_FS_TRUNCATE:
            if (desplazamiento < size) {
                nombreInterfazRecibido = leer_string(buffer, &desplazamiento);
                list_add(lista_datos, nombreInterfazRecibido);
            } else {
                printf("Error: No se puede leer nombreInterfazRecibido\n");
            }
            if (desplazamiento < size) {
                nombreArchivoRecibido = leer_string(buffer, &desplazamiento);
                list_add(lista_datos, nombreArchivoRecibido);
            }
            if (desplazamiento + sizeof(uint32_t) <= size) {
                tamañoRecibido = leer_entero_uint32(buffer, &desplazamiento);
                list_add(lista_datos, malloc_copiar_uint32(tamañoRecibido));
            }
            break;
        case IO_FS_WRITE:
        case IO_FS_READ:
            if (desplazamiento < size) {
                nombreInterfazRecibido = leer_string(buffer, &desplazamiento);
                list_add(lista_datos, nombreInterfazRecibido);
            } else {
                printf("Error: No se puede leer nombreInterfazRecibido\n");
            }
            if (desplazamiento < size) {
                nombreArchivoRecibido = leer_string(buffer, &desplazamiento);
                list_add(lista_datos, nombreArchivoRecibido);
            }
            if (desplazamiento + sizeof(uint32_t) * 3 <= size) {
                direccionRecibida = leer_entero_uint32(buffer, &desplazamiento);
                tamañoRecibido = leer_entero_uint32(buffer, &desplazamiento);
                registroPunteroArchivoRecibido = leer_entero_uint32(buffer, &desplazamiento);
                list_add(lista_datos, malloc_copiar_uint32(direccionRecibida));
                list_add(lista_datos, malloc_copiar_uint32(tamañoRecibido));
                list_add(lista_datos, malloc_copiar_uint32(registroPunteroArchivoRecibido));
            }
            break;
        default:
            break;
    }

    free(buffer);
}

uint32_t* malloc_copiar_uint32(uint32_t valor) {
    uint32_t* ptr = malloc(sizeof(uint32_t));
    if (ptr) {
        *ptr = valor;
    }
    return ptr;
}


void recibirOpMemoria(int SOCKET_CLIENTE_MEMORIA){
    op_code operacion = recibir_operacion(SOCKET_CLIENTE_MEMORIA);
    char *mensaje;
    switch (operacion){
        case IO_STDIN_READ:
            log_info(log_entradasalida, "Mensaje escrito correctamente");
        break;
        case IO_STDOUT_WRITE:
            mensaje = recibir_string(SOCKET_CLIENTE_MEMORIA,log_entradasalida);
            log_info(log_entradasalida, "Valor escrito en memoria: %p", mensaje);
        break;
        case IO_FS_READ:
            log_info(log_entradasalida, "Mensaje escrito correctamente");
        break;
        case IO_FS_WRITE:
            mensaje = recibir_string(SOCKET_CLIENTE_MEMORIA,log_entradasalida);
            dialfs_escribir_archivo(&fs,nombreArchivoRecibido,registroPunteroArchivoRecibido,tamañoRecibido,mensaje);
        break;
        default:
        break;
    }
}

void funcIoGenSleep(int operacion){
    log_info(log_entradasalida, "Generica: PID: <%d> - Gen_Sleep",pidRecibido);
    int unidades = unidadesRecibidas*tiempo_unidad_trabajo;
    log_info(log_entradasalida, "Eperando durante %d unidades",unidades);
    usleep(unidadesRecibidas*tiempo_unidad_trabajo);
    

    log_info(log_entradasalida, "Operacion completada");
    operacion = true;
}

void funcIoStdRead(){
    log_info(log_entradasalida, "Stdin: PID: <%d> - Leer.",pidRecibido);

    char *buffer = (char *)malloc(tamañoRecibido + 1);

    // Leer entrada del usuario
    if (fgets(buffer, tamañoRecibido + 1, stdin) == NULL) {
        log_error(log_entradasalida, "Error al leer desde STDIN");
        free(buffer);
        return;
    }

    // Validar que el tamaño de la entrada no exceda el tamaño especificado
    if (strlen(buffer) > tamañoRecibido) {
        log_warning(log_entradasalida, "El texto ingresado es mayor al tamaño permitido. Se truncará.");
        buffer[tamañoRecibido] = '\0';
    }

    t_string_3enteros *mensaje = malloc(sizeof(t_string_3enteros));
    mensaje->entero1 = direccionRecibida;
    mensaje->entero2 = pidRecibido;
    mensaje->entero3 = tamañoRecibido;
    mensaje->string=buffer;
    // Escribir el valor en la memoria. se encarga memoria yo solo le envio lo que escribio el usuario
    enviar_3_enteros_1_string(conexion_entradasalida_memoria, mensaje, IO_STDIN_READ);

    //recibo un OK de memoria
    conexionRecMem();
    free(buffer);
}

void funcIoStdWrite(){
    log_info(log_entradasalida, "Stdin: PID: <%d> - Escribir.",pidRecibido);

    t_3_enteros *mensaje = malloc(sizeof(t_3_enteros));
    mensaje->entero1 = direccionRecibida;
    mensaje->entero2 = pidRecibido;
    mensaje->entero3 = tamañoRecibido;
    enviar_3_enteros(conexion_entradasalida_memoria,mensaje,IO_STDOUT_WRITE);

    //Que memoria me pase lo leido y yo lo muestro en pantalla
    conexionRecMem();
}

void funcIoFsWrite(){
    log_info(log_entradasalida, "DialFS: PID: <%d> - Leer archivo.",pidRecibido);

    //envio a memoria un paquete con el RegistroTamaño y el RegistroDireccion recibidos de kernel
    t_3_enteros *mensaje = malloc(sizeof(t_3_enteros));
    mensaje->entero1 = direccionRecibida;
    mensaje->entero2 = pidRecibido;
    mensaje->entero3 = tamañoRecibido;
    enviar_3_enteros(conexion_entradasalida_memoria,mensaje,IO_FS_WRITE);

    //Escribo el valor en el archivo
    conexionRecMem();
}

void funcIoFsRead(){
    log_info(log_entradasalida, "DialFS: PID: <%d> - Escribir Archivo.",pidRecibido);

    dialfs_leer_archivo(&fs,nombreArchivoRecibido,direccionRecibida,tamañoRecibido,registroPunteroArchivoRecibido);

    //envio a memoria un paquete con el RegistroTamaño y el RegistroDireccion recibidos de kernel
    t_string_3enteros *mensaje = malloc(sizeof(t_string_3enteros));
    mensaje->entero1 = direccionRecibida;
    mensaje->entero2 = pidRecibido;
    mensaje->entero3 = tamañoRecibido;
    mensaje->string = mensajeLeido;
    enviar_3_enteros_1_string(conexion_entradasalida_memoria,mensaje,IO_FS_READ);

    //recibo un OK de memoria o podemos hacer que se muestre lo que se escribio
    conexionRecMem();
}

void funcIoFsTruncate() {
    dialfs_truncar_archivo(&fs, nombreArchivoRecibido, tamañoRecibido);
}

void funcIoFsCreate() {
    int bloque = dialfs_crear_archivo(&fs, nombreArchivoRecibido, tamañoRecibido);

    if (bloque == -1) {
        return;
    }

    log_info(log_entradasalida, "Archivo creado con éxito: %s en bloque %d", nombreArchivoRecibido, bloque);
}

void funcIoFsDelete() {
    dialfs_eliminar_archivo(&fs,nombreArchivoRecibido);
}

void generar_conexiones(){   
    if(tipoInterfaz != GENERICA_I){
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
    
    enviar_string(conexion_entradasalida_kernel, nombre_interfaz, IDENTIFICACION);

}

void establecer_conexion_memoria(char *ip_memoria, char *puerto_memoria, t_config *config_entradasalida, t_log *loggs){
    log_trace(loggs, "Inicio como cliente Memoria");

    if ((conexion_entradasalida_memoria = crear_conexion(ip_memoria, puerto_memoria)) == -1)
    {
        log_error(loggs, "Error al conectar con Memoria. El servidor no está activo");
        return;
    }

}

void inicializar_interfaz_generica(t_config *config_entradasalida, const char *nombre){
    // Inicializar atributos de la interfaz genérica
    tipoInterfaz = GENERICA_I;
    tiempo_unidad_trabajo = config_get_int_value(config_entradasalida, "TIEMPO_UNIDAD_TRABAJO");
    ip_kernel = config_get_string_value(config_entradasalida, "IP_KERNEL");
    puerto_kernel = config_get_string_value(config_entradasalida, "PUERTO_KERNEL");

    //Ver si es necesario una funcion para mostrar interfaces, esto es para ver si las crea bien
    printf("Interfaz Generica creada:\n");
    printf("  Nombre: %s\n", nombre);
    printf("  Tipo de interfaz: GENERICA\n");
    printf("  Tiempo de unidad de trabajo: %d\n", tiempo_unidad_trabajo);
    printf("  IP Kernel: %s\n", ip_kernel);
    printf("  Puerto Kernel: %s\n", puerto_kernel);
}

void inicializar_interfaz_stdin(t_config *config_entradasalida, const char *nombre){
    // Inicializar atributos de la interfaz stdin
    tipoInterfaz = STDIN_I;
    tiempo_unidad_trabajo = config_get_int_value(config_entradasalida, "TIEMPO_UNIDAD_TRABAJO");
    ip_kernel = config_get_string_value(config_entradasalida, "IP_KERNEL");
    puerto_kernel = config_get_string_value(config_entradasalida, "PUERTO_KERNEL");
    ip_memoria = config_get_string_value(config_entradasalida, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_entradasalida, "PUERTO_MEMORIA");

    printf("Interfaz StdIn creada:\n");
    printf("  Nombre: %s\n", nombre);
    printf("  Tipo de interfaz: STDIN\n");
    printf("  Tiempo de unidad de trabajo: %d\n", tiempo_unidad_trabajo);
    printf("  IP Kernel: %s\n", ip_kernel);
    printf("  Puerto Kernel: %s\n", puerto_kernel);
    printf("  IP Memoria: %s\n", ip_memoria);
    printf("  Puerto Memoria: %s\n", puerto_memoria);
}

void inicializar_interfaz_stdout(t_config *config_entradasalida, const char *nombre){
    // Inicializar atributos de la interfaz stdout
    tipoInterfaz = STDOUT_I;
    tiempo_unidad_trabajo = config_get_int_value(config_entradasalida, "TIEMPO_UNIDAD_TRABAJO");
    ip_kernel = config_get_string_value(config_entradasalida, "IP_KERNEL");
    puerto_kernel = config_get_string_value(config_entradasalida, "PUERTO_KERNEL");
    ip_memoria = config_get_string_value(config_entradasalida, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_entradasalida, "PUERTO_MEMORIA");

    printf("Interfaz StdOut creada:\n");
    printf("  Nombre: %s\n", nombre);
    printf("  Tipo de interfaz: STDOUT\n");
    printf("  Tiempo de unidad de trabajo: %d\n", tiempo_unidad_trabajo);
    printf("  IP Kernel: %s\n", ip_kernel);
    printf("  Puerto Kernel: %s\n", puerto_kernel);
    printf("  IP Memoria: %s\n", ip_memoria);
    printf("  Puerto Memoria: %s\n", puerto_memoria);
}

void inicializar_interfaz_dialfs(t_config *config_entradasalida, const char *nombre){
    // Inicializar atributos de la interfaz stdout
    tipoInterfaz = DIALFS_I;
    tiempo_unidad_trabajo = config_get_int_value(config_entradasalida, "TIEMPO_UNIDAD_TRABAJO");
    ip_kernel = config_get_string_value(config_entradasalida, "IP_KERNEL");
    puerto_kernel = config_get_string_value(config_entradasalida, "PUERTO_KERNEL");
    ip_memoria = config_get_string_value(config_entradasalida, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_entradasalida, "PUERTO_MEMORIA");
    path_base_dialfs = config_get_string_value(config_entradasalida, "PATH_BASE_DIALFS");
    block_size = config_get_int_value(config_entradasalida, "BLOCK_SIZE");
    block_count = config_get_int_value(config_entradasalida, "BLOCK_COUNT");
    retraso_compactacion = config_get_int_value(config_entradasalida, "RETRASO_COMPACTACION");

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

    // Inicializar el sistema de archivos
    dialfs_init(&fs, block_size, block_count,path_base_dialfs);
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

void dialfs_init(DialFS *dialfs, int block_size, int block_count, const char *path_base) {
    dialfs->num_blocks = block_count;
    dialfs->block_size = block_size;

    // Guardar el path base
    dialfs->path_base = strdup(path_base);

    // Inicialización del bitmap de bloques
    dialfs->bitmap = (int *)malloc(block_count * sizeof(int));
    if (dialfs->bitmap == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria para el bitmap de bloques\n");
        exit(EXIT_FAILURE);
    }
    memset(dialfs->bitmap, 0, block_count * sizeof(int));

    // Inicialización de los bloques de datos
    dialfs->blocks = (Block *)malloc(block_count * sizeof(Block));
    if (dialfs->blocks == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria para los bloques de datos\n");
        free(dialfs->bitmap); // Liberar el bitmap antes de salir
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < block_count; ++i) {
        dialfs->blocks[i].data = (uint8_t *)malloc(block_size * sizeof(uint8_t));
        if (dialfs->blocks[i].data == NULL) {
            fprintf(stderr, "Error: No se pudo asignar memoria para los datos del bloque %d\n", i);
            // Liberar la memoria asignada hasta ahora
            for (int j = 0; j < i; ++j) {
                free(dialfs->blocks[j].data);
            }
            free(dialfs->blocks);
            free(dialfs->bitmap);
            exit(EXIT_FAILURE);
        }
        memset(dialfs->blocks[i].data, 0, block_size * sizeof(uint8_t)); // Inicializar los datos a 0
    }

    // Inicialización de la lista de archivos
    dialfs->archivos = list_create();
}

void dialfs_destroy(DialFS *fs) {
    usleep(tiempo_unidad_trabajo);

    // Liberar la memoria de los archivos
    for (int i = 0; i < list_size(fs->archivos); ++i) {
        Archivo *archivo = (Archivo *)list_get(fs->archivos, i);
        free(archivo->nombre_archivo);
        free(archivo);
    }
    list_destroy_and_destroy_elements(fs->archivos, (void *)free);

    // Liberar los bloques y el bitmap
    for (int i = 0; i < fs->num_blocks; ++i) {
        free(fs->blocks[i].data);
    }
    free(fs->blocks);
    free(fs->bitmap);
    free(fs->path_base);
    log_info(log_entradasalida, "DialFS: PID: <%d> - Sistema de archivos destruido.", pidRecibido);
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

// Función para crear un archivo en DialFS
int dialfs_crear_archivo(DialFS *fs, const char *nombre_archivo, size_t tamaño) {
    usleep(tiempo_unidad_trabajo);

    // Buscar un bloque libre para el archivo
    int bloque = dialfs_allocate_block(fs);
    if (bloque == -1) {
        fprintf(stderr, "No hay suficientes bloques libres para crear el archivo.\n");
        return -1;
    }

    // Crear la estructura para el archivo
    Archivo *nuevo_archivo = malloc(sizeof(Archivo));
    if (nuevo_archivo == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria para el nuevo archivo\n");
        return -1;
    }
    nuevo_archivo->nombre_archivo = strdup(nombre_archivo);
    nuevo_archivo->bloque_inicio = bloque;
    nuevo_archivo->tamaño = tamaño;

    // Agregar el archivo a la lista de archivos
    list_add(fs->archivos, nuevo_archivo);

    // Crear el archivo de metadata en el path base
    char *path = malloc(strlen(fs->path_base) + strlen(nombre_archivo) + 2); // 1 para '/' y 1 para '\0'
    sprintf(path, "%s/%s", fs->path_base, nombre_archivo);

    FILE *archivo_fisico = fopen(path, "w");
    if (archivo_fisico == NULL) {
        fprintf(stderr, "Error: No se pudo crear el archivo físico %s\n", path);
        free(nuevo_archivo->nombre_archivo);
        free(nuevo_archivo);
        return -1;
    }
    fprintf(archivo_fisico, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%zu\n", bloque, tamaño);
    fclose(archivo_fisico);
    free(path);

    log_info(log_entradasalida, "DialFS: PID: <%d> - Crear Archivo: %s.", pidRecibido, nombre_archivo);
    return bloque;
}

// Función para eliminar un archivo en DialFS
void dialfs_eliminar_archivo(DialFS *fs, const char *nombre_archivo) {
    usleep(tiempo_unidad_trabajo);
    Archivo *archivo = NULL;

    // Buscar el archivo en la lista de archivos
    for (int i = 0; i < list_size(fs->archivos); ++i) {
        Archivo *a = (Archivo *)list_get(fs->archivos, i);
        if (strcmp(a->nombre_archivo, nombre_archivo) == 0) {
            archivo = a;
            list_remove(fs->archivos, i);
            break;
        }
    }

    if (archivo == NULL) {
        fprintf(stderr, "Archivo '%s' no encontrado.\n", nombre_archivo);
        return;
    }

    // Liberar los bloques usados por el archivo
    size_t bloques_usados = (archivo->tamaño + fs->block_size - 1) / fs->block_size;
    for (size_t i = 0; i < bloques_usados; ++i) {
        dialfs_free_block(fs, archivo->bloque_inicio + i);
    }

    // Eliminar la estructura del archivo
    free(archivo->nombre_archivo);
    free(archivo);

    // Eliminar el archivo de metadata en el path base
    char *path = malloc(strlen(fs->path_base) + strlen(nombre_archivo) + 2); // 1 para '/' y 1 para '\0'
    sprintf(path, "%s/%s", fs->path_base, nombre_archivo);
    remove(path);
    free(path);

    log_info(log_entradasalida, "DialFS: PID: <%d> - Eliminar Archivo: %s.", pidRecibido, nombre_archivo);
}


// Función para redimensionar un archivo en DialFS
void dialfs_truncar_archivo(DialFS *fs, const char *nombre_archivo, size_t nuevo_size) {
    usleep(tiempo_unidad_trabajo);
    Archivo *archivo = NULL;

    // Buscar el archivo en la lista de archivos
    for (int i = 0; i < list_size(fs->archivos); ++i) {
        Archivo *a = (Archivo *)list_get(fs->archivos, i);
        if (strcmp(a->nombre_archivo, nombre_archivo) == 0) {
            archivo = a;
            break;
        }
    }

    if (archivo == NULL) {
        fprintf(stderr, "Archivo '%s' no encontrado.\n", nombre_archivo);
        return;
    }

    // Verificar si el tamaño del archivo necesita cambiar
    if (nuevo_size > archivo->tamaño) {
        // Necesita asignar más bloques si el nuevo tamaño es mayor
        size_t bytes_a_escribir = nuevo_size - archivo->tamaño;
        size_t bytes_por_bloque = fs->block_size;

        // Reasignar bloques si es necesario
        while (bytes_a_escribir > 0) {
            int bloque = dialfs_allocate_block(fs);
            if (bloque == -1) {
                fprintf(stderr, "No hay suficientes bloques libres para redimensionar el archivo.\n");
                return;
            }
            bytes_a_escribir -= bytes_por_bloque;
        }
    } else if (nuevo_size < archivo->tamaño) {
        // Liberar bloques si el nuevo tamaño es menor
        size_t bloques_actuales = (archivo->tamaño + fs->block_size - 1) / fs->block_size;
        size_t nuevos_bloques = (nuevo_size + fs->block_size - 1) / fs->block_size;

        for (size_t i = nuevos_bloques; i < bloques_actuales; ++i) {
            dialfs_free_block(fs, archivo->bloque_inicio + i);
        }
    }

    archivo->tamaño = nuevo_size;

    // Actualizar el archivo de metadata en el path base
    char *path = malloc(strlen(fs->path_base) + strlen(nombre_archivo) + 2); // 1 para '/' y 1 para '\0'
    sprintf(path, "%s/%s", fs->path_base, nombre_archivo);

    FILE *archivo_fisico = fopen(path, "w");
    if (archivo_fisico == NULL) {
        fprintf(stderr, "Error: No se pudo actualizar el archivo físico %s\n", path);
        return;
    }
    fprintf(archivo_fisico, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%zu\n", archivo->bloque_inicio, nuevo_size);
    fclose(archivo_fisico);
    free(path);

    log_info(log_entradasalida, "DialFS: PID: <%d> - Truncar Archivo: %s a tamaño %zu.", pidRecibido, nombre_archivo, nuevo_size);
}

// Función para escribir datos en un archivo en DialFS
void dialfs_escribir_archivo(DialFS *fs, const char *nombre_archivo, size_t offset, size_t size, const void *buffer) {
    usleep(tiempo_unidad_trabajo);
    Archivo *archivo = NULL;

    // Buscar el archivo en la lista de archivos
    for (int i = 0; i < list_size(fs->archivos); ++i) {
        Archivo *a = (Archivo *)list_get(fs->archivos, i);
        if (strcmp(a->nombre_archivo, nombre_archivo) == 0) {
            archivo = a;
            break;
        }
    }

    if (archivo == NULL) {
        fprintf(stderr, "Archivo '%s' no encontrado.\n", nombre_archivo);
        return;
    }

    if (offset + size > archivo->tamaño) {
        fprintf(stderr, "Error: Intento de escritura fuera de los límites del archivo.\n");
        return;
    }

    size_t bytes_escritos = 0;
    size_t bloque_actual = archivo->bloque_inicio + offset / fs->block_size;
    size_t desplazamiento_bloque = offset % fs->block_size;

    while (bytes_escritos < size) {
        size_t bytes_a_escribir = fs->block_size - desplazamiento_bloque;
        if (bytes_a_escribir > size - bytes_escritos) {
            bytes_a_escribir = size - bytes_escritos;
        }

        memcpy(fs->blocks[bloque_actual].data + desplazamiento_bloque, (const uint8_t *)buffer + bytes_escritos, bytes_a_escribir);
        bytes_escritos += bytes_a_escribir;
        bloque_actual++;
        desplazamiento_bloque = 0;
    }

    log_info(log_entradasalida, "DialFS: PID: <%d> - Escribir Archivo: %s desde offset %zu, tamaño %zu.", pidRecibido, nombre_archivo, offset, size);
}

// Función para encontrar un archivo en la lista de archivos
Archivo* buscar_archivo(DialFS *fs, const char *nombre_archivo) {
    for (int i = 0; i < list_size(fs->archivos); ++i) {
        Archivo *archivo = (Archivo *)list_get(fs->archivos, i);
        if (strcmp(archivo->nombre_archivo, nombre_archivo) == 0) {
            return archivo;
        }
    }
    return NULL;
}

// Función para leer un archivo en DialFS
void dialfs_leer_archivo(DialFS *fs, const char *nombre_archivo, int registro_direccion, int registro_tamaño, int registro_puntero_archivo) {
    usleep(tiempo_unidad_trabajo);

    // Encontrar el archivo en la lista de archivos
    Archivo *archivo = buscar_archivo(fs, nombre_archivo);
    if (archivo == NULL) {
        log_error(log_entradasalida, "DialFS: PID: <%d> - Archivo no encontrado: %s", pidRecibido, nombre_archivo);
        return;
    }

    // Verificar que la lectura no exceda el tamaño del archivo
    if (registro_puntero_archivo + registro_tamaño > archivo->tamaño) {
        log_error(log_entradasalida, "DialFS: PID: <%d> - Intento de lectura fuera de los límites del archivo: %s", pidRecibido, nombre_archivo);
        return;
    }

    // Calcular el bloque inicial y el desplazamiento dentro del bloque
    int bloque_inicial = archivo->bloque_inicio + (registro_puntero_archivo / fs->block_size);
    int desplazamiento_inicial = registro_puntero_archivo % fs->block_size;
    int bytes_por_leer = registro_tamaño;

    // Asignar memoria para mensajeLeido
    mensajeLeido = (char *)malloc(registro_tamaño + 1); // +1 para el terminador null
    if (mensajeLeido == NULL) {
        log_error(log_entradasalida, "DialFS: PID: <%d> - Error al asignar memoria para mensajeLeido", pidRecibido);
        return;
    }

    char *ptr_mensaje = mensajeLeido;

    // Leer los datos desde los bloques
    while (bytes_por_leer > 0) {
        int bytes_a_copiar = fs->block_size - desplazamiento_inicial;
        if (bytes_a_copiar > bytes_por_leer) {
            bytes_a_copiar = bytes_por_leer;
        }

        memcpy(ptr_mensaje, fs->blocks[bloque_inicial].data + desplazamiento_inicial, bytes_a_copiar);

        ptr_mensaje += bytes_a_copiar;
        bytes_por_leer -= bytes_a_copiar;
        desplazamiento_inicial = 0; // Sólo el primer bloque puede tener un desplazamiento
        bloque_inicial++;
    }

    // Agregar terminador null al final de mensajeLeido
    mensajeLeido[registro_tamaño] = '\0';

    log_info(log_entradasalida, "DialFS: PID: <%d> - Leer Archivo: %s - Tamaño a Leer: %d - Puntero Archivo: %d", pidRecibido, nombre_archivo, registro_tamaño, registro_puntero_archivo);
}

// Función para compactar archivos en DialFS
void dialfs_compactar_archivos(DialFS *fs) {
    log_info(log_entradasalida, "DialFS: PID: <%d> - Inicio Compactación.", pidRecibido);
    usleep(retraso_compactacion);
    int next_free_block = 0; // Siguiente bloque libre disponible

    // Recorrer todos los bloques del sistema de archivos
    for (int i = 0; i < fs->num_blocks; ++i) {
        if (fs->bitmap[i] == 1) {
            // Si el bloque está ocupado, necesitamos copiarlo al próximo bloque libre disponible
            if (i != next_free_block) {
                // Copiar datos al próximo bloque libre
                memcpy(fs->blocks[next_free_block].data, fs->blocks[i].data, fs->block_size);
                fs->bitmap[next_free_block] = 1; // Marcar el nuevo bloque como ocupado
                fs->bitmap[i] = 0; // Marcar el bloque original como libre
                
                // Actualizar el bloque de inicio en la lista de archivos
                for (int j = 0; j < list_size(fs->archivos); ++j) {
                    Archivo *archivo = (Archivo *)list_get(fs->archivos, j);
                    int bloques_usados = (archivo->tamaño + fs->block_size - 1) / fs->block_size;
                    if (archivo->bloque_inicio <= i && archivo->bloque_inicio + bloques_usados > i) {
                        archivo->bloque_inicio = next_free_block - (i - archivo->bloque_inicio);
                    }
                }
            }
            ++next_free_block; // Mover al siguiente bloque libre
        }
    }

    // Si algún archivo ha sido movido, actualizar su metadata en el sistema de archivos real
    for (int j = 0; j < list_size(fs->archivos); ++j) {
        Archivo *archivo = (Archivo *)list_get(fs->archivos, j);
        char *path = malloc(strlen(fs->path_base) + strlen(archivo->nombre_archivo) + 2); // 1 para '/' y 1 para '\0'
        sprintf(path, "%s/%s", fs->path_base, archivo->nombre_archivo);

        FILE *archivo_fisico = fopen(path, "w");
        if (archivo_fisico == NULL) {
            fprintf(stderr, "Error: No se pudo actualizar el archivo físico %s\n", path);
            free(path);
            continue;
        }
        fprintf(archivo_fisico, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%zu\n", archivo->bloque_inicio, archivo->tamaño);
        fclose(archivo_fisico);
        free(path);
    }

    log_info(log_entradasalida, "DialFS: PID: <%d> - Fin Compactación.", pidRecibido);
}

void inicializar_listas() {
    lista_operaciones = list_create();
    lista_pids = list_create();
    lista_datos = list_create();
}

void avanzar_a_siguiente_operacion() {
    if (!list_is_empty(lista_operaciones)) {
        list_remove(lista_operaciones, 0);  // Remueve la operación actual
    }
}