#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lectura.h"
#include "packarch.h"
#include "vector_dinamico.h"

struct packarch_t {
	char* nombre;
	size_t tam;
	char* copia;
	vector_t* registro;
};

size_t pot (size_t num1, size_t num2) {
	size_t resultado = 1;
	for (size_t j = 1; j <= num2; j++) resultado = resultado*num1;
	return resultado;
}

packarch_t* crear_packarch (char* nom, size_t n, char* reg) {
	packarch_t* nuevo = calloc (1,sizeof(packarch_t));
	if (!nuevo) return NULL;
	nuevo->registro = vector_crear(NULL);
	if(!nuevo->registro) {
		free (nuevo);
		return NULL;
	}
	nuevo->nombre = calloc (1,sizeof(char)*(strlen(nom)+1));
	strcpy(nuevo->nombre,nom);
	nuevo->tam	= pot(2,n)*FACTOR;
	nuevo->copia = calloc (1,sizeof(char)*(strlen(reg)+1));
	strcpy(nuevo->copia,reg);
	char* campo = strtok(nuevo->copia, ",");
	while (campo) {
		vector_guardar (nuevo->registro, campo);
		campo = strtok(NULL,",");
	}
	return nuevo;	
}

void packarch_destruir (packarch_t* pack) {
	vector_destruir(pack->registro);
	free(pack->nombre);
	free(pack->copia);
	free(pack);
}

const char* nombre_archivo (packarch_t* archivo) {
	return archivo->nombre;
}

size_t tamanio_bloque (packarch_t* archivo) {
	return archivo->tam;
}

vector_t* registro_archivo (packarch_t* archivo) {
	return archivo->registro;
}
