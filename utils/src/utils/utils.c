#include"utils.h"

t_log* logger;

int iniciar_servidor(char *puerto, t_log* loggs)
{

    int socket_servidor;

    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;


    getaddrinfo(NULL, puerto, &hints, &servinfo);

    // Creamos el socket de escucha del servidor
    socket_servidor = socket(servinfo->ai_family,
                         servinfo->ai_socktype,
                         servinfo->ai_protocol);

    // Asociamos el socket a un puerto
    bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

    // Escuchamos las conexiones entrantes
    listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

    return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{
    // Aceptamos un nuevo cliente
    int socket_cliente = accept(socket_servidor,NULL,NULL);


    return socket_cliente;
}


int recibir_operacion(int socket_cliente)
{
    int cod_op;
    if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0){
        return cod_op;
    }else
    {
        close(socket_cliente);
        return -1;
    }
}

void* recibir_buffer(int* size, int socket_cliente)
{
    void * buffer;

    recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
    buffer = malloc(*size);
    recv(socket_cliente, buffer, *size, MSG_WAITALL);

    return buffer;
}

void recibir_mensaje(int socket_cliente, t_log* loggs)
{
    int size;
    char* buffer = recibir_buffer(&size, socket_cliente);
    log_trace(loggs, "Me llego el mensaje %s", buffer);
    free(buffer);
}

t_list* recibir_paquete(int socket_cliente)
{
    int size;
    int desplazamiento = 0;
    void * buffer;
    t_list* valores = list_create();
    int tamanio;

    buffer = recibir_buffer(&size, socket_cliente);
    while(desplazamiento < size)
    {
        memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
        desplazamiento+=sizeof(int);
        char* valor = malloc(tamanio);
        memcpy(valor, buffer+desplazamiento, tamanio);
        desplazamiento+=tamanio;
        list_add(valores, valor);
    }
    free(buffer);
    return valores;
}

void* serializar_paquete(t_paquete* paquete, int bytes)
{
    void * magic = malloc(bytes);
    int desplazamiento = 0;

    memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
    desplazamiento+= sizeof(int);
    memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
    desplazamiento+= sizeof(int);
    memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
    desplazamiento+= paquete->buffer->size;

    return magic;
}

int crear_conexion(char *ip, char* puerto)
{
    struct addrinfo hints;
    struct addrinfo *server_info;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &server_info);

    // Ahora vamos a crear el socket.
    int socket_cliente = socket(server_info->ai_family,server_info->ai_socktype,server_info->ai_protocol);
    
    if (socket_cliente == -1) {
        perror("Error al crear el socket");
        freeaddrinfo(server_info);
        return -1;
    }

    // Ahora que tenemos el socket, vamos a conectarlo
    if (connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1) {
        perror("Error al conectar");
        freeaddrinfo(server_info);
        close(socket_cliente); // Cerrar el socket antes de retornar
        return -1;
    }

    freeaddrinfo(server_info);

    return socket_cliente;
}

void enviar_mensaje(char* mensaje, int socket_cliente)
{
    t_paquete* paquete = malloc(sizeof(t_paquete));

    paquete->codigo_operacion = MENSAJE;
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = strlen(mensaje) + 1;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

    int bytes = paquete->buffer->size + 2*sizeof(int);

    void* a_enviar = serializar_paquete(paquete, bytes);

    send(socket_cliente, a_enviar, bytes, 0);

    free(a_enviar);
    eliminar_paquete(paquete);
}


void crear_buffer(t_paquete* paquete)
{
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = 0;
    paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(void)
{
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = PAQUETE;
    crear_buffer(paquete);
    return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, uint32_t tamanio)
{
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

    memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(uint32_t));
    memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(uint32_t), valor, tamanio);

    paquete->buffer->size += tamanio + sizeof(uint32_t);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
    int bytes = paquete->buffer->size + 2*sizeof(int);
    void* a_enviar = serializar_paquete(paquete, bytes);

    send(socket_cliente, a_enviar, bytes, 0);

    free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

void liberar_conexion(int socket_cliente)
{
    close(socket_cliente);
}

t_config* iniciar_config(char *ruta)
{
    t_config* nuevo_config = config_create(ruta);
    if(nuevo_config == NULL){
        printf("No se puede crear la config");
        exit(1);
    }

    return nuevo_config;
}

// Serializacion

void agregar_entero_a_paquete(t_paquete *paquete, uint32_t numero){

    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint32_t));
    memcpy(paquete->buffer->stream + paquete->buffer->size, &numero, sizeof(uint32_t));
    paquete->buffer->size += sizeof(uint32_t);

}



void agregar_entero_uint8_a_paquete(t_paquete *paquete, uint8_t numero){

    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint8_t));
    memcpy(paquete->buffer->stream + paquete->buffer->size, &numero, sizeof(uint8_t));
    paquete->buffer->size += sizeof(uint8_t);

}

void agregar_string_a_paquete(t_paquete *paquete, char* palabra){

    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(char*));
    memcpy(paquete->buffer->stream + paquete->buffer->size, &palabra, sizeof(char*));
    paquete->buffer->size += sizeof(char*);
    
}

void enviar_entero (int conexion, uint32_t numero, int codop){
    t_paquete* paquete = crear_paquete_op(codop);

    agregar_entero_a_paquete(paquete,numero);
    enviar_paquete(paquete,conexion);
    eliminar_paquete(paquete);
}



