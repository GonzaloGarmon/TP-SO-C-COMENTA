#include <kernel.h>

int main(int argc, char* argv[]) {

    log_kernel = log_create("./kernel.log", "KERNEL", 1, LOG_LEVEL_TRACE);

    log_info(log_kernel, "INICIA EL MODULO DE KERNEL");

    leer_config();
    
    iniciar_semaforos();

    generar_conexiones();


    socket_servidor_kernel_dispatch = iniciar_servidor(puerto_escucha, log_kernel);
    log_info(log_kernel, "INICIO SERVIDOR");


    log_info(log_kernel, "Listo para recibir a EntradaSalida");
    socket_cliente_entradasalida = esperar_cliente(socket_servidor_kernel_dispatch);

    //pthread_mutex_lock(&conexion);
    //list_add(conexiones_io.conexiones_io,socket_cliente_entradasalida);
    //pthread_mutex_unlock(&conexion);

    pthread_t atiende_nuevas_interfaces;
    pthread_create(&atiende_nuevas_interfaces, NULL, (void *)esperar_cliente_especial, (void *) (intptr_t) socket_servidor_kernel_dispatch);
    pthread_detach(atiende_nuevas_interfaces);

    planificar();

    iniciar_consola();

    log_info(log_kernel, "Finalizo conexion con cliente");
    finalizar_programa();

    return 0;
}

/*
------------------------CONFIGS, INICIACION, COMUNICACIONES-------------------------------------
*/

void leer_config(){
    config_kernel = iniciar_config("/home/utnso/tp-2024-1c-GoC/kernel/config/kernel.config");
        
    puerto_escucha = config_get_string_value(config_kernel, "PUERTO_ESCUCHA");
    ip_memoria = config_get_string_value(config_kernel, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_kernel, "PUERTO_MEMORIA");
    ip_cpu = config_get_string_value(config_kernel, "IP_CPU");
    puerto_cpu_interrupt = config_get_string_value(config_kernel, "PUERTO_CPU_INTERRUPT");
    puerto_cpu_dispatch = config_get_string_value(config_kernel, "PUERTO_CPU_DISPATCH");

    algoritmo = config_get_string_value(config_kernel, "ALGORITMO_PLANIFICACION");
    if (strcmp(algoritmo, "FIFO") == 0)
    {
        algoritmo_planificacion = FIFO;
    }
    else if (strcmp(algoritmo, "RR") == 0)
    {
        algoritmo_planificacion = RR;
    }
    else if (strcmp(algoritmo, "VRR") == 0)
    {
        algoritmo_planificacion = VRR;
    }
    else
    {
        log_error(log_kernel, "El algoritmo no es valido");
    }
    quantum = config_get_int_value(config_kernel, "QUANTUM");

    recursos = config_get_array_value(config_kernel, "RECURSOS");
    instancias_recursos = convertirArrayDeNumeros(config_get_array_value(config_kernel, "INSTANCIAS_RECURSOS"));
    grado_multiprogramacion = config_get_int_value(config_kernel, "GRADO_MULTIPROGRAMACION");

    log_info(log_kernel, "levanto la configuracion del kernel");
}

void generar_conexiones(){

    establecer_conexion_cpu_dispatch(ip_cpu, puerto_cpu_dispatch, config_kernel, log_kernel);


    sleep(3);


    establecer_conexion_cpu_interrupt(ip_cpu, puerto_cpu_interrupt, config_kernel, log_kernel);
    
    establecer_conexion_memoria(ip_memoria, puerto_memoria, config_kernel, log_kernel);

    log_info(log_kernel, "Se generaron correctamente las conexiones");
}

void iniciar_semaforos(){
    pthread_mutex_init(&mutex_cola_ready, NULL);
    pthread_mutex_init(&mutex_cola_new, NULL);
    pthread_mutex_init(&mutex_cola_exec, NULL);
    pthread_mutex_init(&mutex_cola_exit, NULL);
    pthread_mutex_init(&conexion, NULL);

    sem_init(&sem_multiprogamacion, 0, grado_multiprogramacion);
    sem_init(&sem_listos_para_ready, 0, 0);
    sem_init(&sem_listos_para_exec, 0, 0);
    sem_init(&sem_listos_para_exit, 0, 0);
    sem_init(&sem_empezar_quantum, 0, 0);
    sem_init(&sem_iniciar_consola, 0, 0);
    cola_new = list_create();
    cola_ready = list_create();
    cola_exec = list_create();
    cola_exit = list_create();
    generador_pid = 0;
    conexiones_io.conexiones_io = list_create();
    conexiones_io.conexiones_io_nombres = list_create();
    
    lista_recurso = (t_list**)malloc(cantidad_recursos * sizeof(t_list*));

    for (int i = 0; i < cantidad_recursos; i++) {
        lista_recurso[i] = list_create();
    }
}

int* convertirArrayDeNumeros(char** caracteres){
    int size = string_array_size(caracteres);
    cantidad_recursos = size;
    int* intArray = (int*)malloc(size * sizeof(int));

        for (int i = 0; i < size; ++i) {
            intArray[i] = atoi(caracteres[i]);
        }
        
    return intArray;
}

