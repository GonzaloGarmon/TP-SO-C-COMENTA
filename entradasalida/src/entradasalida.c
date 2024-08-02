#include "entradasalida.h"

//REVISAR TODOS LOS LOGS

int main(int argc, char *argv[]){
    log_entradasalida = log_create("./entradasalida.log", "ENTRADASALIDA", 1, LOG_LEVEL_TRACE);
    log_info(log_entradasalida, "INICIA EL MODULO DE ENTRADASALIDA");

    nombre_interfaz = malloc(100 * sizeof(char));
    ruta_archivo = malloc(100 * sizeof(char));

    if (nombre_interfaz == NULL || ruta_archivo == NULL) {
        log_error(log_entradasalida, "Error al asignar memoria para nombre_interfaz o ruta_archivo");
        return EXIT_FAILURE;
    }

    ruta_completa = "/home/utnso/tp-2024-1c-GoC/entradasalida/config/";
    printf("Ingresa nombre de interfaz: ");
    scanf("%99s", nombre_interfaz);
    printf("\nIngresa el config: ");
    scanf("%99s", ruta_archivo);

    char* ruta_final = malloc(strlen(ruta_completa) + strlen(ruta_archivo) + 1);
    if (ruta_final == NULL) {
        log_error(log_entradasalida, "Error al asignar memoria para ruta_final");
        free(nombre_interfaz);
        free(ruta_archivo);
        return EXIT_FAILURE;
    }

    strcpy(ruta_final, ruta_completa);
    strcat(ruta_final, ruta_archivo);

    crear_interfaz(nombre_interfaz, ruta_final);

    generar_conexiones();
    inicializar_listas();

    pthread_t atiende_cliente_kernel;
    pthread_create(&atiende_cliente_kernel, NULL, (void *)recibirOpKernel, (void *)(intptr_t)conexion_entradasalida_kernel);
    pthread_join(atiende_cliente_kernel, NULL);

    free(ruta_final);
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
    liberar_conexion(conexion_entradasalida_kernel);
    liberar_conexion(conexion_entradasalida_memoria);
    free(nombre_interfaz);
    free(ruta_archivo);
    log_destroy(log_entradasalida);
    config_destroy(config_entradasalida);
    liberar_listas();

    if (tipoInterfaz == DIALFS_I) {
        dialfs_destroy(&fs);
    }
}

