#include <cpu.h>

int main(int argc, char** argv) {
    
    log_cpu = log_create("./cpu.log", "CPU", 1, LOG_LEVEL_TRACE);

    log_info(log_cpu, "INICIA EL MODULO DE CPU");


    //para poner un config es:
    // ./bin/cpu ./config/MemoriaTLB
    leer_config(argv[1]);
    
    
    establecer_conexion(ip_memoria,puerto_memoria, config_cpu, log_cpu);
    //generar_conexiones();

    log_info(log_cpu, "INICIO SERVIDOR DISPATCH");
    socket_servidor_cpu_dispatch = iniciar_servidor(puerto_escucha_dispatch, log_cpu);


    pthread_t atiende_cliente_kernel_dispatch;
    log_info(log_cpu, "Listo para recibir a kernel dispatch");
    socket_cliente_kernel_dispatch = esperar_cliente(socket_servidor_cpu_dispatch);
   
    pthread_create(&atiende_cliente_kernel_dispatch, NULL, (void *)recibir_kernel_dispatch, (void *) (intptr_t) socket_cliente_kernel_dispatch);
    pthread_detach(atiende_cliente_kernel_dispatch);

    socket_servidor_cpu_interrupt = iniciar_servidor(puerto_escucha_interrupt, log_cpu);
    log_info(log_cpu, "INICIO SERVIDOR INTERRUPT");

    pthread_t atiende_cliente_kernel_interrupt;
    log_info(log_cpu, "Listo para recibir a kernel interrupt");
    socket_cliente_kernel_interrupt = esperar_cliente(socket_servidor_cpu_interrupt);


    
    pthread_create(&atiende_cliente_kernel_interrupt, NULL, (void *)recibir_kernel_interrupt, (void *) (intptr_t) socket_cliente_kernel_interrupt);
    log_info(log_cpu, "Listo Hilos con kernel");
    pthread_join(atiende_cliente_kernel_interrupt, NULL);
    
    log_info(log_cpu, "Finalizo conexion con cliente");
    terminar_programa();
    

    return 0;
}

void leer_config(char* path){
    config_cpu = iniciar_config(path);

    ip_memoria = config_get_string_value(config_cpu, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_cpu, "PUERTO_MEMORIA");
    puerto_escucha_dispatch = config_get_string_value(config_cpu, "PUERTO_ESCUCHA_DISPATCH");
    puerto_escucha_interrupt = config_get_string_value(config_cpu, "PUERTO_ESCUCHA_INTERRUPT");
    cantidad_entradas_tlb = config_get_int_value(config_cpu, "CANTIDAD_ENTRADAS_TLB");
    algoritmo_tlb = config_get_string_value(config_cpu, "ALGORITMO_TLB");
    
    entrada_tlb = malloc(cantidad_entradas_tlb * sizeof(tlb));
    // Inicializa el arreglo
    for (int i = 0; i < tamanioTLB; i++) {
        entrada_tlb[i].pid = 0;
        entrada_tlb[i].numero_de_pagina = 0;
        entrada_tlb[i].marco = 0;
        entrada_tlb[i].contador_reciente = 0;
    }
    tamanioActualTlb = 0;

    log_info(log_cpu, "TLB inicializada con %d entradas", tamanioTLB);
    log_info(log_cpu, "levanto la configuracion del cpu");
}

void generar_conexiones(){
    establecer_conexion(ip_memoria,puerto_memoria, config_cpu, log_cpu);
    
}

void terminar_programa(){
    log_destroy(log_cpu);
    config_destroy(config_cpu);
    liberar_conexion(socket_servidor_cpu_dispatch);
    liberar_conexion(socket_servidor_cpu_interrupt);
    liberar_conexion(socket_cliente_kernel_dispatch);
    liberar_conexion(socket_cliente_kernel_interrupt);
}

void recibir_kernel_dispatch(int SOCKET_CLIENTE_KERNEL_DISPATCH){
    enviar_string(SOCKET_CLIENTE_KERNEL_DISPATCH, "hola desde cpu dispatch", MENSAJE);
    int noFinalizar = 0;
    while(noFinalizar != -1){
        int codOperacion = recibir_operacion(SOCKET_CLIENTE_KERNEL_DISPATCH);
        switch (codOperacion)
        {
        case EXEC:
            log_trace(log_cpu, "llego contexto de ejecucion");
            contexto = recibir_contexto(SOCKET_CLIENTE_KERNEL_DISPATCH);
            ejecutar_ciclo_de_instruccion(log_cpu);
            //sem_post(&sem_fin_de_ciclo);
            log_trace(log_cpu, "ejecute correctamente el ciclo de instruccion");
            break;
        case -1:
            noFinalizar=codOperacion;
            break;
        default:
            break;
        }
    }
}

void recibir_kernel_interrupt(int SOCKET_CLIENTE_KERNEL_INTERRUPT){
    enviar_string(SOCKET_CLIENTE_KERNEL_INTERRUPT, "hola desde cpu interrupt", MENSAJE);
    int noFinalizar = 0;
    while(noFinalizar != -1){
        int codOperacion = recibir_operacion(SOCKET_CLIENTE_KERNEL_INTERRUPT);
        switch (codOperacion)
        {
        case FIN_QUANTUM_RR:
            pid_interrupt = recibir_entero_uint32(SOCKET_CLIENTE_KERNEL_INTERRUPT,log_cpu);
            hay_interrupcion = 1;
            log_trace(log_cpu,"recibi una interrupcion para el pid: %d", pid_interrupt);
            break;
        case INTERRUPCION_USUARIO:
            pid_interrupt = recibir_entero_uint32(SOCKET_CLIENTE_KERNEL_INTERRUPT,log_cpu);
            hay_interrupcion = 1;
            es_por_usuario = 1;        
        case -1:
            noFinalizar=codOperacion;
            break;
        default:
            break;
        }
    }
}

void establecer_conexion(char * ip_memoria, char* puerto_memoria, t_config* config, t_log* loggs){


    log_trace(loggs, "Inicio como cliente");

    log_trace(loggs,"Lei la IP %s , el Puerto Memoria %s ", ip_memoria, puerto_memoria);

    if((conexion_memoria = crear_conexion(ip_memoria, puerto_memoria)) == -1){
        log_trace(loggs, "Error al conectar con Memoria. El servidor no esta activo");

        exit(2);
    }

    log_trace(loggs, "Todavía no recibí Op");
    log_trace(loggs, "Recibí Op");
    
}

void ejecutar_ciclo_de_instruccion(t_log* loggs){
    seguir_ejecutando = 1;
    while(seguir_ejecutando){
    
    t_instruccion* instruccion = fetch(contexto->pid, contexto->pc);
    
    op_code instruccion_nombre = decode(instruccion);

    execute(instruccion_nombre, instruccion);

    contexto->pc++;
    if(seguir_ejecutando){
    checkInturrupt(contexto->pid);
    }

    }

 }


