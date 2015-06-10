#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "vector_dinamico.h"
#include "heap.h"

struct vector {
	size_t tam;
	size_t cantidad;
	void** datos;
	destructor_vec func;
};

vector_t *vector_crear(destructor_vec func) {
	vector_t* vector = malloc(sizeof(vector_t));
	if (vector == NULL) return NULL;

	vector->datos = calloc(TAM_IN, sizeof(void*));
	if (vector->datos == NULL)
	{
	    free(vector);
	    return NULL;
	}
	for (size_t i=0; i<TAM_IN; i++) vector->datos[i] = NULL;
	vector->tam = TAM_IN;
	vector->cantidad = 0;
	vector->func = func;
	return vector;
}

void vector_ordenar (vector_t* vector, cmp_vec_t funcion) {
	heap_sort(vector->datos, vector_obtener_cantidad(vector), funcion);
}

static bool vector_redimensionar(vector_t *vector, size_t tam_nuevo) {
	void** datos_nuevo = realloc(vector->datos, tam_nuevo * sizeof(void*));
	if (tam_nuevo > 0 && !datos_nuevo) return false;
	vector->datos = datos_nuevo;
	vector->tam = tam_nuevo;
	return true;
}

size_t vector_obtener_cantidad(vector_t *vector){	
	return vector->cantidad;
}

size_t vector_obtener_tamanio(vector_t *vector){	
	return vector->tam;
}

long int vector_buscar (vector_t* vector, void* valor) {
	for (size_t i = 0; i< vector->cantidad; i++) {
		if (vector->datos[i] == valor) return i;
	}
	return -1;
}		

bool vector_borrar_posicion (vector_t* vector, size_t pos) {
	if (pos>=vector->cantidad) return false;
	for (size_t i=pos; i<vector->cantidad-1; i++) vector->datos[i] = vector->datos[i+1];
	vector->datos[vector->cantidad-1] = NULL;
	vector->cantidad--;
	if (vector->cantidad <= vector->tam/4 && vector->tam > TAM_IN)
								vector_redimensionar (vector, vector->tam/2);
	return true;
}			

bool vector_borrar_valor (vector_t* vector, void* valor) {
	size_t pos = vector_buscar(vector, valor);
	if (pos==-1) return false;
	return vector_borrar_posicion(vector,pos);
}

void vector_destruir(vector_t *vector) {	
	if (vector->func) {
		for (size_t i=0; i<vector_obtener_cantidad(vector); i++) vector->func(vector->datos[i]);
	}	
	free (vector->datos);
	free (vector);
}

void* vector_obtener (vector_t *vector, size_t pos) {
	if (vector->tam>0 && pos < vector->tam) {
		return vector->datos[pos];
	}
	return NULL;		
}	

bool vector_guardar(vector_t *vector, void* valor) {
	if (!vector || !vector->datos) return false;
	vector->datos[vector->cantidad++]= valor;
	if (vector->cantidad==vector->tam) vector_redimensionar (vector, vector->tam*2);
	return true;
}
