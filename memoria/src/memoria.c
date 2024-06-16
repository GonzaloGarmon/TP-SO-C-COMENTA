#include <memoria.h>

int main(int argc, char* argv[]) {
    
    log_memoria = log_create("./memoria.log", "MEMORIA", 1, LOG_LEVEL_TRACE);

    log_info(log_memoria, "INICIA EL MODULO DE MEMORIA");

    leer_config();

    socket_servidor_memoria_dispatch = iniciar_servidor(puerto_escucha, log_memoria);

    log_info(log_memoria, "Inicia el servidor de memoria");
    
    pthread_t atiende_cliente_cpu, atiende_cliente_kernel, atiende_cliente_entradasalida;

    log_info(log_memoria, "Listo para recibir a CPU");
    socket_cliente_cpu = esperar_cliente(socket_servidor_memoria_dispatch);
    
    pthread_create(&atiende_cliente_cpu, NULL, (void *)recibir_cpu, (void *) (intptr_t) socket_cliente_cpu);
    pthread_detach(atiende_cliente_cpu);
    
    log_info(log_memoria, "Listo para recibir a Kernel");
    socket_cliente_kernel = esperar_cliente(socket_servidor_memoria_dispatch);

    pthread_create(&atiende_cliente_kernel, NULL, (void *)recibir_kernel, (void *) (intptr_t) socket_cliente_kernel);
    pthread_detach(atiende_cliente_kernel);

    log_info(log_memoria, "Listo para recibir a EntradaSalida");
    socket_cliente_entradasalida = esperar_cliente(socket_servidor_memoria_dispatch);
    
    pthread_create(&atiende_cliente_entradasalida, NULL, (void *)recibir_entradasalida, (void *) (intptr_t) socket_cliente_entradasalida);
    pthread_join(atiende_cliente_entradasalida, NULL);

    log_info(log_memoria, "Finalizo conexion con clientes");
    finalizar_programa();
    return 0;
}

void leer_config(){
    config_memoria = iniciar_config("/home/utnso/tp-2024-1c-GoC/memoria/config/memoria.config");

    puerto_escucha = config_get_string_value(config_memoria, "PUERTO_ESCUCHA");
    tam_memoria = config_get_int_value(config_memoria, "TAM_MEMORIA");
    tam_pagina = config_get_int_value(config_memoria, "TAM_PAGINA");
    path_instrucciones = config_get_string_value(config_memoria, "PATH_INSTRUCCIONES");
    retardo_respuesta = config_get_int_value(config_memoria, "RETARDO_RESPUESTA");
}

void finalizar_programa(){
    liberar_conexion(socket_servidor_memoria_dispatch);
    liberar_conexion(socket_cliente_cpu);
    liberar_conexion(socket_cliente_entradasalida);
    liberar_conexion(socket_cliente_kernel);
    log_destroy(log_memoria);
    config_destroy(config_memoria);
}




// t_instruccion *instrucciones[instrucciones_maximas];
// t_instruccion ins a enviar = instrucciones[pid];

void* cargar_instrucciones_desde_archivo(char* nombre_archivo, t_instruccion* instrucciones[instrucciones_maximas]) {
    FILE* archivo = fopen(nombre_archivo, "r");
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    int indice_instruccion = 0;
    char linea[longitud_maxima];

    while (fgets(linea, longitud_maxima, archivo) != NULL && indice_instruccion < instrucciones_maximas) {
        t_instruccion* instruccion = malloc(sizeof(t_instruccion));
        char* token = strtok(linea, " \t\n");
        int param_count = 0;

        while (token != NULL && param_count < parametros_maximos) {
            switch (param_count) {
                case 0:
                    instruccion->parametros1 = strdup(token);
                    break;
                case 1:
                    instruccion->parametros2 = strdup(token);
                    break;
                case 2:
                    instruccion->parametros3 = strdup(token);
                    break;
                case 3:
                    instruccion->parametros4 = strdup(token);
                    break;
                case 4:
                    instruccion->parametros5 = strdup(token);
                    break;
                case 5:
                    instruccion->parametros6 = strdup(token);
                    break;
                default:
                    break;
            }
            token = strtok(NULL, " \t\n");
            param_count++;
        }
        instrucciones[indice_instruccion] = instruccion;
        indice_instruccion++;
    }

    fclose(archivo);
    return instrucciones;
}



