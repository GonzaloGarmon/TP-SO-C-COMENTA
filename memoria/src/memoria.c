#include <memoria.h>

int main(int argc, char** argv) {
    
    log_memoria = log_create("./memoria.log", "MEMORIA", 1, LOG_LEVEL_TRACE);

    log_info(log_memoria, "INICIA EL MODULO DE MEMORIA");


    //para poner un config es:
    //./bin/memoria ./config/PruebaFS
    leer_config(argv[1]);

    inicializar_memoria();

    

    socket_servidor_memoria_dispatch = iniciar_servidor(puerto_escucha, log_memoria);

    log_info(log_memoria, "Inicia el servidor de memoria");
    
    pthread_t atiende_cliente_cpu, atiende_cliente_kernel;

    log_info(log_memoria, "Listo para recibir a CPU");
    socket_cliente_cpu = esperar_cliente(socket_servidor_memoria_dispatch);


    pthread_create(&atiende_cliente_cpu, NULL, (void *)recibir_cpu, (void *) (intptr_t) socket_cliente_cpu);
    pthread_detach(atiende_cliente_cpu);
    
    log_info(log_memoria, "Listo para recibir a Kernel");
    socket_cliente_kernel = esperar_cliente(socket_servidor_memoria_dispatch);

    pthread_create(&atiende_cliente_kernel, NULL, (void *)recibir_kernel, (void *) (intptr_t) socket_cliente_kernel);
    pthread_detach(atiende_cliente_kernel); 
    
    while(1){
        log_info(log_memoria, "Listo para recibir a EntradaSalida");
        socket_cliente_entradasalida = esperar_cliente(socket_servidor_memoria_dispatch);
        log_info(log_memoria, "seconecto entrada salida");

        pthread_t atiende_cliente_entradasalida;
        pthread_create(&atiende_cliente_entradasalida, NULL, (void *)recibir_entradasalida, (void *) (intptr_t) socket_cliente_entradasalida);
        pthread_detach(atiende_cliente_entradasalida);

    }
    //sem_wait(&finModulo);

    log_info(log_memoria, "Finalizo conexion con clientes");
    finalizar_programa();
    return 0;
}

void inicializar_memoria() {
    marcos_libres = tam_memoria / tam_pagina;

    pids_archivos.nombres_archivos = list_create();
    pids_archivos.pids = list_create();
}