t_instruccion* fetch(uint32_t pid, uint32_t pc){
    pedir_instruccion_memoria(pid, pc, log_cpu);
    log_info(log_cpu, "PID: %i - FETCH - Program Counter: %i", pid, pc);
    t_instruccion* instruccion = malloc(sizeof(t_instruccion));
    op_code codigo = recibir_operacion(conexion_memoria);
    instruccion = recibir_instruccion(conexion_memoria);
  return instruccion;
}

void pedir_instruccion_memoria(uint32_t pid, uint32_t pc, t_log *logg){
    t_paquete* paquete = crear_paquete_op(PEDIR_INSTRUCCION_MEMORIA);
    agregar_entero_a_paquete(paquete,pid);
    agregar_entero_a_paquete(paquete,pc);
    
    //log_info(logg, "serializacion %i %i", pid, pc); ya esta el log
    enviar_paquete(paquete,conexion_memoria);
    eliminar_paquete(paquete);

}

void execute(op_code instruccion_nombre, t_instruccion* instruccion) {
    switch (instruccion_nombre) {
        case SET:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s - PARAMETRO 2: %s", instruccion->parametros1, instruccion->parametros2, instruccion->parametros3);
            funcSet(instruccion);
            break;
        case SUM:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s - PARAMETRO 2: %s", instruccion->parametros1, instruccion->parametros2, instruccion->parametros3);
            funcSum(instruccion);
            break;
        case SUB:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s - PARAMETRO 2: %s", instruccion->parametros1, instruccion->parametros2, instruccion->parametros3);
            funcSub(instruccion);
            break;
        case JNZ:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s - PARAMETRO 2: %s", instruccion->parametros1, instruccion->parametros2, instruccion->parametros3);
            funcJnz(instruccion);
            break;
        case IO_GEN_SLEEP:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s - PARAMETRO 2: %s", instruccion->parametros1, instruccion->parametros2, instruccion->parametros3);
            funcIoGenSleep(instruccion);
            esperar_devolucion_pcb();
            break;
        case WAIT:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s", instruccion->parametros1, instruccion->parametros2);
            funcWait(instruccion);
            esperar_devolucion_pcb();
            break;
        case SIGNAL:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s", instruccion->parametros1, instruccion->parametros2);
            funcSignal(instruccion);
            esperar_devolucion_pcb();
            break;
        case EXIT:
            funcExit(instruccion);
        break;
        case MOV_IN:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s - PARAMETRO 2: %s", instruccion->parametros1, instruccion->parametros2, instruccion->parametros3);
            funcMovIn(instruccion);
            break;
        case MOV_OUT:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s - PARAMETRO 2: %s", instruccion->parametros1, instruccion->parametros2, instruccion->parametros3);
            funcMovOut(instruccion);
            break;
        case RESIZE:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s", instruccion->parametros1, instruccion->parametros2);
            funcResize(instruccion);
            break;
        case COPY_STRING:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s", instruccion->parametros1, instruccion->parametros2);
            funcCopyString(instruccion);
            break;
        case IO_STDIN_READ:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s - PARAMETRO 2: %s - PARAMETRO 3: %s", instruccion->parametros1, instruccion->parametros2, instruccion->parametros3, instruccion->parametros4);
            funcIoStdinRead(instruccion);
            esperar_devolucion_pcb();
            break;
        case IO_STDOUT_WRITE:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s - PARAMETRO 2: %s - PARAMETRO 3: %s", instruccion->parametros1, instruccion->parametros2, instruccion->parametros3, instruccion->parametros4);
            funcIoStdOutWrite(instruccion);
            esperar_devolucion_pcb();
        case IO_FS_CREATE:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s - PARAMETRO 2: %s", instruccion->parametros1, instruccion->parametros2, instruccion->parametros3);
            funcIoFsCreate(instruccion);
            esperar_devolucion_pcb();
            break;
        case IO_FS_DELETE:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s - PARAMETRO 2: %s", instruccion->parametros1, instruccion->parametros2, instruccion->parametros3);
            funcIoFsDelete(instruccion);
            esperar_devolucion_pcb();
            break;
        case IO_FS_TRUNCATE:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s - PARAMETRO 2: %s - PARAMETRO 3: %s", instruccion->parametros1, instruccion->parametros2, instruccion->parametros3, instruccion->parametros4);
            funcIoFsTruncate(instruccion);
            esperar_devolucion_pcb();
            break;
        case IO_FS_WRITE:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s - PARAMETRO 2: %s - PARAMETRO 3: %s - PARAMETRO 4: %s - PARAMETRO 5: %s", instruccion->parametros1, instruccion->parametros2, instruccion->parametros3, instruccion->parametros4, instruccion->parametros5, instruccion->parametros6);
            funcIoFsWrite(instruccion);
            esperar_devolucion_pcb();
            break;
        case IO_FS_READ:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s - PARAMETRO 2: %s - PARAMETRO 3: %s - PARAMETRO 4: %s - PARAMETRO 5: %s", instruccion->parametros1, instruccion->parametros2, instruccion->parametros3, instruccion->parametros4, instruccion->parametros5, instruccion->parametros6);
            funcIoFsRead(instruccion);
            esperar_devolucion_pcb();
            break;
        default:
            log_info(log_cpu, "Instrucción desconocida\n");
            break;
    }

    
}

void checkInturrupt(uint32_t pid){
    if (hay_interrupcion){
        hay_interrupcion = 0;
        if(contexto->pid == pid_interrupt){
            seguir_ejecutando = 0;
            if(!es_por_usuario){
            enviar_contexto(socket_cliente_kernel_dispatch, contexto,INTERRUPCION);
            }else{
            enviar_contexto(socket_cliente_kernel_dispatch, contexto,INTERRUPCION_USUARIO);
            }
        }
    }
}

