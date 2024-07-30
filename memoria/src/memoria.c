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
    marcos_totales = tam_memoria / tam_pagina;
    log_info(log_memoria, "CANT MARCOS %i", marcos_totales);

    marcos_asignados = (uint32_t*)malloc(marcos_totales * sizeof(uint32_t));

    for (int i = 0; i < marcos_totales; i++) {
        marcos_asignados[i] = 0;
    }

    pids_ejecucion = list_create();
    
    listas_instrucciones = malloc(sizeof(t_list*));
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

void cargar_instrucciones_desde_archivo(char* nombre_archivo,  uint32_t pid) {
    //log_info(log_memoria, "log 1 %s", nombre_archivo);
    //size_t path_len = strlen(path_instrucciones) + strlen(nombre_archivo) + 1;
    //char* path_compl = malloc(path_len);    
    //strcpy(path_compl, path_instrucciones);
    //strcat(path_compl, nombre_archivo); 
    //log_info(log_memoria, "log 2");
    FILE* archivo = fopen(nombre_archivo, "r");
    
    // Liberar path_compl ya que no se necesita más
    //free(path_compl);
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
                    instruccion->parametros1 = "";
                    instruccion->parametros1 = strdup(token);
                    //log_info(log_memoria, "case instruccion %s", instruccion->parametros1);
                    break;
                case 1:
                    instruccion->parametros2 = "";
                    instruccion->parametros2 = strdup(token);
                    //log_info(log_memoria, "parametro 1 %s", instruccion->parametros2);
                    break;
                case 2:
                    instruccion->parametros3 = "";
                    instruccion->parametros3 = strdup(token);
                    //log_info(log_memoria, "parametro 2 %s", instruccion->parametros3);
                    break;
                case 3:
                    instruccion->parametros4 = "";
                    instruccion->parametros4 = strdup(token);
                    //log_info(log_memoria, "parametro 3 %s", instruccion->parametros4);
                    break;
                case 4:
                    instruccion->parametros5 = "";
                    instruccion->parametros5 = strdup(token);
                    //log_info(log_memoria, "parametro 4 %s", instruccion->parametros5);
                    break;
                case 5:
                    instruccion->parametros6 = "";
                    instruccion->parametros6 = strdup(token);
                    //log_info(log_memoria, "parametro 5 %s", instruccion->parametros6);
                    break;
                default:
                    break;
            }
            token = strtok(NULL, " \t\n");
            param_count++;
        }

        //instrucciones[indice_instruccion] = instruccion;
        list_add(listas_instrucciones[pid-1],instruccion);
        indice_instruccion++;
        
    }

    fclose(archivo);
    
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
            uint32_t cant_paginas = tam_memoria / tam_pagina; //recibir_entero_uint32(SOCKET_CLIENTE_KERNEL, log_memoria);
            //devolver tabla inicial de alguna manera
            log_info(log_memoria, "Creacion del proceso PID %d", pid);
            crear_tabla_pagina(pid, cant_paginas);
            log_info(log_memoria, "PID: %i - Tamanio: %i", pid,cant_paginas);
            
            list_add(pids_ejecucion, (void *)(uintptr_t)pid);

            listas_instrucciones = realloc(listas_instrucciones, sizeof(t_list*) * list_size(pids_ejecucion));

            listas_instrucciones[pid-1] = list_create();
            
            cargar_instrucciones_desde_archivo(path, pid);
            sem_post(&sem);
            //enviar_mensaje("Proceso creado", SOCKET_CLIENTE_KERNEL);
            
            break;
        case FINALIZAR_PROCESO:
            usleep(retardo_respuesta * 1000);
            uint32_t proceso = recibir_entero_uint32(SOCKET_CLIENTE_KERNEL, log_memoria);
            log_info(log_memoria, "Finalizacion de proceso PID: %i", proceso);
            //finalizar_proceso(proceso);
            finalizar_proceso(proceso);
            enviar_mensaje("Proceso finalizado", SOCKET_CLIENTE_KERNEL);
            break;
        case ACCESO_TABLA_PAGINAS: 
            usleep(retardo_respuesta * 1000);
            t_2_enteros *accesoPagina_kernel = recibir_2_enteros(SOCKET_CLIENTE_KERNEL);
            uint32_t pidx_kernel = accesoPagina_kernel->entero1;
            uint32_t num_paginax_kernel = accesoPagina_kernel->entero2;
            
            log_info(log_memoria, "num pagina : %i", num_paginax_kernel);
            log_info(log_memoria, "num pid : %i", pidx_kernel);
            uint32_t marco_correspondiente_kernel = obtener_marco_pagina(pidx_kernel, num_paginax_kernel);
            log_info(log_memoria, "MARCO CORRESPONDIENTE: %i", marco_correspondiente_kernel);
            
            enviar_entero(SOCKET_CLIENTE_KERNEL, marco_correspondiente_kernel, ACCESO_TABLA_PAGINAS_OK);
            log_info(log_memoria, "Acceso a tabla de pagina PID: %d - Numero de pagina: %d - Marco: %d", pidx_kernel, num_paginax_kernel, marco_correspondiente_kernel);
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
            usleep(retardo_respuesta * 1000);  // Cambio sleep por usleep
            t_list* enteros = recibir_doble_entero(SOCKET_CLIENTE_CPU);
            uint32_t ins = (uint32_t)(uintptr_t)list_get(enteros, 1);
            uint32_t pid = (uint32_t)(uintptr_t)list_get(enteros, 0);
            
        
            enviar_instruccion(SOCKET_CLIENTE_CPU,list_get(listas_instrucciones[pid-1], ins) , READY);
            sem_post(&sem);
            break;
        case RESIZE: 
            usleep(retardo_respuesta * 1000);
            t_2_enteros* resize = recibir_2_enteros(SOCKET_CLIENTE_CPU);
            uint32_t nuevo_tam = resize->entero1;
            uint32_t pid_resize = resize->entero2;
            op_code res = ajustar_tamanio_proceso(nuevo_tam,pid_resize);
            enviar_codop(SOCKET_CLIENTE_CPU, res);
            log_info(log_memoria, "MANDE RESPUESTA A CPU");
            
            break;
        case PEDIR_TAM_MEMORIA:
            usleep(retardo_respuesta * 1000);
            uint32_t tamanio_pagina_cpu = tam_pagina;
            enviar_entero(SOCKET_CLIENTE_CPU, tamanio_pagina_cpu, TAMANIO_RECIBIDO);
            break;
        case MOV_IN:
            usleep(retardo_respuesta * 1000); 
            pthread_mutex_lock(&mutex_memoria);

            t_3_enteros* mov_in_data = recibir_3_enteros(SOCKET_CLIENTE_CPU);
            uint32_t dir_fisica = mov_in_data->entero1;
            uint32_t pid_mov = mov_in_data->entero2;
            uint32_t tam_a_leer = mov_in_data->entero3;

            char* valor_mov_in = leer(dir_fisica, tam_a_leer);


            log_info(log_memoria, "PID: %d - Acción: LEER - Dirección física: %d - Tamaño: %d",
                        pid_mov, dir_fisica, tam_a_leer);
            enviar_paquete_string(SOCKET_CLIENTE_CPU, valor_mov_in, MOV_IN_OK, tam_a_leer);

            free(valor_mov_in);
            

            pthread_mutex_unlock(&mutex_memoria);
            break;
        case MOV_OUT:
            usleep(retardo_respuesta * 1000);
            pthread_mutex_lock(&mutex_memoria);

            t_string_3enteros* mov_out_data = recibir_string_3_enteros(SOCKET_CLIENTE_CPU);
            uint32_t direccion_fisica = mov_out_data->entero1;
            uint32_t pid_mov_out = mov_out_data->entero2;
            uint32_t tam_a_escribir = mov_out_data->entero3;
            char *escritura = mov_out_data->string;

            char* valor_mov_out = malloc(tam_a_escribir);
            strcpy(valor_mov_out, escritura);
            // Escribir en la dirección física
            escribir(direccion_fisica, valor_mov_out, tam_a_escribir);

            log_info(log_memoria, "PID: %d - Acción: ESCRIBIR - Dirección física: %d - Tamaño: %u",
                pid_mov_out, direccion_fisica, tam_a_escribir);

            enviar_codop(SOCKET_CLIENTE_CPU, MOV_OUT_OK);
            
            free(valor_mov_out);
            free(mov_out_data);
            
            pthread_mutex_unlock(&mutex_memoria);
            break;
        case ACCESO_TABLA_PAGINAS:
            usleep(retardo_respuesta * 1000);
            t_2_enteros *accesoPagina = recibir_2_enteros(SOCKET_CLIENTE_CPU);
            uint32_t pidx = accesoPagina->entero1;
            uint32_t num_paginax = accesoPagina->entero2;
            
            log_info(log_memoria, "num pagina : %i", num_paginax);
            log_info(log_memoria, "num pid : %i", pidx);
            uint32_t marco_correspondiente_cpu = obtener_marco_pagina(pidx, num_paginax);
            log_info(log_memoria, "MARCO CORRESPONDIENTE: %i", marco_correspondiente_cpu);
            
            enviar_entero(SOCKET_CLIENTE_CPU, marco_correspondiente_cpu, ACCESO_TABLA_PAGINAS_OK);
            log_info(log_memoria, "Acceso a tabla de pagina PID: %d - Numero de pagina: %d - Marco: %d", pidx, num_paginax, marco_correspondiente_cpu);
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

            case IO_STDIN_READ:
                usleep(retardo_respuesta * 1000);
                pthread_mutex_lock(&mutex_memoria);

                t_string_3enteros* stdout_write = recibir_string_3_enteros(SOCKET_CLIENTE_ENTRADASALIDA);
                uint32_t direccion_fisica_stdout_write = stdout_write->entero1; // Cambiado el nombre de dir_fisica a direccion_fisica
                uint32_t pid_stdout_write = stdout_write->entero2;
                uint32_t tam_a_escribir_stdout_write = stdout_write->entero2;
                char* escritura_stdout = stdout_write->string;

                char* valor_stdout_write = malloc(tam_a_escribir_stdout_write);

                strcpy(valor_stdout_write,escritura_stdout);

                escribir(direccion_fisica_stdout_write, escritura_stdout, tam_a_escribir_stdout_write);
                enviar_codop(SOCKET_CLIENTE_ENTRADASALIDA, IO_STDIN_READ);
                log_info(log_memoria, "PID: %d - Acción: ESCRIBIR - Dirección física: %d - Tamaño: %u",
                pid_stdout_write, direccion_fisica_stdout_write, tam_a_escribir_stdout_write);


                free(valor_stdout_write);
                pthread_mutex_unlock(&mutex_memoria);

                break;
            case IO_STDOUT_WRITE:
                usleep(retardo_respuesta * 1000);
                pthread_mutex_lock(&mutex_memoria);

                t_3_enteros* stdin_data = recibir_3_enteros(SOCKET_CLIENTE_ENTRADASALIDA);
                uint32_t pid_leer_archivo_stdin = stdin_data->entero1;
                uint32_t dir_fisica_leer_archivo_stdin = stdin_data->entero2;
                uint32_t tamanio_io_read_stdin = stdin_data->entero3;

                char* valor_leer_archivo_stdin = leer(dir_fisica_leer_archivo_stdin, tamanio_io_read_stdin);
                enviar_paquete_string(SOCKET_CLIENTE_ENTRADASALIDA, valor_leer_archivo_stdin, IO_STDOUT_WRITE, tamanio_io_read_stdin);
                
                log_info(log_memoria, "PID: %d - Acción: LEER - Dirección física: %d - Tamaño: %d",
                         pid_leer_archivo_stdin, dir_fisica_leer_archivo_stdin, tamanio_io_read_stdin);

                free(valor_leer_archivo_stdin);

                pthread_mutex_unlock(&mutex_memoria);
                
                break;
            case IO_FS_READ:
            
                usleep(retardo_respuesta * 1000);
                pthread_mutex_lock(&mutex_memoria);

                t_string_3enteros* io_write = recibir_string_3_enteros(SOCKET_CLIENTE_ENTRADASALIDA);
                uint32_t direccion_fisica_io_write = io_write->entero1; // Cambiado el nombre de dir_fisica a direccion_fisica
                uint32_t pid_io_write = io_write->entero2;
                uint32_t tam_a_escribir_io_write = io_write->entero2;
                char* escritura_io = io_write->string;

                char* valor_io_write = malloc(tam_a_escribir_io_write);

                strcpy(valor_io_write,escritura_io);

                escribir(direccion_fisica_io_write, escritura_io, tam_a_escribir_io_write);
                enviar_codop(SOCKET_CLIENTE_ENTRADASALIDA, IO_FS_READ);
                log_info(log_memoria, "PID: %d - Acción: ESCRIBIR - Dirección física: %d - Tamaño: %u",
                         pid_io_write, direccion_fisica_io_write, tam_a_escribir_io_write);


                free(valor_io_write);
                pthread_mutex_unlock(&mutex_memoria);

                break;
            case IO_FS_WRITE:
                
                usleep(retardo_respuesta * 1000);
                pthread_mutex_lock(&mutex_memoria);

                t_3_enteros* fread_data = recibir_3_enteros(SOCKET_CLIENTE_ENTRADASALIDA);
                uint32_t pid_leer_archivo = fread_data->entero1;
                uint32_t dir_fisica_leer_archivo = fread_data->entero2;
                uint32_t tamanio_io_read = fread_data->entero3;

                char* valor_leer_archivo = leer(dir_fisica_leer_archivo, tamanio_io_read);
                enviar_paquete_string(SOCKET_CLIENTE_ENTRADASALIDA, valor_leer_archivo, IO_FS_WRITE, tamanio_io_read);
                free(valor_leer_archivo);

                log_info(log_memoria, "PID: %d - Acción: LEER - Dirección física: %d - Tamaño: %d",
                         pid_leer_archivo, dir_fisica_leer_archivo, tamanio_io_read);

                free(valor_leer_archivo);

                pthread_mutex_unlock(&mutex_memoria);
                break;
            case ACCESO_TABLA_PAGINAS: 
                usleep(retardo_respuesta * 1000);
                t_2_enteros *accesoPagina_entrada = recibir_2_enteros(SOCKET_CLIENTE_ENTRADASALIDA);
                uint32_t pidx_entrada = accesoPagina_entrada->entero1;
                uint32_t num_paginax_entrada = accesoPagina_entrada->entero2;
                
                log_info(log_memoria, "num pagina : %i", num_paginax_entrada);
                log_info(log_memoria, "num pid : %i", pidx_entrada);
                uint32_t marco_correspondiente_entrada = obtener_marco_pagina(pidx_entrada, num_paginax_entrada);
                log_info(log_memoria, "MARCO CORRESPONDIENTE: %i", marco_correspondiente_entrada);
                
                enviar_entero(SOCKET_CLIENTE_ENTRADASALIDA, marco_correspondiente_entrada, ACCESO_TABLA_PAGINAS_OK);
                log_info(log_memoria, "Acceso a tabla de pagina PID: %d - Numero de pagina: %d - Marco: %d", pidx_entrada, num_paginax_entrada, marco_correspondiente_entrada);
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
    ESPACIO_USUARIO = malloc(tam_memoria);
    ESPACIO_LIBRE_TOTAL = tam_memoria;

    LISTA_ESPACIOS_LIBRES = list_create();
    LISTA_TABLA_PAGINAS = list_create();

    t_esp* espacio_inicial = malloc(sizeof(t_esp));
    espacio_inicial->base = 0;
    espacio_inicial->limite = tam_memoria;

    list_add(LISTA_ESPACIOS_LIBRES, espacio_inicial);


}