void recibir_entradasalida(int SOCKET_CLIENTE_ENTRADASALIDA){
    //enviar_string(socket_cliente_entradasalida, "hola desde kernel", MENSAJE);
    //Se deben enviar la cantidad de unidades de trabakp necesarias, crear una nueva funcion
    int noFinalizar = 0;
    while(noFinalizar != -1){
        int op_code = recibir_operacion(SOCKET_CLIENTE_ENTRADASALIDA);
        switch (op_code)
        {
        case IDENTIFICACION:
            char* nombre_interfaz = recibir_string(SOCKET_CLIENTE_ENTRADASALIDA,log_kernel);
            pthread_mutex_lock(&conexion);
            list_add(conexiones_io.conexiones_io_nombres, nombre_interfaz);
            pthread_mutex_unlock(&conexion);
            log_trace(log_kernel, "me llego la interfaz: %s", nombre_interfaz);
            break;
        case GENERICA_I:
            log_trace(log_kernel, "entre a GENERICA_I");
            recibir_string(SOCKET_CLIENTE_ENTRADASALIDA,log_kernel);
            break; 
        case STDIN_I:
            log_trace(log_kernel, "entre a STDIN_I");
            recibir_string(SOCKET_CLIENTE_ENTRADASALIDA,log_kernel);
            break;
        case STDOUT_I:
            log_trace(log_kernel, "entre a STDOUT_I");
            recibir_string(SOCKET_CLIENTE_ENTRADASALIDA,log_kernel);
            break; 
        case DIALFS_I:
            log_trace(log_kernel, "entre a DIALFS_I");
            recibir_string(SOCKET_CLIENTE_ENTRADASALIDA,log_kernel);
            break;                                      
        case -1:
        log_trace(log_kernel, "se desconecto un modulo io");
        for (int i = 0; i < list_size(conexiones_io.conexiones_io); i++) {
        int socket = list_get(conexiones_io.conexiones_io, i);
        if ( socket == SOCKET_CLIENTE_ENTRADASALIDA) {
            list_remove(conexiones_io.conexiones_io,i);
            list_remove(conexiones_io.conexiones_io_nombres,i);
            }
        }
        break;
        default:
            break;
        }
    }
}

