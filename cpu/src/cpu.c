#include <cpu.h>

int main(int argc, char* argv[]) {
    
    log_cpu = log_create("./cpu.log", "CPU", 1, LOG_LEVEL_TRACE);

    log_info(log_cpu, "INICIA EL MODULO DE CPU");

    config_cpu = iniciar_config("/home/utnso/tp-2024-1c-GoC/cpu/config/cpu.config");

    ip_memoria = config_get_string_value(config_cpu, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_cpu, "PUERTO_MEMORIA");
    puerto_escucha_dispatch = config_get_string_value(config_cpu, "PUERTO_ESCUCHA_DISPATCH");
    puerto_escucha_interrupt = config_get_string_value(config_cpu, "PUERTO_ESCUCHA_INTERRUPT");
    cantidad_entradas_tlb = config_get_int_value(config_cpu, "CANTIDAD_ENTRADAS_TLB");
    algoritmo_tlb = config_get_string_value(config_cpu, "ALGORITMO_TLB");

    log_info(log_cpu, "levanto la configuracion del cpu");

    establecer_conexion(ip_memoria,puerto_memoria, config_cpu, log_cpu);

    log_info(log_cpu, "Finalizo conexion con servidor");


    socket_servidor_cpu_dispatch = iniciar_servidor(puerto_escucha_dispatch, log_cpu);

    log_info(log_cpu, "INICIO SERVIDOR");

    log_info(log_cpu, "Listo para recibir a kernel");
    socket_cliente_kernel = esperar_cliente(socket_servidor_cpu_dispatch);

    pthread_t atiende_cliente_kernel;
    pthread_create(&atiende_cliente_kernel, NULL, (void *)recibir_kernel, (void *) (intptr_t) socket_cliente_kernel);
    pthread_detach(atiende_cliente_kernel);
    
    log_info(log_cpu, "Finalizo conexion con cliente");

    

    return 0;
}


void recibir_kernel(int SOCKET_CLIENTE_KERNEL){
    enviar_string(SOCKET_CLIENTE_KERNEL, "recibido kernel", MENSAJE);
}

void establecer_conexion(char * ip_memoria, char* puerto_memoria, t_config* config, t_log* loggs){


    log_trace(loggs, "Inicio como cliente");

    log_trace(loggs,"Lei la IP %s , el Puerto Memoria %s ", ip_memoria, puerto_memoria);

    // Enviamos al servidor el valor de ip como mensaje si es que levanta el cliente
    if((conexion_cpu = crear_conexion(ip_memoria, puerto_memoria)) == -1){
        log_trace(loggs, "Error al conectar con Memoria. El servidor no esta activo");

        exit(2);
    }
    
    recibir_operacion(conexion_cpu);
    recibir_string(conexion_cpu, loggs);
}

void ejecutar_ciclo_de_instruccion(){ //Incompleto
    // t_instruccion *instruccion = fetch(contexto->pid, contexto->pc); // de donde sacar el contexto
    // t_nombre_instruccion instruccion_nombre = decode(instruccion);
    // execute(instruccion, contexto);
 }


//pedir a la memoria la proxima instruccion a ejecutar
//t_instruccion *fetch(uint32_t pid, uint32_t pc){
//    t_instruccion *instr = pedir_instruccion_memoria(pid, pc);
//   return instr;
//}

void execute(t_instruccion *instruccion, t_pcb *contexto) {
    switch (decode(instruccion)) {
        case SET:
            funcSet(instruccion, contexto);
            break;
        case SUM:
            funcSum(instruccion, contexto);
            break;
        case SUB:
            funcSub(instruccion, contexto);
            break;
        case JNZ:
            funcJnz(instruccion, contexto);
            break;
        case IO_GEN_SLEEP:
            funcIoGenSleep(instruccion);
            break;
        default:
            printf("Instrucción desconocida\n");
            break;
    }

    contexto->pc++;
}

// }

op_code decode(t_instruccion *instruccion) {
    if (strcmp(instruccion->nombre, "SET") == 0) {
        return SET;
    } else if (strcmp(instruccion->nombre, "SUM") == 0) {
        return SUM;
    } else if (strcmp(instruccion->nombre, "SUB") == 0) {
        return SUB;
    } else if (strcmp(instruccion->nombre, "JNZ") == 0) {
        return JNZ;
    } else if (strcmp(instruccion->nombre, "IO_GEN_SLEEP") == 0) {
        return IO_GEN_SLEEP;
    }
    // Agregar otras instrucciones según sea necesario
    return -1; // Código de operación no válido
}


void funcSet(t_instruccion *instruccion, t_pcb *contexto) {
    if (strcmp(instruccion->parametros1, "AX") == 0) {
        contexto->registros->AX = atoi(instruccion->parametros2);
    } else if (strcmp(instruccion->parametros1, "BX") == 0) {
        contexto->registros->BX = atoi(instruccion->parametros2);
    } else if (strcmp(instruccion->parametros1, "CX") == 0) {
        contexto->registros->CX = atoi(instruccion->parametros2);
    } else if (strcmp(instruccion->parametros1, "DX") == 0) {
        contexto->registros->DX = atoi(instruccion->parametros2);
    } else if (strcmp(instruccion->parametros1, "EAX") == 0) {
        contexto->registros->EAX = atoi(instruccion->parametros2);
    } else if (strcmp(instruccion->parametros1, "EBX") == 0) {
        contexto->registros->EBX = atoi(instruccion->parametros2);
    } else if (strcmp(instruccion->parametros1, "ECX") == 0) {
        contexto->registros->ECX = atoi(instruccion->parametros2);
    } else if (strcmp(instruccion->parametros1, "EDX") == 0) {
        contexto->registros->EDX = atoi(instruccion->parametros2);
    } else {
        printf("Registro desconocido: %s\n", instruccion->parametros1);
    }
}

// void funSum(t_instruccion * instruccion){
//     instruccion->parametro1 = instruccion->parametro1 + instruccion->parametro2;
// }

// void funcSub(t_instruccion * instruccion){
//     instruccion->parametro1 = instruccion->parametro1 - instruccion->parametro2;
// }

void funcJnz(t_instruccion *instruccion, t_pcb *contexto) {
    uint32_t reg_value;
    if (strcmp(instruccion->parametros1, "AX") == 0) {
        reg_value = contexto->registros->AX;
    } else if (strcmp(instruccion->parametros1, "BX") == 0) {
        reg_value = contexto->registros->BX;
    } else if (strcmp(instruccion->parametros1, "CX") == 0) {
        reg_value = contexto->registros->CX;
    } else if (strcmp(instruccion->parametros1, "DX") == 0) {
        reg_value = contexto->registros->DX;
    } else if (strcmp(instruccion->parametros1, "EAX") == 0) {
        reg_value = contexto->registros->EAX;
    } else if (strcmp(instruccion->parametros1, "EBX") == 0) {
        reg_value = contexto->registros->EBX;
    } else if (strcmp(instruccion->parametros1, "ECX") == 0) {
        reg_value = contexto->registros->ECX;
    } else if (strcmp(instruccion->parametros1, "EDX") == 0) {
        reg_value = contexto->registros->EDX;
    } else {
        printf("Registro desconocido: %s\n", instruccion->parametros1);
        return;
    }

    if (reg_value != 0) {
        contexto->pc = atoi(instruccion->parametros2);
    }
}