op_code decode(t_instruccion *instruccion) {
    if (strcmp(instruccion->parametros1, "SET") == 0) {
        return SET;
    } else if (strcmp(instruccion->parametros1, "MOV_IN") == 0) {
        return MOV_IN;
    } else if (strcmp(instruccion->parametros1, "MOV_OUT") == 0) {
        return MOV_OUT;
    } else if (strcmp(instruccion->parametros1, "SUM") == 0) {
        return SUM;
    } else if (strcmp(instruccion->parametros1, "SUB") == 0) {
        return SUB;
    } else if (strcmp(instruccion->parametros1, "JNZ") == 0) {
        return JNZ;
    } else if (strcmp(instruccion->parametros1, "RESIZE") == 0) {
        return RESIZE;
    } else if (strcmp(instruccion->parametros1, "COPY_STRING") == 0) {
        return COPY_STRING;
    } else if (strcmp(instruccion->parametros1, "WAIT") == 0) {
        return WAIT;
    } else if (strcmp(instruccion->parametros1, "SIGNAL") == 0) {
        return SIGNAL;
    } else if (strcmp(instruccion->parametros1, "IO_GEN_SLEEP") == 0) {
        return IO_GEN_SLEEP;
    } else if (strcmp(instruccion->parametros1, "IO_STDIN_READ") == 0) {
        return IO_STDIN_READ;
    } else if (strcmp(instruccion->parametros1, "IO_STDOUT_WRITE") == 0) {
        return IO_STDOUT_WRITE;
    } else if (strcmp(instruccion->parametros1, "IO_FS_CREATE") == 0) {
        return IO_FS_CREATE;
    } else if (strcmp(instruccion->parametros1, "IO_FS_DELETE") == 0) {
        return IO_FS_DELETE;
    } else if (strcmp(instruccion->parametros1, "IO_FS_TRUNCATE") == 0) {
        return IO_FS_TRUNCATE;
    } else if (strcmp(instruccion->parametros1, "IO_FS_WRITE") == 0) {
        return IO_FS_WRITE;
    } else if (strcmp(instruccion->parametros1, "IO_FS_READ") == 0) {
        return IO_FS_READ;
    } else if (strcmp(instruccion->parametros1, "EXIT") == 0) {
        return EXIT;
    }
    
    return -1; // Código de operación no válido
}

void esperar_devolucion_pcb(){
    op_code codigo = recibir_operacion(socket_cliente_kernel_dispatch);
    contexto = recibir_contexto(socket_cliente_kernel_dispatch);
    if(codigo != BLOCK && codigo != EXIT){
    }else{
        seguir_ejecutando = 0;
    }
}

void funcSet(t_instruccion* instruccion) {
    if (strcmp(instruccion->parametros2, "AX") == 0) {
        contexto->registros->AX = atoi(instruccion->parametros3);
    } else if (strcmp(instruccion->parametros2, "BX") == 0) {
        contexto->registros->BX = atoi(instruccion->parametros3);
    } else if (strcmp(instruccion->parametros2, "CX") == 0) {
        contexto->registros->CX = atoi(instruccion->parametros3);
    } else if (strcmp(instruccion->parametros2, "DX") == 0) {
        contexto->registros->DX = atoi(instruccion->parametros3);
    } else if (strcmp(instruccion->parametros2, "EAX") == 0) {
        contexto->registros->EAX = atoi(instruccion->parametros3);
    } else if (strcmp(instruccion->parametros2, "EBX") == 0) {
        contexto->registros->EBX = atoi(instruccion->parametros3);
    } else if (strcmp(instruccion->parametros2, "ECX") == 0) {
        contexto->registros->ECX = atoi(instruccion->parametros3);
    } else if (strcmp(instruccion->parametros2, "EDX") == 0) {
        contexto->registros->EDX = atoi(instruccion->parametros3);
    } else if (strcmp(instruccion->parametros2, "PC") == 0) {
        contexto->pc = atoi(instruccion->parametros3);
    } else {
        printf("Registro desconocido: %s\n", instruccion->parametros1);
    }
}

void funcSum(t_instruccion * instruccion){
    
     if (strcmp(instruccion->parametros2, "AX") == 0) {

        uint8_t valor_registro_origen = obtener_valor_registro_XX(instruccion->parametros2);
        uint8_t valor_registro_destino = obtener_valor_registro_XX(instruccion->parametros3);
        contexto->registros->AX = valor_registro_origen + valor_registro_destino;

    } else if (strcmp(instruccion->parametros2, "BX") == 0) {

        uint8_t valor_registro_origen = obtener_valor_registro_XX(instruccion->parametros2);
        uint8_t valor_registro_destino = obtener_valor_registro_XX(instruccion->parametros3);
        contexto->registros->BX = valor_registro_origen + valor_registro_destino;

    } else if (strcmp(instruccion->parametros2, "CX") == 0) {

        uint8_t valor_registro_origen = obtener_valor_registro_XX(instruccion->parametros2);
        uint8_t valor_registro_destino = obtener_valor_registro_XX(instruccion->parametros3);
        contexto->registros->CX = valor_registro_origen + valor_registro_destino;

    } else if (strcmp(instruccion->parametros2, "DX") == 0) {

        uint8_t valor_registro_origen = obtener_valor_registro_XX(instruccion->parametros2);
        uint8_t valor_registro_destino = obtener_valor_registro_XX(instruccion->parametros3);
        contexto->registros->DX = valor_registro_origen + valor_registro_destino;

    } else if (strcmp(instruccion->parametros2, "EAX") == 0) {

        uint32_t valor_registro_origen = obtener_valor_registro_XXX(instruccion->parametros2);
        uint32_t valor_registro_destino = obtener_valor_registro_XXX(instruccion->parametros3);
        contexto->registros->EAX = valor_registro_origen + valor_registro_destino;

    } else if (strcmp(instruccion->parametros2, "EBX") == 0) {

        uint32_t valor_registro_origen = obtener_valor_registro_XXX(instruccion->parametros2);
        uint32_t valor_registro_destino = obtener_valor_registro_XXX(instruccion->parametros3);
        contexto->registros->EBX = valor_registro_origen + valor_registro_destino;

    } else if (strcmp(instruccion->parametros2, "ECX") == 0) {

        uint32_t valor_registro_origen = obtener_valor_registro_XXX(instruccion->parametros2);
        uint32_t valor_registro_destino = obtener_valor_registro_XXX(instruccion->parametros3);
        contexto->registros->ECX = valor_registro_origen + valor_registro_destino;

    } else if (strcmp(instruccion->parametros2, "EDX") == 0) {

        uint32_t valor_registro_origen = obtener_valor_registro_XXX(instruccion->parametros2);
        uint32_t valor_registro_destino = obtener_valor_registro_XXX(instruccion->parametros3);
        contexto->registros->EDX = valor_registro_origen + valor_registro_destino;

    } else {
        printf("Registro desconocido: %s\n", instruccion->parametros1);
    }
}

uint8_t obtener_valor_registro_XX(char* parametro){
    uint8_t valor_registro;
    if (strcmp(parametro, "AX") == 0) {
        valor_registro = contexto->registros->AX;
    } else if (strcmp(parametro, "BX") == 0) {
        valor_registro = contexto->registros->BX;
    } else if (strcmp(parametro, "CX") == 0) {
        valor_registro = contexto->registros->CX;
    } else if (strcmp(parametro, "DX") == 0) {
        valor_registro = contexto->registros->DX;
    }else {
        printf("Registro desconocido: %s\n", parametro);
        return 0;
    }
    return valor_registro;
}