void leer_config(char* path){
    config_memoria = iniciar_config(path);

    puerto_escucha = config_get_string_value(config_memoria, "PUERTO_ESCUCHA");
    tam_memoria = config_get_int_value(config_memoria, "TAM_MEMORIA");
    tam_pagina = config_get_int_value(config_memoria, "TAM_PAGINA");
    
    path_instrucciones = config_get_string_value(config_memoria, "PATH_INSTRUCCIONES");
    retardo_respuesta = config_get_int_value(config_memoria, "RETARDO_RESPUESTA");
    sem_init(&sem, 0, 0);
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
    //log_info(log_memoria, "log 1 %s", nombre_archivo);
    size_t path_len = strlen(path_instrucciones) + strlen(nombre_archivo) + 1;
    char* path_compl = malloc(path_len);    
    strcpy(path_compl, path_instrucciones);
    strcat(path_compl, nombre_archivo); 
    //log_info(log_memoria, "log 2");
    FILE* archivo = fopen(path_compl, "r");
    
    // Liberar path_compl ya que no se necesita más
    free(path_compl);
    //log_info(log_memoria, "log 3");
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }
    //log_info(log_memoria, "log 4");
    int indice_instruccion = 0;
    char linea[longitud_maxima];
    //log_info(log_memoria, "log 5");

    while (fgets(linea, longitud_maxima, archivo) != NULL && indice_instruccion < instrucciones_maximas) {
        //log_info(log_memoria, "log 6");
        t_instruccion* instruccion = malloc(sizeof(t_instruccion));
        char* token = strtok(linea, " \t\n");
        int param_count = 0;

        while (token != NULL && param_count < parametros_maximos) {
            switch (param_count) {
                case 0:
                    instruccion->parametros1 = strdup(token);
                    //log_info(log_memoria, "case instruccion %s", instruccion->parametros1);
                    break;
                case 1:
                    instruccion->parametros2 = strdup(token);
                    //log_info(log_memoria, "parametro 1 %s", instruccion->parametros2);
                    break;
                case 2:
                    instruccion->parametros3 = strdup(token);
                    //log_info(log_memoria, "parametro 2 %s", instruccion->parametros3);
                    break;
                case 3:
                    instruccion->parametros4 = strdup(token);
                    //log_info(log_memoria, "parametro 3 %s", instruccion->parametros4);
                    break;
                case 4:
                    instruccion->parametros5 = strdup(token);
                    //log_info(log_memoria, "parametro 4 %s", instruccion->parametros5);
                    break;
                case 5:
                    instruccion->parametros6 = strdup(token);
                    //log_info(log_memoria, "parametro 5 %s", instruccion->parametros6);
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
    
    enviar_string(socket_cliente_kernel,"hola desde memoria", MENSAJE);
    int codigoOperacion = 0;
    while(codigoOperacion != -1){
        int codOperacion = recibir_operacion(SOCKET_CLIENTE_KERNEL);
        switch (codOperacion)
        {
        case CREAR_PROCESO:
            log_info(log_memoria, "Antes de levantar estruc admin");
            levantar_estructuras_administrativas();
            usleep(retardo_respuesta * 1000); // aca dice memoria_config.retardo rari
           t_string_mas_entero *data = recibir_string_mas_entero(SOCKET_CLIENTE_KERNEL, log_memoria);
            uint32_t pid = data->entero1;
            char *path = data->string;
            uint32_t cant_paginas = 5; //recibir_entero_uint32(SOCKET_CLIENTE_KERNEL, log_memoria);
            //devolver tabla inicial de alguna manera
            log_info(log_memoria, "Creacion del proceso PID %d", pid);
            crear_tabla_pagina(pid, cant_paginas);
            log_info(log_memoria, "PID: %i - Tamanio: %i", pid,cant_paginas);
            list_add(pids_archivos.nombres_archivos,path);
            list_add(pids_archivos.pids,pid);
            //cargar_instrucciones_desde_archivo(path, &instrucciones);
            sem_post(&sem);
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
            uint32_t pidr = recibir_entero_uint32(SOCKET_CLIENTE_KERNEL, log_memoria);
            uint32_t marco_correspondiente = obtener_marco_pagina(pid, num_pagina);
            
            enviar_entero(SOCKET_CLIENTE_KERNEL, marco_correspondiente, ACCESO_TABLA_PAGINAS_OK);
            log_info(log_memoria, "Acceso a tabla de pagina PID: %d - Numero e pagina: %d - Marco: %d", pidr, num_pagina, marco_correspondiente);
            break;
            case -1:
            codigoOperacion=codOperacion;
        default:
            log_trace(log_memoria, "Recibi el codigo de opreacion %d y entre en DEFAULT", codigoOperacion);
            break;
        }
    }
    log_warning(log_memoria, "Se desconecto kernel");
}



void recibir_cpu(int SOCKET_CLIENTE_CPU){

    
    
    int codigoOperacion = 0;
    while(codigoOperacion != -1){
        int codOperacion = recibir_operacion(SOCKET_CLIENTE_CPU);
        switch (codOperacion)
        {
        case PEDIR_INSTRUCCION_MEMORIA:
            sem_wait(&sem);
            sleep(retardo_respuesta/1000);
            t_list* enteros = recibir_doble_entero(SOCKET_CLIENTE_CPU);
            uint32_t ins = list_get(enteros,1);
            for (int i = 0; i < list_size(pids_archivos.nombres_archivos); i++)
            {
                if (list_get(pids_archivos.pids, i) == list_get(enteros,0)){
                    cargar_instrucciones_desde_archivo(list_get(pids_archivos.nombres_archivos, i),&instrucciones);
                }
            }
            
            enviar_instruccion(SOCKET_CLIENTE_CPU, instrucciones[ins],READY);
            sem_post(&sem);
        break;
        case RESIZE: 
            usleep(retardo_respuesta * 1000);
            uint32_t nuevo_tam = recibir_entero_uint32(SOCKET_CLIENTE_CPU, log_memoria);
            t_contexto *contexto = recibir_contexto(SOCKET_CLIENTE_CPU);
            op_code res = ajustar_tamanio_proceso(nuevo_tam);
            enviar_codop(SOCKET_CLIENTE_CPU, res);
            //log_info(log_memoria, "PID: <PID> - %i: - Tamanio actual: %i - Tamanio a ampliar %i", contexto->pid, , nuevo_tam);

            break;
        case MOV_IN:
            usleep(retardo_respuesta * 1000); 
            pthread_mutex_lock(&mutex_memoria);

            t_string_2enteros* mov_in_data = recibir_string_2enteros(SOCKET_CLIENTE_CPU);
            uint32_t pid_mov = mov_in_data->entero1;
            uint32_t dir_fisica = mov_in_data->entero2;
            char *tam_a_leer = mov_in_data->string;

            char* valor = leer(dir_fisica, tam_a_leer);

            log_trace(log_memoria,"El valor en memoria es: %s",valor);
            enviar_paquete_string(SOCKET_CLIENTE_CPU, valor, MOV_IN_OK, tam_a_leer);

            free(valor);
            log_info(log_memoria, "PID: %d - Acción: LEER - Dirección física: %d - Tamaño: %d - Origen: CPU",
                        pid_mov, dir_fisica, tam_a_leer);

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
        case ACCESO_ESPACIO_USUARIO:
                usleep(retardo_respuesta * 1000);
                uint32_t pid = recibir_entero_uint32(SOCKET_CLIENTE_CPU, log_memoria);
                uint32_t num_pagina = recibir_entero_uint32(SOCKET_CLIENTE_CPU, log_memoria);
                uint32_t offset = recibir_entero_uint32(SOCKET_CLIENTE_CPU, log_memoria);
                char *tam_a_leerr = recibir_entero_uint32(SOCKET_CLIENTE_CPU, log_memoria);

                //char *valor_t = leer_memoria(pid, num_pagina, offset, tam_a_leer); aca podriamos poner leer para leer memoria
                //enviar_string(SOCKET_CLIENTE_CPU, valor_t, ACCESO); 
                //“PID: <PID> - Accion: <LEER / ESCRIBIR> - Direccion fisica: <DIRECCION_FISICA>” - Tamaño <TAMAÑO A LEER / ESCRIBIR>
                int direc_fisica;
                char *accion;
                log_info(log_memoria, "PID: %d - Accion: LEER/ESCRIBIR %s - Direccion Fisica: %i - Tamanio a leer/escribir : %s", pid, accion/*accion*/, direc_fisica, tam_a_leerr);
                //free(valor_t);
                break;
        case ACCESO_TABLA_PAGINAS: 
            usleep(retardo_respuesta * 1000);
            uint32_t num_paginax = recibir_entero_uint32(SOCKET_CLIENTE_CPU, log_memoria);
            uint32_t pidx = recibir_entero_uint32(SOCKET_CLIENTE_CPU, log_memoria);
            uint32_t marco_correspondiente = obtener_marco_pagina(pid, num_pagina);
            
            enviar_entero(SOCKET_CLIENTE_CPU, marco_correspondiente, ACCESO_TABLA_PAGINAS_OK);
            log_info(log_memoria, "Acceso a tabla de pagina PID: %d - Numero e pagina: %d - Marco: %d", pidx, num_paginax, marco_correspondiente);
            break;
        case COPY_STRING:
            usleep(retardo_respuesta * 1000);
            uint32_t tamanio = recibir_entero_uint32(SOCKET_CLIENTE_CPU, log_memoria);
            t_contexto *contex = recibir_contexto(SOCKET_CLIENTE_CPU);
            copiarBytes(tamanio, contex);
            log_info(log_memoria, "Se copio tamanio del DI al SI con : %i", tamanio);
            break;
        case -1:
        codigoOperacion=codOperacion;
        break;
        default:
            
            log_trace(log_memoria, "1 Recibí el código de operación %d y entré en DEFAULT", codigoOperacion);
            break;
        }
    }

    log_warning(log_memoria, "Se desconecto CPU");
    
}

void recibir_entradasalida(int SOCKET_CLIENTE_ENTRADASALIDA) {
    log_info(log_memoria, "eempece el hilo");
    enviar_string(SOCKET_CLIENTE_ENTRADASALIDA,"hola desde memoria", MENSAJE);
    log_info(log_memoria, "envie mensaje a entrada salida");
    int codigoOperacion = 0;
    while (codigoOperacion != -1) {
         int codOperacion = recibir_operacion(SOCKET_CLIENTE_ENTRADASALIDA);

        log_info(log_memoria, "mensaje %i", codOperacion);

        switch (codOperacion) {
            case MENSAJE:
                recibir_string(SOCKET_CLIENTE_ENTRADASALIDA,log_memoria);
                break;
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
                uint32_t pid = recibir_entero_uint32(SOCKET_CLIENTE_ENTRADASALIDA, log_memoria);
                uint32_t num_pagina = recibir_entero_uint32(SOCKET_CLIENTE_ENTRADASALIDA, log_memoria);
                uint32_t offset = recibir_entero_uint32(SOCKET_CLIENTE_ENTRADASALIDA, log_memoria);
                uint32_t tam_a_leer = recibir_entero_uint32(SOCKET_CLIENTE_ENTRADASALIDA, log_memoria);

                //char *valor_t = leer_memoria(pid, num_pagina, offset, tam_a_leer); aca podriamos poner leer para leer memoria
                //enviar_string(SOCKET_CLIENTE_CPU, valor_t, ACCESO); 
                int direc_fisica;
                char *accion;
                log_info(log_memoria, "PID: %d - Accion: LEER/ESCRIBIR %s - Direccion Fisica: %i - Tamanio a leer/escribir : %i", pid, accion/*accion*/, direc_fisica, tam_a_leer);
                //free(valor_t);
                break;
            case ACCESO_TABLA_PAGINAS: 
                usleep(retardo_respuesta * 1000);
                num_pagina = recibir_entero_uint32(SOCKET_CLIENTE_ENTRADASALIDA, log_memoria);
                pid = recibir_entero_uint32(SOCKET_CLIENTE_ENTRADASALIDA, log_memoria);
                uint32_t marco_correspondiente = obtener_marco_pagina(pid, num_pagina);
                
                log_info(log_memoria, "Acceso a tabla de pagina PID: %d - Numero e pagina: %d - Marco: %d", pid, num_pagina, marco_correspondiente);
                break;
            case -1:
                codigoOperacion=codOperacion;
                break;
            default:
            codigoOperacion = -1;
                log_trace(log_memoria, "Recibí el código de operación %d y entré en DEFAULT", codigoOperacion);
                break;
        }
    }

    log_warning(log_memoria, "Se desconectó entradaSalida");
}

void levantar_estructuras_administrativas() {
    log_info(log_memoria, "principio de todo el cod");
    ESPACIO_USUARIO = malloc(memoria_config.tam_memoria);
    ESPACIO_LIBRE_TOTAL = memoria_config.tam_memoria;

    LISTA_ESPACIOS_LIBRES = list_create();
    LISTA_TABLA_PAGINAS = list_create();

    t_esp* espacio_inicial = malloc(sizeof(t_esp));
    espacio_inicial->base = 0;
    espacio_inicial->limite = memoria_config.tam_memoria;

    list_add(LISTA_ESPACIOS_LIBRES, espacio_inicial);

    log_info(log_memoria, "fin de todo el cod");
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
    tabla_nueva->tabla_paginas = malloc(sizeof(entrada_tabla_pagina_t) * cant_paginas);
    tabla_nueva->cantidad_paginas = cant_paginas;

    for (int i = 0; i < cant_paginas; i++) {
        tabla_nueva->tabla_paginas[i].numero_pagina = i;
        tabla_nueva->tabla_paginas[i].numero_marco = (void*)((char*)ESPACIO_USUARIO + (i * memoria_config.tam_pagina));
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
            return tabla->tabla_paginas[pc].numero_marco;
        }
    }
    return NULL;
}

uint32_t obtener_marco_pagina(uint32_t pid, uint32_t num_pagina) {
    for (int i = 0; i < list_size(LISTA_TABLA_PAGINAS); i++) {
        tabla_pagina_t* tabla = list_get(LISTA_TABLA_PAGINAS, i);
        if (tabla->pid == pid) {
            if (num_pagina < tabla->cantidad_paginas) {
                return (uint32_t)tabla->tabla_paginas[num_pagina].numero_marco;
            }
        }
    }
    return -1; // Indicar error si no se encuentra
}

void finalizar_proceso(uint32_t proceso) {
    for (int i = 0; i < list_size(LISTA_TABLA_PAGINAS); i++) {
        tabla_pagina_t* tabla = list_get(LISTA_TABLA_PAGINAS, i);
        if (tabla->pid == proceso) {
            free(tabla->tabla_paginas);
            list_remove(LISTA_TABLA_PAGINAS, i);
            free(tabla);
            break;
        }
    }
}

op_code ajustar_tamanio_proceso(uint32_t nuevo_tam) {
    op_code ret;
    for (int i = 0; i < list_size(LISTA_TABLA_PAGINAS); i++) {
        tabla_pagina_t* tabla = list_get(LISTA_TABLA_PAGINAS, i);
        
            if (nuevo_tam > tabla->cantidad_paginas) {
                // aca se amplia el tamanio del proceso
                uint32_t paginas_a_asignar = nuevo_tam - tabla->cantidad_paginas;
                if(marcos_libres < paginas_a_asignar) {
                    log_info(log_memoria, "No se pueden solicitar mas marcos -> Memoria llena");
                    ret = OUT_OF_MEMORY;
                    return ret; 
                }

                //hay marcos disponibles 
                tabla->tabla_paginas = realloc(tabla->tabla_paginas, sizeof(entrada_tabla_pagina_t) *nuevo_tam);
                for(uint32_t i = tabla->cantidad_paginas; i > nuevo_tam; i++) {
                    tabla->tabla_paginas[i].numero_pagina = i;
                    tabla->tabla_paginas[i].numero_marco = (uint32_t)((char*)ESPACIO_USUARIO + (i * memoria_config.tam_pagina));
                }

                //actualizo los marcos libres restando la cant de pag que tuve que asignar
                marcos_libres -= paginas_a_asignar;

            }else if(nuevo_tam < tabla->cantidad_paginas) {
                // aca se reduce el tamanio del proceso
                for(uint32_t i = nuevo_tam; i > tabla->cantidad_paginas; i++) {
                    tabla->tabla_paginas[i].numero_marco = 0;
                }
                tabla->tabla_paginas = realloc(tabla->tabla_paginas, sizeof(entrada_tabla_pagina_t) *nuevo_tam);

                //actualizo los marcos disponibles
                marcos_libres += (tabla->cantidad_paginas - nuevo_tam);
            }

            tabla->cantidad_paginas = nuevo_tam;
            ret = RESIZE_OK;

            
    }

}


void copiarBytes(uint32_t tamanio, t_contexto *contexto) {

    uint32_t valorSI = contexto->registros->SI;
    uint32_t valorDI = contexto->registros->DI;

    char *origen = (char*)(uintptr_t)valorSI;
    char *destino = (char*)(uintptr_t)valorDI;

    memcpy(destino, origen, tamanio);
}