void recibirOpKernel(int SOCKET_CLIENTE_KERNEL) {
    int noFinalizar = 0;
    while (noFinalizar != -1) {
        recibir_y_procesar_paquete(SOCKET_CLIENTE_KERNEL);
        sem_wait(&sem_termino);
        int *operacionActualPtr = (int *)list_get(lista_operaciones, 0);
        
        int operacionActual = *operacionActualPtr;
        uint32_t pid = *(uint32_t *)list_get(lista_pids, 0);
        bool operacionRealizada = false;
        t_entero_bool* ejecucion = malloc(sizeof(t_entero_bool));
        ejecucion->entero = pid;
        ejecucion->operacion = operacionRealizada;

        switch (operacionActual) {
            case IO_GEN_SLEEP:
                if (strcmp(nombreInterfazRecibido, nombre_interfaz) == 0 && es_operacion_compatible(tipoInterfaz, operacionActual)) {
                    pthread_t ejecutar_sleep;
                    log_info(log_entradasalida, "entre a gen sleep");
                    pthread_create(&ejecutar_sleep, NULL, (void*)funcIoGenSleep, (void*)(intptr_t)&ejecucion);
                    pthread_detach(ejecutar_sleep);
                }
                break;
            case IO_STDIN_READ:
                if (strcmp(nombreInterfazRecibido, nombre_interfaz) == 0 && es_operacion_compatible(tipoInterfaz, operacionActual)) {
                    pthread_t ejecutar_StdRead;
                    log_info(log_entradasalida, "entre a read");
                    pthread_create(&ejecutar_StdRead, NULL, (void*)funcIoStdRead, (void*)(intptr_t)&ejecucion);
                    pthread_detach(ejecutar_StdRead);
                }
                break;
            case IO_STDOUT_WRITE:
                if (strcmp(nombreInterfazRecibido, nombre_interfaz) == 0 && es_operacion_compatible(tipoInterfaz, operacionActual)) {
                    pthread_t ejecutar_StdWrite;
                    log_info(log_entradasalida, "entre a write");
                    pthread_create(&ejecutar_StdWrite, NULL, (void*)funcIoStdWrite, (void*)(intptr_t)&ejecucion);
                    pthread_detach(ejecutar_StdWrite);
                }
                break;
            case IO_FS_READ:
                if (strcmp(nombreInterfazRecibido, nombre_interfaz) == 0 && es_operacion_compatible(tipoInterfaz, operacionActual)) {
                    pthread_t ejecutar_FsRead;
                    log_info(log_entradasalida, "entre a fs read");
                    pthread_create(&ejecutar_FsRead, NULL, (void*)funcIoFsRead, (void*)(intptr_t)&ejecucion);
                    pthread_detach(ejecutar_FsRead);
                }
                break;
            case IO_FS_WRITE:
                if (strcmp(nombreInterfazRecibido, nombre_interfaz) == 0 && es_operacion_compatible(tipoInterfaz, operacionActual)) {
                    pthread_t ejecutar_FsWrite;
                    log_info(log_entradasalida, "entre a fs write");
                    pthread_create(&ejecutar_FsWrite, NULL, (void*)funcIoFsWrite, (void*)(intptr_t)&ejecucion);
                    pthread_detach(ejecutar_FsWrite);
                }
                break;
            case IO_FS_CREATE:
                if (strcmp(nombreInterfazRecibido, nombre_interfaz) == 0 && es_operacion_compatible(tipoInterfaz, operacionActual)) {
                    pthread_t ejecutar_FsCreate;
                    log_info(log_entradasalida, "entre a fs create");
                    pthread_create(&ejecutar_FsCreate, NULL, (void*)funcIoFsCreate, (void*)(intptr_t)&ejecucion);
                    pthread_detach(ejecutar_FsCreate);
                }
                break;
            case IO_FS_DELETE:
                if (strcmp(nombreInterfazRecibido, nombre_interfaz) == 0 && es_operacion_compatible(tipoInterfaz, operacionActual)) {
                    pthread_t ejecutar_FsDelete;
                    log_info(log_entradasalida, "entre a fs delete");
                    pthread_create(&ejecutar_FsDelete, NULL, (void*)funcIoFsDelete, (void*)(intptr_t)&ejecucion);
                    pthread_detach(ejecutar_FsDelete);
                }
                break;
            case IO_FS_TRUNCATE:
                if (strcmp(nombreInterfazRecibido, nombre_interfaz) == 0 && es_operacion_compatible(tipoInterfaz, operacionActual)) {
                    pthread_t ejecutar_FsTruncate;
                    log_info(log_entradasalida, "entre a fs truncate");
                    pthread_create(&ejecutar_FsTruncate, NULL, (void*)funcIoFsTruncate, (void*)(intptr_t)&ejecucion);
                    pthread_detach(ejecutar_FsTruncate);
                }
                break;
            case EXIT:
                log_info(log_entradasalida, "Finalizo conexion con servidores");
                finalizar_programa();
                break;
            case -1:
                noFinalizar = operacionActual;
                break;
            default:
                break;
        }
    }
}