uint32_t obtener_valor_registro_XXX(char* parametro){
    uint32_t valor_registro;
    if (strcmp(parametro, "EAX") == 0) {
        valor_registro = contexto->registros->EAX;
    } else if (strcmp(parametro, "EBX") == 0) {
        valor_registro = contexto->registros->EBX;
    } else if (strcmp(parametro, "ECX") == 0) {
        valor_registro = contexto->registros->ECX;
    } else if (strcmp(parametro, "EDX") == 0) {
        valor_registro = contexto->registros->EDX;
    }else {
        printf("Registro desconocido: %s\n", parametro);
        return 0;
    }
    return valor_registro;
}

void funcSub(t_instruccion * instruccion){
   if (strcmp(instruccion->parametros2, "AX") == 0) {

        uint8_t valor_registro_origen = obtener_valor_registro_XX(instruccion->parametros2);
        uint8_t valor_registro_destino = obtener_valor_registro_XX(instruccion->parametros3);
        contexto->registros->AX = valor_registro_origen - valor_registro_destino;

    } else if (strcmp(instruccion->parametros2, "BX") == 0) {

        uint8_t valor_registro_origen = obtener_valor_registro_XX(instruccion->parametros2);
        uint8_t valor_registro_destino = obtener_valor_registro_XX(instruccion->parametros3);
        contexto->registros->BX = valor_registro_origen - valor_registro_destino;

    } else if (strcmp(instruccion->parametros2, "CX") == 0) {

        uint8_t valor_registro_origen = obtener_valor_registro_XX(instruccion->parametros2);
        uint8_t valor_registro_destino = obtener_valor_registro_XX(instruccion->parametros3);
        contexto->registros->CX = valor_registro_origen - valor_registro_destino;

    } else if (strcmp(instruccion->parametros2, "DX") == 0) {

        uint8_t valor_registro_origen = obtener_valor_registro_XX(instruccion->parametros2);
        uint8_t valor_registro_destino = obtener_valor_registro_XX(instruccion->parametros3);
        contexto->registros->DX = valor_registro_origen - valor_registro_destino;

    } else if (strcmp(instruccion->parametros2, "EAX") == 0) {

        uint32_t valor_registro_origen = obtener_valor_registro_XXX(instruccion->parametros2);
        uint32_t valor_registro_destino = obtener_valor_registro_XXX(instruccion->parametros3);
        contexto->registros->EAX = valor_registro_origen - valor_registro_destino;

    } else if (strcmp(instruccion->parametros2, "EBX") == 0) {

        uint32_t valor_registro_origen = obtener_valor_registro_XXX(instruccion->parametros2);
        uint32_t valor_registro_destino = obtener_valor_registro_XXX(instruccion->parametros3);
        contexto->registros->EBX = valor_registro_origen - valor_registro_destino;

    } else if (strcmp(instruccion->parametros2, "ECX") == 0) {

        uint32_t valor_registro_origen = obtener_valor_registro_XXX(instruccion->parametros2);
        uint32_t valor_registro_destino = obtener_valor_registro_XXX(instruccion->parametros3);
        contexto->registros->ECX = valor_registro_origen - valor_registro_destino;

    } else if (strcmp(instruccion->parametros2, "EDX") == 0) {

        uint32_t valor_registro_origen = obtener_valor_registro_XXX(instruccion->parametros2);
        uint32_t valor_registro_destino = obtener_valor_registro_XXX(instruccion->parametros3);
        contexto->registros->EDX = valor_registro_origen - valor_registro_destino;

    } else {
        printf("Registro desconocido: %s\n", instruccion->parametros1);
    }
}

void funcJnz(t_instruccion *instruccion) {
    uint32_t reg_value;
    if (strcmp(instruccion->parametros2, "AX") == 0) {
        reg_value = contexto->registros->AX;
    } else if (strcmp(instruccion->parametros2, "BX") == 0) {
        reg_value = contexto->registros->BX;
    } else if (strcmp(instruccion->parametros2, "CX") == 0) {
        reg_value = contexto->registros->CX;
    } else if (strcmp(instruccion->parametros2, "DX") == 0) {
        reg_value = contexto->registros->DX;
    } else if (strcmp(instruccion->parametros2, "EAX") == 0) {
        reg_value = contexto->registros->EAX;
    } else if (strcmp(instruccion->parametros2, "EBX") == 0) {
        reg_value = contexto->registros->EBX;
    } else if (strcmp(instruccion->parametros2, "ECX") == 0) {
        reg_value = contexto->registros->ECX;
    } else if (strcmp(instruccion->parametros2, "EDX") == 0) {
        reg_value = contexto->registros->EDX;
    } else {
        printf("Registro desconocido: %s\n", instruccion->parametros2);
        return;
    }

    if (reg_value != 0) {
        contexto->pc = atoi(instruccion->parametros3);
    }
}

//-----SOLO SE ENVIAN LA SOLICITUD AL KERNEL -> EL KERNEL SERA EL ENCARGADO DE REALIZAR DICHA ACCION-----
void funcIoGenSleep(t_instruccion *instruccion) {
    
    t_paquete *paquete = crear_paquete_op(EJECUTAR_IO_GEN_SLEEP);
    agregar_a_paquete(paquete,instruccion->parametros2, strlen(instruccion->parametros2)+1);
    agregar_entero_a_paquete(paquete, atoi(instruccion->parametros3));
    agregar_contexto_a_paquete(paquete, contexto);
    enviar_paquete(paquete,socket_cliente_kernel_dispatch);
    eliminar_paquete(paquete);

}

void funcSignal(t_instruccion *instruccion){
    t_paquete* paquete = crear_paquete_op(EJECUTAR_SIGNAL);
    
    agregar_a_paquete(paquete,instruccion->parametros2, strlen(instruccion->parametros2)+1);
    agregar_contexto_a_paquete(paquete,contexto);
    enviar_paquete(paquete,socket_cliente_kernel_dispatch);
    
}

void funcWait(t_instruccion *instruccion){
    t_paquete* paquete = crear_paquete_op(EJECUTAR_WAIT);
    
    agregar_a_paquete(paquete,instruccion->parametros2, strlen(instruccion->parametros2)+1);
    agregar_contexto_a_paquete(paquete,contexto);
    enviar_paquete(paquete,socket_cliente_kernel_dispatch);
    eliminar_paquete(paquete);
}

void funcExit(t_instruccion *instruccion) {
    seguir_ejecutando = 0;
    t_paquete *paquete = crear_paquete_op(TERMINO_PROCESO);
    agregar_contexto_a_paquete(paquete,contexto);
    enviar_paquete(paquete, socket_cliente_kernel_dispatch);
    eliminar_paquete(paquete);
}