void recibir_cpu_dispatch(int conexion_kernel_cpu_dispatch){
    int noFinalizar = 0;
    while(noFinalizar != -1){
        op_code codigo = recibir_operacion(conexion_kernel_cpu_dispatch);
        switch (codigo)
        {
        case TERMINO_PROCESO:
            t_contexto* contexto_finaliza = malloc(sizeof(t_contexto));
            contexto_finaliza = recibir_contexto(conexion_kernel_cpu_dispatch);
            log_trace(log_kernel,"recibi un pcb por finalizacion de proceso");
            actualizar_pcb_envia_exit(contexto_finaliza,SUCCESS);
            sem_post(&sem_listos_para_exit);
            sem_post(&sem_listos_para_exec);
            break;
        case INTERRUPCION:
            t_contexto* pcb_interrumpido = malloc(sizeof(t_contexto));
            pcb_interrumpido = recibir_contexto(conexion_kernel_cpu_dispatch);
            actualizar_pcb_envia_ready(pcb_interrumpido);
            sem_post(&sem_listos_para_exec);
            //log_trace(log_kernel,"recibi un pcb por fin de quantum");
            break;
        case EJECUTAR_WAIT:
            t_contexto* pcb_wait;
            char* recurso_wait;
            recibir_string_mas_contexto(conexion_kernel_cpu_dispatch,&pcb_wait,&recurso_wait);
            
            if(existe_recurso(recurso_wait)){

                for(int i = 0; i < cantidad_recursos; i++) {
                    if(strcmp(recursos[i],recurso_wait) == 0) {
                        
                        if(instancias_recursos[i]-1>0){
                            instancias_recursos[i]--;
                            actualizar_contexto(pcb_wait);
                            enviar_contexto(conexion_kernel_cpu_dispatch,pcb_wait,EXEC);
                        }else{
                            
                            actualizar_pcb_con_cambiar_lista(pcb_wait, lista_recurso[i]);
                            enviar_contexto(conexion_kernel_cpu_dispatch,pcb_wait,BLOCK);
                            sem_post(&sem_listos_para_exec);
                            
                        }

                    }
                }

            }else{
                actualizar_pcb_envia_exit(pcb_wait,INVALID_RESOURCE);
                enviar_contexto(conexion_kernel_cpu_dispatch,pcb_wait,EXIT);
                sem_post(&sem_listos_para_exec);
            }
            break;
        case EJECUTAR_SIGNAL:
            t_contexto* pcb_signal;
            char* recurso_signal;
            recibir_string_mas_contexto(conexion_kernel_cpu_dispatch,&pcb_signal,&recurso_signal);
            if(existe_recurso(recurso_signal)){

                for(int i = 0; i < cantidad_recursos; i++) {
                    if(strcmp(recursos[i],recurso_wait) == 0) {
                        instancias_recursos[i]++;
                        desbloquear_proceso(lista_recurso[i]);
                        actualizar_contexto(pcb_signal);
                        
                    }
                }                
            enviar_contexto(conexion_kernel_cpu_dispatch,pcb_signal,EXEC);    
            }else{
                actualizar_pcb_envia_exit(pcb_signal,INVALID_RESOURCE);
                enviar_contexto(conexion_kernel_cpu_dispatch,pcb_wait,EXIT);
                sem_post(&sem_listos_para_exec);
            }
            break;
        case EJECUTAR_IO_GEN_SLEEP:
            char* interfaz_gen_sleep;
            uint32_t tiempo_trabajo;
            t_contexto* pcb_IO_GEN_SLEEP;
            recibir_string_mas_u32_con_contexto(conexion_kernel_cpu_dispatch,&interfaz_gen_sleep, &tiempo_trabajo,&pcb_IO_GEN_SLEEP);
            
            if(existe_interfaz_conectada(interfaz_gen_sleep)){
                if (admite_operacion_con_u32(interfaz_gen_sleep, IO_GEN_SLEEP,tiempo_trabajo)){
                    
                }else{
                    actualizar_pcb_envia_exit(pcb_IO_GEN_SLEEP,INVALID_INTERFACE);
                    enviar_contexto(conexion_kernel_cpu_dispatch,pcb_IO_GEN_SLEEP,EXIT);
                    sem_post(&sem_listos_para_exec);
                }

            }else{
                actualizar_pcb_envia_exit(pcb_IO_GEN_SLEEP,INVALID_INTERFACE);
                enviar_contexto(conexion_kernel_cpu_dispatch,pcb_IO_GEN_SLEEP,EXIT);
                sem_post(&sem_listos_para_exec);
            }

            break;
        case EJECUTAR_IO_STDIN_READ:
            t_string_2enteros* parametros_stdin_read;
            t_contexto* pcb_IO_STDIN_READ;
            parametros_stdin_read = recibir_string_2enteros_con_contexto(conexion_kernel_cpu_dispatch, &pcb_IO_STDIN_READ);
            if(existe_interfaz_conectada(parametros_stdin_read->string)){
                if (admite_operacion_con_2u32(parametros_stdin_read->string, IO_STDIN_READ,parametros_stdin_read->entero1, parametros_stdin_read->entero2)){

                }else{
                    actualizar_pcb_envia_exit(pcb_IO_STDIN_READ,INVALID_INTERFACE);
                    enviar_contexto(conexion_kernel_cpu_dispatch,pcb_IO_STDIN_READ,EXIT);
                    sem_post(&sem_listos_para_exec);
                }

            }else{
                actualizar_pcb_envia_exit(pcb_IO_STDIN_READ,INVALID_INTERFACE);
                enviar_contexto(conexion_kernel_cpu_dispatch,pcb_IO_STDIN_READ,EXIT);
                sem_post(&sem_listos_para_exec);
            }            
            break;
        case EJECUTAR_IO_STDOUT_WRITE:
            t_string_2enteros* parametros_stdin_write;
            t_contexto* pcb_IO_STDOUT_WRITE;
            parametros_stdin_write = recibir_string_2enteros_con_contexto(conexion_kernel_cpu_dispatch,&pcb_IO_STDOUT_WRITE);
            if(existe_interfaz_conectada(parametros_stdin_write->string)){
                if (admite_operacion_con_2u32(parametros_stdin_write->string, IO_STDOUT_WRITE, parametros_stdin_write->entero1, parametros_stdin_write->entero2)){

                }else{
                    actualizar_pcb_envia_exit(pcb_IO_STDOUT_WRITE,INVALID_INTERFACE);
                    enviar_contexto(conexion_kernel_cpu_dispatch,pcb_IO_STDOUT_WRITE,EXIT);
                    sem_post(&sem_listos_para_exec);
                }

            }else{
                actualizar_pcb_envia_exit(pcb_IO_STDOUT_WRITE,INVALID_INTERFACE);
                enviar_contexto(conexion_kernel_cpu_dispatch,pcb_IO_STDOUT_WRITE,EXIT);
                sem_post(&sem_listos_para_exec);
            }            
            break;
        case EJECUTAR_IO_FS_CREATE:
            char* interfaz_fs_create;
            char* nombre_archivo_create;
            t_contexto* pcb_IO_FS_CREATE;
            recibir_2_string_con_contexto(conexion_kernel_cpu_dispatch, &interfaz_fs_create, &nombre_archivo_create, &pcb_IO_FS_CREATE);
            if(existe_interfaz_conectada(interfaz_fs_create)){
                if (admite_operacion_con_string(interfaz_fs_create, IO_FS_CREATE,nombre_archivo_create)){

                }else{
                    actualizar_pcb_envia_exit(pcb_IO_FS_CREATE,INVALID_INTERFACE);
                    enviar_contexto(conexion_kernel_cpu_dispatch,pcb_IO_FS_CREATE,EXIT);
                    sem_post(&sem_listos_para_exec);
                }

            }else{
                actualizar_pcb_envia_exit(pcb_IO_FS_CREATE,INVALID_INTERFACE);
                enviar_contexto(conexion_kernel_cpu_dispatch,pcb_IO_FS_CREATE,EXIT);
                sem_post(&sem_listos_para_exec);

            }            
            break;
        case EJECUTAR_IO_FS_DELETE:
            char* interfaz_fs_delete;
            char* nombre_archivo_delete;
            t_contexto* pcb_IO_FS_DELETE;
            recibir_2_string_con_contexto(conexion_kernel_cpu_dispatch, &interfaz_fs_delete, &nombre_archivo_delete, &pcb_IO_FS_DELETE);
            if(existe_interfaz_conectada(interfaz_fs_delete)){
                if (admite_operacion_con_string(interfaz_fs_delete, IO_FS_DELETE,nombre_archivo_delete)){

                }else{
                    actualizar_pcb_envia_exit(pcb_IO_FS_DELETE,INVALID_INTERFACE);
                    enviar_contexto(conexion_kernel_cpu_dispatch,pcb_IO_FS_DELETE,EXIT);
                    sem_post(&sem_listos_para_exec);

                }

            }else{
                actualizar_pcb_envia_exit(pcb_IO_FS_DELETE,INVALID_INTERFACE);
                enviar_contexto(conexion_kernel_cpu_dispatch,pcb_IO_FS_DELETE,EXIT);
                sem_post(&sem_listos_para_exec);

            }            
            break;
        case EJECUTAR_IO_FS_TRUNCATE:
            char* interfaz_fs_truncate;
            char* nombre_archivo_truncate;
            uint32_t registro1_truncate;
            t_contexto* pcb_IO_FS_TRUNCATE;
            recibir_2_string_mas_u32_con_contexto(conexion_kernel_cpu_dispatch, &interfaz_fs_truncate, &nombre_archivo_truncate, &registro1_truncate, &pcb_IO_FS_TRUNCATE);
            if(existe_interfaz_conectada(interfaz_fs_truncate)){
                if (admite_operacion_con_string_u32(interfaz_fs_truncate, IO_FS_TRUNCATE, nombre_archivo_truncate, registro1_truncate)){

                }else{
                    actualizar_pcb_envia_exit(pcb_IO_FS_TRUNCATE,INVALID_INTERFACE);
                    enviar_contexto(conexion_kernel_cpu_dispatch,pcb_IO_FS_TRUNCATE,EXIT);
                    sem_post(&sem_listos_para_exec);

                }

            }else{
                actualizar_pcb_envia_exit(pcb_IO_FS_TRUNCATE,INVALID_INTERFACE);
                enviar_contexto(conexion_kernel_cpu_dispatch,pcb_IO_FS_TRUNCATE,EXIT);
                sem_post(&sem_listos_para_exec);

            }
            break;
        case EJECUTAR_IO_FS_WRITE:
            char* interfaz_fs_write;
            char* nombre_archivo_write;
            uint32_t registro1_write;
            uint32_t registro2_write; 
            uint32_t registro3_write;
            t_contexto* pcb_IO_FS_WRITE;
            recibir_2_string_mas_3_u32_con_contexto(conexion_kernel_cpu_dispatch, &interfaz_fs_write, &nombre_archivo_write, &registro1_write, &registro2_write, &registro3_write, &pcb_IO_FS_WRITE);             
            if(existe_interfaz_conectada(interfaz_fs_write)){
                if (admite_operacion_con_string_3u32(interfaz_fs_write, IO_FS_WRITE, nombre_archivo_write, registro1_write, registro2_write, registro3_write)){

                }else{
                    actualizar_pcb_envia_exit(pcb_IO_FS_WRITE,INVALID_INTERFACE);
                    enviar_contexto(conexion_kernel_cpu_dispatch,pcb_IO_FS_WRITE,EXIT);
                    sem_post(&sem_listos_para_exec);

                }

            }else{
                actualizar_pcb_envia_exit(pcb_IO_FS_WRITE,INVALID_INTERFACE);
                enviar_contexto(conexion_kernel_cpu_dispatch,pcb_IO_FS_WRITE,EXIT);
                sem_post(&sem_listos_para_exec);

            }            
            break;
        case EJECUTAR_IO_FS_READ:
            char* interfaz_fs_read;
            char* nombre_archivo_read;
            uint32_t registro1_read;
            uint32_t registro2_read; 
            uint32_t registro3_read;
            t_contexto* pcb_IO_FS_READ;
            recibir_2_string_mas_3_u32_con_contexto(conexion_kernel_cpu_dispatch, &interfaz_fs_read, &nombre_archivo_read, &registro1_read, &registro2_read, &registro3_read, &pcb_IO_FS_READ);          
            if(existe_interfaz_conectada(interfaz_fs_read)){
                if (admite_operacion_con_string_3u32(interfaz_fs_read, IO_FS_READ, nombre_archivo_read, registro1_read, registro2_read, registro3_read)){

                }else{
                    actualizar_pcb_envia_exit(pcb_IO_FS_READ,INVALID_INTERFACE);
                    enviar_contexto(conexion_kernel_cpu_dispatch,pcb_IO_FS_READ,EXIT);
                    sem_post(&sem_listos_para_exec);

                }

            }else{
                actualizar_pcb_envia_exit(pcb_IO_FS_READ,INVALID_INTERFACE);
                enviar_contexto(conexion_kernel_cpu_dispatch,pcb_IO_GEN_SLEEP,EXIT);
                sem_post(&sem_listos_para_exec);

            }            
            break;
        default:
            break;
        }
    }
}