void recibir_y_procesar_paquete(int socket_cliente) {
    int size = 0;
    int desplazamiento = 0;
    char* buffer;

    // RECIBO OPERACION PARA CHEQUEAR SI ES VALIDA
    int operacion = recibir_operacion(socket_cliente);
    buffer = recibir_buffer(&size, socket_cliente);

    //ESTO LO RECIBO YA QUE TENGO QUE ENVIAR ALGO
    pidRecibido = leer_entero_uint32(buffer, &desplazamiento);

    log_info(log_entradasalida, "recibi codigo: %d", operacion);

    if(es_operacion_compatible(tipoInterfaz,operacion)){

    //SI ES VALIDA LE ENVIO UN 1 Y RECIBO TODO
    enviar_entero(conexion_entradasalida_kernel,1,OBTENER_VALIDACION);
    //sleep(3);

    desplazamiento = 0;
    size = 0;

    int operacion2 = recibir_operacion(socket_cliente);

    int *operacionPtr = malloc(sizeof(int));
    *operacionPtr = operacion2;
    list_add(lista_operaciones, operacionPtr);

    // Recibir el buffer
    buffer = recibir_buffer(&size, socket_cliente);

    pidRecibido = leer_entero_uint32(buffer, &desplazamiento);
    uint32_t *pidPtr = malloc(sizeof(uint32_t));
    *pidPtr = pidRecibido;
    list_add(lista_pids, pidPtr);

    // Procesar según operación
    switch (operacion) {
        case IO_GEN_SLEEP:
                nombreInterfazRecibido = leer_string(buffer, &desplazamiento);
                list_add(lista_datos, nombreInterfazRecibido);
                unidadesRecibidas = leer_entero_uint32(buffer, &desplazamiento);
                list_add(lista_datos, malloc_copiar_uint32(unidadesRecibidas));
            break;
        case IO_STDIN_READ:
        case IO_STDOUT_WRITE:
                nombreInterfazRecibido = leer_string(buffer, &desplazamiento);
                list_add(lista_datos, nombreInterfazRecibido);
                direccionRecibida = leer_entero_uint32(buffer, &desplazamiento);
                list_add(lista_datos, malloc_copiar_uint32(direccionRecibida));
                tamañoRecibido = leer_entero_uint32(buffer, &desplazamiento);
                list_add(lista_datos, malloc_copiar_uint32(tamañoRecibido));
            break;
        case IO_FS_CREATE:
        case IO_FS_DELETE:
                nombreInterfazRecibido = leer_string(buffer, &desplazamiento);
                list_add(lista_datos, nombreInterfazRecibido);
                nombreArchivoRecibido = leer_string(buffer, &desplazamiento);
                list_add(lista_datos, nombreArchivoRecibido);
            break;
        case IO_FS_TRUNCATE:
                nombreInterfazRecibido = leer_string(buffer, &desplazamiento);
                list_add(lista_datos, nombreInterfazRecibido);
                nombreArchivoRecibido = leer_string(buffer, &desplazamiento);
                list_add(lista_datos, nombreArchivoRecibido);
                tamañoRecibido = leer_entero_uint32(buffer, &desplazamiento);
                list_add(lista_datos, malloc_copiar_uint32(tamañoRecibido));
            break;
        case IO_FS_WRITE:
        case IO_FS_READ:
                nombreInterfazRecibido = leer_string(buffer, &desplazamiento);
                list_add(lista_datos, nombreInterfazRecibido);
                nombreArchivoRecibido = leer_string(buffer, &desplazamiento);
                list_add(lista_datos, nombreArchivoRecibido);
                direccionRecibida = leer_entero_uint32(buffer, &desplazamiento);
                list_add(lista_datos, malloc_copiar_uint32(direccionRecibida));
                tamañoRecibido = leer_entero_uint32(buffer, &desplazamiento);
                list_add(lista_datos, malloc_copiar_uint32(tamañoRecibido));
                registroPunteroArchivoRecibido = leer_entero_uint32(buffer, &desplazamiento);    
                list_add(lista_datos, malloc_copiar_uint32(registroPunteroArchivoRecibido));
            break;
        default:
            break;
    }

    free(buffer);
    }else{
        //SI NO ES VALIDA ENVIO 0 Y NO RECIBO NADA
        enviar_entero(conexion_entradasalida_kernel,0,OBTENER_VALIDACION);
    }
}

uint32_t* malloc_copiar_uint32(uint32_t valor) {
    uint32_t* ptr = malloc(sizeof(uint32_t));
    if (ptr) {
        *ptr = valor;
    }
    return ptr;
}

void recibirOpMemoria(){
    op_code operacion = recibir_operacion(conexion_entradasalida_memoria);
    log_info(log_entradasalida, "codop rec de memoria: %i",operacion);
    char *mensaje;
    uint32_t pid = *(uint32_t *)list_get(lista_pids, 0);
    switch (operacion){
        case IO_STDIN_READ:
            log_info(log_entradasalida, "Mensaje escrito correctamente");
            enviar_entero(conexion_entradasalida_kernel, pid, TERMINO_INTERFAZ);
            log_info(log_entradasalida, "Operación completada");
            avanzar_a_siguiente_operacion();
            sem_post(&sem_termino);
        break;
        case IO_STDOUT_WRITE:
            mensaje = recibir_string(conexion_entradasalida_memoria,log_entradasalida);
            log_info(log_entradasalida, "Valor escrito en memoria: %p", mensaje);
            enviar_entero(conexion_entradasalida_kernel,pid,TERMINO_INTERFAZ);
            log_info(log_entradasalida, "Operacion completada");
            avanzar_a_siguiente_operacion();
            sem_post(&sem_termino);
        break;
        case IO_FS_READ:  
            log_info(log_entradasalida, "Mensaje escrito correctamente");
            enviar_entero(conexion_entradasalida_kernel,pid,TERMINO_INTERFAZ);
            log_info(log_entradasalida, "Operacion completada");
            avanzar_a_siguiente_operacion();
            sem_post(&sem_termino);
        break;
        case IO_FS_WRITE:
            mensaje = recibir_string(conexion_entradasalida_memoria,log_entradasalida);
            dialfs_escribir_archivo(&fs,nombreArchivoRecibido,registroPunteroArchivoRecibido,tamañoRecibido,mensaje);
            enviar_entero(conexion_entradasalida_kernel,pid,TERMINO_INTERFAZ);
            log_info(log_entradasalida, "Operacion completada");
            avanzar_a_siguiente_operacion();
            sem_post(&sem_termino);
        break;
        case IO_FS_READ_ERROR:
            log_error(log_entradasalida, "ERROR DE IO FS READ");
            avanzar_a_siguiente_operacion();
            sem_post(&sem_termino);
        break;
        case IO_FS_WRITE_ERROR:
            log_error(log_entradasalida, "ERROR DE IO FS WRITE");
            avanzar_a_siguiente_operacion();
            sem_post(&sem_termino);
        break;
        case IO_STDIN_READ_ERROR:
            log_error(log_entradasalida, "ERROR DE IO STDIN READ");
            avanzar_a_siguiente_operacion();
            sem_post(&sem_termino);
        break;
        case IO_STDOUT_WRITE_ERROR:
            log_error(log_entradasalida, "ERROR DE IO STDOUT WRITE");
            avanzar_a_siguiente_operacion();
            sem_post(&sem_termino);
        break;
        default:
        break;
    }
}