void enviar_string (int conexion, char* palabra, int codop){
    t_paquete* paquete = crear_paquete_op(codop);

    agregar_a_paquete(paquete,palabra,strlen(palabra)+1);
    enviar_paquete(paquete,conexion);
    eliminar_paquete(paquete);
}


void enviar_contexto (int conexion, t_contexto* pcb, int codop){
	t_paquete* paquete = crear_paquete_op(codop);

	agregar_contexto_a_paquete(paquete,pcb);
	enviar_paquete(paquete,conexion);
	eliminar_paquete(paquete);

}

void enviar_instruccion (int conexion, t_instruccion* instruccion_nueva, int codop){
    t_paquete* paquete = crear_paquete_op(codop);

    agregar_instruccion_a_paquete(paquete,instruccion_nueva);
    enviar_paquete(paquete,conexion);
    eliminar_paquete(paquete);
}

void enviar_2_enteros(int conexion, t_2_enteros* enteros, int codop){
    t_paquete* paquete = crear_paquete_op(codop);

    agregar_2_enteros_a_paquete(paquete,enteros);
    enviar_paquete(paquete,conexion);
    eliminar_paquete(paquete);
}

void enviar_3_enteros(int conexion, t_3_enteros* enteros, int codop){
    t_paquete* paquete = crear_paquete_op(codop);

    agregar_3_enteros_a_paquete(paquete,enteros);
    enviar_paquete(paquete,conexion);
    eliminar_paquete(paquete);
}

void enviar_codop(int conexion, op_code cod_op){
    t_paquete* codigo = crear_paquete_op(cod_op);
    
    enviar_paquete(codigo, conexion);

    eliminar_paquete(codigo);

}

void enviar_paquete_string(int conexion, char* string, op_code codOP, int tamanio)
{
	t_paquete * paquete = crear_paquete_op(codOP);
	agregar_a_paquete(paquete, string, tamanio);
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}

void enviar_codigo (t_paquete * codop, int socket_cliente){

    void *magic = malloc(sizeof(int));

    memcpy(magic, &(codop->codigo_operacion), sizeof(int));

    send(socket_cliente,magic,sizeof(int),0);

    free(magic); 
}

void eliminar_codigo(t_paquete* codop){
    free(codop);
}

void enviar_2_enteros_1_string(int conexion, t_string_2enteros* enteros_string, int codop){
    t_paquete* paquete = crear_paquete_op(codop);

    agregar_2_enteros_1_string_a_paquete(paquete,enteros_string);
    enviar_paquete(paquete,conexion);
    eliminar_paquete(paquete);
}

void enviar_3_enteros_1_string(int conexion, t_string_3enteros* enteros_string, int codop){
    t_paquete* paquete = crear_paquete_op(codop);

    agregar_3_enteros_1_string_a_paquete(paquete,enteros_string);
    enviar_paquete(paquete,conexion);
    eliminar_paquete(paquete);
}



t_paquete* crear_paquete_op(op_code codop)
{
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = codop;
    crear_buffer(paquete);
    return paquete;
}

void agregar_registros_a_paquete(t_paquete * paquete, t_registros_cpu * registros){

    agregar_entero_uint8_a_paquete(paquete,registros->AX);
    agregar_entero_uint8_a_paquete(paquete,registros->BX);
    agregar_entero_uint8_a_paquete(paquete,registros->CX);
    agregar_entero_uint8_a_paquete(paquete,registros->DX);
    agregar_entero_a_paquete(paquete,registros->EAX);
    agregar_entero_a_paquete(paquete,registros->EBX);
    agregar_entero_a_paquete(paquete,registros->ECX);
    agregar_entero_a_paquete(paquete,registros->EDX);
    agregar_entero_a_paquete(paquete, registros->SI);
    agregar_entero_a_paquete(paquete, registros->DI);
}

void agregar_entero_int_a_paquete(t_paquete *paquete, int numero){
    
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(int));
    memcpy(paquete->buffer->stream + paquete->buffer->size, &numero, sizeof(int));
    paquete->buffer->size += sizeof(int);
}






void agregar_contexto_a_paquete(t_paquete *paquete, t_contexto * pcb){
	agregar_entero_a_paquete(paquete, pcb->pid);
	agregar_entero_a_paquete(paquete, pcb->pc);
	agregar_registros_a_paquete(paquete, pcb->registros);
}

void agregar_2_enteros_a_paquete(t_paquete *paquete, t_2_enteros * enteros){
    agregar_entero_a_paquete(paquete, enteros->entero1);
    agregar_entero_a_paquete(paquete, enteros->entero2);

}

void agregar_3_enteros_a_paquete(t_paquete *paquete, t_3_enteros * enteros){
    agregar_entero_a_paquete(paquete, enteros->entero1);
    agregar_entero_a_paquete(paquete, enteros->entero2);
    agregar_entero_a_paquete(paquete, enteros->entero3);

}

void agregar_2_enteros_1_string_a_paquete(t_paquete *paquete, t_string_2enteros * enteros_string){
    agregar_entero_a_paquete(paquete, enteros_string->entero1);
    agregar_entero_a_paquete(paquete, enteros_string->entero2);
    agregar_string_a_paquete(paquete,enteros_string->string); 
}