void recibir_cpu_interrupt(int conexion_kernel_cpu_interrupt){
    int noFinalizar = 0;
    while(noFinalizar != -1){
        int op_code = recibir_operacion(conexion_kernel_cpu_interrupt);
    }
}

void establecer_conexion_cpu_dispatch(char * ip_cpu, char* puerto_cpu, t_config* config, t_log* loggs){

    log_trace(loggs, "Inicio como cliente");

    log_trace(loggs,"Lei la IP %s , el Puerto CPU %s ", ip_cpu, puerto_cpu);

    //log_info(loggs, "Verifico si la conex:kernel es distinto de -1");
    //log_trace(loggs, "Es de numero %d", conexion_kernel);

    // Enviamos al servidor el valor de ip como mensaje si es que levanta el cliente
    if((conexion_kernel_cpu_dispatch = crear_conexion(ip_cpu, puerto_cpu)) == -1){
        log_trace(loggs, "Error al conectar con CPU. El servidor no esta activo");

        exit(2);
    }

    log_info(loggs, "Paso el chequeo");

    recibir_operacion(conexion_kernel_cpu_dispatch);
    log_info(loggs, "Paso recibir operacion");
    recibir_string(conexion_kernel_cpu_dispatch, loggs);
    log_info(loggs, "Paso recibir string");
}

void establecer_conexion_cpu_interrupt(char * ip_cpu, char* puerto_cpu, t_config* config, t_log* loggs){

    log_trace(loggs, "Inicio como cliente");

    log_trace(loggs,"Lei la IP %s , el Puerto CPU %s ", ip_cpu, puerto_cpu);

    //log_info(loggs, "Verifico si la conex:kernel es distinto de -1");
    //log_trace(loggs, "Es de numero %d", conexion_kernel);

    // Enviamos al servidor el valor de ip como mensaje si es que levanta el cliente
    if((conexion_kernel_cpu_interrupt = crear_conexion(ip_cpu, puerto_cpu)) == -1){
        log_trace(loggs, "Error al conectar con CPU. El servidor no esta activo");

        exit(2);
    }

    log_info(loggs, "Paso el chequeo");

    recibir_operacion(conexion_kernel_cpu_interrupt);
    log_info(loggs, "Paso recibir operacion");
    recibir_string(conexion_kernel_cpu_interrupt, loggs);
    log_info(loggs, "Paso recibir string");
}