void funcIoGenSleep(t_entero_bool** ejecucion){
    log_info(log_entradasalida, "Generica: PID: <%d> - Gen_Sleep",pidRecibido);
    int unidades = unidadesRecibidas*tiempo_unidad_trabajo;
    log_info(log_entradasalida, "Eperando durante %d unidades",unidades);

    //el *1000 es ya que el usleep hace en microsegundos y nosotros necesitamos milisegundos
    usleep(unidadesRecibidas*tiempo_unidad_trabajo*1000);
        
    enviar_entero(conexion_entradasalida_kernel,(*ejecucion)->entero, TERMINO_INTERFAZ);

    log_info(log_entradasalida, "Operacion completada");
    (*ejecucion)->operacion = true;

    avanzar_a_siguiente_operacion();
    sem_post(&sem_termino);
}

void limpiar_buffer_entrada() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void funcIoStdRead(t_entero_bool** ejecucion) {
    log_info(log_entradasalida, "Stdin: PID: <%d> - Leer.", pidRecibido);

    // Asegurarse de que stdin esté en modo de búfer de línea
    if (setvbuf(stdin, NULL, _IOLBF, 0) != 0) {
        log_error(log_entradasalida, "No se pudo configurar el modo de búfer de stdin.");
        return;
    }

    char *buffer = malloc(tamañoRecibido + 1);

    while (true) {
        printf("Ingrese un mensaje menor o igual a %d caracteres: ", tamañoRecibido);
        fflush(stdout);

        // Limpiar el búfer de entrada antes de leer
        limpiar_buffer_entrada();

        if (fgets(buffer, tamañoRecibido + 1, stdin) != NULL) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }

            if (strlen(buffer) <= tamañoRecibido) {
                log_info(log_entradasalida, "Mensaje válido recibido.");
                break; // Mensaje válido, salir del bucle
            } else {
                printf("El mensaje ingresado supera el tamaño permitido. Intente nuevamente.\n");
            }
        } else {
            log_error(log_entradasalida, "Error al leer la entrada del usuario.");
            free(buffer);
            return;
        }
    }

    t_string_3enteros *mensaje = malloc(sizeof(t_string_3enteros));
    mensaje->entero1 = direccionRecibida;
    mensaje->entero2 = pidRecibido;
    mensaje->entero3 = tamañoRecibido;
    mensaje->string = buffer;

    enviar_3_enteros_1_string(conexion_entradasalida_memoria, mensaje, IO_STDIN_READ);
    free(mensaje);
    recibirOpMemoria();
    free(buffer);
}

void funcIoStdWrite(t_entero_bool** ejecucion){
    log_info(log_entradasalida, "Stdin: PID: <%d> - Escribir.",pidRecibido);

    t_3_enteros *mensaje = malloc(sizeof(t_3_enteros));
    mensaje->entero1 = direccionRecibida;
    mensaje->entero2 = pidRecibido;
    mensaje->entero3 = tamañoRecibido+75;


    enviar_3_enteros(conexion_entradasalida_memoria,mensaje,IO_STDOUT_WRITE);

    //Que memoria me pase lo leido y yo lo muestro en pantalla
    free(mensaje);
    recibirOpMemoria();
}