void agregar_3_enteros_1_string_a_paquete(t_paquete *paquete, t_string_3enteros * enteros_string){
    agregar_entero_a_paquete(paquete, enteros_string->entero1);
    agregar_entero_a_paquete(paquete, enteros_string->entero2);
    agregar_entero_a_paquete(paquete, enteros_string->entero3);
    agregar_string_a_paquete(paquete,enteros_string->string); 
}


void agregar_instruccion_a_paquete(t_paquete *paquete, t_instruccion * instruccion_nueva){
    agregar_a_paquete(paquete,instruccion_nueva->parametros1,strlen(instruccion_nueva->parametros1)+1);

    if(strcmp(instruccion_nueva->parametros1, "SET") == 0){
        agregar_a_paquete(paquete,instruccion_nueva->parametros2,strlen(instruccion_nueva->parametros2)+1);
        agregar_a_paquete(paquete,instruccion_nueva->parametros3,strlen(instruccion_nueva->parametros3)+1);
    }if(strcmp(instruccion_nueva->parametros1, "SUM") == 0){
        agregar_a_paquete(paquete,instruccion_nueva->parametros2,strlen(instruccion_nueva->parametros2)+1);
        agregar_a_paquete(paquete,instruccion_nueva->parametros3,strlen(instruccion_nueva->parametros3)+1);
    }if(strcmp(instruccion_nueva->parametros1, "SUB") == 0){
        agregar_a_paquete(paquete,instruccion_nueva->parametros2,strlen(instruccion_nueva->parametros2)+1);
        agregar_a_paquete(paquete,instruccion_nueva->parametros3,strlen(instruccion_nueva->parametros3)+1);
    }if(strcmp(instruccion_nueva->parametros1, "MOV_IN") == 0){
        agregar_a_paquete(paquete,instruccion_nueva->parametros2,strlen(instruccion_nueva->parametros2)+1);
        agregar_a_paquete(paquete,instruccion_nueva->parametros3,strlen(instruccion_nueva->parametros3)+1);
    }if(strcmp(instruccion_nueva->parametros1, "MOV_OUT") == 0){
        agregar_a_paquete(paquete,instruccion_nueva->parametros2,strlen(instruccion_nueva->parametros2)+1);
        agregar_a_paquete(paquete,instruccion_nueva->parametros3,strlen(instruccion_nueva->parametros3)+1);
    }if(strcmp(instruccion_nueva->parametros1, "RESIZE") == 0){
        agregar_a_paquete(paquete,instruccion_nueva->parametros2,strlen(instruccion_nueva->parametros2)+1);
       
    }if(strcmp(instruccion_nueva->parametros1, "JNZ") == 0){
        agregar_a_paquete(paquete,instruccion_nueva->parametros2,strlen(instruccion_nueva->parametros2)+1);
        agregar_a_paquete(paquete,instruccion_nueva->parametros3,strlen(instruccion_nueva->parametros3)+1);
    }if(strcmp(instruccion_nueva->parametros1, "COPY_STRING") == 0){
        agregar_a_paquete(paquete,instruccion_nueva->parametros2,strlen(instruccion_nueva->parametros2)+1);
        
    }if(strcmp(instruccion_nueva->parametros1, "IO_GEN_SLEEP") == 0){
        agregar_a_paquete(paquete,instruccion_nueva->parametros2,strlen(instruccion_nueva->parametros2)+1);
        agregar_a_paquete(paquete,instruccion_nueva->parametros3,strlen(instruccion_nueva->parametros3)+1);
    }if(strcmp(instruccion_nueva->parametros1, "IO_STDIN_READ") == 0){
        agregar_a_paquete(paquete,instruccion_nueva->parametros2,strlen(instruccion_nueva->parametros2)+1);
        agregar_a_paquete(paquete,instruccion_nueva->parametros3,strlen(instruccion_nueva->parametros3)+1);
        agregar_a_paquete(paquete,instruccion_nueva->parametros4,strlen(instruccion_nueva->parametros4)+1);
    }if(strcmp(instruccion_nueva->parametros1, "IO_STDOUT_WRITE") == 0){
        agregar_a_paquete(paquete,instruccion_nueva->parametros2,strlen(instruccion_nueva->parametros2)+1);
        agregar_a_paquete(paquete,instruccion_nueva->parametros3,strlen(instruccion_nueva->parametros3)+1);
        agregar_a_paquete(paquete,instruccion_nueva->parametros4,strlen(instruccion_nueva->parametros4)+1);
    }if(strcmp(instruccion_nueva->parametros1, "IO_FS_CREATE") == 0){
        agregar_a_paquete(paquete,instruccion_nueva->parametros2,strlen(instruccion_nueva->parametros2)+1);
        agregar_a_paquete(paquete,instruccion_nueva->parametros3,strlen(instruccion_nueva->parametros3)+1);
    }if(strcmp(instruccion_nueva->parametros1, "IO_FS_DELETE") == 0){
        agregar_a_paquete(paquete,instruccion_nueva->parametros2,strlen(instruccion_nueva->parametros2)+1);
        agregar_a_paquete(paquete,instruccion_nueva->parametros3,strlen(instruccion_nueva->parametros3)+1);
    }if(strcmp(instruccion_nueva->parametros1, "IO_FS_TRUNCATE") == 0){
        agregar_a_paquete(paquete,instruccion_nueva->parametros2,strlen(instruccion_nueva->parametros2)+1);
        agregar_a_paquete(paquete,instruccion_nueva->parametros3,strlen(instruccion_nueva->parametros3)+1);
        agregar_a_paquete(paquete,instruccion_nueva->parametros4,strlen(instruccion_nueva->parametros4)+1);
    }if(strcmp(instruccion_nueva->parametros1, "IO_FS_WRITE") == 0){
        agregar_a_paquete(paquete,instruccion_nueva->parametros2,strlen(instruccion_nueva->parametros2)+1);
        agregar_a_paquete(paquete,instruccion_nueva->parametros3,strlen(instruccion_nueva->parametros3)+1);
        agregar_a_paquete(paquete,instruccion_nueva->parametros4,strlen(instruccion_nueva->parametros4)+1);
        agregar_a_paquete(paquete,instruccion_nueva->parametros5,strlen(instruccion_nueva->parametros5)+1);
        agregar_a_paquete(paquete,instruccion_nueva->parametros6,strlen(instruccion_nueva->parametros6)+1);
    }if(strcmp(instruccion_nueva->parametros1, "IO_FS_READ") == 0){
        agregar_a_paquete(paquete,instruccion_nueva->parametros2,strlen(instruccion_nueva->parametros2)+1);
        agregar_a_paquete(paquete,instruccion_nueva->parametros3,strlen(instruccion_nueva->parametros3)+1);
        agregar_a_paquete(paquete,instruccion_nueva->parametros4,strlen(instruccion_nueva->parametros4)+1);
        agregar_a_paquete(paquete,instruccion_nueva->parametros5,strlen(instruccion_nueva->parametros5)+1);
        agregar_a_paquete(paquete,instruccion_nueva->parametros6,strlen(instruccion_nueva->parametros6)+1);
    }if(strcmp(instruccion_nueva->parametros1, "WAIT") == 0){
        agregar_a_paquete(paquete,instruccion_nueva->parametros2,strlen(instruccion_nueva->parametros2)+1);
        
    }if(strcmp(instruccion_nueva->parametros1, "SIGNAL") == 0){
        agregar_a_paquete(paquete,instruccion_nueva->parametros2,strlen(instruccion_nueva->parametros2)+1);
        
    }if(strcmp(instruccion_nueva->parametros1, "EXIT") == 0){
    }

}