void establecer_conexion_memoria(char* ip_memoria, char* puerto_memoria_dispatch, t_config* config, t_log* loggs){


    log_trace(loggs, "Inicio como cliente");

    log_trace(loggs,"Lei la IP %s , el Puerto Memoria %s ", ip_memoria, puerto_memoria_dispatch);

    // Enviamos al servidor el valor de ip como mensaje si es que levanta el cliente
    if((conexion_kernel_memoria = crear_conexion(ip_memoria, puerto_memoria_dispatch)) == -1){
        log_trace(loggs, "Error al conectar con Memoria. El servidor no esta activo");

        exit(2);
    }
    
    recibir_operacion(conexion_kernel_memoria);
    recibir_string(conexion_kernel_memoria, loggs);
}

void finalizar_programa(){
    liberar_conexion(socket_servidor_kernel_dispatch);
    liberar_conexion(socket_servidor_kernel_interrupt);
    liberar_conexion(socket_cliente_entradasalida);
    
    log_destroy(log_kernel);
    config_destroy(config_kernel);
}
/*
------------------------CONFIGS, INICIACION, COMUNICACIONES-------------------------------------
*/

void esperar_cliente_especial(int socket_servidor_kernel_dispatch)
{   int no_fin = 1;
    while(no_fin != 0){
    // Aceptamos un nuevo cliente
    int socket_cliente = esperar_cliente(socket_servidor_kernel_dispatch);
    log_trace(log_kernel, "conecte una interfaz");
    pthread_mutex_lock(&conexion);
    list_add(conexiones_io.conexiones_io,socket_cliente);
    pthread_mutex_unlock(&conexion);
    pthread_t atiende_cliente_entradasalida;
    pthread_create(&atiende_cliente_entradasalida, NULL, (void *)recibir_entradasalida, (void *) (intptr_t) socket_cliente);
    pthread_detach(atiende_cliente_entradasalida);
    }

 
}

void iniciar_consola(){
    
    int eleccion;
    printf("Operaciones disponibles para realizar:\n");
    printf("1. Ejecutar Script de Operaciones\n");
    printf("2. Iniciar proceso\n");
    printf("3. Finalizar proceso\n");
    printf("4. Iniciar planificación\n");
    printf("5. Detener planificación\n");
    printf("6. Listar procesos por estado\n");

    printf("seleccione la opción que desee: ");
    scanf("%d", &eleccion);

    switch (eleccion)
    {
    case 1:
        ejecutar_script();
        break;
    case 2:
        iniciar_proceso();
        break;
    case 3:
        uint32_t pid;
        printf("\n ingrese el pid del proceso que desea finalizar: ");
        scanf("%d", &pid);
        finalizar_proceso(pid);
        break;
    case 4:
        iniciar_planificacion();
        break;
    case 5:
        detener_planificacion();
        break;
    case 6:
        listar_procesos_estado();
    case 7:
        int grado;
        printf("\n ingrese el grado de multiprogramacion que quiere tener: ");
        scanf("%d", &grado);
        grado_multiprogramacion = grado;
        break;
    default:
        printf("Opción no valida intente de nuevo!\n");
        iniciar_consola();
        break;
    }
    
    iniciar_consola();
}


void ejecutar_script(){

}

void iniciar_proceso(){
    char* path = malloc(30*sizeof(char));

    printf("Por favor ingrese el path: ");
    scanf("%s", path);


    //enviar_string(conexion_kernel_memoria,path,CREAR_PROCESO);
    
    generador_pid++;
    
    t_registros_cpu* registros = inicializar_registros();
    t_contexto* contexto = malloc(sizeof(t_contexto));
    t_pcb* pcb_nuevo = malloc(sizeof(t_pcb));
    pcb_nuevo->contexto = contexto;
    pcb_nuevo->quantum_utilizado = quantum;
    pcb_nuevo->contexto->pid = generador_pid;
    pcb_nuevo->contexto->pc = 0;
    pcb_nuevo ->contexto->registros = registros;
    
    t_paquete* paquete = crear_paquete_op(CREAR_PROCESO);
    agregar_a_paquete(paquete,path, strlen(path)+1);
    agregar_entero_a_paquete(paquete,pcb_nuevo->contexto->pid);
    enviar_paquete(paquete,conexion_kernel_memoria);
    eliminar_paquete(paquete);

    pthread_mutex_lock(&mutex_cola_new);
    list_add(cola_new, pcb_nuevo);
    pthread_mutex_unlock(&mutex_cola_new);
    sem_post(&sem_listos_para_ready);
}

void finalizar_proceso(uint32_t pid){
    
    if(esta_en_esta_lista(cola_new, pid)){
        sacar_de_lista_mover_exit(cola_new,mutex_cola_new,pid);
        sem_post(&sem_listos_para_exit);
    }

    if(esta_en_esta_lista(cola_ready, pid)){
        sacar_de_lista_mover_exit(cola_ready,mutex_cola_ready,pid);
        sem_post(&sem_listos_para_exit);
    }

    if(esta_en_esta_lista(cola_exec, pid)){
        sacar_de_lista_mover_exit(cola_exec,mutex_cola_exec,pid);
        sem_post(&sem_listos_para_exit);
    }

    for(int i = 0; i < cantidad_recursos; i++) {
        if(esta_en_esta_lista(lista_recurso[i],pid)) {

           sacar_de_lista_mover_exit_recurso(lista_recurso[i],pid);
           sem_post(&sem_listos_para_exit);
         }
        }

}