//-----COMUNICACION ENTRE KERNEL, CPU, I/O-----

void recibir_kernel(int SOCKET_CLIENTE_KERNEL){

    enviar_mensaje("Recibido KERNEL", SOCKET_CLIENTE_KERNEL);
    int codigoOp = 0;
    while(codigoOp != -1){
        int codigoOperacion = recibir_operacion(SOCKET_CLIENTE_KERNEL);
        switch (codigoOperacion)
        {
        case CREAR_PROCESO:
         usleep(retardo_respuesta * 1000); // aca dice memoria_config.retardo rari
            uint32_t pid = recibir_entero_uint32(SOCKET_CLIENTE_KERNEL, log_memoria);
            uint32_t cant_paginas = recibir_entero_uint32(SOCKET_CLIENTE_KERNEL, log_memoria);
            //devolver tabla inicial de alguna manera
            crear_tabla_pagina(pid, cant_paginas);
            log_info(log_memoria, "Creacion del proceso PID %d", pid);
            enviar_mensaje("Proceso creado", SOCKET_CLIENTE_KERNEL);
            break;
        case FINALIZAR_PROCESO:
            usleep(retardo_respuesta * 1000);
            uint32_t proceso = recibir_entero_uint32(SOCKET_CLIENTE_KERNEL, log_memoria);
            log_info(log_memoria, "Finalizacion de proceso PID: %d", proceso);
            //finalizar_proceso(proceso);
            finalizar_proceso(proceso);
            log_info(log_memoria, "Finalizacion del proceso PID: %d", proceso);
            enviar_mensaje("Proceso finalizado", SOCKET_CLIENTE_KERNEL);
            break;
        case ACCESO_TABLA_PAGINAS: 
            usleep(retardo_respuesta * 1000);
            uint32_t num_pagina = recibir_entero_uint32(SOCKET_CLIENTE_KERNEL, log_memoria);
            uint32_t pid = recibir_entero_uint32(SOCKET_CLIENTE_KERNEL, log_memoria);
            uint32_t marco_correspondiente = obtener_marco_pagina(pid, num_pagina);
            
            enviar_entero(SOCKET_CLIENTE_KERNEL, marco_correspondiente, /*ACA FALTA ALGO*/); //ver en kernel
            log_info(log_memoria, "Acceso a tabla de pagina PID: %d - Numero e pagina: %d - Marco: %d", pid, num_pagina, marco_correspondiente);
            break;
        case -1:
            codigoOp = codigoOperacion;
            break;
        default:
            log_trace(log_memoria, "Recibi el codigo de opreacion %d y entre en DEFAULT", codigoOperacion);
            break;
        }
    }
    log_warning(log_memoria, "Se desconecto kernel");
}