// Una vez serializado -> recibimos y leemos estas variables


int leer_entero(char *buffer, int * desplazamiento)
{
    int entero;
    memcpy(&entero, buffer + (*desplazamiento), sizeof(int));
    (*desplazamiento) += sizeof(int);
    return entero;
}

uint32_t leer_entero_uint32(char *buffer, int * desplazamiento)
{
    uint32_t entero;
    memcpy(&entero, buffer + (*desplazamiento), sizeof(uint32_t));
    (*desplazamiento) += sizeof(uint32_t);
    return entero;
}

uint8_t leer_entero_uint8(char *buffer, int * desplazamiento)
{
    uint8_t entero;
    memcpy(&entero, buffer + (*desplazamiento), sizeof(uint8_t));
    (*desplazamiento) += sizeof(uint8_t);
    return entero;
}

char* leer_string(char* buffer, int* desplazamiento) {
    int tamanio = leer_entero(buffer, desplazamiento);

    char* palabra = malloc(tamanio + 1);

    memcpy(palabra, buffer + *desplazamiento, tamanio);

    palabra[tamanio] = '\0';

    *desplazamiento += tamanio;

    return palabra;
}

// t_registros_cpu * leer_registros(char* buffer, int* desp){

//  int tamanio = leer_entero(buffer,desp);
//  t_registros_cpu * retorno = malloc(tamanio);
//  leer_entero_uint8(buffer,desp);
//  memcpy(retorno->AX, buffer + (*desp), 8);
//  (*desp) += 8;
//  leer_entero_uint8(buffer,desp);
//  memcpy(retorno->BX, buffer + (*desp), 8);
//  (*desp) += 8;
//  leer_entero_uint8(buffer,desp);
//  memcpy(retorno->CX, buffer + (*desp), 8);
//  (*desp) += 8;
//  leer_entero_uint8(buffer,desp);
//  memcpy(retorno->DX, buffer + (*desp), 8);
//  (*desp) += 8;
//  leer_entero_uint32(buffer,desp);
//  memcpy(retorno->EAX, buffer + (*desp), 8);
//  (*desp) += 32;
//  leer_entero_uint32(buffer,desp);
//  memcpy(retorno->EBX, buffer + (*desp), 8);
//  (*desp) += 32;
//  leer_entero_uint32(buffer,desp);
//  memcpy(retorno->ECX, buffer + (*desp), 8);
//  (*desp) += 32;
//  leer_entero_uint32(buffer,desp);
//  memcpy(retorno->EDX, buffer + (*desp), 8);
//  (*desp) += 32;


// }

uint32_t recibir_entero_uint32(int socket, t_log* loggs){

    int size = 0;
    char* buffer;
    int desp = 0;


	buffer = recibir_buffer(&size, socket);
	u_int32_t entero_nuevo32 = leer_entero_uint32(buffer, &desp);
	//log_trace(loggs, "Me llego en numero %i", entero_nuevo32);
	free(buffer);
	return entero_nuevo32;

}

char* recibir_string(int socket,t_log* loggs){
    
    int size = 0;
    char* buffer;
    int desp = 0;
        
    buffer = recibir_buffer(&size, socket);
    char* nuevo_string = leer_string(buffer, &desp);
    log_trace(loggs, "Recibi el mensaje %s", nuevo_string);

    free(buffer);
    return nuevo_string;
}



