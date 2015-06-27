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

size_t binarisebyte (unsigned char num) {
	return (num==0 || num==1 ? num : ((num%2)+10*binarise(num/2)));
}

unsigned char debinarisebyte (size_t num) {
	return (num == 0 || num == 1 ? num : ((num%10) + 2*debinarise(num/10))); 
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
		if (longitud < comp->match) {
			iteracion=1;
			compresion[copiados++] = 0;
			size_t binario = binarise(src[i]);
			for (int j = LARGOCAR-1; j>=0; j--) {
				compresion[copiados++] = (size_t) binario/(size_t) pow(10, j);
				binario = (size_t) binario%((size_t) pow(10,j));
			}
		} else {
			iteracion = longitud;
			compresion[copiados++] = 1;
			longitud = binarise(longitud-comp->match);
			posicion = binarise(posicion);
			for (int j = longpos-1; j>=0; j--) {
				compresion[copiados++] = posicion/(size_t)(pow(10,j));
				posicion = posicion%(size_t) pow(10,j);
			}
			for (int j = longlong-1; j>=0; j--) {
				compresion[copiados++] = longitud/(size_t)(pow(10,longlong-j));
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
	size_t tamaniofinal = ceil((double)copiados/(double)8);
	char* compresionfinal = calloc(tamaniofinal,1);
	for (size_t i = 0; i<tamaniofinal; i++) {
		size_t numero = 0;
		for (size_t j = 0; j<8; j++) {
			numero = numero*10 + compresion[i*8+j];
		}
		compresionfinal[i] = debinarise(numero);
	}
	free(compresion);
	return compresionfinal;
}

char* descomprimir(lz77* comp, char* src, size_t largo) {
	printf("\n");
	size_t longlong = ceil(log2(fmin(comp->taminsp,comp->tammem)-(comp->match)+1));
	size_t longpos = ceil(log2((comp->tammem)-(comp->match)+1));
	size_t largopar = longlong + longpos +1;
	for (size_t i = 0; i < (comp->tammem); i++) comp->memoria[i] = ' ';
	for (size_t i = 0; i < (comp->taminsp); i++) comp->inspeccion[i] = src[i];
	char* srcbits = calloc(largo,8);
	char* descomprimido = calloc(ceil((largo/largopar))*fmin(comp->taminsp,comp->tammem),8);
	for (size_t i = 0; i<largo; i++) {
		size_t fuente = binarisebyte(src[i]);
		printf("%08u ", fuente);
		for (size_t j = 0; j< 8; j++) {
			srcbits[i*8+j] = (size_t) fuente/(size_t) pow(10,7-j);
			fuente = (size_t) fuente%(size_t) pow(10,7-j);
		}	
	}
	size_t i = 0;
	size_t desc = 0;
	while (i<largo*8) {
		if (srcbits[i++] == 0) {
			size_t fuente = 0;
			for (size_t j = 0; j<8; j++) {
				fuente = fuente*10 + srcbits[i++];
			}
			for(size_t j = 0; j<comp->tammem-1; j++) {
				comp->memoria[j] = comp->memoria[j+1];
			}
			comp->memoria[comp->tammem-1] = fuente;
			descomprimido[desc++] = fuente;
		}	else {
			size_t posicion = 0;
			size_t longitud = 0;
			for(size_t j = 0; j<longpos; j++) {
				posicion = posicion * 10 + srcbits[i++];
			}
			posicion = debinarise(posicion);
			for(size_t j = 0; j<longlong; j++) {
				longitud = longitud * 10 + srcbits[i++];
			}
			longitud = debinarise(longitud) + comp->match;
			for (size_t j = 0; j<longitud; j++) {
				descomprimido[desc] = comp->memoria[posicion];
				for (size_t k = 0; k<comp->tammem-1; k++)	comp->memoria[k] = comp->memoria[k+1];
				comp->memoria[comp->tammem] = descomprimido[desc++];
			}	
		}	
	}	
	return descomprimido;
}					
