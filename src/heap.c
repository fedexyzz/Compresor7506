#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "heap.h"

#define TAM_INICIAL 10

struct heap {
    void** datos;
    size_t cantidad;
    size_t tamanio;
    cmp_func_t heap_cmp;
};

bool redimensionar_datos (heap_t* heap, size_t tam) {
	void** datos_n = realloc (heap->datos, tam * sizeof (void*));
	if (!datos_n) return false;
	heap->datos = datos_n;
	heap->tamanio = tam;
	return true;
}

heap_t *heap_crear(cmp_func_t cmp){
    heap_t* heap = malloc(sizeof(heap_t));
    if(!heap) return NULL;
    heap->datos = calloc(TAM_INICIAL, sizeof(void*));
	if (!heap->datos)
	{
	    free(heap);
	    return NULL;
	}
    heap->cantidad = 0;
    heap->tamanio = TAM_INICIAL;
    heap->heap_cmp = cmp;
    return heap;
}

size_t heap_cantidad(const heap_t *heap){
    return heap->cantidad;
}

bool heap_esta_vacio(const heap_t *heap){
    return !heap->cantidad;
}

void swap (void** x, void** y){
    void* aux = *x;
    *x = *y;
    *y = aux;
    return;
}

bool upheap (heap_t* heap, size_t pos){
    if (pos == 0) return false;
    if (heap->heap_cmp(heap->datos[pos],heap->datos[(pos-1)/2]) > 0){
        swap(&heap->datos[pos],&heap->datos[(pos-1)/2]);
        upheap(heap,(pos-1)/2);
    }
    return true;
}

bool downheap (heap_t* heap, size_t pos){
	if (heap->cantidad <= 1) return false;
	if (pos*2 +1 > (heap->cantidad)-1) return false;
	size_t hijomay;
	if (pos*2 +2 > (heap->cantidad)-1) {
		hijomay = 2*pos +1;
	}	else if (heap->heap_cmp(heap->datos[pos*2+1], heap->datos[pos*2+2]) >= 0) {
				hijomay = 2*pos +1;
		} else  hijomay = 2*pos +2;
	if (heap->heap_cmp(heap->datos[pos],heap->datos [hijomay]) < 0){
        swap(&heap->datos[pos],&heap->datos[hijomay]);
        downheap(heap, hijomay);
    }
    return true;
}

void* heap_ver_max(const heap_t *heap){
    if(!heap->cantidad) return NULL;
    return heap->datos[0];
}

void* heap_desencolar(heap_t *heap){
    if(heap_esta_vacio (heap))return NULL;
    void* aux = heap->datos[0];
    if (heap_cantidad(heap)>1) swap(&heap->datos[0],&heap->datos[heap->cantidad-1]);
    heap->datos[heap->cantidad-1]=NULL;
    heap->cantidad--;
    downheap(heap,0);
    if (heap->tamanio >= heap->cantidad * 4 && heap->tamanio > TAM_INICIAL) {
		 redimensionar_datos (heap,heap->tamanio/2);
    }
    return aux;
}

bool heap_encolar(heap_t *heap, void *elem){
    if (!elem) return false;
    heap->datos[heap->cantidad] = elem;
    upheap(heap,heap->cantidad);
    if (heap->tamanio == ++heap->cantidad) {
        redimensionar_datos (heap, heap->tamanio * 2);
	}
    return true;
}

void heap_destruir(heap_t *heap, void destruir_elemento(void *e)){
    while (!heap_esta_vacio(heap)) {
        if (destruir_elemento != NULL){
            destruir_elemento(heap_desencolar(heap));
        } else heap_desencolar(heap);
    }
    free(heap->datos);
    free(heap);
    return;
}

void heapify(heap_t* heap,size_t tam){
int j;
    for(j=((tam-1)/2);j>=0;j--){
        downheap(heap,j);
    }
    return;
}

void heap_sort(void* elementos[], size_t cant, cmp_func_t cmp){
size_t i;
	heap_t* heap=heap_crear(cmp);
	free(heap->datos);
	heap->datos = elementos;
	heap->cantidad = cant;
    heapify(heap,cant);
	for(i=cant-1;i>=1;i--){
       swap(&heap->datos[0],&heap->datos[i]);
       heap->cantidad--;
       downheap(heap,0);
	}
	free(heap);
	return;
}