t_contexto* recibir_contexto(int socket){

	t_contexto* nuevo_contexto = malloc(sizeof(t_contexto));
	//t_registros_cpu* registros = malloc(sizeof(t_registros_cpu));
	nuevo_contexto->registros = malloc(sizeof(t_registros_cpu));
	int size = 0;
	char* buffer;
	int desp = 0;
		
	buffer = recibir_buffer(&size, socket);

	nuevo_contexto->pid = leer_entero_uint32(buffer,&desp);
	nuevo_contexto->pc = leer_entero_uint32(buffer,&desp);
	nuevo_contexto->registros->AX = leer_entero_uint8(buffer,&desp);
	nuevo_contexto->registros->BX = leer_entero_uint8(buffer,&desp);
	nuevo_contexto->registros->CX = leer_entero_uint8(buffer,&desp);
	nuevo_contexto->registros->DX = leer_entero_uint8(buffer,&desp);
	nuevo_contexto->registros->EAX = leer_entero_uint32(buffer,&desp);
	nuevo_contexto->registros->EBX = leer_entero_uint32(buffer,&desp);
	nuevo_contexto->registros->ECX = leer_entero_uint32(buffer,&desp);
	nuevo_contexto->registros->EDX = leer_entero_uint32(buffer,&desp);
    nuevo_contexto->registros->SI = leer_entero_uint32(buffer,&desp);
    nuevo_contexto->registros->DI = leer_entero_uint32(buffer,&desp);

	free(buffer);
	return nuevo_contexto;
}

t_list* recibir_doble_entero(int socket){
	int size = 0;
	char* buffer;
	int desp = 0;
	t_list* devolver = list_create();
	buffer = recibir_buffer(&size, socket);
	u_int32_t entero_nuevo1 = leer_entero_uint32(buffer, &desp);
	u_int32_t entero_nuevo2 = leer_entero_uint32(buffer, &desp);
	list_add(devolver, entero_nuevo1);
	list_add(devolver, entero_nuevo2);
	free(buffer);

	return devolver;
}

void recibir_string_mas_contexto(int conexion_kernel_cpu_dispatch,t_contexto** pcb_wait,char** recurso_wait){
    *pcb_wait = malloc(sizeof(t_contexto));
    t_registros_cpu* registros = malloc(sizeof(t_registros_cpu));
    (*pcb_wait)->registros = registros;
    int size = 0;
	char* buffer;
	int desp = 0;
    	
	buffer = recibir_buffer(&size,conexion_kernel_cpu_dispatch);
	*recurso_wait = leer_string(buffer, &desp);

	(*pcb_wait)->pid = leer_entero_uint32(buffer,&desp);
	(*pcb_wait)->pc = leer_entero_uint32(buffer,&desp);
	(*pcb_wait)->registros->AX = leer_entero_uint8(buffer,&desp);
	(*pcb_wait)->registros->BX = leer_entero_uint8(buffer,&desp);
	(*pcb_wait)->registros->CX = leer_entero_uint8(buffer,&desp);
	(*pcb_wait)->registros->DX = leer_entero_uint8(buffer,&desp);
	(*pcb_wait)->registros->EAX = leer_entero_uint32(buffer,&desp);
	(*pcb_wait)->registros->EBX = leer_entero_uint32(buffer,&desp);
	(*pcb_wait)->registros->ECX = leer_entero_uint32(buffer,&desp);
	(*pcb_wait)->registros->EDX = leer_entero_uint32(buffer,&desp);

	free(buffer);
  }

  void recibir_string_mas_u32_con_contexto(int conexion_kernel_cpu_dispatch,char** palabra,uint32_t* numero, t_contexto** contexto){
    int size = 0;
	char* buffer;
	int desp = 0;
    *numero = malloc(sizeof(uint32_t));	
    *contexto = malloc(sizeof(t_contexto));
    t_registros_cpu* registros = malloc(sizeof(t_registros_cpu));
    (*contexto)->registros = registros;
	buffer = recibir_buffer(&size,conexion_kernel_cpu_dispatch);
	*palabra = leer_string(buffer, &desp);
    *numero = leer_entero_uint32(buffer,&desp);
    (*contexto)->pid = leer_entero_uint32(buffer,&desp);
	(*contexto)->pc = leer_entero_uint32(buffer,&desp);
	(*contexto)->registros->AX = leer_entero_uint8(buffer,&desp);
	(*contexto)->registros->BX = leer_entero_uint8(buffer,&desp);
	(*contexto)->registros->CX = leer_entero_uint8(buffer,&desp);
	(*contexto)->registros->DX = leer_entero_uint8(buffer,&desp);
	(*contexto)->registros->EAX = leer_entero_uint32(buffer,&desp);
	(*contexto)->registros->EBX = leer_entero_uint32(buffer,&desp);
	(*contexto)->registros->ECX = leer_entero_uint32(buffer,&desp);
	(*contexto)->registros->EDX = leer_entero_uint32(buffer,&desp);
    (*contexto)->registros->SI = leer_entero_uint32(buffer,&desp);
    (*contexto)->registros->DI = leer_entero_uint32(buffer,&desp);
    free(buffer);
  }

  void recibir_3_string(int conexion_kernel_cpu_dispatch, char** palabra1,char** palabra2, char** palabra3){
    int size = 0;
	char* buffer;
	int desp = 0;
    buffer = recibir_buffer(&size,conexion_kernel_cpu_dispatch);
    *palabra1 = leer_string(buffer, &desp);
    *palabra2 = leer_string(buffer, &desp);
    *palabra3 = leer_string(buffer, &desp);

  }