void crear_tabla_pagina(uint32_t pid, uint32_t tamanio) {
    
    tabla_pagina_t* tabla = malloc(sizeof(tabla_pagina_t));
    int cantPaginas = tamanio/ tam_pagina;
    tabla->pid = pid;
    tabla->tabla_paginas = list_create();

    for (uint32_t i = 0; i < cantPaginas; i++) {
        entrada_tabla_pagina_t *nueva_pagina = malloc(sizeof(entrada_tabla_pagina_t));
        nueva_pagina->numero_marco = buscar_marco_libre();
        log_trace(log_memoria, "MARCO ELEGIDO PARA LA PAGINA %d: %d", i, nueva_pagina->numero_marco);
        log_trace(log_memoria, "LA CANT DE PAG ES %d", cantPaginas);
        list_add(tabla->tabla_paginas, nueva_pagina);
        marco_ocupado(nueva_pagina->numero_marco);
    }
    pthread_mutex_lock(&mutex_de_tabla_Paginas);
    list_add(LISTA_TABLA_PAGINAS, tabla);
    pthread_mutex_unlock(&mutex_de_tabla_Paginas);
}

uint32_t buscar_marco_libre() {
    for (uint32_t i = 0; i < marcos_totales; i++) {
        if (marcos_asignados[i] == 0)
            return i;
    }
    return (uint32_t)-1;
}

