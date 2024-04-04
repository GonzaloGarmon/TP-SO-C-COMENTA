#include <cpu/src/cpu.h>

int main(int argc, char* argv[]) {
    
    log_cpu = log_create("./runlogs/cpu.log", "CPU", 1, LOG_LEVEL_INFO);

    config_cpu = crear_config("./config/cpu.config");

    ip_memoria = config_get_string_value(config_cpu, "IP_MEMORIA");
    puerto_memoria = config_get_int_value(config_cpu, "PUERTO_MEMORIA");
    puerto_escucha_dispatch = config_get_int_value(config_cpu, "PUERTO_ESCUCHA_DISPATCH");
    puerto_escucha_interrupt = config_get_int_value(config_cpu, "PUERTO_ESCUCHA_INTERRUPT");
    cantidad_entradas_tlb = config_get_int_value(config_cpu, "CANTIDAD_ENTRADAS_TLB");
    algoritmo_tlb = config_get_string_value(config_cpu, "ALGORITMO_TLB");

    establecer_conexion(ip_memoria,puerto_memoria);

    socket_servidor_cpu_dispatch = inciar_servidor(puerto_escucha_dispatch);

    pthread_t atiende_cliente_memoria, atiende_cliente_kernel;

    socket_cliente_kernel = esperar_cliente(socket_servidor_cpu_dispatch);
    pthread_create(&atiende_cliente_kernel, NULL, (void *)recibir_kernel, (void *) socket_cliente_kernel);
    pthread_detach(atiende_cliente_kernel);




    return 0;
}


void recibir_kernel(int SOCKET_CLIENTE_KERNEL){
    enviar_mensaje("recibido kernel", SOCKET_CLIENTE_KERNEL);
}