void iniciar_planificacion(){

}

void detener_planificacion(){

}


void listar_procesos_estado(){

}

t_registros_cpu* inicializar_registros(){
    t_registros_cpu* registros = malloc(sizeof(t_registros_cpu));

    registros->AX = 0;
    registros->BX = 0;
    registros->CX = 0;
    registros->DX = 0;
    registros->EAX = 0;
    registros->EBX = 0;
    registros->ECX = 0;
    registros->EDX = 0;

    return registros;
}

void planificar(){
    planificar_largo_plazo();
    planificar_corto_plazo();

    switch (algoritmo_planificacion)
    {
    case RR:
        planificar_rr();
        break;
    case VRR:
        planificar_vrr();
        break;
    default:
        break;
    }
}

void planificar_rr(){

    pthread_t hilo_manejo_quantum;
    pthread_create(&hilo_manejo_quantum, NULL, (void *)contador_quantum_RR, NULL);
    pthread_detach(hilo_manejo_quantum);

}
void planificar_vrr(){
    pthread_t hilo_manejo_VRR;
    pthread_create(&hilo_manejo_VRR, NULL, (void *)manejar_VRR, NULL);
    pthread_detach(hilo_manejo_VRR);
}

void planificar_largo_plazo(){
    pthread_t hilo_ready;
    pthread_t hilo_exit;
    
    pthread_create(&hilo_ready, NULL, (void *)pcb_ready, NULL);
    pthread_create(&hilo_exit, NULL, (void *)pcb_exit, NULL);

    pthread_detach(hilo_ready);
    pthread_detach(hilo_exit);
}

void planificar_corto_plazo(){
    pthread_t hilo_corto_plazo;
    pthread_create(&hilo_corto_plazo, NULL, (void *)exec_pcb, NULL);
    pthread_detach(hilo_corto_plazo);
}

void contador_quantum_RR(){
    while(1){
        sem_wait(&sem_empezar_quantum);
        
        sleep(quantum / 1000);
        if (strcmp(algoritmo, "RR") == 0)
        {
            enviar_interrupcion();
        }
    }

    
}

void manejar_VRR(){
    sem_wait(&sem_empezar_quantum);


}

void enviar_interrupcion(){
    t_paquete* paquete = crear_paquete_op(FIN_QUANTUM_RR);
    pthread_mutex_lock(&mutex_cola_exec);
    t_pcb* pcb_interrumpir = list_get(cola_exec,0);
    pthread_mutex_unlock(&mutex_cola_exec);
    agregar_entero_a_paquete(paquete,pcb_interrumpir->contexto->pid);
    enviar_paquete(paquete,conexion_kernel_cpu_interrupt);
    eliminar_paquete(paquete);
}

t_pcb* remover_pcb_de_lista(t_list *list, pthread_mutex_t *mutex)
{
    t_pcb* pcbDelProceso = malloc(sizeof(t_pcb));
    pthread_mutex_lock(mutex);
    pcbDelProceso = list_remove(list, 0);
    pthread_mutex_unlock(mutex);
    return pcbDelProceso;
}

void pcb_exit(){
    while(1){
    sem_wait(&sem_listos_para_exit);
    
    pthread_mutex_lock(&mutex_cola_exit);
    t_pcb_exit* pcb_finaliza = list_remove(cola_exit,0);
    pthread_mutex_unlock(&mutex_cola_exit);
    t_paquete* paquete = crear_paquete_op(FINALIZAR_PROCESO);
    agregar_entero_a_paquete(paquete,pcb_finaliza->pcb->contexto->pid);
    enviar_paquete(paquete,conexion_kernel_memoria);
    eliminar_paquete(paquete);
    //sem_post(&sem_multiprogamacion);
    free(pcb_finaliza);   
    }
}

void exec_pcb()
{
    while(1){
        if(!list_is_empty(cola_ready)){
        sem_wait(&sem_listos_para_exec);
        t_pcb* pcb_enviar = elegir_pcb_segun_algoritmo();

        dispatch(pcb_enviar);
        }
    }
}

void pcb_ready(){
 while(1){
    if (proceso_activos() < grado_multiprogramacion){
    sem_wait(&sem_listos_para_ready);
    t_pcb* pcb = remover_pcb_de_lista(cola_new, &mutex_cola_new);
    //sem_wait(&sem_multiprogamacion);
    pthread_mutex_lock(&mutex_cola_ready);
    list_add(cola_ready,pcb);
    pthread_mutex_unlock(&mutex_cola_ready);
    sem_post(&sem_listos_para_exec);
    }
 }
}

int proceso_activos(){
    int procesos = 0;
    procesos = list_size(cola_exec) + list_size(cola_exit) + list_size(cola_ready);
     for(int i = 0; i < cantidad_recursos; i++){
        procesos = procesos + list_size(lista_recurso[i]);
     }
    return procesos;
}

t_pcb* elegir_pcb_segun_algoritmo(){
    t_pcb* pcb_ejecutar;

    switch (algoritmo_planificacion)
    {
    case RR:
    case FIFO:
        pthread_mutex_lock(&mutex_cola_ready);
        pcb_ejecutar = list_remove(cola_ready,0);
        pthread_mutex_unlock(&mutex_cola_ready);
        break;
    case VRR:
        break; 
    default:
        break;
    }
    
    return pcb_ejecutar;
}