void marco_ocupado(uint32_t numero_marco) {
    if (numero_marco >= 0 && numero_marco < marcos_totales) {
        marcos_asignados[numero_marco] = 1;
    } else {
        log_info(log_memoria, "Marco invalido");
    }
}

void marco_libre(uint32_t numero_marco) {
    if (numero_marco >= 0 && numero_marco < marcos_totales) {
        marcos_asignados[numero_marco] = 0;
    } else {
        log_info(log_memoria, "Marco invalido");
    }
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

// char* obtener_instruccion(uint32_t pid, uint32_t pc) {
//     for (int i = 0; i < list_size(LISTA_TABLA_PAGINAS); i++) {
//         tabla_pagina_t* tabla = list_get(LISTA_TABLA_PAGINAS, i);
//         if (tabla->pid == pid) {
//             return tabla->tabla_paginas[pc].numero_marco;
//         }
//     }
//     return NULL;
// }

tabla_pagina_t* obtenerTablaPorPID(uint32_t pid) {
    log_info(log_memoria,"TABLA DE PAGINAS INICIO"); //log obligatorio
    for (int i = 0; i < list_size(LISTA_TABLA_PAGINAS); i++) {
        tabla_pagina_t* tabla = list_get(LISTA_TABLA_PAGINAS, i);
        if (tabla->pid == pid)
            return tabla;
    }
    return NULL;
}

// uint32_t obtener_marco_pagina(uint32_t pid, uint32_t num_pagina) {
//     tabla_pagina_t* tabla = obtenerTablaPorPID(pid);
//     log_info(log_memoria,"TABLA DE PAGINAS INICIO"); //log obligatorio
// 	entrada_tabla_pagina_t *pagina = list_get(tabla->tabla_paginas, num_pagina);
//     log_info(log_memoria,"TABLA DE PAGINAS INICIO"); //log obligatorio
// 	if (pagina->numero_marco >= 0 && pagina->numero_marco < marcos_totales){
// 		log_info(log_memoria,"PID: <%d> - Pagina: <%d> - Marco: <%d>", pid, num_pagina, pagina->numero_marco); //log obligatorio
// 		return pagina->numero_marco;
// 	}
//     else{
// 		log_trace(log_memoria,"No encontre frame de la pag %d", num_pagina); //log obligatorio
// 		return (uint32_t)-1;
//     }
// }

uint32_t obtener_marco_pagina(uint32_t pid, uint32_t num_pagina) {
    tabla_pagina_t* tabla = obtenerTablaPorPID(pid);
    if (tabla == NULL) {
        log_trace(log_memoria, "Tabla de páginas no encontrada para PID %d", pid); // Log de error
        return (uint32_t)-1;
    }

    log_info(log_memoria, "TABLA DE PAGINAS INICIO"); // Log obligatorio

    if (num_pagina >= list_size(tabla->tabla_paginas)) {
        log_trace(log_memoria, "Número de página %d fuera de rango", num_pagina); // Log de error
        return (uint32_t)-1;
    }

    entrada_tabla_pagina_t* pagina = list_get(tabla->tabla_paginas, num_pagina);
    if (pagina == NULL) {
        log_trace(log_memoria, "Página no encontrada en la tabla de páginas"); // Log de error
        return (uint32_t)-1;
    }

    log_info(log_memoria, "TABLA DE PAGINAS INICIO"); // Log obligatorio

    if (pagina->numero_marco >= 0 && pagina->numero_marco < marcos_totales) {
        log_info(log_memoria, "PID: <%d> - Pagina: <%d> - Marco: <%d>", pid, num_pagina, pagina->numero_marco); // Log obligatorio
        return pagina->numero_marco;
    } else {
        log_trace(log_memoria, "No encontré marco para la página %d", num_pagina); // Log obligatorio
        return (uint32_t)-1;
    }
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

op_code ajustar_tamanio_proceso(uint32_t nuevo_tam, uint32_t pid){   
        tabla_pagina_t* tabla = list_get(LISTA_TABLA_PAGINAS, pid-1);
        uint32_t tamanio_actual = list_size(tabla->tabla_paginas) * tam_pagina;
        op_code codigo_devolucion = RESIZE_OK;
        if (tabla->pid == pid) {
            if  (nuevo_tam >= list_size(tabla->tabla_paginas) * tam_pagina) {
                //  AGRANDAR TAMANIO PROCESO
                codigo_devolucion = ampliar(tabla, nuevo_tam);
                log_info(log_memoria, "PID: %i - Tamanio actual: %i - Tamanio a Ampliar: %i",pid, tamanio_actual, nuevo_tam - tamanio_actual);
            // REDUCIR TAMANIO PROCESO
            } 
            else {
                reducir(tabla, nuevo_tam);
                if(nuevo_tam != 0) { 
                    log_info(log_memoria, "PID: <%d> - Tamaño Actual: <%d> - Tamaño a Reducir: <%d>", pid, tamanio_actual, tamanio_actual-nuevo_tam); //log obligatorio
                }else{ 
                    log_info(log_memoria, "PID: <%d> - Tamaño Actual: <%d> - Tamaño a Reducir: <%d", pid, tamanio_actual, nuevo_tam);
	
                }
            }
            return codigo_devolucion;
        }
    
    return -1;
}

op_code ampliar(tabla_pagina_t *tabla, uint32_t nuevo_tam) {
    uint32_t cant_paginas_nuevo_tamanio = ceil(nuevo_tam / (double)tam_pagina);
    log_info(log_memoria, "CANT PAGINAS NUEVO TAM: %i", cant_paginas_nuevo_tamanio);
    uint32_t cantidad_paginas_actual = list_size(tabla->tabla_paginas);
    log_info(log_memoria, "CANT PAGINAS actuales: %i", cantidad_paginas_actual);

    if(cantidad_marcos_vacios() >= cant_paginas_nuevo_tamanio-cantidad_paginas_actual) {
        log_info(log_memoria, "debugging");
        for(int i=list_size(tabla->tabla_paginas); i<cant_paginas_nuevo_tamanio; i++){
            entrada_tabla_pagina_t* nueva_pagina = malloc(sizeof(entrada_tabla_pagina_t));
            log_info(log_memoria, "debugging");
            if (nueva_pagina == NULL) {
                log_trace(log_memoria, "Error: No se pudo asignar memoria para nuevaPagina");
                exit(EXIT_FAILURE);
            }
            nueva_pagina->numero_marco = buscar_marco_libre();
            log_trace(log_memoria, "MARCO ELEGIDO PARA LA PAGINA %d: %d", i, nueva_pagina->numero_marco);
            list_add(tabla->tabla_paginas, nueva_pagina);
            marco_ocupado(nueva_pagina->numero_marco);
        }
    }else return OUT_OF_MEMORY;
    return RESIZE_OK;
}

void reducir(tabla_pagina_t* tabla, uint32_t nuevo_tam){
    uint32_t cant_paginas_nuevo_tamanio = ceil(nuevo_tam / (double)tam_pagina);

    for(int i=list_size(tabla->tabla_paginas); i>cant_paginas_nuevo_tamanio; i--){
        entrada_tabla_pagina_t* pag = list_get(tabla->tabla_paginas, i-1);
        marco_libre(pag->numero_marco);
        list_remove(tabla->tabla_paginas, i-1);
        free(pag);
    }
}

void copiarBytes(uint32_t tamanio, t_contexto *contexto) {

    uint32_t valorSI = contexto->registros->SI;
    uint32_t valorDI = contexto->registros->DI;

    char *origen = (char*)(uintptr_t)valorSI;
    char *destino = (char*)(uintptr_t)valorDI;

    memcpy(destino, origen, tamanio);
}

uint32_t cantidad_marcos_vacios() {
    uint32_t marcosVacios = 0;

    for (uint32_t i = 0; i < marcos_totales; ++i) {
        if (marcos_asignados[i] == 0)
            marcosVacios ++;
    }
    return marcosVacios;
}