void funcIoStdinRead(t_instruccion *instruccion) {
    
    uint32_t registro_direccion = obtener_valor_registro(instruccion->parametros3);
    uint32_t registro_tamanio = obtener_valor_registro(instruccion->parametros4);
    t_paquete *paquete = crear_paquete_op(EJECUTAR_IO_STDIN_READ);
    agregar_entero_a_paquete(paquete,registro_direccion);
    agregar_entero_a_paquete(paquete,registro_tamanio);
    agregar_a_paquete(paquete,instruccion->parametros2, strlen(instruccion->parametros2)+1); //interfaz
    agregar_contexto_a_paquete(paquete, contexto);
    enviar_paquete(paquete, socket_cliente_kernel_dispatch);
    eliminar_paquete(paquete);
}

void funcIoStdOutWrite(t_instruccion *instruccion) {
    
    uint32_t registro_direccion = obtener_valor_registro(instruccion->parametros3);
    uint32_t registro_tamanio = obtener_valor_registro(instruccion->parametros4);
    t_paquete *paquete = crear_paquete_op(EJECUTAR_IO_STDOUT_WRITE);
    agregar_entero_a_paquete(paquete,registro_direccion);
    agregar_entero_a_paquete(paquete,registro_tamanio);
    agregar_a_paquete(paquete,instruccion->parametros2, strlen(instruccion->parametros2)+1); //interfaz
    agregar_contexto_a_paquete(paquete, contexto);
    enviar_paquete(paquete, socket_cliente_kernel_dispatch);
    eliminar_paquete(paquete);
}

void funcIoFsCreate(t_instruccion *instruccion) {

    t_paquete *paquete = crear_paquete_op(EJECUTAR_IO_FS_CREATE);  
    agregar_a_paquete(paquete,instruccion->parametros2, strlen(instruccion->parametros2)+1); //interfaz
    agregar_a_paquete(paquete,instruccion->parametros3, strlen(instruccion->parametros3)+1); //nombre archivo
    agregar_contexto_a_paquete(paquete, contexto);
    enviar_paquete(paquete, socket_cliente_kernel_dispatch);
    
    eliminar_paquete(paquete);
}

void funcIoFsDelete(t_instruccion *instruccion) {

    t_paquete *paquete = crear_paquete_op(EJECUTAR_IO_FS_DELETE);
    agregar_a_paquete(paquete,instruccion->parametros2, strlen(instruccion->parametros2)+1); //interfaz
    agregar_a_paquete(paquete,instruccion->parametros3, strlen(instruccion->parametros3)+1); //nombre archivo
    agregar_contexto_a_paquete(paquete, contexto);
    enviar_paquete(paquete, socket_cliente_kernel_dispatch);
    
    eliminar_paquete(paquete);
}

void funcIoFsTruncate(t_instruccion *instruccion) {
    uint32_t registro_tamanio;
    registro_tamanio = obtener_valor_registro(instruccion->parametros4);
    t_paquete* paquete = crear_paquete_op(EJECUTAR_IO_FS_TRUNCATE);
    agregar_a_paquete(paquete,instruccion->parametros2, strlen(instruccion->parametros2)+1); //interfaz
    agregar_a_paquete(paquete,instruccion->parametros3, strlen(instruccion->parametros3)+1); //nombre archivo
    agregar_entero_a_paquete(paquete,registro_tamanio);
    agregar_contexto_a_paquete(paquete, contexto);
    enviar_paquete(paquete, socket_cliente_kernel_dispatch);
    eliminar_paquete(paquete);
}

void funcIoFsWrite(t_instruccion* instruccion) {
    uint32_t registro_direccion;
    uint32_t registro_tamanio;
    uint32_t registro_puntero;
    registro_direccion = obtener_valor_registro(instruccion->parametros4);
    registro_tamanio = obtener_valor_registro(instruccion->parametros5);
    registro_puntero = obtener_valor_registro(instruccion->parametros6);
    t_paquete* paquete = crear_paquete_op(EJECUTAR_IO_FS_WRITE);
    agregar_a_paquete(paquete,instruccion->parametros2, strlen(instruccion->parametros2)+1); //interfaz
    agregar_a_paquete(paquete,instruccion->parametros3, strlen(instruccion->parametros3)+1); //nombre archivo
    agregar_entero_a_paquete(paquete,registro_direccion);
    agregar_entero_a_paquete(paquete,registro_tamanio);
    agregar_entero_a_paquete(paquete,registro_puntero);
    agregar_contexto_a_paquete(paquete, contexto);
    enviar_paquete(paquete, socket_cliente_kernel_dispatch);
    eliminar_paquete(paquete);
}

void funcIoFsRead(t_instruccion* instruccion) {
    uint32_t registro_direccion;
    uint32_t registro_tamanio;
    uint32_t registro_puntero;
    registro_direccion = obtener_valor_registro(instruccion->parametros4);
    registro_tamanio = obtener_valor_registro(instruccion->parametros5);
    registro_puntero = obtener_valor_registro(instruccion->parametros6);
    t_paquete* paquete = crear_paquete_op(EJECUTAR_IO_FS_READ);
    agregar_a_paquete(paquete,instruccion->parametros2, strlen(instruccion->parametros2)+1); //interfaz
    agregar_a_paquete(paquete,instruccion->parametros3, strlen(instruccion->parametros3)+1); //nombre archivo
    agregar_entero_a_paquete(paquete,registro_direccion);
    agregar_entero_a_paquete(paquete,registro_tamanio);
    agregar_entero_a_paquete(paquete,registro_puntero);
    agregar_contexto_a_paquete(paquete, contexto);
    enviar_paquete(paquete, socket_cliente_kernel_dispatch);
    eliminar_paquete(paquete);
}

void funcCopyString(t_instruccion *instruccion) {
    
    uint32_t tamanio = atoi(instruccion->parametros2);

    t_paquete *paquete = crear_paquete_op(COPY_STRING);
    agregar_entero_a_paquete(paquete, tamanio);
    agregar_contexto_a_paquete(paquete, contexto);
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete); 
}

void funcResize(t_instruccion* instruccion){
    uint32_t tamanio = atoi(instruccion->parametros2);

    t_paquete *paquete = crear_paquete_op(RESIZE);
    agregar_entero_a_paquete(paquete, tamanio);
    agregar_entero_a_paquete(paquete, contexto->pid);
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);
    log_info(log_cpu, "Tamanio enviado");

    op_code cod_op = recibir_operacion(conexion_memoria);
        
        switch(cod_op) {
            case RESIZE_OK:
            log_trace(log_cpu, "Entro en RESIZE COD : %d", cod_op);
            log_info(log_cpu, "Se ajusta tamanio de proceso");
            break;
            case OUT_OF_MEMORY:
            log_trace(log_cpu, "Entro en OUT OF MEMORY COD : %d", cod_op);
            enviar_contexto(socket_cliente_kernel_interrupt, contexto, cod_op);
            break;
            default:
            log_warning(log_cpu, "Llego un codigo de operacion desconocido, %d", cod_op);
            break;
        }
    }