void dispatch(t_pcb* pcb_enviar){

        
        log_trace(log_kernel, "envio pcb de pid: %d", pcb_enviar->contexto->pid);
        log_trace(log_kernel, "envio pcb de pc: %d", pcb_enviar->contexto->pc);
        log_trace(log_kernel, "envio pcb de qq: %d", pcb_enviar->quantum);
        //ENVIAR CONTEXTO DE EJECUCION A CPU
        enviar_contexto(conexion_kernel_cpu_dispatch, pcb_enviar->contexto,EXEC);


        pthread_mutex_lock(&mutex_cola_exec);
        list_add(cola_exec, pcb_enviar);
        pthread_mutex_unlock(&mutex_cola_exec);
        sem_post(&sem_empezar_quantum);
        
}

int existe_recurso(char* recurso){

        for(int i = 0; i < cantidad_recursos; i++) {
        if(strcmp(recursos[i],recurso) == 0) {
           return 1;
         }
        }
    
    return 0;
}

void actualizar_contexto(t_contexto* pcb_wait){

    bool encontrar_pcb(t_pcb* pcb){
        return pcb->contexto->pid == pcb_wait->pid;
    };
    

    pthread_mutex_lock(&mutex_cola_exec);
    t_pcb* pcb_encontrado = list_find(cola_exec, (void*) encontrar_pcb);
    t_pcb* pcb_nuevo = malloc(sizeof(t_pcb));
    pcb_nuevo->contexto = pcb_wait;
    //pcb_nuevo->quantum = pcb_encontrado->quantum;
    pcb_nuevo->quantum_utilizado = pcb_encontrado->quantum_utilizado;
    list_replace_by_condition(cola_exec, (void*) encontrar_pcb, pcb_nuevo);
    pthread_mutex_unlock(&mutex_cola_exec);
}

void actualizar_pcb_con_cambiar_lista(t_contexto* pcb_wait, t_list* lista_bloq_recurso){
    
    bool encontrar_pcb(t_pcb* pcb){
        return pcb->contexto->pid == pcb_wait->pid;
        };
    

    pthread_mutex_lock(&mutex_cola_exec);
    t_pcb* pcb_encontrado = list_find(cola_exec, (void*) encontrar_pcb);
    list_remove_element(cola_exec,pcb_encontrado);
    free(pcb_encontrado);
    t_pcb* pcb_nuevo = malloc(sizeof(t_pcb));
    pcb_nuevo->contexto = pcb_wait;
    //pcb_nuevo->quantum = pcb_encontrado->quantum;
    pcb_nuevo->quantum_utilizado = pcb_encontrado->quantum_utilizado;
    pthread_mutex_unlock(&mutex_cola_exec);

    list_add(lista_bloq_recurso,pcb_nuevo);
}

void actualizar_pcb_envia_exit(t_contexto* pcb_wait, op_code codigo){
    
    bool encontrar_pcb(t_pcb* pcb){
        return pcb->contexto->pid == pcb_wait->pid;
        };
    
    
    pthread_mutex_lock(&mutex_cola_exec);
    t_pcb* pcb_encontrado = list_find(cola_exec, (void*) encontrar_pcb);
    list_remove_element(cola_exec,pcb_encontrado);
    
    pthread_mutex_unlock(&mutex_cola_exec);
    
    t_pcb_exit* pcb_exit_ok = malloc(sizeof(t_pcb_exit));
    pcb_exit_ok->pcb = pcb_encontrado;
    pcb_exit_ok->pcb->contexto = pcb_wait;
    pcb_exit_ok->pcb->quantum_utilizado = pcb_encontrado->quantum_utilizado;
    pcb_exit_ok->motivo = codigo;
    //free(pcb_encontrado);
    pthread_mutex_lock(&mutex_cola_exit);
    list_add(cola_exit,pcb_exit_ok);
    pthread_mutex_unlock(&mutex_cola_exit);
}

void actualizar_pcb_envia_ready(t_contexto* pcb_wait){
    
    bool encontrar_pcb(t_pcb* pcb){
        return pcb->contexto->pid == pcb_wait->pid;
        };
    

    pthread_mutex_lock(&mutex_cola_exec);
    t_pcb* pcb_encontrado = list_find(cola_exec, (void*) encontrar_pcb);
    list_remove_element(cola_exec,pcb_encontrado);
    pthread_mutex_unlock(&mutex_cola_exec);
    pcb_encontrado->contexto = pcb_wait;

    pthread_mutex_lock(&mutex_cola_ready);
    list_add(cola_ready,pcb_encontrado);
    pthread_mutex_unlock(&mutex_cola_ready);
}

void desbloquear_proceso(t_list* lista_recurso_liberar){
    if (!list_is_empty(lista_recurso_liberar)){
        
        t_pcb* pcb_desbloqueado = list_get(lista_recurso_liberar,0);
        pthread_mutex_lock(&mutex_cola_ready);
        list_add(lista_recurso_liberar,pcb_desbloqueado);
        pthread_mutex_unlock(&mutex_cola_ready);
        sem_post(&sem_listos_para_ready);
    }
}

bool esta_en_esta_lista(t_list* lista, uint32_t pid_encontrar){

        bool encontrar_pcb(t_pcb* pcb){
        return pcb->contexto->pid == pid_encontrar;
        };

    return list_any_satisfy(lista, (void*) encontrar_pcb);
}