void recibir_2_string_con_contexto(int conexion_kernel_cpu_dispatch, char** palabra1,char** palabra2, t_contexto** contexto){
    int size = 0;
	char* buffer;
	int desp = 0;
    buffer = recibir_buffer(&size,conexion_kernel_cpu_dispatch);
    *palabra1 = leer_string(buffer, &desp);
    *palabra2 = leer_string(buffer, &desp);

    *contexto = malloc(sizeof(t_contexto));
    t_registros_cpu* registros = malloc(sizeof(t_registros_cpu));
    (*contexto)->registros = registros;
    (*contexto)->pid = leer_entero_uint32(buffer,&desp);
	(*contexto)->pc = leer_entero_uint32(buffer,&desp);
	(*contexto)->registros->AX = leer_entero_uint8(buffer,&desp);
	(*contexto)->registros->BX = leer_entero_uint8(buffer,&desp);
	(*contexto)->registros->CX = leer_entero_uint8(buffer,&desp);
	(*contexto)->registros->DX = leer_entero_uint8(buffer,&desp);
	(*contexto)->registros->EAX = leer_entero_uint32(buffer,&desp);
	(*contexto)->registros->EBX = leer_entero_uint32(buffer,&desp);
	(*contexto)->registros->ECX = leer_entero_uint32(buffer,&desp);
	(*contexto)->registros->EDX = leer_entero_uint32(buffer,&desp);
  }

t_instruccion* recibir_instruccion(int socket){
    
    t_instruccion* instruccion_nueva = malloc(sizeof(t_instruccion));
    int size = 0;
    char* buffer;
    int desp = 0;
        
    buffer = recibir_buffer(&size, socket);

    instruccion_nueva->parametros1  = leer_string(buffer, &desp);

    if(strcmp(instruccion_nueva->parametros1, "SET") == 0){
            instruccion_nueva->parametros2  = leer_string(buffer, &desp);
            instruccion_nueva->parametros3  = leer_string(buffer, &desp);

    }if(strcmp(instruccion_nueva->parametros1, "SUM") == 0){
            instruccion_nueva->parametros2  = leer_string(buffer, &desp);
            instruccion_nueva->parametros3  = leer_string(buffer, &desp);

    }if(strcmp(instruccion_nueva->parametros1, "SUB") == 0){
            instruccion_nueva->parametros2  = leer_string(buffer, &desp);
            instruccion_nueva->parametros3  = leer_string(buffer, &desp);

    }if(strcmp(instruccion_nueva->parametros1, "MOV_IN") == 0){
            instruccion_nueva->parametros2  = leer_string(buffer, &desp);
            instruccion_nueva->parametros3  = leer_string(buffer, &desp);
 
    }if(strcmp(instruccion_nueva->parametros1, "MOV_OUT") == 0){
            instruccion_nueva->parametros2  = leer_string(buffer, &desp);
            instruccion_nueva->parametros3  = leer_string(buffer, &desp);

    }if(strcmp(instruccion_nueva->parametros1, "RESIZE") == 0){
        instruccion_nueva->parametros2  = leer_string(buffer, &desp);
       
    }if(strcmp(instruccion_nueva->parametros1, "JNZ") == 0){
            instruccion_nueva->parametros2  = leer_string(buffer, &desp);
            instruccion_nueva->parametros3  = leer_string(buffer, &desp);
        
    }if(strcmp(instruccion_nueva->parametros1, "COPY_STRING") == 0){
        
        instruccion_nueva->parametros2  = leer_string(buffer, &desp);

    }if(strcmp(instruccion_nueva->parametros1, "IO_GEN_SLEEP") == 0){
            instruccion_nueva->parametros2  = leer_string(buffer, &desp);
            instruccion_nueva->parametros3  = leer_string(buffer, &desp);
        
    }if(strcmp(instruccion_nueva->parametros1, "IO_STDIN_READ") == 0){
            instruccion_nueva->parametros2  = leer_string(buffer, &desp);
    instruccion_nueva->parametros3  = leer_string(buffer, &desp);
    instruccion_nueva->parametros4  = leer_string(buffer, &desp);
        
    }if(strcmp(instruccion_nueva->parametros1, "IO_STDOUT_WRITE") == 0){
            instruccion_nueva->parametros2  = leer_string(buffer, &desp);
            instruccion_nueva->parametros3  = leer_string(buffer, &desp);
            instruccion_nueva->parametros4  = leer_string(buffer, &desp);
        
    }if(strcmp(instruccion_nueva->parametros1, "IO_FS_CREATE") == 0){
            instruccion_nueva->parametros2  = leer_string(buffer, &desp);
            instruccion_nueva->parametros3  = leer_string(buffer, &desp);
        
    }if(strcmp(instruccion_nueva->parametros1, "IO_FS_DELETE") == 0){
            instruccion_nueva->parametros2  = leer_string(buffer, &desp);
            instruccion_nueva->parametros3  = leer_string(buffer, &desp);
        
    }if(strcmp(instruccion_nueva->parametros1, "IO_FS_TRUNCATE") == 0){
            instruccion_nueva->parametros2  = leer_string(buffer, &desp);
            instruccion_nueva->parametros3  = leer_string(buffer, &desp);
            instruccion_nueva->parametros4  = leer_string(buffer, &desp);
        
    }if(strcmp(instruccion_nueva->parametros1, "IO_FS_WRITE") == 0){
            instruccion_nueva->parametros2  = leer_string(buffer, &desp);
            instruccion_nueva->parametros3  = leer_string(buffer, &desp);
            instruccion_nueva->parametros4  = leer_string(buffer, &desp);
            instruccion_nueva->parametros5  = leer_string(buffer, &desp);
            instruccion_nueva->parametros6  = leer_string(buffer, &desp);
    }if(strcmp(instruccion_nueva->parametros1, "IO_FS_READ") == 0){
            instruccion_nueva->parametros2  = leer_string(buffer, &desp);
            instruccion_nueva->parametros3  = leer_string(buffer, &desp);
            instruccion_nueva->parametros4  = leer_string(buffer, &desp);
            instruccion_nueva->parametros5  = leer_string(buffer, &desp);
            instruccion_nueva->parametros6  = leer_string(buffer, &desp);
        
    }if(strcmp(instruccion_nueva->parametros1, "WAIT") == 0){
        instruccion_nueva->parametros2  = leer_string(buffer, &desp);
        
    }if(strcmp(instruccion_nueva->parametros1, "SIGNAL") == 0){
        instruccion_nueva->parametros2  = leer_string(buffer, &desp);
        
    }if(strcmp(instruccion_nueva->parametros1, "EXIT") == 0){
    }
    
    free(buffer);
    return instruccion_nueva;

}
/*
t_pcb* recibir_pcb(int socket){

    t_pcb* nuevo_pcb = malloc(sizeof(t_pcb));

    int size = 0;
    char* buffer;
    int desp = 0;
        
    buffer = recibir_buffer(&size, socket);

    nuevo_pcb->pid = leer_entero_uint32(buffer,&desp);
    nuevo_pcb->pc = leer_entero_uint32(buffer,&desp);
    nuevo_pcb->qq = leer_entero(buffer,&desp);
    nuevo_pcb->registros->AX = leer_entero_uint8(buffer,&desp);
    nuevo_pcb->registros->BX = leer_entero_uint8(buffer,&desp);
    nuevo_pcb->registros->CX = leer_entero_uint8(buffer,&desp);
    nuevo_pcb->registros->DX = leer_entero_uint8(buffer,&desp);
    nuevo_pcb->registros->EAX = leer_entero_uint32(buffer,&desp);
    nuevo_pcb->registros->EBX = leer_entero_uint32(buffer,&desp);
    nuevo_pcb->registros->ECX = leer_entero_uint32(buffer,&desp);
    nuevo_pcb->registros->EDX = leer_entero_uint32(buffer,&desp);

    free(buffer);
    return nuevo_pcb;
}
*/
t_2_enteros* recibir_2_enteros(int socket){
    
    t_2_enteros* nuevos_enteros = malloc(sizeof(t_2_enteros));

    int size = 0;
    char* buffer;
    int desp = 0;
        
    buffer = recibir_buffer(&size, socket);

    nuevos_enteros->entero1  = leer_entero_uint32(buffer, &desp);
    nuevos_enteros->entero2  = leer_entero_uint32(buffer, &desp);
    
    
    free(buffer);
    return nuevos_enteros;
}