uint32_t obtener_valor_registro(char* registro){
    uint32_t devolver;
     if (strcmp(registro, "AX") == 0) {  
        devolver = contexto->registros->AX;
    } else if (strcmp(registro, "BX") == 0) {
        devolver = contexto->registros->BX;
    } else if (strcmp(registro, "CX") == 0) {
        devolver = contexto->registros->CX;
    } else if (strcmp(registro, "DX") == 0) {
        devolver = contexto->registros->DX;
    } else if (strcmp(registro, "EAX") == 0) {
        devolver = contexto->registros->EAX;
    } else if (strcmp(registro, "EBX") == 0) {
        devolver = contexto->registros->EBX;
    } else if (strcmp(registro, "ECX") == 0) {
        devolver = contexto->registros->ECX;
    } else if (strcmp(registro, "EDX") == 0) {
        devolver = contexto->registros->EDX;
    } else {
        printf("Registro desconocido: %s\n", registro);
    }

    return devolver;
}

uint32_t traducirDireccion(uint32_t dirLogica, uint32_t tamanio_pagina) {
    // uint32_t numero_pagina = dirLogica / tamanio_pagina;
    // uint32_t desplazamiento = dirLogica - numero_pagina * tamanio_pagina;
    // log_info(log_cpu, "direccion logica : %i", dirLogica);
    // log_info(log_cpu, "numero pag : %i", numero_pagina);
    // log_info(log_cpu, "desplazamiento : %i", desplazamiento);
    // int marco = -1;

    // // Búsqueda en la TLB
    // for (int i = 0; i < cantidad_entradas_tlb; i++) {
    //     if (contexto->pid == entrada_tlb[i].pid && numero_pagina == entrada_tlb[i].numero_de_pagina) { // TLB hit
    //         log_info(log_cpu, "TLB Hit: PID: %i - Pagina: %i", contexto->pid, numero_pagina);
    //         dirFisica = entrada_tlb[i].marco * tamanio_pagina + desplazamiento;
    //         return dirFisica;
    //     }
    // }

    // // TLB miss
    // log_info(log_cpu, "TLB Miss: PID: %i - Pagina: %i", contexto->pid, numero_pagina);
    // t_2_enteros algo = { .entero1 = numero_pagina, .entero2 = contexto->pid };
    // enviar_2_enteros(conexion_memoria, &algo, ACCESO_TABLA_PAGINAS);

    // uint32_t marco;
    // while (1) {
    //     int cod_op = recibir_operacion(conexion_memoria);
    //     if (cod_op == ACCESO_TABLA_PAGINAS_OK) {
    //         log_info(log_cpu, "Codigo de operacion recibido en CPU: %d", cod_op);
    //         marco = recibir_entero_uint32(conexion_memoria, log_cpu);
    //         log_info(log_cpu, "Recibido marco: %i", marco);
    //         break;
    //     } else {
    //         log_warning(log_cpu, "Codigo de operacion desconocido: %d", cod_op);
    //     }
    // }

    // log_info(log_cpu, "PID: %i - OBTENER MARCO - Página: %i - Marco: %i", contexto->pid, numero_pagina, marco);
    
    // // Actualizar la TLB
    // agregar_entrada_tlb(contexto->pid, marco, numero_pagina);

    // dirFisica = marco * tamanio_pagina + desplazamiento;
    // return dirFisica;
    //nuevo---------------------
    uint32_t numero_pagina = dirLogica / tamanio_pagina;
    log_info(log_cpu, "num pagina calculada: %i", numero_pagina);
    uint32_t desplazamiento = dirLogica - numero_pagina * tamanio_pagina;
    log_info(log_cpu, "num DESPLAZA: %i", desplazamiento);
    int marco = -1;
    log_info(log_cpu, "DIR LOGICA _%i - TAM PAGINA: %i", dirLogica, tamanio_pagina);
    if(cantidad_entradas_tlb) {
        marco = buscarMarcoEnTLB(contexto->pid, numero_pagina);
    }

    if(marco == -1) {
        log_info(log_cpu, "EL MARCO ES -1");
        //crear estruct necesarias
        t_paquete *paquete = crear_paquete_op(ACCESO_TABLA_PAGINAS);
        t_2_enteros *pagMarco = malloc(sizeof(t_2_enteros));
        pagMarco->entero1 = contexto->pid;
        pagMarco->entero2 = numero_pagina;
        log_info(log_cpu, "pid: %i", contexto->pid);
        log_info(log_cpu, "num pagina: %i", numero_pagina);
        agregar_2_enteros_a_paquete(paquete, pagMarco);
        enviar_paquete(paquete, conexion_memoria);
        eliminar_paquete(paquete);
        log_info(log_cpu, "EL MARCO ES -1");

        op_code cod = recibir_operacion(conexion_memoria);
        log_info(log_cpu, "CODIGO RECIBIDO DE MEMORIA: %i", cod);

        if(cod == ACCESO_TABLA_PAGINAS_OK) {
            log_info(log_cpu, "ENTRO EN ACC TABLA PAG");
            marco = recibir_entero_uint32(conexion_memoria, log_cpu);
            log_info(log_cpu, "Marco: %i", marco);
            agregarEntradaTLB(contexto->pid, numero_pagina, marco);
        }
        else {
            log_info(log_cpu, "codigo :%i", cod);
            perror("FALLO en la obtencion del marco");
            marco = (uint32_t)-1;
        }


    }

    int dirFisica = marco * tamanio_pagina + desplazamiento;

    log_info(log_cpu, "PID: <%i> - OBTENER MARCO - Página: <%i> - Marco: <%i>", contexto->pid, numero_pagina, marco);

	log_info(log_cpu, "direccion fisica = marco * tam_pagina + desplazamiento: %i", dirFisica);

    return dirFisica;

}

