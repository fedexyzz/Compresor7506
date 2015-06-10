#ifndef _VECTOR_DINAMICO_H
#define _VECTOR_DINAMICO_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "heap.h"
#define TAM_IN 10

typedef struct vector vector_t;

typedef void (*destructor_vec) (void*);
typedef void (*comparar_vec) (void*);
typedef int (*cmp_vec_t) (const void *a, const void *b);

void vector_ordenar (vector_t* vector, cmp_vec_t funcion);

// Crea un vector de tamaño TAM_IN, con una función destructora de sus 
// datos.
vector_t *vector_crear(destructor_vec func);

// Devuelve la cantidad de elementos almacenados.
size_t vector_obtener_cantidad(vector_t *vector);

// Devuelve el tamaño actual del vector.
size_t vector_obtener_tamanio(vector_t *vector);

// Devuelve el dato almacenado en la posición indicada.
void* vector_obtener (vector_t *vector, size_t pos);

// Guarda el valor en la primer posición libre. Devuelve true si el
// valor es almacenado, false si no.
bool vector_guardar(vector_t *vector, void* valor);

// Borra el valor almacenado en la posicion indicada. Devuelve true si
// se pudo borrar, false sino.
bool vector_borrar_posicion (vector_t* vector, size_t pos);

// Borra el valor indicado. Devuelve true si
// se pudo borrar, false sino.
bool vector_borrar_valor (vector_t* vector, void* valor);

// Busca el valor y devuelve su posición (-1 si no lo encuentra).
long int vector_buscar (vector_t* vector, void* valor);

// Destruye el vector. Si se indic{o una funci{on destructora, destruye
// los datos.
void vector_destruir(vector_t *vector);

#endif // _VECTOR_DINAMICO_H