void recibir_cpu(int SOCKET_CLIENTE_CPU){

    enviar_mensaje("Recibido CPU", SOCKET_CLIENTE_CPU);
    int codigoOp = 0;
    while(codigoOp != -1){
        int codigoOperacion = recibir_operacion(SOCKET_CLIENTE_CPU);
        switch (codigoOperacion)
        {
        case PEDIR_INSTRUCCION_MEMORIA:
        //t_list* enteros = recibir_doble_entero(SOCKET_CLIENTE_CPU);
        //RECIBO PID Y PC PRIMERO HAY QUE IDENTIFICAR QUE INSTRUCCIONES CORRESPONDE A PID
        //LUEGO HACER INSTRUCCIONES[PC], DEJO ESTO PARA PODER CONTINUAR CON LA EJECUCION
        //PERO TODAVIA FALTA VER COMO HACER LO DE IDENTIFICAR
        //uint32_t ins = list_get(enteros,1);
        //enviar_instruccion(SOCKET_CLIENTE_CPU, instrucciones[ins],READY);
            // uint32_t pid = recibir_entero_uint32(SOCKET_CLIENTE_CPU, log_memoria);
            // uint32_t pc = recibir_entero_uint32(SOCKET_CLIENTE_CPU, log_memoria);

            // char *instruccion = obtener_instruccion(pid, pc);
            // if(instruccion != NULL){
            //     enviar_instruccion(SOCKET_CLIENTE_CPU, instruccion, READY);
            // }else{
            //     log_info(log_memoria, "No se encontro la instruccion para el PID: %d y PC %d", pid, pc);
            //     enviar_mensaje("ERROR", SOCKET_CLIENTE_CPU);
            // }
        break;
        case RESIZE: // preguntar de donde viene
            usleep(retardo_respuesta * 1000);
            uint32_t nuevo_tam = recibir_entero_uint32(SOCKET_CLIENTE_KERNEL, log_memoria);
            ajustar_tamanio_proceso(nuevo_tam); // falta mostrar OUTOFMEMORY -> memoria llena
            log_info(log_memoria, "Ajuste de tamanio de proceso PID: %d - Nuevo tamanio: %d", pid_ajuste, nuevo_tam);
            break;
        case MOV_IN:
            usleep(retardo_respuesta * 1000); 
            pthread_mutex_lock(&mutex_memoria);

            t_2_enteros* mov_in_data = recibir_2_enteros(SOCKET_CLIENTE_CPU);
            uint32_t pid_mov = mov_in_data->entero1;
            uint32_t dir_fisica = mov_in_data->entero2;

            char* valor = leer(dir_fisica, memoria_config.tam_pagina);

            //enviar_paquete_string(SOCKET_CLIENTE_CPU, valor, MOV_IN_OK, memoria_config.tam_pagina);
            free(valor);
            log_info(log_memoria, "PID: %d - Acción: LEER - Dirección física: %d - Tamaño: %d - Origen: CPU",
                        pid_mov, dir_fisica, memoria_config.tam_pagina);

            pthread_mutex_unlock(&mutex_memoria);
            break;
        case MOV_OUT:
            usleep(retardo_respuesta * 1000);
            pthread_mutex_lock(&mutex_memoria);

            t_string_2enteros* mov_out_data = recibir_string_2enteros(SOCKET_CLIENTE_CPU);
            uint32_t direccion_fisica = mov_out_data->entero1; // Cambiado el nombre de dir_fisica a direccion_fisica
            uint32_t pid_mov_out = mov_out_data->entero2;
            char* escritura = mov_out_data->string;

            // Escribir en la dirección física
            escribir(direccion_fisica, escritura, strlen(escritura));

            enviar_codop(SOCKET_CLIENTE_CPU, MOV_OUT_OK);

            log_info(log_memoria, "PID: %d - Acción: ESCRIBIR - Dirección física: %d - Tamaño: %zu - Origen: CPU",
                        pid_mov_out, direccion_fisica, strlen(escritura));

            pthread_mutex_unlock(&mutex_memoria);
            break;
        case ACCESO_TABLA_PAGINAS: 
            usleep(retardo_respuesta * 1000);
            uint32_t num_pagina = recibir_entero_uint32(SOCKET_CLIENTE_KERNEL, log_memoria);
            uint32_t pid = recibir_entero_uint32(SOCKET_CLIENTE_KERNEL, log_memoria);
            uint32_t marco_correspondiente = obtener_marco_pagina(pid, num_pagina);
            
            enviar_entero(SOCKET_CLIENTE_KERNEL, marco_correspondiente, /*ACA FALTA ALGO*/); //ver en kernel
            log_info(log_memoria, "Acceso a tabla de pagina PID: %d - Numero e pagina: %d - Marco: %d", pid, num_pagina, marco_correspondiente);
            break;
        default:
            log_trace(log_memoria, "Recibí el código de operación %d y entré en DEFAULT", codigoOperacion);
            break;
        }
    }

    log_warning(log_memoria, "Se desconecto CPU");
    
}

