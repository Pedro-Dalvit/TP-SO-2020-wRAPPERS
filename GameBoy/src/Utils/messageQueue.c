#include "../Utils/messageQueue.h"

// CONEXIONES

int crear_conexion(char* ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);
	int socket_cliente;

	if((socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol)) == -1){
		printf("Error en crear socket. \n");
		exit(1);
	}

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1){
		printf("Error en conectar socket. \n");
		exit(2);
	}

	freeaddrinfo(server_info);
	return socket_cliente;
}

// SERIALIZAR PAQUETE

void* serializar_paquete(t_paquete* paquete, int *bytes)
{
	void* a_enviar = malloc((*bytes));
	int offset = 0;

	memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(op_code));
	offset += sizeof(op_code);

	memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(int));
	offset +=sizeof(int);

	memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

	return a_enviar;
}

// ENVIO DE MENSAJES

// MENSAJES A BROKER

void enviar_mensaje_a_broker(int socket_cliente, op_code codigo_operacion, char* argv[])
{
	int tamanio_paquete = 0;
	void* a_enviar;

	switch(codigo_operacion){
	case 0:
		a_enviar = suscribirse_a_cola(&tamanio_paquete,argv);
		break;
	case 1:
		a_enviar = iniciar_paquete_serializado_NewPokemon(&tamanio_paquete,argv);
		break;
	case 2:
		a_enviar = iniciar_paquete_serializado_AppearedPokemon(&tamanio_paquete,argv);
		break;
	case 3:
		a_enviar = iniciar_paquete_serializado_CatchPokemon(&tamanio_paquete,argv);
		break;
	case 4:
		a_enviar = iniciar_paquete_serializado_CaughtPokemon(&tamanio_paquete,argv);
		break;
	case 5:
		a_enviar = iniciar_paquete_serializado_GetPokemon(&tamanio_paquete,argv);
		break;
	case 6:
		break;
	case 7:
		break;
	}

	if(send(socket_cliente,a_enviar,tamanio_paquete,0) == -1){
		printf("Error en enviar por el socket");
		exit(3);
	}

	free(a_enviar);

//fflush(stdout);
}

// BROKER - SUSCRIPTOR

