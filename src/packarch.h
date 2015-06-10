#ifndef _PACKARCH_H
#define _PACKARCH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lectura.h"
#include "vector_dinamico.h"
#define FACTOR 512

typedef struct packarch_t packarch_t;

size_t pot (size_t num1, size_t num2); 

packarch_t* crear_packarch (char* nom, size_t n, char* reg);

void packarch_destruir (packarch_t* pack);

const char* nombre_archivo (packarch_t* archivo);

size_t tamanio_bloque (packarch_t* archivo);

vector_t* registro_archivo (packarch_t* archivo); 

#endif