void recibir_entradasalida(int SOCKET_CLIENTE_ENTRADASALIDA) {
    enviar_mensaje("Recibido ENTRADASALIDA", SOCKET_CLIENTE_ENTRADASALIDA);

    int codigoOP = 0;
    while (codigoOP != -1) {
        int codigoOperacion = recibir_operacion(SOCKET_CLIENTE_ENTRADASALIDA);
        switch (codigoOperacion) {
            case IO_FS_READ:
                usleep(retardo_respuesta * 1000);
                pthread_mutex_lock(&mutex_memoria);

                t_string_2enteros* fread_data = recibir_string_2enteros(SOCKET_CLIENTE_ENTRADASALIDA);
                uint32_t pid_leer_archivo = fread_data->entero1;
                uint32_t dir_fisica_leer_archivo = fread_data->entero2;

                char* valor_leer_archivo = leer(dir_fisica_leer_archivo, memoria_config.tam_pagina);
                //enviar_paquete_string(SOCKET_CLIENTE_ENTRADASALIDA, valor_leer_archivo, IO_FS_READ_OK, memoria_config.tam_pagina);
                free(valor_leer_archivo);

                log_info(log_memoria, "PID: %d - Acción: LEER - Dirección física: %d - Tamaño: %d - Origen: FS",
                         pid_leer_archivo, dir_fisica_leer_archivo, memoria_config.tam_pagina);

                pthread_mutex_unlock(&mutex_memoria);
                break;
            case IO_FS_WRITE:
                usleep(retardo_respuesta * 1000);
                pthread_mutex_lock(&mutex_memoria);

                t_string_2enteros* fwrite_data = recibir_string_2enteros(SOCKET_CLIENTE_ENTRADASALIDA);
                uint32_t pid_escribir_archivo = fwrite_data->entero1;
                uint32_t dir_fisica_escribir_archivo = fwrite_data->entero2;
                char* escritura = fwrite_data->string;

                escribir(dir_fisica_escribir_archivo, escritura, strlen(escritura));
                enviar_codop(SOCKET_CLIENTE_ENTRADASALIDA, IO_FS_WRITE_OK);
                log_info(log_memoria, "PID: %d - Acción: ESCRIBIR - Dirección física: %d - Tamaño: %zu - Origen: FS",
                         pid_escribir_archivo, dir_fisica_escribir_archivo, strlen(escritura));

                pthread_mutex_unlock(&mutex_memoria);
                break;
            case ACCESO_ESPACIO_USUARIO:
                usleep(retardo_respuesta * 1000);
                uint32_t pid = recibir_entero_uint32(SOCKET_CLIENTE_CPU, log_memoria);
                uint32_t num_pagina = recibir_entero_uint32(SOCKET_CLIENTE_CPU, log_memoria);
                uint32_t offset = recibir_entero_uint32(SOCKET_CLIENTE_CPU, log_memoria);
                uint32_t tam_a_leer = recibir_entero_uint32(SOCKET_CLIENTE_CPU, log_memoria);

                //char *valor_t = leer_memoria(pid, num_pagina, offset, tam_a_leer); aca podriamos poner leer para leer memoria
                //enviar_string(SOCKET_CLIENTE_CPU, valor_t, ACCESO); 
                log_info(log_memoria, "PID: %d - Accion: LEER - Numero de pagina: %d - offset: %d- Tamanio: %d - Origen: CPU", pid, num_pagina, offset, tam_a_leer);
                //free(valor_t);
                break;
            case ACCESO_TABLA_PAGINAS: 
                usleep(retardo_respuesta * 1000);
                uint32_t num_pagina = recibir_entero_uint32(SOCKET_CLIENTE_KERNEL, log_memoria);
                uint32_t pid = recibir_entero_uint32(SOCKET_CLIENTE_KERNEL, log_memoria);
                uint32_t marco_correspondiente = obtener_marco_pagina(pid, num_pagina);
                
                enviar_entero(SOCKET_CLIENTE_KERNEL, marco_correspondiente, /*ACA FALTA ALGO*/); //ver en kernel
                log_info(log_memoria, "Acceso a tabla de pagina PID: %d - Numero e pagina: %d - Marco: %d", pid, num_pagina, marco_correspondiente);
                break;    
            default:
                log_trace(log_memoria, "Recibí el código de operación %d y entré en DEFAULT", codigoOperacion);
                break;
        }
    }

    log_warning(log_memoria, "Se desconectó entradaSalida");
}

void levantar_estructuras_administrativas() {
    ESPACIO_USUARIO = malloc(memoria_config.tam_memoria);
    ESPACIO_LIBRE_TOTAL = memoria_config.tam_memoria;

    LISTA_ESPACIOS_LIBRES = list_create();
    LISTA_TABLA_PAGINAS = list_create();

    t_esp* espacio_inicial = malloc(sizeof(t_esp));
    espacio_inicial->base = 0;
    espacio_inicial->limite = memoria_config.tam_memoria;

    list_add(LISTA_ESPACIOS_LIBRES, espacio_inicial);

    //cambios de nombre
}