void* suscribirse_a_cola(int* tamanio_paquete,char* argv[]){

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = SUSCRIPTOR;
	paquete->buffer = malloc(sizeof(t_buffer));

	uint32_t proccess_id = obtener_id_propio(); //getpid();
	char* cola_leida = argv[2];
	uint cola = cola_a_suscribirse(cola_leida);
	uint32_t tiempo_de_suscripcion;
	sscanf(argv[3], "%d", &tiempo_de_suscripcion);
	paquete->buffer->size = sizeof(uint32_t)+sizeof(uint32_t)+sizeof(uint32_t);
	void* stream = malloc(paquete->buffer->size);
	int offset = 0;

		memcpy(stream + offset, &proccess_id, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		memcpy(stream + offset, &cola, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		memcpy(stream + offset, &tiempo_de_suscripcion, sizeof(uint32_t));

	paquete->buffer->stream = stream;

						// TAMAÑO STREAM + OP CODE + VARIABLE SIZE
	*tamanio_paquete = (paquete->buffer->size)+sizeof(op_code)+sizeof(uint32_t);
	void* a_enviar = serializar_paquete(paquete, tamanio_paquete);

	free(paquete->buffer);
	free(stream);
	free(paquete);

	return a_enviar;
}

uint32_t cola_a_suscribirse(char* cola_leida){
	if(string_equals_ignore_case(cola_leida, "NEW_POKEMON"))
			return 1;
	else if(string_equals_ignore_case(cola_leida, "APPEARED_POKEMON"))
			return 2;
	else if(string_equals_ignore_case(cola_leida, "CATCH_POKEMON"))
			return 3;
	else if (string_equals_ignore_case(cola_leida, "CAUGHT_POKEMON"))
			return 4;
	else if (string_equals_ignore_case(cola_leida, "GET_POKEMON"))
			return 5;
	else if (string_equals_ignore_case(cola_leida, "LOCALIZED_POKEMON"))
			return 6;
	else{
		puts("No existe esta cola de mensajes \n");
		return 0;
	}
}

// BROKER - NEW POKEMON

void* iniciar_paquete_serializado_NewPokemon(int* tamanio_paquete,char* argv[]){

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = NEW_POKEMON;
	paquete->buffer = malloc(sizeof(t_buffer));

	char* pokemon = argv[3];
	uint32_t caracteresPokemon = strlen(pokemon) + 1;
	uint32_t posX;
	sscanf(argv[4], "%d", &posX);
	uint32_t posY ;
	sscanf(argv[5], "%d", &posY);
	uint32_t cantidad_pokemon;
	sscanf(argv[6], "%d", &cantidad_pokemon);
															//INT CARACTERES + POKEMON + POSX + POSY + CANTIDAD
	paquete->buffer->size = sizeof(uint32_t) + caracteresPokemon +sizeof(uint32_t)+sizeof(uint32_t)+sizeof(uint32_t);
	void* stream = malloc(paquete->buffer->size);
	int offset = 0;

		memcpy(stream + offset, &caracteresPokemon, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		memcpy(stream + offset, pokemon, caracteresPokemon);
		offset += caracteresPokemon;

		memcpy(stream + offset, &posX, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		memcpy(stream + offset, &posY, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		memcpy(stream + offset, &cantidad_pokemon, sizeof(uint32_t));

	paquete->buffer->stream = stream;

						// TAMAÑO STREAM + OP CODE + VARIABLE SIZE
	*tamanio_paquete = (paquete->buffer->size)+sizeof(op_code)+sizeof(uint32_t);
	void* a_enviar = serializar_paquete(paquete, tamanio_paquete);

	free(paquete->buffer);
	free(stream);
	free(paquete);

	return a_enviar;

}

// BROKER - APPEARED POKEMON

void* iniciar_paquete_serializado_AppearedPokemon(int* tamanio_paquete,char* argv[]){

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = APPEARED_POKEMON;
	paquete->buffer = malloc(sizeof(t_buffer));

	char* pokemon = argv[3];
	uint32_t caracteresPokemon = strlen(pokemon) + 1;
	uint32_t posX;
	sscanf(argv[4], "%d",&posX);
	uint32_t posY ;
	sscanf(argv[5], "%d",&posY);
	uint32_t id_mensaje_correlativo;
	sscanf(argv[6], "%d",&id_mensaje_correlativo); //el id lo definimos nosotros y debe ser único

						//INT CARACTERES + POKEMON + POSX + POSY + ID_MENSAJE_CORRELATIVO
	paquete->buffer->size =sizeof(uint32_t) + caracteresPokemon +sizeof(uint32_t)+sizeof(uint32_t)+sizeof(uint32_t);
	void* stream = malloc(paquete->buffer->size);
	int offset = 0;

		memcpy(stream + offset, &caracteresPokemon, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		memcpy(stream + offset, pokemon, caracteresPokemon);
		offset += caracteresPokemon;

		memcpy(stream + offset, &posX, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		memcpy(stream + offset, &posY, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		memcpy(stream + offset, &id_mensaje_correlativo, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		paquete->buffer->stream = stream;

						// TAMAÑO STREAM + OP CODE + VARIABLE SIZE
	*tamanio_paquete = (paquete->buffer->size)+sizeof(op_code)+sizeof(uint32_t);
	void* a_enviar = serializar_paquete(paquete, tamanio_paquete);

	free(stream);
	free(paquete->buffer);
	free(paquete);

	return a_enviar;

}

// BROKER - CATCH POKEMON

void* iniciar_paquete_serializado_CatchPokemon(int* tamanio_paquete,char* argv[]){

	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = CATCH_POKEMON;

	paquete->buffer = malloc(sizeof(t_buffer));

	char* pokemon = argv[3];
	uint32_t caracteresPokemon = strlen(pokemon) + 1;
	uint32_t posX;
	sscanf(argv[4], "%d",&posX);
	uint32_t posY ;
	sscanf(argv[5], "%d",&posY);

						//INT CARACTERES + POKEMON + POSX + POSY
	paquete->buffer->size = sizeof(uint32_t) + caracteresPokemon + sizeof(uint32_t) + sizeof(uint32_t);
	void* stream = malloc(paquete->buffer->size);
	int offset = 0;

		memcpy(stream + offset, &caracteresPokemon, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		memcpy(stream + offset, pokemon, caracteresPokemon);
		offset += caracteresPokemon;

		memcpy(stream + offset, &posX, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		memcpy(stream + offset, &posY, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		paquete->buffer->stream = stream;

						// TAMAÑO STREAM + OP CODE + VARIABLE SIZE
	*tamanio_paquete = (paquete->buffer->size)+sizeof(op_code)+sizeof(uint32_t);
	void* a_enviar = serializar_paquete(paquete, tamanio_paquete);

	free(stream);
	free(paquete->buffer);
	free(paquete);

	return a_enviar;
}

// BROKER - CAUGHT POKEMON

void* iniciar_paquete_serializado_CaughtPokemon(int* tamanio_paquete,char* argv[]){

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = CAUGHT_POKEMON;
	paquete->buffer = malloc(sizeof(t_buffer));

	uint32_t id_mensaje_correlativo;
	sscanf(argv[3], "%d", &id_mensaje_correlativo);

	uint32_t se_pudo_atrapar; // devuelve 1 o 0 dependiendo de si se pudo o no -> lo mandamos asi o como true/false string?
	char* ok_o_fail = argv[4];

	if(!(strcmp(ok_o_fail, "OK"))){
		se_pudo_atrapar = 1;
	}
	if(!(strcmp(ok_o_fail, "FAIL"))){
		se_pudo_atrapar = -1;
	}

		   // ID_MENSAJE_CORRELATIVO + OK/FAIL
	paquete->buffer->size = sizeof(uint32_t) + sizeof(uint32_t);
	void* stream = malloc(paquete->buffer->size);
	int offset = 0;

		memcpy(stream + offset, &id_mensaje_correlativo, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		memcpy(stream + offset, &se_pudo_atrapar, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		paquete->buffer->stream = stream;

						// TAMAÑO STREAM + OP CODE + VARIABLE SIZE
	*tamanio_paquete = (paquete->buffer->size)+sizeof(op_code)+sizeof(uint32_t);
	void* a_enviar = serializar_paquete(paquete, tamanio_paquete);

	free(stream);
	free(paquete->buffer);
	free(paquete);

	return a_enviar;
}

// BROKER - GET POKEMON

void* iniciar_paquete_serializado_GetPokemon(int* tamanio_paquete,char* argv[]){

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = GET_POKEMON;
	paquete->buffer = malloc(sizeof(t_buffer));

	char* pokemon = argv[3];
	uint32_t caracteresPokemon = strlen(pokemon) + 1;

						//INT CARACTERES + POKEMON
	paquete->buffer->size = sizeof(uint32_t) + caracteresPokemon;
	void* stream = malloc(paquete->buffer->size);
	int offset = 0;

		memcpy(stream + offset, &caracteresPokemon, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		memcpy(stream + offset, pokemon, caracteresPokemon);
		offset += caracteresPokemon;

		paquete->buffer->stream = stream;

						// TAMAÑO STREAM + OP CODE + VARIABLE SIZE
	*tamanio_paquete = (paquete->buffer->size)+sizeof(op_code)+sizeof(uint32_t);
	void* a_enviar = serializar_paquete(paquete, tamanio_paquete);

	free(stream);
	free(paquete->buffer);
	free(paquete);

	return a_enviar;
}

// MENSAJE A TEAM

void enviar_mensaje_a_team(int socket_cliente, op_code codigo_operacion, char* argv[])
{
	int tamanio_paquete = 0;
	void* a_enviar;

	a_enviar = iniciar_paquete_serializado_AppearedPokemonTeam(&tamanio_paquete,argv);

	send(socket_cliente,a_enviar,tamanio_paquete,0);
	free(a_enviar);
}

// TEAM - APPEARED POKEMON

void* iniciar_paquete_serializado_AppearedPokemonTeam(int* tamanio_paquete,char* argv[]){

	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = APPEARED_POKEMON;

	paquete->buffer = malloc(sizeof(t_buffer));

	char* pokemon = argv[3];
	uint32_t caracteresPokemon = strlen(pokemon) + 1;
	uint32_t posX;
	sscanf(argv[4], "%d",&posX);
	uint32_t posY ;
	sscanf(argv[5], "%d",&posY);

							//INT CARACTERES + POKEMON + POSX + POSY
	paquete->buffer->size = sizeof(uint32_t) + caracteresPokemon +sizeof(uint32_t)+sizeof(uint32_t);
	void* stream = malloc(paquete->buffer->size);
	int offset = 0;

		memcpy(stream + offset, &caracteresPokemon, sizeof(int));
		offset += sizeof(uint32_t);

		memcpy(stream + offset, pokemon, caracteresPokemon);
		offset +=caracteresPokemon;

		memcpy(stream + offset, &posX, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		memcpy(stream + offset, &posY, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		paquete->buffer->stream = stream;

						// TAMAÑO STREAM + OP CODE + VARIABLE SIZE
	*tamanio_paquete = (paquete->buffer->size)+sizeof(op_code)+sizeof(uint32_t);
	void* a_enviar = serializar_paquete(paquete, tamanio_paquete);

	free(stream);
	free(paquete->buffer);
	free(paquete);

	return a_enviar;
}

// MENSAJES A GAMECARD

void enviar_mensaje_a_gamecard(int socket_cliente, op_code codigo_operacion, char* argv[]){
	int tamanio_paquete = 0;
	void* a_enviar;

	switch(codigo_operacion){
		case 0:
			break;
		case 1:
			a_enviar = iniciar_paquete_serializado_NewPokemonGC(&tamanio_paquete,argv);
			break;
		case 2:
			break;
		case 3:
			a_enviar = iniciar_paquete_serializado_CatchPokemonGC(&tamanio_paquete,argv);
			break;
		case 4:
			break;
		case 5:
			a_enviar = iniciar_paquete_serializado_GetPokemonGC(&tamanio_paquete,argv);
			break;
		case 6:
			break;
		case 7:
			break;
		}

	send(socket_cliente,a_enviar,tamanio_paquete,0);
	free(a_enviar);
}

// GAMECARD - NEW POKEMON

void* iniciar_paquete_serializado_NewPokemonGC(int* tamanio_paquete,char* argv[]){

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = NEW_POKEMON;
	paquete->buffer = malloc(sizeof(t_buffer));

	char* pokemon = argv[3];
	uint32_t caracteresPokemon = strlen(pokemon) + 1;
	uint32_t posX;
	sscanf(argv[4], "%d",&posX);
	uint32_t posY;
	sscanf(argv[5], "%d",&posY);
	uint32_t cantidad_pokemon;
	sscanf(argv[6], "%d",&cantidad_pokemon);
	uint32_t id_mensaje;
	sscanf(argv[7], "%d",&id_mensaje);

						//INT CARACTERES + POKEMON + POSX + POSY + CANTIDAD + ID MENSAJE
	paquete->buffer->size =sizeof(uint32_t) + caracteresPokemon +sizeof(uint32_t)+sizeof(uint32_t)+sizeof(uint32_t)+sizeof(uint32_t);
	void* stream = malloc(paquete->buffer->size);
	int offset = 0;

		memcpy(stream + offset, &caracteresPokemon, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		memcpy(stream + offset, pokemon, caracteresPokemon);
		offset += caracteresPokemon;

		memcpy(stream + offset, &posX, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		memcpy(stream + offset, &posY, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		memcpy(stream + offset, &cantidad_pokemon, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		memcpy(stream + offset, &id_mensaje, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		paquete->buffer->stream = stream;

							// TAMAÑO STREAM + OP CODE + VARIABLE SIZE
	*tamanio_paquete = (paquete->buffer->size)+sizeof(op_code)+sizeof(uint32_t);
	void* a_enviar = serializar_paquete(paquete, tamanio_paquete);

	free(stream);
	free(paquete->buffer);
	free(paquete);

	return a_enviar;
}

// GAMECARD - CATCH POKEMON

void* iniciar_paquete_serializado_CatchPokemonGC(int* tamanio_paquete,char* argv[]){

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = CATCH_POKEMON;
	paquete->buffer = malloc(sizeof(t_buffer));

	char* pokemon = argv[3];
	uint32_t caracteresPokemon = strlen(pokemon) + 1;
	uint32_t posX;
	sscanf(argv[4], "%d",&posX);
	uint32_t posY ;
	sscanf(argv[5], "%d",&posY);
	uint32_t id_mensaje;
	sscanf(argv[6], "%d",&id_mensaje);

						//INT CARACTERES + POKEMON + POSX + POSY + ID MENSAJE
	paquete->buffer->size = sizeof(uint32_t) + caracteresPokemon + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t);
	void* stream = malloc(paquete->buffer->size);
	int offset = 0;

		memcpy(stream + offset, &caracteresPokemon, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		memcpy(stream + offset, pokemon, caracteresPokemon);
		offset +=caracteresPokemon;

		memcpy(stream + offset, &posX, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		memcpy(stream + offset, &posY, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		memcpy(stream + offset, &id_mensaje, sizeof(uint32_t));

		paquete->buffer->stream = stream;

						// TAMAÑO STREAM + OP CODE + VARIABLE SIZE
	*tamanio_paquete = (paquete->buffer->size)+sizeof(op_code)+sizeof(uint32_t);
	void* a_enviar = serializar_paquete(paquete, tamanio_paquete);

	free(stream);
	free(paquete->buffer);
	free(paquete);

	return a_enviar;
}

// GAMECARD - GET POKEMON

void* iniciar_paquete_serializado_GetPokemonGC(int* tamanio_paquete,char* argv[]){

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = GET_POKEMON;
	paquete->buffer = malloc(sizeof(t_buffer));

	char* pokemon = argv[3];
	uint32_t caracteresPokemon = strlen(pokemon) + 1;
	uint32_t id_mensaje;
	sscanf(argv[4], "%d",&id_mensaje);

						//INT CARACTERES + POKEMON + ID MENSAJE
	paquete->buffer->size = sizeof(uint32_t) + caracteresPokemon + sizeof(uint32_t);
	void* stream = malloc(paquete->buffer->size);
	int offset = 0;

		memcpy(stream + offset, &caracteresPokemon, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		memcpy(stream + offset, pokemon, caracteresPokemon);
		offset += caracteresPokemon;

		memcpy(stream + offset, &id_mensaje, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		paquete->buffer->stream = stream;

						// TAMAÑO STREAM + OP CODE + VARIABLE SIZE
	*tamanio_paquete = (paquete->buffer->size)+sizeof(op_code)+sizeof(uint32_t);
	void* a_enviar = serializar_paquete(paquete, tamanio_paquete);

	free(stream);
	free(paquete->buffer);
	free(paquete);

	return a_enviar;
}

// RECIBIR MENSAJE

void recibir_mensaje(int socket_cliente){

	op_code codigo_de_operacion;
	recv(socket_cliente, &codigo_de_operacion, sizeof(op_code), MSG_WAITALL);

		char* mensaje = string_from_format("El código de operación es: %d.", codigo_de_operacion);
		puts(mensaje);

	uint32_t buffer_size;
	recv(socket_cliente, &buffer_size, sizeof(uint32_t), MSG_WAITALL);

		char* mensaje2 = string_from_format("El tamanio del buffer es: %d.", buffer_size);
		puts(mensaje2);

	uint32_t caracteresPokemon;
	char* pokemon;
	uint32_t mensajeid;
	uint32_t posX;
	uint32_t posY;
	uint32_t cantidad_pokemon;
	uint32_t id_mensaje_correlativo;
	uint32_t se_pudo_atrapar;

	uint32_t tamanio_leido = 0;

	char* mensaje3;
	char* mensaje4;
	char* mensaje5;
	char* mensaje6;

	switch(codigo_de_operacion){
	case 0:
		break;
	case 1:

		recv(socket_cliente, &caracteresPokemon, sizeof(uint32_t), MSG_WAITALL);
		tamanio_leido += sizeof(uint32_t);

		mensaje3 = string_from_format("La cantidad de caracteres del pokemon es: %d.", caracteresPokemon);
		puts(mensaje3);

		pokemon = (char*)malloc(caracteresPokemon);

		recv(socket_cliente, pokemon, caracteresPokemon, MSG_WAITALL);
		tamanio_leido += caracteresPokemon;

		printf("El pokemon es %s\n", pokemon);

		recv(socket_cliente, &posX, sizeof(uint32_t), MSG_WAITALL);
		tamanio_leido += sizeof(uint32_t);

		mensaje4 = string_from_format("La posicion en X es: %d.", posX);
		puts(mensaje4);

		recv(socket_cliente, &posY, sizeof(uint32_t), MSG_WAITALL);
		tamanio_leido += sizeof(uint32_t);

		mensaje5 = string_from_format("La posicion en Y es: %d.", posY);
		puts(mensaje5);

		recv(socket_cliente, &cantidad_pokemon, sizeof(uint32_t), MSG_WAITALL);
		tamanio_leido += sizeof(uint32_t);

		mensaje6 = string_from_format("La cantidad del pokemon es: %d.", cantidad_pokemon);
		puts(mensaje6);

		free(pokemon);

		break;

	case 2:

		recv(socket_cliente, &caracteresPokemon, sizeof(uint32_t), MSG_WAITALL);

		mensaje3 = string_from_format("La cantidad de caracteres del pokemon es: %d.", caracteresPokemon-1);
		puts(mensaje3);

		pokemon = (char*)malloc(caracteresPokemon);

		recv(socket_cliente, pokemon, caracteresPokemon, MSG_WAITALL);

		printf("El pokemon es %s\n", pokemon);

		recv(socket_cliente, &posX, sizeof(uint32_t), MSG_WAITALL);

		mensaje4 = string_from_format("La posicion en X es: %d.", posX);
		puts(mensaje4);

		recv(socket_cliente, &posY, sizeof(uint32_t), MSG_WAITALL);

		mensaje5 = string_from_format("La posicion en Y es: %d.", posY);
		puts(mensaje5);

		free(pokemon);

		break;
	case 3:

		recv(socket_cliente, &caracteresPokemon, sizeof(uint32_t), MSG_WAITALL);

		mensaje3 = string_from_format("La cantidad de caracteres del pokemon es: %d.", caracteresPokemon);
		puts(mensaje3);

		pokemon = (char*)malloc(caracteresPokemon);

		recv(socket_cliente, pokemon, caracteresPokemon, MSG_WAITALL);

		printf("El pokemon es %s\n", pokemon);

		recv(socket_cliente, &posX, sizeof(uint32_t), MSG_WAITALL);

		mensaje4 = string_from_format("La posicion en X es: %d.", posX);
		puts(mensaje4);

		recv(socket_cliente, &posY, sizeof(uint32_t), MSG_WAITALL);

		mensaje5 = string_from_format("La posicion en Y es: %d.", posY);
		puts(mensaje5);

		free(pokemon);

		break;
	case 4:

		recv(socket_cliente, &se_pudo_atrapar, sizeof(uint32_t), MSG_WAITALL);

		mensaje3 = string_from_format("Se pudo atrapar es: %d.", se_pudo_atrapar);
		puts(mensaje3);

		recv(socket_cliente, &id_mensaje_correlativo, sizeof(uint32_t), MSG_WAITALL);

		mensaje3 = string_from_format("El id de mensaje correlativo es es: %d.", id_mensaje_correlativo);
		puts(mensaje3);

		break;

	case 5:

		recv(socket_cliente, &caracteresPokemon, sizeof(uint32_t), MSG_WAITALL);

		mensaje3 = string_from_format("La cantidad de caracteres del pokemon es: %d.", caracteresPokemon);
		puts(mensaje3);

		pokemon = (char*)malloc(caracteresPokemon);
		recv(socket_cliente, pokemon, caracteresPokemon, MSG_WAITALL);

		printf("El pokemon es %s\n", pokemon);

		break;
	case 6:
		break;
	case 7:

		recv(socket_cliente, mensaje4, buffer_size, MSG_WAITALL); //no recibe pokemon, recibe el mensaje id

		//mensaje4 = string_from_format("El id de mensaje es: %d.", mensajeid);
		puts(mensaje4);

		break;
	}
/*
	uint32_t identificador = 1;
	int tamanio_paquete = 0;
	void* a_enviar = enviar_ack(socket_cliente,codigo_de_operacion,identificador,&tamanio_paquete);
	send(socket_cliente,a_enviar,tamanio_paquete,0); //no llega el ack
*/
}

void enviar_ACK(int socket_broker, char* mensaje){

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = 7;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->stream = mensaje;
	paquete->buffer->size = strlen(mensaje) + 1;

	int tamanio_paquete = (paquete->buffer->size)+sizeof(op_code)+sizeof(uint32_t);
	void* a_enviar = serializar_paquete(paquete,&tamanio_paquete);

	if(send(socket_broker,a_enviar,tamanio_paquete,0) == -1){
		completar_logger("Error en enviar por el socket","GAMEBOY", LOG_LEVEL_INFO);
		exit(3);
	}

	free(paquete->buffer);
	free(paquete);

	//sem_post(&MUTEX_ACK);
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}
