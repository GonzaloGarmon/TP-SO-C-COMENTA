#include <cpu/src/cpu.h>

int main(int argc, char* argv[]) {
    
    log_cpu = log_create("./runlogs/cpu.log", "CPU", 1, LOG_LEVEL_INFO);

    config_cpu = inciar_config(argv[1]);

    ip_memoria = config_get_string_value(config_cpu, "IP_MEMORIA");
    puerto_memoria = config_get_int_value(config_cpu, "PUERTO_MEMORIA");
    puerto_escucha_dispatch = config_get_int_value(config_cpu, "PUERTO_ESCUCHA_DISPATCH");
    puerto_escucha_interrupt = config_get_int_value(config_cpu, "PUERTO_ESCUCHA_INTERRUPT");
    cantidad_entradas_tlb = config_get_int_value(config_cpu, "CANTIDAD_ENTRADAS_TLB");
    algoritmo_tlb = config_get_string_value(config_cpu, "ALGORITMO_TLB");

    socket_servidor_cpu = inciar_servidor(puerto_escucha_dispatch, log_cpu);
















    return 0;
}