uint32_t buscarMarcoEnTLB(uint32_t pidBuscar, uint32_t numPagBuscar) {
    for (size_t i = 0; i < tamanioActualTlb; ++i) {
        if (entrada_tlb[i].pid == pidBuscar && entrada_tlb[i].numero_de_pagina == numPagBuscar) {
            if (strcmp(algoritmo_tlb, "LRU") == 0) {
                // Mover la entrada encontrada al final (simulando LRU)
                tlb entrada = entrada_tlb[i];
                for (size_t j = i; j < tamanioActualTlb - 1; ++j) {
                    entrada_tlb[j] = entrada_tlb[j + 1];
                }
                entrada_tlb[tamanioActualTlb - 1] = entrada;
            }
            log_info(log_cpu,"PID: <%i> - TLB HIT - Pagina: <%i>", pidBuscar, numPagBuscar);
            return entrada_tlb[tamanioActualTlb - 1].marco;
        }
    }
    log_info(log_cpu, "PID: <%d> - TLB MISS - Pagina: <%d>\n", pidBuscar, numPagBuscar);
    return (uint32_t)-1;
}

void agregarEntradaTLB(uint32_t pid, uint32_t numPag, uint32_t marco) {
    if (strcmp(algoritmo_tlb, "LRU") == 0) {
        agregarEntradaTLB_LRU(pid, numPag, marco);
    } else {
        agregarEntradaTLB_FIFO(pid, numPag, marco);
    }
}

void agregarEntradaTLB_FIFO(uint32_t pid, uint32_t numPag, uint32_t marco) {
    if (tamanioActualTlb == tamanioTLB) {
        // La TLB está llena, hay que eliminar la entrada más antigua (FIFO)
        // En este caso, simplemente desplazamos todas las entradas una posición a la izquierda
        for (size_t i = 0; i < tamanioActualTlb - 1; ++i) {
            entrada_tlb[i] = entrada_tlb[i + 1];
        }
        tamanioActualTlb--;
    }

    // Añadir la nueva entrada al final
    entrada_tlb[tamanioActualTlb].pid = pid;
    entrada_tlb[tamanioActualTlb].numero_de_pagina = numPag;
    entrada_tlb[tamanioActualTlb].marco = marco;
    entrada_tlb[tamanioActualTlb].contador_reciente = 0; // Inicializar contador LRU
    tamanioActualTlb++;
}

void agregarEntradaTLB_LRU(uint32_t pid, uint32_t numPag, uint32_t marco) {
    if (tamanioActualTlb == tamanioTLB) {
        // La TLB está llena, hay que eliminar la entrada menos reciente (LRU)
        // En este caso, simplemente desplazamos todas las entradas una posición a la izquierda
        for (size_t i = 0; i < tamanioActualTlb - 1; ++i) {
            entrada_tlb[i] = entrada_tlb[i + 1];
        }
        tamanioActualTlb--;
    }

    // Añadir la nueva entrada al final
    entrada_tlb[tamanioActualTlb].pid = pid;
    entrada_tlb[tamanioActualTlb].numero_de_pagina = numPag;
    entrada_tlb[tamanioActualTlb].marco = marco;
    entrada_tlb[tamanioActualTlb].contador_reciente = 0; // Inicializar contador LRU
    tamanioActualTlb++;
}

// void agregar_entrada_tlb(uint32_t pid, uint32_t marco, uint32_t pagina) {
//     bool espacio_libre = false;

//     // Buscar un espacio libre en la TLB
//     for (int i = 0; i < cantidad_entradas_tlb; i++) {
//         if (entrada_tlb[i].pid == 0) {
//             entrada_tlb[i].pid = pid;
//             entrada_tlb[i].numero_de_pagina = pagina;
//             entrada_tlb[i].marco = marco;
//             entrada_tlb[i].contador_reciente = 0; // Inicializar el contador LRU
//             espacio_libre = true;
//             break;
//         }
//     }

//     // Si no hay espacio libre, usar el algoritmo de reemplazo
//     if (!espacio_libre) {
//         if (strcmp(algoritmo_tlb, "FIFO") == 0) {
//             reemplazarXFIFO(pid, marco, pagina);
//         } else if (strcmp(algoritmo_tlb, "LRU") == 0) {
//             reemplazarXLRU(pid, marco, pagina);
//         }
//     }
// }

// void reemplazarXFIFO(uint32_t pid, uint32_t marco, uint32_t pagina){
//     int cantidad_entradas_tlb_valor = cantidad_entradas_tlb;  // Obtener el valor entero desde el puntero
    
//     indice_frente = 0; // Obtener índice a reemplazar

//     entrada_tlb[indice_frente].pid = contexto->pid;
//     entrada_tlb[indice_frente].numero_de_pagina = pagina;
//     entrada_tlb[indice_frente].marco = marco;

//     indice_frente = (indice_frente + 1) % cantidad_entradas_tlb_valor;
// }


// void reemplazarXLRU(uint32_t pid, uint32_t marco, uint32_t pagina){
    
//     int indice_menos_reciente = 0;
//     uint32_t menos_reciente = entrada_tlb[0].contador_reciente;

//     for(int i = 1; i < cantidad_entradas_tlb; i++){
//         if (entrada_tlb[i].contador_reciente < menos_reciente){
//             menos_reciente = entrada_tlb[i].contador_reciente;
//             indice_menos_reciente = i;
//         }
//     }

//     //reemplazar la entrada menos reciente usada con la nueva entrada de la tlb
//     entrada_tlb[indice_menos_reciente].pid = contexto->pid;
//     entrada_tlb[indice_menos_reciente].numero_de_pagina = pagina;
//     entrada_tlb[indice_menos_reciente].marco = marco;

//     //reiniciar el cont reciente de la entrada reemplazada, porque se acabo de usar
//     entrada_tlb[indice_menos_reciente].contador_reciente = 0;

//     for(int i = 0; i < cantidad_entradas_tlb ; i++){
//         if(i != indice_menos_reciente){
//             entrada_tlb[i].contador_reciente++;
//         }
//     }

// }

uint32_t tamanio_registro(char *registro){
    if (strcmp(registro, "AX") == 0) return 1;
    else if (strcmp(registro, "BX") == 0) return 1;
    else if (strcmp(registro, "CX") == 0) return 1;
    else if (strcmp(registro, "DX") == 0) return 1;
    else if (strcmp(registro, "EAX") == 0) return 4;
    else if (strcmp(registro, "EBX") == 0) return 4;
    else if (strcmp(registro, "ECX") == 0) return 4;
    else if (strcmp(registro, "EDX") == 0) return 4;
    return 0;
}

