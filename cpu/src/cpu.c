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
    
    tlb entrada_tlb[cantidad_entradas_tlb]; //verificar que funcione asi
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
    log_info(loggs, "paso el while");
    t_instruccion* instruccion = fetch(contexto->pid, contexto->pc);
    log_info(loggs, "paso fetch");
    op_code instruccion_nombre = decode(instruccion);
    log_info(loggs, "paso decode");
    execute(instruccion_nombre, instruccion);
    log_info(log_cpu, "PID -> %i PC -> %i -> Registros -> AX:%i , BX:%i, CX:%i, DX:%i, EAX:%i, EBX:%i, ECX:%i, EDX:%i", 
    contexto->pid, 
    contexto->pc,
    contexto->registros->AX,
    contexto->registros->BX,
    contexto->registros->CX,
    contexto->registros->DX,
    contexto->registros->EAX,
    contexto->registros->EBX,
    contexto->registros->ECX,
    contexto->registros->EDX);
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
    int codigo = recibir_operacion(conexion_memoria);
    instruccion = recibir_instruccion(conexion_memoria);
  return instruccion;
}

t_instruccion* pedir_instruccion_memoria(uint32_t pid, uint32_t pc, t_log *logg){
    t_paquete* paquete = crear_paquete_op(PEDIR_INSTRUCCION_MEMORIA);
    agregar_entero_a_paquete(paquete,pid);
    agregar_entero_a_paquete(paquete,pc);
    
    log_info(logg, "serializacion %i %i", pid, pc);
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
            printf("Instrucción desconocida\n");
            break;
    }

    
}