void crear_tabla_pagina(uint32_t pid_t, uint32_t cant_paginas){
    uint32_t tam_total_paginas = cant_paginas * memoria_config.tam_pagina;

    t_esp* espacio_libre = NULL;
    for (int i = 0; i < list_size(LISTA_ESPACIOS_LIBRES); i++) {
        t_esp* espacio = list_get(LISTA_ESPACIOS_LIBRES, i);
        if (espacio->limite >= tam_total_paginas) {
            espacio_libre = espacio;
            break;
        }
    }

    tabla_pagina_t *tabla_nueva = malloc(sizeof(tabla_pagina_t));
    tabla_nueva->pid = pid_t;
    tabla_nueva->entrada = malloc(sizeof(entrada_tabla_pagina_t) * cant_paginas);
    tabla_nueva->cantidad_entradas = cant_paginas;

    for (int i = 0; i < cant_paginas; i++) {
        tabla_nueva->entrada[i].numero_pagina = i;
        tabla_nueva->entrada[i].direccion_fisica = (void*)((char*)ESPACIO_USUARIO + (i * memoria_config.tam_pagina));
        // tabla->entradas[i].direccion_fisica = ESPACIO_USUARIO + espacio_libre->base + (i * memoria_config.tam_pagina);
    }

    list_add(LISTA_TABLA_PAGINAS, tabla_nueva);

    espacio_libre->base += tam_total_paginas;
    espacio_libre->limite -= tam_total_paginas;
    ESPACIO_LIBRE_TOTAL -= tam_total_paginas;
    
}


void escribir(uint32_t dir_fisca, void* data, uint32_t size) {
    //log_warning(log_memoria,"el size en escribir es :%d", size);
    memcpy(ESPACIO_USUARIO + dir_fisca, data, size);
}

char* leer(uint32_t dir_fisca , uint32_t size) {
    void* data = malloc(size);
    memcpy(data, ESPACIO_USUARIO + dir_fisca, size);
    return data;
}

char* obtener_instruccion(uint32_t pid, uint32_t pc) {
    for (int i = 0; i < list_size(LISTA_TABLA_PAGINAS); i++) {
        tabla_pagina_t* tabla = list_get(LISTA_TABLA_PAGINAS, i);
        if (tabla->pid == pid) {
            return tabla->entradas[pc].direccion_fisica;
        }
    }
    return NULL;
}

uint32_t obtener_marco_pagina(uint32_t pid, uint32_t num_pagina) {
    for (int i = 0; i < list_size(LISTA_TABLA_PAGINAS); i++) {
        tabla_pagina_t* tabla = list_get(LISTA_TABLA_PAGINAS, i);
        if (tabla->pid == pid) {
            if (num_pagina < tabla->cantidad_entradas) {
                return (uint32_t)tabla->entradas[num_pagina].direccion_fisica;
            }
        }
    }
    return -1; // Indicar error si no se encuentra
}

void finalizar_proceso(uint32_t proceso) {
    for (int i = 0; i < list_size(LISTA_TABLA_PAGINAS); i++) {
        tabla_pagina_t* tabla = list_get(LISTA_TABLA_PAGINAS, i);
        if (tabla->pid == proceso) {
            free(tabla->entradas);
            list_remove(LISTA_TABLA_PAGINAS, i);
            free(tabla);
            break;
        }
    }
}

void ajustar_tamanio_proceso(uint32_t nuevo_tam) {
    for (int i = 0; i < list_size(LISTA_TABLA_PAGINAS); i++) {
        tabla_pagina_t* tabla = list_get(LISTA_TABLA_PAGINAS, i);
        
            if (nuevo_tam > tabla->cantidad_entradas) {
                // Ampliación del proceso
                tabla->entradas = realloc(tabla->entradas, sizeof(entrada_tabla_pagina_t) * nuevo_tam);
                for (uint32_t j = tabla->cantidad_entradas; j < nuevo_tam; j++) {
                    tabla->entradas[j].numero_pagina = j;
                    tabla->entradas[j].numero_marco= (uint32_t)((char*)ESPACIO_USUARIO + (j * memoria_config.tam_pagina));
                }
            } else if (nuevo_tam < tabla->cantidad_entradas) {
                // Reducción del proceso
                for (uint32_t j = nuevo_tam; j < tabla->cantidad_entradas; j++) {
                    tabla->entradas[j].numero_marco = 0; // Marca las páginas como libres
                }
                tabla->entradas = realloc(tabla->entradas, sizeof(entrada_tabla_pagina_t) * nuevo_tam);
            }
            tabla->cantidad_entradas = nuevo_tam;
            break;
        
    }
}

// retardo en peticiones OK