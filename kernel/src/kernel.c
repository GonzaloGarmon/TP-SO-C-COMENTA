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
     
    pthread_t atiende_cliente_entradasalida;
    pthread_create(&atiende_cliente_entradasalida, NULL, (void *)recibir_entradasalida, (void *) (intptr_t) socket_cliente_entradasalida);
    pthread_detach(atiende_cliente_entradasalida);

    pthread_t cpu_dispatch;
    pthread_create(&cpu_dispatch, NULL, (void *)recibir_cpu_dispatch, (void *) (intptr_t) conexion_kernel_cpu_dispatch);
    pthread_detach(cpu_dispatch);

    pthread_t cpu_interrupt;
    pthread_create(&cpu_interrupt, NULL, (void *)recibir_cpu_interrupt, (void *) (intptr_t) conexion_kernel_cpu_interrupt);
    pthread_detach(cpu_interrupt);
    
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

    recursos = config_get_string_value(config_kernel, "RECURSOS");
    instancias_recursos = config_get_string_value(config_kernel, "INSTANCIAS_RECURSOS");
    grado_multiprogramacion = config_get_int_value(config_kernel, "GRADO_MULTIPROGRAMACION");

    log_info(log_kernel, "levanto la configuracion del kernel");
}

void generar_conexiones(){

    establecer_conexion_cpu_dispatch(ip_cpu, puerto_cpu_dispatch, config_kernel, log_kernel);

    establecer_conexion_cpu_interrupt(ip_cpu, puerto_cpu_interrupt, config_kernel, log_kernel);
    
    establecer_conexion_memoria(ip_memoria, puerto_memoria, config_kernel, log_kernel);

    log_info(log_kernel, "Se generaron correctamente las conexiones");
}

void iniciar_semaforos(){
    pthread_mutex_init(&mutex_cola_ready, NULL);
    pthread_mutex_init(&mutex_cola_new, NULL);
    pthread_mutex_init(&mutex_cola_exec, NULL);

    sem_init(&sem_multiprogamacion, 0, grado_multiprogramacion);
    sem_init(&sem_listos_para_ready, 0, 0);
    sem_init(&sem_listos_para_exec, 0, 0);
    cola_new = list_create();
    cola_ready = list_create();
    cola_exec = list_create();
    generador_pid = 0;
}

void recibir_entradasalida(int SOCKET_CLIENTE_ENTRADASALIDA){
    enviar_string(socket_cliente_entradasalida, "hola desde kernel", MENSAJE);
    //Se deben enviar la cantidad de unidades de trabakp necesarias, crear una nueva funcion
    int noFinalizar = 0;
    while(noFinalizar != -1){
        int op_code = recibir_operacion(SOCKET_CLIENTE_ENTRADASALIDA);
    }
}

