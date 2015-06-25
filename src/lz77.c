#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "packarch.h"
#define LARGOCAR 8

typedef struct lz77{
	
	size_t match;
	size_t tammem;
	size_t taminsp;
	char* memoria;
	char* inspeccion;
	
} lz77;

size_t debinarise (size_t num) {
	return (num == 0 || num == 1 ? num : ((num%10) + 2*debinarise(num/10))); 
}

size_t binarise (size_t num) {
	return (num==0 || num==1 ? num : ((num%2)+10*binarise(num/2)));
}

lz77* nuevo_lz77(size_t mem, size_t insp, size_t minmatch) {
	lz77* nuevo = malloc(sizeof(lz77));
	nuevo->memoria = calloc(mem,1);
	nuevo->inspeccion = calloc(insp,1);
	nuevo->match = minmatch;
	nuevo->tammem = mem;
	nuevo->taminsp = insp;
	
	return nuevo;
}

void destruir_lz77(lz77* comp) {
	free(comp->memoria);
	free(comp->inspeccion);
	free(comp);
}

size_t comparar_strings(char* str1, char* str2, size_t tam) {
	if(tam == 0 || strncmp(str1,str2,1) != 0) return 0;
	return 1+comparar_strings(str1+1,str2+1,tam-1);
}	

char* comprimir(char* src, lz77* comp, size_t largo) {
	size_t longlong = ceil(log2(fmin(comp->taminsp,comp->tammem)-(comp->match)+1));
	size_t longpos = ceil(log2((comp->tammem)-(comp->match)+1));
	size_t largopar = longlong + longpos + 1;
	char* compresion = calloc(fmax(LARGOCAR+1, largopar),largo);
	for (size_t i = 0; i < (comp->tammem); i++) comp->memoria[i] = ' ';
	for (size_t i = 0; i < (comp->taminsp); i++) comp->inspeccion[i] = src[i];
	size_t copiados = 0;
	size_t i = 0;
	while (i < largo) {
		printf("Mem: %s Insp: %s\n",comp->memoria, comp->inspeccion);
		size_t iteracion = 0;
		size_t longitud = 0;
		size_t posicion = 0;
		for (size_t j = 0; j<comp->tammem-comp->match+1; j++) {
			size_t coincidencia = comparar_strings(comp->memoria+j, comp->inspeccion, fmin(comp->tammem-j,comp->taminsp));
			if(coincidencia > longitud) {
				longitud = coincidencia;
				posicion = j;
			}	
		}
		printf("Pos: %u, Long: %u\n", posicion, longitud);
		if (longitud < comp->match) {
			iteracion=1;
			compresion[copiados++] = 0;
			size_t binario = binarise(src[i]);
			printf("%u\n", binario);
			for (int j = LARGOCAR-1; j>=0; j--) {
				compresion[copiados++] = binario/pow(10, j);
				binario = binario%((size_t) pow(10,j));
			}
								
		} else {
			iteracion = longitud;
			compresion[copiados++] = 1;
			longitud = binarise(longitud-comp->match);
			posicion = binarise(posicion);
			printf("%02u%02u\n", posicion,longitud);
			for (int j = longpos-1; j>=0; j--) {
				compresion[copiados++] = posicion/(pow(10,j));
				posicion = posicion%(size_t) pow(10,j);
			}
			for (int j = longlong-1; j>=0; j--) {
				compresion[copiados++] = longitud/(pow(10,longlong-j));
				longitud = longitud%(size_t) pow(10,j);
			}
		}
		for (size_t k = 0; k<iteracion; k++) {
			for(size_t j = 0; j<comp->tammem-1; j++) {
				comp->memoria[j] = comp->memoria[j+1];
			}
			comp->memoria[comp->tammem-1] = comp->inspeccion[0];
			for(size_t j = 0; j<comp->taminsp-1; j++) {
				comp->inspeccion[j] = comp->inspeccion[j+1];
			}
			comp->inspeccion[comp->taminsp-1] = src[(i++)+comp->taminsp];
		}
	}
	printf("\n");
	for(size_t i = 0; i<copiados; i++) {
		printf("%u",compresion[i]);
	}	
	printf("\n\n");
	size_t tamaniofinal = ceil((copiados)/8);
	char* compresionfinal = calloc(tamaniofinal,1);
	for (size_t i = 0; i<tamaniofinal; i++) {
		size_t numero = 0;
		for (size_t j = 0; j<8; j++) {
			numero = numero + compresion[i*8+j]*pow(10,j);
		}	
		compresionfinal[i] = debinarise(numero);
	}
	free(compresion);
	printf("Tamanio Inicial: %u bits\nTamanio Final: %u bits\nRelación: %.02f%%\n", largo*8 ,copiados, 100*(double)(copiados)/(double)(largo*8));	
	return compresionfinal;
}

char* descomprimir(lz77* comp, char* src, size_t largo) {
	for (size_t i = 0; i < (comp->tammem); i++) comp->memoria[i] = 0;
	for (size_t i = 0; i < (comp->taminsp); i++) comp->inspeccion[i] = src[i];
	size_t longlong = ceil(log2(fmin(comp->taminsp,comp->tammem)-(comp->match)+1));
	size_t longpos = ceil(log2((comp->tammem)-(comp->match)+1));
	char* srcbits = calloc(largo,8);
	char* descomprimido = calloc(8,fmax(1,(fmin(comp->taminsp,comp->tammem)-(comp->match)+1)));
	for (size_t i = 0; i<largo; i++) {
		size_t fuente = binarise(src[i]);
		for (int j = 7; j>= 0; j--) {
			srcbits[i*8+7-j] = fuente/pow(10,j);
			fuente = fuente%(size_t) pow(10,j);
		}	
	}
	size_t i = 0;
	while (i<largo*8) {
		if (srcbits[i++] == 0) {
			size_t fuente = 0;
			for (size_t j = 0; j<8; j++) {
				fuente = fuente*10 + srcbits[i++];
			}	
		}	
		
	}		
	return NULL;
}					