void funcIoFsWrite(t_entero_bool** ejecucion){
    log_info(log_entradasalida, "DialFS: PID: <%d> - Leer archivo.",pidRecibido);

    //envio a memoria un paquete con el RegistroTamaño y el RegistroDireccion recibidos de kernel
    t_3_enteros *mensaje = malloc(sizeof(t_3_enteros));
    mensaje->entero1 = direccionRecibida;
    mensaje->entero2 = pidRecibido;
    mensaje->entero3 = tamañoRecibido;
    enviar_3_enteros(conexion_entradasalida_memoria,mensaje,IO_FS_WRITE);

    //Escribo el valor en el archivo
    free(mensaje);
    recibirOpMemoria();
}

void funcIoFsRead(t_entero_bool** ejecucion){
    log_info(log_entradasalida, "DialFS: PID: <%d> - Escribir Archivo.",pidRecibido);

    dialfs_leer_archivo(&fs, nombreArchivoRecibido, (void *)(intptr_t)direccionRecibida, tamañoRecibido, registroPunteroArchivoRecibido);

    //envio a memoria un paquete con el RegistroTamaño y el RegistroDireccion recibidos de kernel
    t_string_3enteros *mensaje = malloc(sizeof(t_string_3enteros));
    mensaje->entero1 = direccionRecibida;
    mensaje->entero2 = pidRecibido;
    mensaje->entero3 = tamañoRecibido;
    mensaje->string = mensajeLeido;
    enviar_3_enteros_1_string(conexion_entradasalida_memoria,mensaje,IO_FS_READ);

    //recibo un OK de memoria o podemos hacer que se muestre lo que se escribio
    free(mensaje);
    recibirOpMemoria();  
}

void funcIoFsTruncate(t_entero_bool** ejecucion) {
    dialfs_truncar_archivo(&fs, nombreArchivoRecibido, tamañoRecibido);
    enviar_entero(conexion_entradasalida_kernel,(*ejecucion)->entero,TERMINO_INTERFAZ);
    log_info(log_entradasalida, "Operacion completada");
    (*ejecucion)->operacion = true;

    avanzar_a_siguiente_operacion();
    sem_post(&sem_termino);
}

void funcIoFsCreate(t_entero_bool** ejecucion) {
    int bloque = dialfs_crear_archivo(&fs, nombreArchivoRecibido);

    if (bloque == -1) {
        log_info(log_entradasalida, "Error al crear el bloque");
        return;
    }

    log_info(log_entradasalida, "Archivo creado con éxito: %s en bloque %d", nombreArchivoRecibido, bloque);
    enviar_entero(conexion_entradasalida_kernel,(*ejecucion)->entero,TERMINO_INTERFAZ);
    log_info(log_entradasalida, "Operacion completada");
    (*ejecucion)->operacion = true;

    avanzar_a_siguiente_operacion();
    sem_post(&sem_termino);
}

