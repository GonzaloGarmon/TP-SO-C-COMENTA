#include <cpu.h>

int main(int argc, char* argv[]) {
    
    log_cpu = log_create("./cpu.log", "CPU", 1, LOG_LEVEL_TRACE);

    log_info(log_cpu, "INICIA EL MODULO DE CPU");

    leer_config();

    generar_conexiones();

    socket_servidor_cpu_dispatch = iniciar_servidor(puerto_escucha_dispatch, log_cpu);

    log_info(log_cpu, "INICIO SERVIDOR DISPATCH");

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
    pthread_join(atiende_cliente_kernel_interrupt, NULL);

    log_info(log_cpu, "Finalizo conexion con cliente");
    terminar_programa();
    

    return 0;
}

void leer_config(){
    config_cpu = iniciar_config("/home/utnso/tp-2024-1c-GoC/cpu/config/cpu.config");

    ip_memoria = config_get_string_value(config_cpu, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_cpu, "PUERTO_MEMORIA");
    puerto_escucha_dispatch = config_get_string_value(config_cpu, "PUERTO_ESCUCHA_DISPATCH");
    puerto_escucha_interrupt = config_get_string_value(config_cpu, "PUERTO_ESCUCHA_INTERRUPT");
    cantidad_entradas_tlb = config_get_int_value(config_cpu, "CANTIDAD_ENTRADAS_TLB");
    algoritmo_tlb = config_get_string_value(config_cpu, "ALGORITMO_TLB");

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
        op_code codigo = recibir_operacion(SOCKET_CLIENTE_KERNEL_DISPATCH);
        switch (codigo)
        {
        case EXEC:
            log_trace(log_cpu, "llego contexto de ejecucion");
            contexto = recibir_pcb(SOCKET_CLIENTE_KERNEL_DISPATCH);
            ejecutar_ciclo_de_instruccion();
            log_trace(log_cpu, "ejecute correctamente un ciclo de instruccion");
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
        int op_code = recibir_operacion(SOCKET_CLIENTE_KERNEL_INTERRUPT);
    }
}

void establecer_conexion(char * ip_memoria, char* puerto_memoria, t_config* config, t_log* loggs){


    log_trace(loggs, "Inicio como cliente");

    log_trace(loggs,"Lei la IP %s , el Puerto Memoria %s ", ip_memoria, puerto_memoria);

    // Enviamos al servidor el valor de ip como mensaje si es que levanta el cliente
    if((conexion_memoria = crear_conexion(ip_memoria, puerto_memoria)) == -1){
        log_trace(loggs, "Error al conectar con Memoria. El servidor no esta activo");

        exit(2);
    }
    
    //log_trace(loggs, "Todavía no recibí Op");
    recibir_operacion(conexion_memoria);
    //log_trace(loggs, "Recibí Op");
    recibir_string(conexion_memoria, loggs);
    
}

void ejecutar_ciclo_de_instruccion(){
    seguir_ejecutando = 1;
    while(seguir_ejecutando){
    t_instruccion* instruccion = fetch(contexto->pid, contexto->pc);
    op_code instruccion_nombre = decode(instruccion);
    contexto->pc++;
    execute(instruccion_nombre, instruccion);
    }

 }

//pedir a la memoria la proxima instruccion a ejecutar

t_instruccion* fetch(uint32_t pid, uint32_t pc){
    pedir_instruccion_memoria(pid, pc);
    t_instruccion* instruccion = malloc(sizeof(t_instruccion));
    int codigo = recibir_operacion(conexion_memoria);
    log_trace(log_cpu, "pedi instruccion");
    instruccion = recibir_instruccion(conexion_memoria);
    log_trace(log_cpu, "recibi instruccion");
  return instruccion;
}

t_instruccion* pedir_instruccion_memoria(uint32_t pid, uint32_t pc){
    t_paquete* paquete = crear_paquete_op(PEDIR_INSTRUCCION_MEMORIA);
    agregar_entero_a_paquete(paquete,pid);
    agregar_entero_a_paquete(paquete,pc);
    
    enviar_paquete(paquete,conexion_memoria);
    eliminar_paquete(paquete);

}

