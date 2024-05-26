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

void enviar_pcb (int conexion, t_pcb* pcb, int codop){
	t_paquete* paquete = crear_paquete_op(codop);

	agregar_pcb_a_paquete(paquete,pcb);
	enviar_paquete(paquete,conexion);
	eliminar_paquete(paquete);
}

void enviar_instruccion (int conexion, t_instruccion* nueva_instruccion, int codop){
	t_paquete* paquete = crear_paquete_op(codop);

	agregar_instruccion_a_paquete(paquete,nueva_instruccion);
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

}

void agregar_entero_int_a_paquete(t_paquete *paquete, int numero){
	
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size, &numero, sizeof(int));
	paquete->buffer->size += sizeof(int);
}





void agregar_pcb_a_paquete(t_paquete *paquete, t_pcb * pcb){
	agregar_entero_a_paquete(paquete, pcb->pc);
	agregar_entero_a_paquete(paquete, pcb->pid);
	agregar_registros_a_paquete(paquete, pcb->registros);
	agregar_entero_int_a_paquete(paquete,pcb->qq); 
}



void agregar_instruccion_a_paquete(t_paquete *paquete, t_instruccion * instruccion_nueva){
	
	agregar_string_a_paquete(paquete,instruccion_nueva->parametros1);
	agregar_string_a_paquete(paquete,instruccion_nueva->parametros2);
	agregar_string_a_paquete(paquete,instruccion_nueva->parametros3);
	agregar_string_a_paquete(paquete,instruccion_nueva->parametros4);
	agregar_string_a_paquete(paquete,instruccion_nueva->parametros5);
	agregar_string_a_paquete(paquete,instruccion_nueva->parametros6);

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
	memcpy(&entero, buffer + (*desplazamiento), sizeof(uint32_t));
	(*desplazamiento) += sizeof(uint32_t);
	return entero;
}

char* leer_string(char* buffer, int* desplazamiento) {
    int tamanio = leer_entero(buffer, desplazamiento);

    if (*desplazamiento + tamanio > strlen(buffer)) {
        return NULL;
    }

    char* palabra = malloc(tamanio + 1);
    if (palabra == NULL) {
        return NULL;
    }

    memcpy(palabra, buffer + *desplazamiento, tamanio);
    palabra[tamanio] = '\0';
    *desplazamiento += tamanio;

    return palabra;
}

// t_registros_cpu * leer_registros(char* buffer, int* desp){

// 	int tamanio = leer_entero(buffer,desp);
// 	t_registros_cpu * retorno = malloc(tamanio);
// 	leer_entero_uint8(buffer,desp);
// 	memcpy(retorno->AX, buffer + (*desp), 8);
// 	(*desp) += 8;
// 	leer_entero_uint8(buffer,desp);
// 	memcpy(retorno->BX, buffer + (*desp), 8);
// 	(*desp) += 8;
// 	leer_entero_uint8(buffer,desp);
// 	memcpy(retorno->CX, buffer + (*desp), 8);
// 	(*desp) += 8;
// 	leer_entero_uint8(buffer,desp);
// 	memcpy(retorno->DX, buffer + (*desp), 8);
// 	(*desp) += 8;
// 	leer_entero_uint32(buffer,desp);
// 	memcpy(retorno->EAX, buffer + (*desp), 8);
// 	(*desp) += 32;
// 	leer_entero_uint32(buffer,desp);
// 	memcpy(retorno->EBX, buffer + (*desp), 8);
// 	(*desp) += 32;
// 	leer_entero_uint32(buffer,desp);
// 	memcpy(retorno->ECX, buffer + (*desp), 8);
// 	(*desp) += 32;
// 	leer_entero_uint32(buffer,desp);
// 	memcpy(retorno->EDX, buffer + (*desp), 8);
// 	(*desp) += 32;


// }

uint32_t recibir_entero_uint32(int socket, t_log* loggs){

	int size = 0;
	char* buffer;
	int desp = 0;

	buffer = recibir_buffer(&size, socket);
	u_int32_t entero_nuevo32 = leer_entero_uint32(buffer, &desp);
	log_trace(loggs, "Me llego en numero %i", entero_nuevo32);
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

t_instruccion* recibir_instruccion(int socket){
	
	t_instruccion* nueva_instruccion = malloc(sizeof(t_instruccion));

	int size = 0;
	char* buffer;
	int desp = 0;
		
	buffer = recibir_buffer(&size, socket);

	nueva_instruccion->parametros1  = leer_string(buffer, &desp);
	nueva_instruccion->parametros2  = leer_string(buffer, &desp);
	nueva_instruccion->parametros3  = leer_string(buffer, &desp);
	nueva_instruccion->parametros4  = leer_string(buffer, &desp);
	nueva_instruccion->parametros5  = leer_string(buffer, &desp);
	nueva_instruccion->parametros6  = leer_string(buffer, &desp);
	
	free(buffer);
	return nueva_instruccion;
}

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