void funcIoFsDelete(t_entero_bool** ejecucion) {
    dialfs_eliminar_archivo(&fs,nombreArchivoRecibido);
    enviar_entero(conexion_entradasalida_kernel,(*ejecucion)->entero,TERMINO_INTERFAZ);
    log_info(log_entradasalida, "Operacion completada");
    (*ejecucion)->operacion = true;

    avanzar_a_siguiente_operacion();
    sem_post(&sem_termino);
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

// Función para inicializar DialFS
void dialfs_init(DialFS *dialfs, int block_size, int block_count, const char *path_base) {
    dialfs->block_size = block_size;
    dialfs->block_count = block_count;
    dialfs->path_base = strdup(path_base);

    // Crear o abrir el archivo de bloques
    char bloques_path[256];
    sprintf(bloques_path, "%s/bloques.dat", path_base);
    FILE *bloques_file = fopen(bloques_path, "r+");
    if (bloques_file == NULL) {
        bloques_file = fopen(bloques_path, "w+");
        if (bloques_file == NULL) {
            perror("Error opening block file");
            exit(EXIT_FAILURE);
        }
    }
    ftruncate(fileno(bloques_file), block_size * block_count);
    dialfs->blocks = mmap(NULL, block_size * block_count, PROT_READ | PROT_WRITE, MAP_SHARED, fileno(bloques_file), 0);
    if (dialfs->blocks == MAP_FAILED) {
        perror("Error mapping block file");
        fclose(bloques_file);
        exit(EXIT_FAILURE);
    }
    fclose(bloques_file);

    // Crear o abrir el archivo de bitmap
    char bitmap_path[256];
    sprintf(bitmap_path, "%s/bitmap.dat", path_base);
    FILE *bitmap_file = fopen(bitmap_path, "r+");
    if (bitmap_file == NULL) {
        bitmap_file = fopen(bitmap_path, "w+");
        if (bitmap_file == NULL) {
            perror("Error opening bitmap file");
            exit(EXIT_FAILURE);
        }
    }
    int bitmap_size = (block_count + 7) / 8;
    ftruncate(fileno(bitmap_file), bitmap_size);
    void *bitmap_data = mmap(NULL, bitmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, fileno(bitmap_file), 0);
    if (bitmap_data == MAP_FAILED) {
        perror("Error mapping bitmap file");
        fclose(bitmap_file);
        exit(EXIT_FAILURE);
    }
    dialfs->bitmap = bitarray_create_with_mode(bitmap_data, bitmap_size, LSB_FIRST);
    fclose(bitmap_file);

    // Inicializar la lista de archivos
    dialfs->archivos = list_create();
}

// Función para destruir DialFS
void dialfs_destroy(DialFS *fs) {
    usleep(tiempo_unidad_trabajo*1000);
    for (int i = 0; i < list_size(fs->archivos); ++i) {
        Archivo *archivo = (Archivo *)list_get(fs->archivos, i);
        free(archivo->nombre_archivo);
        free(archivo);
    }
    list_destroy_and_destroy_elements(fs->archivos, (void *)free);

    munmap(fs->blocks, fs->block_size * fs->block_count);
    munmap(fs->bitmap->bitarray, fs->bitmap->size);
    bitarray_destroy(fs->bitmap);
    free(fs->path_base);
}

// Función para reservar un bloque
int dialfs_allocate_block(DialFS *fs) {
    for (int i = 0; i < fs->block_count; i++) {
        if (!bitarray_test_bit(fs->bitmap, i)) {
            bitarray_set_bit(fs->bitmap, i);
            msync(fs->bitmap->bitarray, fs->bitmap->size, MS_SYNC);
            return i;
        }
    }
    return -1;
}

void dialfs_allocate_specific_block(DialFS *fs, int block_index) {
    if (block_index >= 0 && block_index < fs->block_count) {
        bitarray_set_bit(fs->bitmap, block_index);
        msync(fs->bitmap->bitarray, fs->bitmap->size, MS_SYNC);
    }
}

// Función para liberar un bloque
void dialfs_free_block(DialFS *fs, int block_index) {
    if (block_index >= 0 && block_index < fs->block_count) {
        bitarray_clean_bit(fs->bitmap, block_index);
        msync(fs->bitmap->bitarray, fs->bitmap->size, MS_SYNC);
    }
}

// Función para crear un archivo de usuario en DialFS
int dialfs_crear_archivo(DialFS *fs, const char *nombre_archivo) {
    // Verificar si el archivo ya existe
    for (int i = 0; i < list_size(fs->archivos); ++i) {
        Archivo *a = (Archivo *)list_get(fs->archivos, i);
        if (strcmp(a->nombre_archivo, nombre_archivo) == 0) {
            fprintf(stderr, "Error: El archivo '%s' ya existe.\n", nombre_archivo);
            return -1;
        }
    }

    // Crear un nuevo archivo
    Archivo *nuevo_archivo = malloc(sizeof(Archivo));
    nuevo_archivo->nombre_archivo = strdup(nombre_archivo);
    nuevo_archivo->tamaño = 0;

    // Asignar al menos un bloque al archivo
    int bloque_asignado = dialfs_allocate_block(fs);
    if (bloque_asignado == -1) {
        fprintf(stderr, "Error: No hay bloques libres para crear el archivo.\n");
        free(nuevo_archivo->nombre_archivo);
        free(nuevo_archivo);
        return -1;
    }

    nuevo_archivo->bloque_inicio = bloque_asignado;
    list_add(fs->archivos, nuevo_archivo);

    // Crear el archivo de metadata
    char metadata_path[256];
    sprintf(metadata_path, "%s/%s", fs->path_base, nombre_archivo);
    FILE *metadata_file = fopen(metadata_path, "w");
    if (metadata_file == NULL) {
        fprintf(stderr, "Error: No se pudo crear el archivo de metadata %s\n", metadata_path);
        return -1;
    }
    fprintf(metadata_file, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%d\n", bloque_asignado, 0);
    fclose(metadata_file);

    msync(fs->blocks, fs->block_size * fs->block_count, MS_SYNC);
    return bloque_asignado;
}

// Función para eliminar un archivo en DialFS
void dialfs_eliminar_archivo(DialFS *fs, const char *nombre_archivo) {
    usleep(tiempo_unidad_trabajo*1000);
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
    char path[256];
    sprintf(path, "%s/%s", fs->path_base, nombre_archivo);
    remove(path);

    msync(fs->blocks, fs->block_size * fs->block_count, MS_SYNC);
    log_info(log_entradasalida, "DialFS: PID: <%d> - Eliminar Archivo: %s.", pidRecibido, nombre_archivo);
}

// Función para redimensionar un archivo en DialFS
void dialfs_truncar_archivo(DialFS *fs, const char *nombre_archivo, size_t nuevo_size) {
    usleep(tiempo_unidad_trabajo*1000);
    Archivo *archivo = buscar_archivo(fs, nombre_archivo);
    if (archivo == NULL) {
        fprintf(stderr, "Archivo '%s' no encontrado.\n", nombre_archivo);
        return;
    }

    if (nuevo_size > archivo->tamaño) {
        size_t bloques_actuales = (archivo->tamaño + fs->block_size - 1) / fs->block_size;
        size_t nuevos_bloques = (nuevo_size + fs->block_size - 1) / fs->block_size;

        if (!espacioContiguoDisponible(fs, nuevos_bloques - bloques_actuales)) {
            dialfs_compactar_archivos(fs);
            sleep(retraso_compactacion);

            if (!espacioContiguoDisponible(fs, nuevos_bloques - bloques_actuales)) {
                fprintf(stderr, "No hay suficientes bloques libres para redimensionar el archivo incluso después de compactar.\n");
                return;
            }
        }

        for (size_t i = bloques_actuales; i < nuevos_bloques; ++i) {
            int bloque = dialfs_allocate_block(fs);
            if (bloque == -1) {
                fprintf(stderr, "No hay suficientes bloques libres para redimensionar el archivo.\n");
                return;
            }
        }
    } else if (nuevo_size < archivo->tamaño) {
        size_t bloques_actuales = (archivo->tamaño + fs->block_size - 1) / fs->block_size;
        size_t nuevos_bloques = (nuevo_size + fs->block_size - 1) / fs->block_size;

        for (size_t i = nuevos_bloques; i < bloques_actuales; ++i) {
            dialfs_free_block(fs, archivo->bloque_inicio + i);
        }
    }

    archivo->tamaño = nuevo_size;
    char path[256];
    sprintf(path, "%s/%s", fs->path_base, nombre_archivo);

    FILE *archivo_fisico = fopen(path, "w");
    if (archivo_fisico == NULL) {
        fprintf(stderr, "Error: No se pudo actualizar el archivo físico %s\n", path);
        return;
    }
    fprintf(archivo_fisico, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%zu\n", archivo->bloque_inicio, nuevo_size);
    fclose(archivo_fisico);

    msync(fs->blocks, fs->block_size * fs->block_count, MS_SYNC);
    log_info(log_entradasalida, "DialFS: PID: <%d> - Truncar Archivo: %s a tamaño %zu.", pidRecibido, nombre_archivo, nuevo_size);
}

// Función para escribir datos en un archivo en DialFS
void dialfs_escribir_archivo(DialFS *fs, const char *nombre_archivo, size_t offset, size_t size, const void *buffer) {
    usleep(tiempo_unidad_trabajo*1000);
    Archivo *archivo = buscar_archivo(fs, nombre_archivo);
    if (archivo == NULL) {
        log_error(log_entradasalida, "DialFS: PID: <%d> - Archivo no encontrado: %s", pidRecibido, nombre_archivo);
        return;
    }

    // Verificar que la escritura no exceda el tamaño del archivo
    if (offset + size > archivo->tamaño) {
        log_error(log_entradasalida, "DialFS: PID: <%d> - Intento de escritura fuera de los límites del archivo.", pidRecibido);
        return;
    }

    memcpy((char*)fs->blocks + archivo->bloque_inicio * fs->block_size + offset, buffer, size);
    msync(fs->blocks, fs->block_size * fs->block_count, MS_SYNC);
    log_info(log_entradasalida, "DialFS: PID: <%d> - Escribir Archivo: %s desde offset %zu, tamaño %zu.", pidRecibido, nombre_archivo, offset, size);
}

// Función para leer datos de un archivo en DialFS
void dialfs_leer_archivo(DialFS *fs, const char *nombre_archivo, void *registro_direccion, int registro_tamaño, int registro_puntero_archivo) {
    pthread_mutex_lock(&mutex_dialfs);

    usleep(tiempo_unidad_trabajo*1000);
    Archivo *archivo = buscar_archivo(fs, nombre_archivo);
    if (archivo == NULL) {
        log_error(log_entradasalida, "DialFS: PID: <%d> - Archivo no encontrado: %s", pidRecibido, nombre_archivo);
        pthread_mutex_unlock(&mutex_dialfs);
        return;
    }

    if (registro_puntero_archivo + registro_tamaño > archivo->tamaño) {
        log_error(log_entradasalida, "DialFS: PID: <%d> - Intento de lectura fuera de los límites del archivo.", pidRecibido);
        pthread_mutex_unlock(&mutex_dialfs);
        return;
    }

    memcpy(registro_direccion, (char*)fs->blocks + archivo->bloque_inicio * fs->block_size + registro_puntero_archivo, registro_tamaño);
    msync(fs->blocks, fs->block_size * fs->block_count, MS_SYNC);

    log_info(log_entradasalida, "DialFS: PID: <%d> - Leer Archivo: %s desde offset %d, tamaño %d.", pidRecibido, nombre_archivo, registro_puntero_archivo, registro_tamaño);

    pthread_mutex_unlock(&mutex_dialfs);
}

// Función para compactar archivos en DialFS
void dialfs_compactar_archivos(DialFS *fs) {
    usleep(tiempo_unidad_trabajo*1000);

    t_list *archivos_ordenados = list_duplicate(fs->archivos);
    list_sort(archivos_ordenados, (void *)comparar_archivos_por_bloque_inicio);

    int bloque_libre = 0;
    for (int i = 0; i < list_size(archivos_ordenados); ++i) {
        Archivo *archivo = (Archivo *)list_get(archivos_ordenados, i);
        size_t bloques_usados = (archivo->tamaño + fs->block_size - 1) / fs->block_size;

        if (archivo->bloque_inicio != bloque_libre) {
            for (size_t j = 0; j < bloques_usados; ++j) {
                memcpy((char*)fs->blocks + (bloque_libre + j) * fs->block_size, (char*)fs->blocks + (archivo->bloque_inicio + j) * fs->block_size, fs->block_size);
                dialfs_free_block(fs, archivo->bloque_inicio + j);
                dialfs_allocate_specific_block(fs, bloque_libre + j);
            }
            archivo->bloque_inicio = bloque_libre;
        }
        bloque_libre += bloques_usados;
    }

    list_destroy(archivos_ordenados);
    msync(fs->blocks, fs->block_size * fs->block_count, MS_SYNC);
    log_info(log_entradasalida, "DialFS: PID: <%d> - Compactar Archivos.", pidRecibido);
}

// Función para verificar si hay espacio contiguo disponible
bool espacioContiguoDisponible(DialFS *fs, size_t bloques_necesarios) {
    size_t bloques_libres_contiguos = 0;

    for (size_t i = 0; i < fs->block_count; ++i) {
        if (!bitarray_test_bit(fs->bitmap, i)) {
            bloques_libres_contiguos++;
            if (bloques_libres_contiguos >= bloques_necesarios) {
                return true;
            }
        } else {
            bloques_libres_contiguos = 0;
        }
    }

    return false;
}

// Función para comparar archivos por su bloque de inicio
int comparar_archivos_por_bloque_inicio(const void *a, const void *b) {
    Archivo *archivo_a = *(Archivo **)a;
    Archivo *archivo_b = *(Archivo **)b;
    return archivo_a->bloque_inicio - archivo_b->bloque_inicio;
}

// Función para buscar un archivo en la lista de archivos
Archivo* buscar_archivo(DialFS *fs, const char *nombre_archivo) {
    for (int i = 0; i < list_size(fs->archivos); ++i) {
        Archivo *archivo = (Archivo *)list_get(fs->archivos, i);
        if (strcmp(archivo->nombre_archivo, nombre_archivo) == 0) {
            return archivo;
        }
    }
    return NULL;
}

void inicializar_listas() {
    lista_operaciones = list_create();
    lista_pids = list_create();
    lista_datos = list_create();
    sem_init(&sem_termino,0,1);
}

void liberar_listas() {
    list_destroy_and_destroy_elements(lista_operaciones, free);
    list_destroy_and_destroy_elements(lista_pids, free);
    list_destroy_and_destroy_elements(lista_datos, free);
}

void avanzar_a_siguiente_operacion() {
    if (!list_is_empty(lista_operaciones)) {
        list_remove(lista_operaciones, 0);  // Remueve la operación actual
        list_remove(lista_pids, 0);
    }
}