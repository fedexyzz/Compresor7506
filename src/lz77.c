#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "packarch.h"

typedef struct lz77{
	char* memoria;
	char* inspeccion;
	size_t match;
} lz77;

lz77* nuevo_lz77(size_t mem, size_t insp, size_t minmatch) {
	lz77* nuevo = calloc(1,sizeof(lz77));
	nuevo->match = minmatch;
	nuevo->memoria = calloc(mem, sizeof(char));
	nuevo->inspeccion = calloc(insp, sizeof(char));
	
	return nuevo;
}

void destruir_lz77(lz77* comp) {
	free(comp->memoria);
	free(comp->inspeccion);
	free(comp);
}

char* comprimir(char* src) {			