t_3_enteros* recibir_3_enteros(int socket){
    
    t_3_enteros* nuevos_enteros = malloc(sizeof(t_3_enteros));

    int size = 0;
    char* buffer;
    int desp = 0;
        
    buffer = recibir_buffer(&size, socket);

    nuevos_enteros->entero1  = leer_entero_uint32(buffer, &desp);
    nuevos_enteros->entero2  = leer_entero_uint32(buffer, &desp);
    nuevos_enteros->entero3  = leer_entero_uint32(buffer, &desp);
    
    
    free(buffer);
    return nuevos_enteros;
}

t_string_3enteros* recibir_string_3_enteros(int socket){
    
    t_string_3enteros* nuevos_enteros = malloc(sizeof(t_string_3enteros));

    int size = 0;
    char* buffer;
    int desp = 0;
        
    buffer = recibir_buffer(&size, socket);

    nuevos_enteros->entero1  = leer_entero_uint32(buffer, &desp);
    nuevos_enteros->entero2  = leer_entero_uint32(buffer, &desp);
    nuevos_enteros->entero3  = leer_entero_uint32(buffer, &desp);
	nuevos_enteros->string  = leer_string(buffer, &desp);
    
    
    free(buffer);
    return nuevos_enteros;
}

t_string_2enteros* recibir_string_2enteros(int socket){
    
    t_string_2enteros* nuevos_enteros_string = malloc(sizeof(t_string_2enteros));

    int size = 0;
    char* buffer;
    int desp = 0;
        
    buffer = recibir_buffer(&size, socket);

    nuevos_enteros_string->entero1  = leer_entero_uint32(buffer, &desp);
    nuevos_enteros_string->entero2  = leer_entero_uint32(buffer, &desp);
    nuevos_enteros_string->string  = leer_string(buffer, &desp);
    
    free(buffer);
    return nuevos_enteros_string;
}