void recibir_cpu_dispatch(int conexion_kernel_cpu_dispatch){
    int noFinalizar = 0;
    while(noFinalizar != -1){
        int op_code = recibir_operacion(conexion_kernel_cpu_dispatch);
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
        finalizar_proceso();
        break;
    case 4:
        iniciar_planificacion();
        break;
    case 5:
        detener_planificacion();
        break;
    case 6:
        listar_procesos_estado();
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

    //ACA HAY QUE AVISARLE A MEMORIA QUE SE CREA UN PROCESO DE ESE PATH, DEBERIA DEVOLVER ALGUNA INFO?
    t_paquete* paquete = crear_paquete_op(INICIO_NUEVO_PROCESO);
    agregar_string_a_paquete(paquete,path);
    enviar_paquete(paquete,conexion_kernel_memoria);
    eliminar_paquete(paquete);
    generador_pid++;
    //creamos PCB
    t_registros_cpu* registros = inicializar_registros();
    t_pcb* pcb_nuevo = malloc(sizeof(t_pcb));
    pcb_nuevo->qq = quantum;
    pcb_nuevo->pid = generador_pid;
    pcb_nuevo->pc = 0;
    pcb_nuevo ->registros = registros;
    
    pthread_mutex_lock(&mutex_cola_new);
    list_add(cola_new, pcb_nuevo);
    pthread_mutex_unlock(&mutex_cola_new);
    sem_post(&sem_listos_para_ready);
}

void finalizar_proceso(){

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

    if(strcmp(algoritmo, "RR") == 0){
        planificar_rr();
    }
}

void planificar_rr(){

    pthread_t hilo_manejo_quantum;
    pthread_create(&hilo_manejo_quantum, NULL, (void *)contador_quantum_RR, NULL);
    pthread_detach(hilo_manejo_quantum);

}

void planificar_largo_plazo(){
    pthread_t hilo_ready;
    
    pthread_create(&hilo_ready, NULL, (void *)pcb_ready, NULL);

    pthread_detach(hilo_ready);
}

void planificar_corto_plazo(){
    pthread_t hilo_corto_plazo;
    pthread_create(&hilo_corto_plazo, NULL, (void *)exec_pcb, NULL);
    pthread_detach(hilo_corto_plazo);
}

void contador_quantum_RR(){
    
    while(1){
        //HAY QUE VER COMO HACER PARA UTILIZAR LO DEL QUANTUM Y MANDARLE LA INTERRUPCION A CPU
        sleep(quantum / 1000);
        if (strcmp(algoritmo, "RR") == 0)
        {
            //enviar_interrupcion(conexion_cpu_interrupt, interrupcion);
        }


    }
}

t_pcb* remover_pcb_de_lista(t_list *list, pthread_mutex_t *mutex)
{
    t_pcb* pcbDelProceso = malloc(sizeof(t_pcb));
    pthread_mutex_lock(mutex);
    pcbDelProceso = list_remove(list, 0);
    pthread_mutex_unlock(mutex);
    return pcbDelProceso;
}

void exec_pcb()
{
    while (1)
    {

        sem_wait(&sem_listos_para_exec);
        t_pcb* pcb_enviar = elegir_pcb_segun_algoritmo();

        dispatch(pcb_enviar);
    }
}

void pcb_ready(){
    sem_wait(&sem_listos_para_ready);
    t_pcb* pcb = remover_pcb_de_lista(cola_new, &mutex_cola_new);
    sem_wait(&sem_multiprogamacion);
    pthread_mutex_lock(&mutex_cola_ready);
    list_add(cola_ready,pcb);
    pthread_mutex_unlock(&mutex_cola_ready);
    sem_post(&sem_listos_para_exec);
}

t_pcb* elegir_pcb_segun_algoritmo(){
    t_pcb* pcb_ejecutar = malloc(sizeof(t_pcb));

    switch (algoritmo_planificacion)
    {
    case FIFO:
        pthread_mutex_lock(&mutex_cola_ready);
        pcb_ejecutar = list_remove(cola_ready,0);
        pthread_mutex_unlock(&mutex_cola_ready);
        break;
    case RR:
        pthread_mutex_lock(&mutex_cola_ready);
        pcb_ejecutar = list_remove(cola_ready,0);
        pthread_mutex_lock(&mutex_cola_ready);
        break;
    case VRR:
        break; 
    default:
        break;
    }
    
    return pcb_ejecutar;
}


void dispatch(t_pcb* pcb_enviar){

        
        log_trace(log_kernel, "envio pcb de pid: %d", pcb_enviar->pid);
        log_trace(log_kernel, "envio pcb de pc: %d", pcb_enviar->pc);
        log_trace(log_kernel, "envio pcb de qq: %d", pcb_enviar->qq);
        //ENVIAR CONTEXTO DE EJECUCION A CPU
        enviar_pcb(conexion_kernel_cpu_dispatch, pcb_enviar,EXEC);


        pthread_mutex_lock(&mutex_cola_exec);
        pcb_enviar = list_add(cola_exec, pcb_enviar);
        pthread_mutex_unlock(&mutex_cola_exec);
}