void checkInturrupt(uint32_t pid){
    if (hay_interrupcion){
        hay_interrupcion = 0;
        if(contexto->pid = pid_interrupt){
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
        return;
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
        return;
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
    
    uint32_t tamanio = instruccion->parametros2;

    t_paquete *paquete = crear_paquete_op(COPY_STRING);
    agregar_entero_a_paquete(paquete, tamanio);
    agregar_contexto_a_paquete(paquete, contexto);
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete); 
}

void funcResize(t_instruccion* instruccion){
    uint32_t tamanio = instruccion->parametros2;

    t_paquete *paquete = crear_paquete_op(RESIZE);
    agregar_entero_a_paquete(paquete, tamanio);
    agregar_contexto_a_paquete(paquete, contexto);
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);
    log_info(log_cpu, "Tamanio enviado");

    while(1) {
        int cod_op = recibir_operacion(conexion_memoria);
        switch(cod_op) {
            case RESIZE_OK:
            log_trace(log_cpu, "Codigo de operacion recibido en cpu : %d", cod_op);
            log_info(log_cpu, "Se ajusta tamanio de proceso");
            break;
            case OUT_OF_MEMORY:
            log_trace(log_cpu, "Codigo de operacion recibido en cpu : %d", cod_op);
            enviar_contexto(socket_cliente_kernel_interrupt, contexto, cod_op);
            break;
            default:
            log_warning(log_cpu, "Llego un codigo de operacion desconocido, %d", cod_op);
            break;
        }
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

    uint32_t numero_pagina = floor(dirLogica / tamanio_pagina);
    uint32_t desplazamiento = dirLogica - numero_pagina * tamanio_pagina;
    uint32_t dirFisica;
    
    //codigo para buscar en tlb
     for(int i=0; i < cantidad_entradas_tlb; i++){
        if(contexto->pid == entrada_tlb[i].pid && numero_pagina == entrada_tlb[i].numero_de_pagina){ //TLB hit
        log_info(log_cpu, "TLB Hit: “PID: %i - TLB HIT - Pagina: %i", contexto->pid, numero_pagina);
        dirFisica = entrada_tlb[i].marco * tamanio_pagina + desplazamiento;

        return dirFisica;
        }
    }


    //TLB miss
    log_info(log_cpu, "TLB Hit: “PID: %i - TLB MISS - Pagina: %i", contexto->pid, numero_pagina);
    t_2_enteros *algo;
    algo->entero1 = numero_pagina;
    algo->entero2 = contexto->pid;
    enviar_2_enteros(conexion_memoria, algo, ACCESO_TABLA_PAGINAS);
    uint32_t marco;

    while(1) {
        int cod_op = recibir_operacion(conexion_memoria);
        switch (cod_op)
        {
        case 0:
            log_error(log_cpu, "Llego codigo operacion 0");
            break;
        case ACCESO_TABLA_PAGINAS_OK:
            log_info(log_cpu, "Codigo de operacion recibido en cpu : %d", cod_op);
            marco = recibir_entero_uint32(conexion_memoria, log_cpu);
            log_info(log_cpu, "Recibo marco :%i", marco);
            break;
        default:
            log_warning(log_cpu, "Llego un codigo de operacion desconocido, %d", cod_op);
            break;
        }
    }
    log_info(log_cpu, "PID: %i - OBTENER MARCO - Página: %i - Marco: %i", contexto->pid, numero_pagina, marco);
    
    //preguntarle a nico
    //if(numero_marco == -1) {//Fallo de pagina (page fault)
        //buscar en disco, no se como
        //manenjar ese page fault
        //actualizar: tabla de paginas y memoriaP (la tlb la actualizamos abajo)
    //}
    
    //actulalizar TLB
    agregar_entrada_tlb(contexto->pid, marco, numero_pagina);
    

    return dirFisica;
}

void agregar_entrada_tlb(uint32_t pid, uint32_t marco, uint32_t pagina){ //actualizar tlb

//PREGUNTAR NICO 
// 
// 
// 
//     
    bool  espacio_libre = false; //flag
    // todavia tengo espacios libres
    for(int i=pid ;i < cantidad_entradas_tlb; i++){
        if(entrada_tlb[i].pid == NULL){
            entrada_tlb[i].pid = pid;
            entrada_tlb[i].numero_de_pagina = pagina;
            entrada_tlb[i].marco = marco;
            espacio_libre = true;
            break;
        }
    }
    if(!espacio_libre) {
        if(strcmp(algoritmo_tlb, "FIFO") == 0) {
            reemplazarXFIFO(pid, marco, pagina);
        } else if(strcmp(algoritmo_tlb, "LRU") == 0) {
            reemplazarXLRU(pid, marco, pagina);
        }
    }
}

void reemplazarXFIFO(uint32_t pid, uint32_t marco, uint32_t pagina){
    int cantidad_entradas_tlb_valor = cantidad_entradas_tlb;  // Obtener el valor entero desde el puntero
    
    indice_frente = 0; // Obtener índice a reemplazar

    entrada_tlb[indice_frente].pid = contexto->pid;
    entrada_tlb[indice_frente].numero_de_pagina = pagina;
    entrada_tlb[indice_frente].marco = marco;

    indice_frente = (indice_frente + 1) % cantidad_entradas_tlb_valor;
}


void reemplazarXLRU(uint32_t pid, uint32_t marco, uint32_t pagina){
    
    int indice_menos_reciente = 0;
    uint32_t menos_reciente = entrada_tlb[0].contador_reciente;

    for(int i = 1; i < cantidad_entradas_tlb; i++){
        if (entrada_tlb[i].contador_reciente < menos_reciente){
            menos_reciente = entrada_tlb[i].contador_reciente;
            indice_menos_reciente = i;
        }
    }

    //reemplazar la entrada menos reciente usada con la nueva entrada de la tlb
    entrada_tlb[indice_menos_reciente].pid = contexto->pid;
    entrada_tlb[indice_menos_reciente].numero_de_pagina = pagina;
    entrada_tlb[indice_menos_reciente].marco = marco;

    //reiniciar el cont reciente de la entrada reemplazada, porque se acabo de usar
    entrada_tlb[indice_menos_reciente].contador_reciente = 0;

    for(int i = 0; i < cantidad_entradas_tlb ; i++){
        if(i != indice_menos_reciente){
            entrada_tlb[i].contador_reciente++;
        }
    }

}

uint32_t tamanio_registro(char *registro){
    if (strcmp(registro, "AX") == 0) return 4;
    else if (strcmp(registro, "BX") == 0) return 4;
    else if (strcmp(registro, "CX") == 0) return 4;
    else if (strcmp(registro, "DX") == 0) return 4;
    else if (strcmp(registro, "EAX") == 0) return 8;
    else if (strcmp(registro, "EBX") == 0) return 8;
    else if (strcmp(registro, "ECX") == 0) return 8;
    else if (strcmp(registro, "EDX") == 0) return 8;
}

char *leer_valor_de_memoria(uint32_t direccionFisica, uint32_t tamanio) {

    t_paquete *paquete = crear_paquete_op(MOV_IN);
    agregar_string_a_paquete(paquete, direccionFisica);
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

            log_info(log_cpu, "PID: %d - Acción: LEER - Dirección física: %d - Valor: %d",
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
        strncpy(contexto->registros->AX , val, 1);
    }
    else if (strcmp(reg, "BX") == 0) {
        strncpy(contexto->registros->BX, val, 1);
    }
    else if (strcmp(reg, "CX") == 0) {
        strncpy(contexto->registros->CX, val, 1);
    }
    else if (strcmp(reg, "DX") == 0) {
        strncpy(contexto->registros->DX, val, 1);
    }
    else if (strcmp(reg, "EAX") == 0) {
        strncpy(contexto->registros->EAX, val, 4);
    }
    else if (strcmp(reg, "EBX") == 0) {
        strncpy(contexto->registros->EBX, val, 4);
    }
    else if (strcmp(reg, "ECX") == 0) {
        strncpy(contexto->registros->ECX, val, 4);
    }
    else if (strcmp(reg, "EDX") == 0) {
        strncpy(contexto->registros->EDX, val, 4);
    }
    
}

void funcMovIn(t_instruccion *instruccion) { 
    log_info(log_cpu, "Instruccion MOV_IN ejecutada");
    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s - %s", contexto->pid, instruccion->parametros1, instruccion->parametros2, instruccion->parametros3);

    char *registroDatos = instruccion->parametros2;
    char *registroDireccionLogica = instruccion->parametros3;

    //ahora tendria que traducir la direccion logica a fisica
    int tamanio_regDatos = tamanio_registro(registroDatos);
    uint32_t direccionFisica = traducirDireccion(registroDireccionLogica, tamanio_regDatos);

    char *valor = leer_valor_de_memoria(direccionFisica, tamanio_regDatos);

    //guardo el valor en registro datos
    guardar_valor_en_registro(valor, registroDatos);
    log_info(log_cpu, "PID: %d - Acción: ESCRIBIR - Dirección Fisica: %d", contexto->pid, direccionFisica);
    log_info(log_cpu, "Valor guardado en registro");
}

char *leer_valor_de_registro(char *registro) {
    if (strcmp(registro, "AX") == 0) return contexto->registros->AX;
    else if (strcmp(registro, "BX") == 0) return contexto->registros->BX;
    else if (strcmp(registro, "CX") == 0) return contexto->registros->CX;
    else if (strcmp(registro, "DX") == 0) return contexto->registros->DX;
    else if (strcmp(registro, "EAX") == 0) return contexto->registros->EAX;
    else if (strcmp(registro, "EBX") == 0) return contexto->registros->EBX;
    else if (strcmp(registro, "ECX") == 0) return contexto->registros->ECX;
    else if (strcmp(registro, "EDX") == 0) return contexto->registros->EDX;
    return NULL;
}

void escribir_valor_en_memoria(uint32_t direccionFisica, char *valor, int tamanio) {
    t_paquete *paquete = crear_paquete_op(MOV_OUT);
    agregar_entero_a_paquete(paquete, direccionFisica);
    agregar_entero_a_paquete(paquete, contexto->pid);
    agregar_entero_a_paquete(paquete, tamanio);
    agregar_string_a_paquete(paquete, valor);

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
            log_info(log_cpu, "PID: %d - Acción: ESCRIBIR - Dirección física: %d - Valor: %d",
                        contexto->pid, direccionFisica, valor);
            return;
        default:
            log_warning(log_cpu, "Llegó un código de operación desconocido, %d", cod_op);
            break;
        }
    }

}

void funcMovOut(t_instruccion* instruccion){
    log_info(log_cpu, "Instruccion MOV_OUT ejecutada");
    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s - %s", contexto->pid, instruccion->parametros1, instruccion->parametros2, instruccion->parametros3);

    char *registroDireccionLogica = instruccion->parametros2;
    char *registroDatos = instruccion->parametros3;

    // Traduzco la direc
    int tamanio_regDatos = tamanio_registro(registroDatos);
    uint32_t direccionFisica = traducirDireccion(registroDireccionLogica, tamanio_regDatos);

    // Leer el valor del registro de datos
    char *valor = leer_valor_de_registro(registroDatos);

    // Escribir el valor en memoria
    escribir_valor_en_memoria(direccionFisica, valor, tamanio_regDatos);

    log_info(log_cpu, "PID: %d - Acción: ESCRIBIR - Dirección Fisica: %d", contexto->pid, direccionFisica);
    log_info(log_cpu, "Valor escrito en memoria");
}
