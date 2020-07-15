#include "bloques.h"

/*
int bloque_esta_vacio(int bloque){
	if(bloque_tamanio_libre(bloque) == obtener_tamanio_bloques()){
	return 1;
	}
	return 0;

}
*/

int existe_bloque(char* path_bloques,char* numero_de_bloque){
	char* path_bloque = string_new();
	string_append(&path_bloque, path_bloques);
	string_append(&path_bloque,"/" );
	string_append(&path_bloque,numero_de_bloque);
	string_append(&path_bloque, ".bin");
	struct stat buffer;
	return stat(path_bloque,&buffer);
}

int existe_file(char* path){
	struct stat buffer;
	return stat(path,&buffer);
}

void actualizar_valores_pokemon(char* path_metadata_pokemon,int posX,int posY,int cantidad){

	t_config* metadata = leer_metadata_pokemon(path_metadata_pokemon);

	char** bloques_pokemon = obtener_bloques_pokemon(metadata);
	int cantidad_bloques = obtener_cantidad_bloques_pokemon(metadata);

	for(int i =0; i<cantidad_bloques ; i++){
		//actualizar_valores_bloque(bloques_pokemon[i],)
		}


}
 int espacio_libre(int bloque);

//USAR CONFIG PARA PATH

 /*
  * FSEEK

	1-1=2/n
	2-3=1/n
	1-/o

	2=3/n
	1-2=3
  */





 int obtener_tamanio_bloque(){
 	return 26;
 }



 char* obtener_path_bloque(char* bloque){
 	char* path = string_new();
 	string_append(&path, "/home/utnso/Documentos/Prueba/");
 	string_append(&path, bloque);
 	string_append(&path, ".bin");
 	return path;
 }

 int diferente_largo(char* numero1,char * numero2){
 	int largo1 = strlen(numero1);
 	int largo2 = strlen(numero2);

 	if(largo1 != largo2){

 		return 1;
 	}
 	return 0;
 }


 int tamanio_libre_bloque(char* bloque){
 	char* path = obtener_path_bloque(bloque);

 	struct stat st;
 	stat(path,&st);

 	int tamanio_actual = st.st_size;

 	free(path);

 	return(obtener_tamanio_bloque() - tamanio_actual);
 }