t_string_mas_entero* recibir_string_mas_entero(int socket, t_log *loggs){
    
    t_string_mas_entero* nuevos_entero_string = malloc(sizeof(t_string_mas_entero));

    int size = 0;
    char* buffer;
    int desp = 0;
        
    buffer = recibir_buffer(&size, socket);


    nuevos_entero_string->entero1  = leer_entero_uint32(buffer, &desp);
    nuevos_entero_string->string  = leer_string(buffer, &desp);
    
    free(buffer);
    return nuevos_entero_string;
}



void recibir_2_string_mas_u32_con_contexto(int socket, char** palabra1,char** palabra2, uint32_t* valor1, t_contexto** contexto){

    int size = 0;
	char* buffer;
	int desp = 0;


    buffer = recibir_buffer(&size,socket);
    *palabra1 = leer_string(buffer, &desp);
    *palabra2 = leer_string(buffer, &desp);
    *valor1 = leer_entero_uint32(buffer,&desp);
    *contexto = malloc(sizeof(t_contexto));
    t_registros_cpu* registros = malloc(sizeof(t_registros_cpu));
    (*contexto)->registros = registros;

    (*contexto)->pid = leer_entero_uint32(buffer,&desp);
	(*contexto)->pc = leer_entero_uint32(buffer,&desp);
	(*contexto)->registros->AX = leer_entero_uint8(buffer,&desp);
	(*contexto)->registros->BX = leer_entero_uint8(buffer,&desp);
	(*contexto)->registros->CX = leer_entero_uint8(buffer,&desp);
	(*contexto)->registros->DX = leer_entero_uint8(buffer,&desp);
	(*contexto)->registros->EAX = leer_entero_uint32(buffer,&desp);
	(*contexto)->registros->EBX = leer_entero_uint32(buffer,&desp);
	(*contexto)->registros->ECX = leer_entero_uint32(buffer,&desp);
	(*contexto)->registros->EDX = leer_entero_uint32(buffer,&desp);
    (*contexto)->registros->SI = leer_entero_uint32(buffer,&desp);
    (*contexto)->registros->DI = leer_entero_uint32(buffer,&desp);
    free(buffer);
}

t_string_2enteros* recibir_string_2enteros_con_contexto(int socket, t_contexto** contexto){
    int size = 0;
	char* buffer;
	int desp = 0;
    t_string_2enteros* devolver = malloc(sizeof(t_string_2enteros));

    buffer = recibir_buffer(&size,socket);
    devolver->entero1 = leer_entero_uint32(buffer, &desp);
    devolver->entero2 = leer_entero_uint32(buffer, &desp);
    devolver->string = leer_string(buffer, &desp);
    *contexto = malloc(sizeof(t_contexto));
    t_registros_cpu* registros = malloc(sizeof(t_registros_cpu));
    (*contexto)->registros = registros;

    (*contexto)->pid = leer_entero_uint32(buffer,&desp);
	(*contexto)->pc = leer_entero_uint32(buffer,&desp);
	(*contexto)->registros->AX = leer_entero_uint8(buffer,&desp);
	(*contexto)->registros->BX = leer_entero_uint8(buffer,&desp);
	(*contexto)->registros->CX = leer_entero_uint8(buffer,&desp);
	(*contexto)->registros->DX = leer_entero_uint8(buffer,&desp);
	(*contexto)->registros->EAX = leer_entero_uint32(buffer,&desp);
	(*contexto)->registros->EBX = leer_entero_uint32(buffer,&desp);
	(*contexto)->registros->ECX = leer_entero_uint32(buffer,&desp);
	(*contexto)->registros->EDX = leer_entero_uint32(buffer,&desp);
    (*contexto)->registros->SI = leer_entero_uint32(buffer,&desp);
    (*contexto)->registros->DI = leer_entero_uint32(buffer,&desp);
    free(buffer);

    return devolver;
}

void recibir_2_string_mas_3_u32_con_contexto(int socket, char** palabra1,char** palabra2, uint32_t* valor1, uint32_t* valor2, uint32_t* valor3, t_contexto** contexto){
    int size = 0;
	char* buffer;
	int desp = 0;
    buffer = recibir_buffer(&size,socket);
    *palabra1 = leer_string(buffer, &desp);
    *palabra2 = leer_string(buffer, &desp);
    *valor1 = leer_entero_uint32(buffer,&desp);
    *valor2 = leer_entero_uint32(buffer,&desp);
    *valor3 = leer_entero_uint32(buffer,&desp);
    *contexto = malloc(sizeof(t_contexto));
    t_registros_cpu* registros = malloc(sizeof(t_registros_cpu));
    (*contexto)->registros = registros;

    (*contexto)->pid = leer_entero_uint32(buffer,&desp);
	(*contexto)->pc = leer_entero_uint32(buffer,&desp);
	(*contexto)->registros->AX = leer_entero_uint8(buffer,&desp);
	(*contexto)->registros->BX = leer_entero_uint8(buffer,&desp);
	(*contexto)->registros->CX = leer_entero_uint8(buffer,&desp);
	(*contexto)->registros->DX = leer_entero_uint8(buffer,&desp);
	(*contexto)->registros->EAX = leer_entero_uint32(buffer,&desp);
	(*contexto)->registros->EBX = leer_entero_uint32(buffer,&desp);
	(*contexto)->registros->ECX = leer_entero_uint32(buffer,&desp);
	(*contexto)->registros->EDX = leer_entero_uint32(buffer,&desp); 
    (*contexto)->registros->SI = leer_entero_uint32(buffer,&desp);
    (*contexto)->registros->DI = leer_entero_uint32(buffer,&desp);
    free(buffer); 
}