void execute(op_code instruccion_nombre, t_instruccion* instruccion) {
    switch (instruccion_nombre) {
        case SET:
            funcSet(instruccion);
            break;
        case SUM:
            funcSum(instruccion);
            break;
        case SUB:
            funcSub(instruccion);
            break;
        case JNZ:
            funcJnz(instruccion);
            break;
        case IO_GEN_SLEEP:
            funcIoGenSleep(instruccion);
            break;
        case EXIT:
            funcExit(instruccion);
        /*case MOV_IN:
            funcMovIn(instruccion);
            break;
        case MOV_OUT:
            funcMovOut(instruccion);
            break;
        case RESIZE:
            funcResize(instruccion);
            break;
        case COPY_STRING:
            funcCopyString(instruccion);
            break;
        */
        case IO_STDIN_READ:
            funcIoStdinRead(instruccion);
            break;
        case IO_STDOUT_WRITE:
            funcIoStdOutWrite(instruccion);
        case IO_FS_CREATE:
            funcIoFsCreate(instruccion);
            break;
        case IO_FS_DELETE:
            funcIoFsDelete(instruccion);
            break;
        case IO_FS_TRUNCATE:
            funcIoFsTruncate(instruccion);
            break;
        case IO_FS_WRITE:
            funcIoFsWrite(instruccion);
            break;
        case IO_FS_READ:
            funcIoFsRead(instruccion);
            break;
        default:
            printf("Instrucción desconocida\n");
            break;
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
    // Agregar otras instrucciones según sea necesario
    return -1; // Código de operación no válido
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
    agregar_string_a_paquete(paquete, instruccion->parametros2);
    agregar_entero_a_paquete(paquete, atoi(instruccion->parametros3));
    enviar_paquete(paquete,socket_cliente_kernel_dispatch);
};

void funcExit(t_instruccion *instruccion) {
    seguir_ejecutando = 0;
    t_paquete *paquete = crear_paquete_op(TERMINO_PROCESO);
    agregar_pcb_a_paquete(paquete,contexto);
    enviar_paquete(paquete, socket_cliente_kernel_dispatch);
    eliminar_paquete(paquete);
}

void funcIoStdinRead(t_instruccion *instruccion) {

    t_paquete *paquete = crear_paquete_op(IO_STDIN_READ);
    agregar_string_a_paquete(paquete, instruccion->parametros2); //interfaz
    agregar_entero_a_paquete(paquete, instruccion->parametros3); //regDir
    agregar_entero_a_paquete(paquete, instruccion->parametros4); //regtam
    enviar_paquete(paquete, socket_cliente_kernel_dispatch);
    eliminar_paquete(paquete);
}

void funcIoStdOutWrite(t_instruccion *instruccion) {

    t_paquete *paquete = crear_paquete_op(IO_STDOUT_WRITE);
    agregar_entero_a_paquete(paquete, atoi(instruccion->parametros2)); // Interfaz
    agregar_entero_a_paquete(paquete, atoi(instruccion->parametros3)); // regDir
    agregar_entero_a_paquete(paquete, atoi(instruccion->parametros4)); // regTam
    enviar_paquete(paquete, socket_cliente_kernel_dispatch);
}

void funcIoFsCreate(t_instruccion *instruccion) {

    t_paquete *paquete = crear_paquete_op(IO_FS_CREATE);  
    agregar_string_a_paquete(paquete, instruccion->parametros2); // Interfaz
    agregar_string_a_paquete(paquete, instruccion->parametros3); // nomArch
    enviar_paquete(paquete, socket_cliente_kernel_dispatch);
    eliminar_paquete(paquete);
}

void funcIoFsDelete(t_instruccion *instruccion) {

    t_paquete *paquete = crear_paquete_op(IO_FS_DELETE);
    agregar_string_a_paquete(paquete, instruccion->parametros2); // Interfaz
    agregar_string_a_paquete(paquete, instruccion->parametros3); // nomArch
    enviar_paquete(paquete, socket_cliente_kernel_dispatch);
    eliminar_paquete(paquete);
}

void funcIoFsTruncate(t_instruccion *instruccion) {
    
    t_paquete* paquete = crear_paquete_op(IO_FS_TRUNCATE);
    agregar_string_a_paquete(paquete, instruccion->parametros2); // Interfaz
    agregar_string_a_paquete(paquete, instruccion->parametros3); // nomArch
    agregar_entero_int_a_paquete(paquete, atoi(instruccion->parametros4)); // tam
    enviar_paquete(paquete, socket_cliente_kernel_dispatch);
    eliminar_paquete(paquete);
}

void funcIoFsWrite(t_instruccion* instruccion) {
    t_paquete* paquete = crear_paquete_op(IO_FS_WRITE);
    agregar_string_a_paquete(paquete, instruccion->parametros2); // Interfaz
    agregar_string_a_paquete(paquete, instruccion->parametros3); // nomArch
    agregar_entero_a_paquete(paquete, atoi(instruccion->parametros4)); // tam
    agregar_entero_a_paquete(paquete, atoi(instruccion->parametros5)); // Puntero Arch
    enviar_paquete(paquete, socket_cliente_kernel_dispatch);
    eliminar_paquete(paquete);
}

void funcIoFsRead(t_instruccion* instruccion) {
    t_paquete* paquete = crear_paquete_op(IO_FS_READ);
    agregar_string_a_paquete(paquete, instruccion->parametros2); // Interfaz
    agregar_string_a_paquete(paquete, instruccion->parametros3); // nomArch
    agregar_entero_a_paquete(paquete, atoi(instruccion->parametros4)); // tam
    agregar_entero_a_paquete(paquete, atoi(instruccion->parametros5)); // Puntero Arch
    enviar_paquete(paquete, socket_cliente_kernel_dispatch);
    eliminar_paquete(paquete);
}