void sacar_de_lista_mover_exit(t_list* lista, pthread_mutex_t mutex_lista, uint32_t pid){
    bool encontrar_pcb(t_pcb* pcb){
        return pcb->contexto->pid == pid;
        };
    

    pthread_mutex_lock(&mutex_lista);
    t_pcb* pcb_encontrado = list_find(lista, (void*) encontrar_pcb);
    list_remove_element(lista,pcb_encontrado);
    pthread_mutex_unlock(&mutex_lista);

    t_pcb_exit* pcb_exit_ok = malloc(sizeof(t_pcb_exit));
    pcb_exit_ok->pcb = pcb_encontrado;
    pcb_exit_ok->motivo = INTERRUPTED_BY_USER;
    pthread_mutex_lock(&mutex_cola_exit);
    list_add(cola_exit,pcb_exit_ok);
    pthread_mutex_unlock(&mutex_cola_exit);
}

void sacar_de_lista_mover_exit_recurso(t_list* lista, uint32_t pid){
    bool encontrar_pcb(t_pcb* pcb){
        return pcb->contexto->pid == pid;
        };
    

    
    t_pcb* pcb_encontrado = list_find(lista, (void*) encontrar_pcb);
    list_remove_element(lista,pcb_encontrado);
    

    t_pcb_exit* pcb_exit_ok = malloc(sizeof(t_pcb_exit));
    pcb_exit_ok->pcb = pcb_encontrado;
    pcb_exit_ok->motivo = INTERRUPTED_BY_USER;
    pthread_mutex_lock(&mutex_cola_exit);
    list_add(cola_exit,pcb_exit_ok);
    pthread_mutex_unlock(&mutex_cola_exit);
}

int existe_interfaz_conectada(char* nombre_interfaz){

    for(int i = 0; i < list_size(conexiones_io.conexiones_io_nombres); i++) {
        if(strcmp(list_get(conexiones_io.conexiones_io_nombres,i),nombre_interfaz) == 0) {
           return 1;
         }
        }
    return 0;
}

int admite_operacion_con_u32(char* nombre_interfaz, op_code codigo, uint32_t entero32){
    int devolver;

    for(int i = 0; i < list_size(conexiones_io.conexiones_io_nombres); i++) {
        if(strcmp(list_get(conexiones_io.conexiones_io_nombres,i),nombre_interfaz) == 0) {
           t_paquete* paquete = crear_paquete_op(codigo);
           agregar_entero_a_paquete(paquete, entero32);
           enviar_paquete(paquete,list_get(conexiones_io.conexiones_io,i));
           eliminar_paquete(paquete);
           devolver = recibir_operacion(list_get(conexiones_io.conexiones_io,i));
           return devolver;
         }
        }
}

int admite_operacion_con_2u32(char* nombre_interfaz, op_code codigo, uint32_t primer_entero32, uint32_t segundo_entero32){
    int devolver;

    for(int i = 0; i < list_size(conexiones_io.conexiones_io_nombres); i++) {
        if(strcmp(list_get(conexiones_io.conexiones_io_nombres,i),nombre_interfaz) == 0) {
           t_paquete* paquete = crear_paquete_op(codigo);
           agregar_entero_a_paquete(paquete, primer_entero32);
           agregar_entero_a_paquete(paquete, segundo_entero32);
           enviar_paquete(paquete,list_get(conexiones_io.conexiones_io,i));
           eliminar_paquete(paquete);
           devolver = recibir_operacion(list_get(conexiones_io.conexiones_io,i));
           return devolver;
         }
        }
}

int admite_operacion_con_string(char* nombre_interfaz, op_code codigo, char* palabra){
    int devolver;

    for(int i = 0; i < list_size(conexiones_io.conexiones_io_nombres); i++) {
        if(strcmp(list_get(conexiones_io.conexiones_io_nombres,i),nombre_interfaz) == 0) {
           t_paquete* paquete = crear_paquete_op(codigo);
           agregar_a_paquete(paquete, palabra, strlen(palabra+1));
           enviar_paquete(paquete,list_get(conexiones_io.conexiones_io,i));
           eliminar_paquete(paquete);
           devolver = recibir_operacion(list_get(conexiones_io.conexiones_io,i));
           return devolver;
         }
        }
}

int admite_operacion_con_string_u32(char* nombre_interfaz, op_code codigo, char* palabra, uint32_t primer_entero32){
    int devolver;

    for(int i = 0; i < list_size(conexiones_io.conexiones_io_nombres); i++) {
        if(strcmp(list_get(conexiones_io.conexiones_io_nombres,i),nombre_interfaz) == 0) {
           t_paquete* paquete = crear_paquete_op(codigo);
           agregar_a_paquete(paquete, palabra, strlen(palabra+1));
           agregar_entero_a_paquete(paquete, primer_entero32);
           enviar_paquete(paquete,list_get(conexiones_io.conexiones_io,i));
           eliminar_paquete(paquete);
           devolver = recibir_operacion(list_get(conexiones_io.conexiones_io,i));
           return devolver;
         }
        }
}

int admite_operacion_con_string_3u32(char* nombre_interfaz, op_code codigo,char* palabra, uint32_t primer_entero32, uint32_t segundo_entero32, uint32_t tercer_entero32){
    int devolver;

    for(int i = 0; i < list_size(conexiones_io.conexiones_io_nombres); i++) {
        if(strcmp(list_get(conexiones_io.conexiones_io_nombres,i),nombre_interfaz) == 0) {
           t_paquete* paquete = crear_paquete_op(codigo);
           agregar_a_paquete(paquete, palabra, strlen(palabra+1));
           agregar_entero_a_paquete(paquete, primer_entero32);
           agregar_entero_a_paquete(paquete, segundo_entero32);
           agregar_entero_a_paquete(paquete, tercer_entero32);
           enviar_paquete(paquete,list_get(conexiones_io.conexiones_io,i));
           eliminar_paquete(paquete);
           devolver = recibir_operacion(list_get(conexiones_io.conexiones_io,i));
           return devolver;
         }
        }
}