char *leer_valor_de_memoria(uint32_t direccionFisica, uint32_t tamanio) {

    t_paquete *paquete = crear_paquete_op(MOV_IN);
    agregar_entero_a_paquete(paquete, direccionFisica);
    agregar_entero_a_paquete(paquete, contexto->pid);
    agregar_entero_a_paquete(paquete, tamanio);

    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);
    log_trace(log_cpu, "MOV IV enviado");

    while(1) {
        int cod_op = recibir_operacion(conexion_memoria);
        switch (cod_op)
        {
        case 0:
            log_error(log_cpu, "Llego codigo operacion 0");
            break;
        case MOV_IN_OK:
            log_info(log_cpu, "Codigo de operacion recibido en cpu : %d", cod_op);
            
            char *valor_recibido = recibir_string(conexion_memoria, log_cpu);

            log_info(log_cpu, "PID: %d - Acción: LEER - Dirección física: %d - Valor: %s",
                        contexto->pid, direccionFisica, valor_recibido);
            log_info(log_cpu, "Recibo string :%s", valor_recibido);
            return valor_recibido;
            break;
        default:
            log_warning(log_cpu, "Llego un codigo de operacion desconocido, %d", cod_op);
            break;
        }
    }

}

void guardar_valor_en_registro(char *valor, char *registro) {

    log_trace(log_cpu, "El registro %s quedara el valor: %s",registro ,valor);

    agregar_valor_a_registro(registro, valor);

}

void agregar_valor_a_registro(char *reg, char *val) { 
    log_trace(log_cpu, "Caracteres a sumarle al registro %s", val);
    if (strcmp(reg, "AX") == 0) {
        contexto->registros->AX = (uint8_t) atoi(val);
    } else if (strcmp(reg, "BX") == 0) {
        contexto->registros->BX = (uint8_t) atoi(val);
    } else if (strcmp(reg, "CX") == 0) {
        contexto->registros->CX = (uint8_t) atoi(val);
    } else if (strcmp(reg, "DX") == 0) {
        contexto->registros->DX = (uint8_t) atoi(val);
    } else if (strcmp(reg, "EAX") == 0) {
        contexto->registros->EAX = (uint32_t) strtoul(val, NULL, 10);
    } else if (strcmp(reg, "EBX") == 0) {
        contexto->registros->EBX = (uint32_t) strtoul(val, NULL, 10);
    } else if (strcmp(reg, "ECX") == 0) {
        contexto->registros->ECX = (uint32_t) strtoul(val, NULL, 10);
    } else if (strcmp(reg, "EDX") == 0) {
        contexto->registros->EDX = (uint32_t) strtoul(val, NULL, 10);
    }
}


void funcMovIn(t_instruccion *instruccion) { 
    log_info(log_cpu, "Instruccion MOV_IN ejecutada");
    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s - %s", contexto->pid, instruccion->parametros1, instruccion->parametros2, instruccion->parametros3);

    char *registroDatos = instruccion->parametros2;
    char *registroDireccionLogica = instruccion->parametros3;

    // Convertir la dirección lógica de cadena a entero
    uint32_t direccionLogica = (uint32_t) strtoul(registroDireccionLogica, NULL, 10);

    // Ahora traducir la dirección lógica a física
    int tamanio_regDatos = tamanio_registro(registroDatos);
    uint32_t direccionFisica = traducirDireccion(direccionLogica, tamanio_regDatos);

    char *valor = leer_valor_de_memoria(direccionFisica, tamanio_regDatos);

    // Guardar el valor en registro datos
    guardar_valor_en_registro(valor, registroDatos);
    log_info(log_cpu, "PID: %d - Acción: ESCRIBIR - Dirección Fisica: %d", contexto->pid, direccionFisica);
    log_info(log_cpu, "Valor guardado en registro");
}

uint32_t leer_valor_de_registro(char *registro) {
    
    if (!strncmp(registro, "AX", strlen("AX"))){
        return contexto->registros->AX;
    }
    if (!strncmp(registro, "BX", strlen("BX"))) {
        return contexto->registros->BX;
     }
    if (!strncmp(registro, "CX", strlen("CX"))) {
        return contexto->registros->CX;
     }
    if (!strncmp(registro, "DX", strlen("DX"))) {
        return contexto->registros->DX;
     }
    if (!strncmp(registro, "EAX", strlen("EAX"))) {
        return contexto->registros->EAX;
     }
    if (!strncmp(registro, "EBX", strlen("EBX"))) {
        return contexto->registros->EBX;
     }
    if (!strncmp(registro, "ECX", strlen("ECX"))) {
        return contexto->registros->ECX;
     }
    if (!strncmp(registro, "EDX", strlen("EDX"))) {
        return contexto->registros->EDX;
     }
    if (!strncmp(registro, "SI", strlen("SI"))) {
        return contexto->registros->SI;
     }
    if (!strncmp(registro, "DI", strlen("DI"))) {
        return contexto->registros->DI;
     }

     return 0;
}


void escribir_valor_en_memoria(uint32_t direccionFisica, uint32_t valor, uint32_t tamanio) {
    t_paquete *paquete = crear_paquete_op(MOV_OUT);
    agregar_entero_a_paquete(paquete, direccionFisica);
    agregar_entero_a_paquete(paquete, contexto->pid);
    agregar_entero_a_paquete(paquete, tamanio);
    agregar_entero_a_paquete(paquete, valor);

    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);
    log_trace(log_cpu, "MOV OUT enviado");

    while(1) {
        int cod_op = recibir_operacion(conexion_memoria);
        switch (cod_op) {
        case 0:
            log_error(log_cpu, "Llego código operación 0");
            break;
        case MOV_OUT_OK:
            log_info(log_cpu, "Código de operación recibido en cpu: %d", cod_op);
            log_info(log_cpu, "Valor escrito en memoria correctamente");
            log_info(log_cpu, "PID: %d - Acción: ESCRIBIR - Dirección física: %i - Valor: %i",
                        contexto->pid, direccionFisica, valor);
            return;
        default:
            log_warning(log_cpu, "Llegó un código de operación desconocido, %i", cod_op);
            break;
        }
    }

}

void funcMovOut(t_instruccion* instruccion){
    log_info(log_cpu, "ENTRANDO A FUNC MOV OUT");
    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s - %s", contexto->pid, instruccion->parametros1, instruccion->parametros2, instruccion->parametros3);

    char *registroDireccionLogica = instruccion->parametros2;
    char *registroDatos = instruccion->parametros3;

    // Traduzco la direc
    uint32_t tamanio_regDatos = tamanio_registro(registroDatos);
    log_info(log_cpu, "tam reg datos :%i", tamanio_regDatos);
    uint32_t direccionFisica = traducirDireccion(strlen(registroDireccionLogica), tamanio_regDatos);

    // Leer el valor del registro de datos
    uint32_t valor = leer_valor_de_registro(registroDatos);

    // Escribir el valor en memoria
    escribir_valor_en_memoria(direccionFisica, valor, tamanio_regDatos);

    log_info(log_cpu, "PID: %d - Acción: ESCRIBIR - Dirección Fisica: %d", contexto->pid, direccionFisica);
    log_info(log_cpu, "Valor escrito en memoria");
    log_info(log_cpu, "Instruccion MOV_OUT ejecutada");
}
