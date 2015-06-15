#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>
#include "packarch.h"
#include "heap.h"
#include "vector_dinamico.h"
#include "lectura.h"
#define ARCHIVO "/home/fvb/Desktop/Workspace/TP7506/Ejemplo_1.dat"
#define ARCHR "/home/fvb/Desktop/Workspace/TP7506/Ejemplo_1_r.dat"
#define ARCH0 "/home/fvb/Desktop/Workspace/TP7506/Ejemplo_1_o.dat"
#define TEMP "/home/fvb/Desktop/Workspace/TP7506"
#define REG "i4,i2,i8,sL,i1,d"
#define TAM_ID 4
#define INT8 "i8"
#define INT4 "i4"
#define INT2 "i2"
#define INT1 "i1"
#define STRL "sL"
#define STRD "sD"
#define FEC1 "d"
#define FEC2 "dt"
#define CARFINS "\0"
#define CARFINL "\n"
#define UMIN 5
#define EXT1 "csv"
#define EXT2 "_r.dat"
#define EXT3 "_o.dat"
#define EXT4 "lz77"
#define EXT5 "_d.dat"
#define CHUNK 8
#define MEM 6
#define INSP 5
#define MATCH 2
#define STRING "RAPATAPATAPARAPAPA"

size_t debinarise (size_t num) {
	return (num == 0 || num == 1 ? num : ((num%10) + 2*debinarise(num/10))); 
}

size_t binarise (size_t num) {
	return (num==0 || num==1 ? num : ((num%2)+10*binarise(num/2)));
}

char* crear_bitmap (char* bloque, size_t tam) {
	size_t binario,j;
	char* bitmap = calloc (1,tam*8);
	if (!bitmap) return NULL;
	for (size_t i = 0; i<tam; i++) {
		if (bloque[i]<0) binario = binarise(bloque[i]+256);
		else binario = binarise(bloque[i]);
		for (j=0; j<8; j++) {
			bitmap[8*i+j] = binario/pot(10,7-j);
			binario = binario-(binario/pot(10,7-j))*pot(10,7-j);
		}
	}
	return bitmap;
}

char* guardar_bitmap (char* bitmap, size_t tam) {
	size_t binario,j;
	char* bloque = calloc (1,tam);
	if (!bloque) return NULL;
	for (size_t i = 0; i<tam; i++) {
		binario=0;
		for (j=0; j<8; j++) {
			binario = binario*10 + bitmap[i*8+j];
		}
		bloque[i] = debinarise(binario);
	}
	return bloque;
}

size_t strtonum (char* palab) {
	size_t i;
	size_t numero = 0;
	if (!palab) return 0;
	for (i=0; i<strlen (palab); i++) {
		numero = numero * 10 + (palab[i] - '0');
	}
	return numero;
}

void print_test(char* name, bool result){
	printf("%s: %s\n", name, result? "OK" : "ERROR");
}

size_t reconocer_campo (char* campo) {
	if (strcmp(campo,FEC1)==0) return 1;
	if (strcmp(campo,FEC2)==0) return 2;
	if (strcmp(campo,INT1)==0) return 3;
	if (strcmp(campo,INT2)==0) return 4;
	if (strcmp(campo,INT4)==0) return 5;
	if (strcmp(campo,INT8)==0) return 6;
	if (strcmp(campo,STRL)==0) return 7;
	if (strcmp(campo,STRD)==0) return 8;
	return 0;
}

size_t medir_registro (char* source, vector_t*reg) {
	size_t i=0;
	size_t z=0;
	unsigned int id = *((unsigned int*)(source+i));
	if (id==0) return 0;
	i=i+4;
	for (z = 0; z<vector_obtener_cantidad(reg); z++) {
		switch (reconocer_campo(vector_obtener(reg,z))) {
			case 1:	{
				i=i+8;
				break;
			}	
			case 2: {
				i=i+15;
				break;
			}	
			case 3: {
				i++;
				break;
			}	
			case 4: {
				i=i+2;
				break;
			}	
			case 5: {
				i=i+4;
				break;
			}	
			case 6: {
				i=i+8;
				break;
			}	
			case 7: {
				i=i+((unsigned char) source[i])+1;
				break;
			}	
			case 8: {
				i=i+strlen((char*)(source+i))+1;
				break;
			}	
		}
	}
	return i;
}	

size_t contar_registros (char* bloque, packarch_t* pack) {
	size_t i=0;
	size_t largo=0;
	while (medir_registro(bloque+largo,registro_archivo(pack))!=0) {
		largo= largo+ medir_registro(bloque+largo,registro_archivo(pack));
		i++;
	}
	return i;		
}

size_t espacio_ocupado (char* source, packarch_t* pack) {
	size_t i=0;
	vector_t* reg = registro_archivo(pack);
	while (i < tamanio_bloque(pack) && medir_registro(source+i,reg)!=0) {
		i=i+medir_registro(source+i,reg);
	}
	return i;
}		

size_t espacio_disponible (char* source, packarch_t* pack) {
	return tamanio_bloque(pack) - espacio_ocupado(source, pack);		
}	

size_t copiar_registros (char* source, FILE* dest, packarch_t* data){
	size_t z = 0;
	size_t i = 0;
	size_t cantidad = 0;
	vector_t* reg = registro_archivo(data);
	while (i < tamanio_bloque(data)-TAM_ID ) {
		unsigned int idASCII = *((unsigned int*)(source +i));
		if (idASCII==0) return cantidad;
		cantidad ++;
		char* campo = malloc(11*sizeof(char));
		sprintf(campo,"%u",idASCII);
		fwrite(campo,strlen(campo)+1,1,dest);
		fwrite(";",1,1,dest);
		free(campo);
		i=i+4;
		for (z = 0; z<vector_obtener_cantidad(reg); z++) {
			switch (reconocer_campo(vector_obtener(reg,z))) {
				case 1: {
					char* campo= malloc(8*sizeof(char)+1);
					snprintf(campo,9,"%s",(char*)(source+i));
					fwrite(campo,strlen(campo)+1,1,dest);
					i=i+8;
					free(campo);
					break;
				}	
				case 2: {
					char* campo= malloc(15*sizeof(char)+1);
					snprintf(campo,16,"%s",(char*) (source+i));
					i=i+15;
					fwrite(campo,strlen(campo)+1,1,dest);
					free(campo);
					break;
				}
				case 3: {
					char* campo = malloc(4*sizeof(char));
					sprintf(campo,"%i",*(char*)(source+i));
					fwrite(campo,strlen(campo)+1,1,dest);
					i=i+1;
					free(campo);
					break;
				}	
				case 4: {
					char* campo = malloc(7*sizeof(char));
					sprintf(campo,"%i",*(short*) (source+i));
					fwrite(campo,strlen(campo)+1,1,dest);
					i=i+2;
					free(campo);
					break;
				}	
				case 5: {
					char* campo = malloc(12*sizeof(char));
					sprintf(campo,"%i",*(int*) (source+i));
					i=i+4;	 
					fwrite(campo,strlen(campo)+1,1,dest);
					free(campo);
					break;
				}	
				case 6: {
					char* campo = malloc(21*sizeof(char));
					sprintf(campo,"%lli",*((long long*)(source+i)));
					fwrite(campo,strlen(campo)+1,1,dest);
					i=i+8;
					free(campo);
					break;
				}	
				case 7: {
					char* campo = malloc(((unsigned char)source[i])+1);
					snprintf(campo, ((unsigned char) source[i])+1,"%s",(char*)(source+i+1));
					fwrite(campo, strlen(campo)+1, 1, dest);
					i= i+((unsigned char) source[i])+1;
					free(campo);
					break;
				}	
				case 8: {
					fwrite(source+i,strlen((char*)(source+i))+1,1,dest);
					i= i+strlen((char*)(source+i))+1;
					break;
				}
			}	
			fwrite (";",1,1,dest);
		}
		fwrite (CARFINL,2,1,dest);	
	}
	return cantidad;
}	

bool exportar (packarch_t* arch){
	char* nuevo = calloc (1,strlen(nombre_archivo(arch)) +1);
	if (!nuevo) return false;
	strncpy (nuevo, nombre_archivo (arch), strlen(nombre_archivo (arch))-3);
	strcat (nuevo,EXT1);
	printf(" %s\n",nuevo);
	char* bloque = malloc(tamanio_bloque(arch)*sizeof(char));
	if (!bloque) {
		free (nuevo);
		return false;
	}	
	FILE* viejo = fopen(nombre_archivo(arch),"r");
	fseek (viejo, 0, SEEK_SET);
	fread (bloque, tamanio_bloque(arch), 1, viejo);
	char* bitmap = crear_bitmap (bloque, tamanio_bloque(arch));
	if (!bitmap) {
		free(nuevo);
		free(bloque);
		return false;
	}
	FILE* archnew = fopen(nuevo,"w");
	for (size_t i=0; i < tamanio_bloque(arch); i++) {
		if (bitmap[i] == 1) {
			fseek(viejo,(i+1)*tamanio_bloque(arch),SEEK_SET);
			fread(bloque,tamanio_bloque(arch),1,viejo);
			copiar_registros(bloque, archnew, arch);
		}
	}
	fclose(viejo);
	fclose(archnew);
	free(nuevo);
	free(bloque);
	free(bitmap);				
	return true;
}

char* rearmar_bitmap (char* bitmap,size_t tam){
	char* bloque = malloc(tam);
	size_t z=0;
	for (size_t i= 0; i<tam; i++) {
		bloque[i] = 0;
		for (size_t j=0; j<8; j++) {
			bloque[i]= bloque[i]*2+ bitmap[z++];
		}
	}
	return bloque;	
}	

void relocalizar_registros(packarch_t* pack, FILE* src, FILE* dest, size_t umbral) {
	size_t bmin=1;
	size_t bmax;
	char* bloque = malloc (tamanio_bloque(pack));
	char* bloque2 = malloc (tamanio_bloque(pack));
	fseek(src,0,SEEK_SET);
	fread(bloque, tamanio_bloque(pack), 1, src);
	char* bitmap = crear_bitmap(bloque, tamanio_bloque(pack));
	for (size_t i = 0; i<tamanio_bloque(pack); i++) if (bitmap[i] == 1) bmax = i+1; //bitmap creado
	size_t cantidad= 0;
	fseek(dest,0,SEEK_SET);
	fwrite(bloque,tamanio_bloque(pack),1,dest);
	for (size_t i=1; i<= bmax; i++) {
		fseek(src,tamanio_bloque(pack)*i,SEEK_SET);
		fread(bloque,tamanio_bloque(pack),1,src);
		cantidad=cantidad+contar_registros(bloque,pack);
		printf (" %02i %02u |", i, contar_registros(bloque,pack));
		fseek(dest,tamanio_bloque(pack)*i,SEEK_SET);
		fwrite(bloque,tamanio_bloque(pack),1,dest);
	}
	printf("\n");
	printf(" Cantidad de registros: %u\n", cantidad);
	cantidad =0;
	while(bmin<bmax) {
		fseek(dest,tamanio_bloque(pack)*bmin,SEEK_SET);
		fread(bloque,tamanio_bloque(pack),1,dest);
		fseek(dest,tamanio_bloque(pack)*bmax,SEEK_SET);
		fread(bloque2,tamanio_bloque(pack),1,dest);
		vector_t* reg = registro_archivo(pack);
		size_t largo=medir_registro(bloque2,reg);
		if (largo==0) {
			bitmap[bmax-1]=0;
			bmax--;
		} else {
			size_t espacio=espacio_disponible(bloque,pack);
			if (espacio<umbral) {
				bitmap[bmin-1]=1;
				bmin++;
			} else {
				if (espacio>=largo) {
					for (size_t i=0; i<largo; i++) bloque[tamanio_bloque(pack)-espacio+i]=bloque2[i];	//Copio al b1 un reg
					size_t aux = espacio_ocupado(bloque2, pack)-largo; // Busco el nuevo espacio ocupado b2
					for (size_t i=0; i<aux; i++) bloque2[i]=bloque2[largo+i];// Justifico a la izquierda al b2
					for (size_t i=0; i<tamanio_bloque(pack)-aux; i++) bloque2[aux+i]=0; //Borro
					fseek(dest,tamanio_bloque(pack)*bmin,SEEK_SET);
					fwrite(bloque,tamanio_bloque(pack),1,dest);
					fseek(dest,tamanio_bloque(pack)*bmax,SEEK_SET);
					fwrite(bloque2,tamanio_bloque(pack),1,dest);	//Guardo los cambios
					bitmap[bmin-1]=1;
					bmin=1;	
					cantidad++;				
				} else {
					bitmap[bmin-1]=1;
					bmin++;
				}
			}
		}
	}
	printf(" Registros copiados: %u\n",cantidad);
	cantidad =0;
	for (size_t i=1; i<= bmax; i++) {
		fseek(dest,tamanio_bloque(pack)*i,SEEK_SET);
		fread(bloque,tamanio_bloque(pack),1,dest);
		printf(" %02i %02u |", i, contar_registros(bloque,pack)); 
		cantidad=cantidad+contar_registros(bloque,pack);
	}
	printf("\n");
	free(bloque);
	bloque= rearmar_bitmap(bitmap,tamanio_bloque(pack));
	fseek(dest,0,SEEK_SET);
	fwrite(bloque,tamanio_bloque(pack),1, dest);
	free(bloque);
	free(bloque2);
	free(bitmap);				
}

bool relocalizar (packarch_t* arch, size_t mini){
	if (mini>=100) {
		printf("Umbral no válido.\n");
		return false;
	}	
	char* nuevo = calloc (1,strlen(nombre_archivo(arch)) +3);
	if (!nuevo) return false;
	strncpy (nuevo, nombre_archivo (arch), strlen(nombre_archivo (arch))-4);
	strcat (nuevo,EXT2);
	printf(" %s\n",nuevo);
	printf(" Umbral: %02u%%\n", mini);
	size_t umbral = tamanio_bloque(arch) * mini / 100;
	FILE* archivo = fopen(nombre_archivo(arch),"r");
	FILE* reloc = fopen(nuevo,"w+");
	relocalizar_registros(arch,archivo,reloc,umbral);
	fclose(archivo);
	fclose(reloc);
	free(nuevo);
	return true;
}

int auxiliar_cmp (const void* reg1, const void* reg2) {
	if (*(unsigned int*) reg1 < *(unsigned int*) reg2) return -1;
	return 1;
}

size_t generar_archivos(packarch_t* pack, size_t chunk, char* carpeta) {
	size_t n = 0;
	char* bloque = calloc (1,tamanio_bloque(pack));
	FILE* src = fopen (nombre_archivo(pack), "r");
	fread (bloque, tamanio_bloque(pack), 1, src);
	char* bitmap = crear_bitmap(bloque, tamanio_bloque(pack));
	for(size_t i=0; i<tamanio_bloque(pack); i++) if (bitmap[i]==1){
			fseek(src,(i+1)*tamanio_bloque(pack),SEEK_SET);
			fread(bloque, tamanio_bloque(pack),1, src);
			n= n+contar_registros(bloque,pack);
	}
	free(bitmap);
	size_t regxarch = n/chunk; //Calculo la cantidad de registros por archivo.
	size_t sobrantes = n%chunk;
	size_t desplazamiento = 0;
	fseek(src,tamanio_bloque(pack), SEEK_SET); //Me posiciono en el primer bloque
	fread(bloque,tamanio_bloque(pack),1,src); //Lo levanto a memoria
	for (n = 1; n<=chunk; n++) {	//Ciclo de creacion de archivos
		void** registros = calloc(regxarch+1, sizeof(char*)); //Armo un vector de registros
		for (size_t indice = 0; indice<regxarch; indice++){ //Ciclo de rellenado del vector
			while (*((unsigned int*) (bloque+desplazamiento)) == 0) { //Si apunta a 0 cambio de bloque
				fread(bloque,tamanio_bloque(pack),1,src); //Levanto el siguiente bloque
				desplazamiento = 0; //Reseteo el desplazamiento
			}
			size_t largo = medir_registro(bloque+desplazamiento,registro_archivo(pack));
			char* registro = calloc(1,largo);
			for (size_t i=0; i<largo; i++)	registro[i]=*(char*)(bloque+desplazamiento+i);
			registros[indice] = registro;
			desplazamiento = desplazamiento + largo;
		}
		if(sobrantes>0) {
			while(*((unsigned int*) (bloque+desplazamiento)) == 0) {
				fread(bloque,tamanio_bloque(pack),1,src);
				desplazamiento=0;
			}
			sobrantes--;	
			if(*((unsigned int*) (bloque+desplazamiento)) == 219) printf("219\n");	
			size_t largo = medir_registro(bloque+desplazamiento,registro_archivo(pack));
			char* registro = calloc(1,largo);
			for (size_t i=0; i<largo; i++)	registro[i]=*(char*)(bloque+desplazamiento+i);
			desplazamiento = desplazamiento + largo;
			registros[regxarch] = registro;
			heap_sort(registros,regxarch+1,auxiliar_cmp); //Ordeno
		}	else {
			registros[regxarch]=NULL;
			heap_sort(registros,regxarch,auxiliar_cmp); //Ordeno
			}
		char* newbitmap = calloc(1,tamanio_bloque(pack)*8);
		size_t ibitmap = 0;		
		size_t ocupado = 0;
		char* nombretemp = calloc(1,strlen(carpeta)+20);
		sprintf(nombretemp, "%s/temp%i.dat", carpeta, n);
		FILE* dest = fopen(nombretemp,"w+");
		fwrite (bloque, tamanio_bloque(pack), 1, dest);
		for (size_t i = 0; i<=regxarch; i++) {
			if (registros[i]) {
				if (ocupado+medir_registro(registros[i],registro_archivo(pack)) > tamanio_bloque(pack)) {
					ibitmap++;
					fseek(dest,tamanio_bloque(pack)*(ibitmap+1),SEEK_SET);
				}
				newbitmap[ibitmap]=1;
				fwrite(registros[i],medir_registro(registros[i],registro_archivo(pack)),1,dest);
				ocupado = ocupado+medir_registro(registros[i],registro_archivo(pack));
			}
		}
		char* regfinal = calloc(1,4);
		for (size_t i= 0; i<4; i++) regfinal[i]=0;
		fwrite(regfinal,4,1,dest);
		bitmap = guardar_bitmap(newbitmap, tamanio_bloque(pack));
		fseek(dest,0,SEEK_SET);	
		fwrite(bitmap,tamanio_bloque(pack),1,dest);
		for (size_t i = 0; i<=regxarch; i++) if (registros[i]) free (registros[i]);
		free(nombretemp);
		fclose(dest);
		free(regfinal);
		free(bitmap);
		free(registros);
		free(newbitmap);
	}
	free(bloque);
	fclose(src);
	return n;
}	

void copyfile (FILE* dest, FILE* src, packarch_t* pack) {
	char* bloque = calloc(1,tamanio_bloque(pack));
	for (size_t i = 0; i<=tamanio_bloque(pack); i++) {
		fread(bloque,tamanio_bloque(pack),1,src);
		fwrite(bloque,tamanio_bloque(pack),1,dest);
	}
	free (bloque);
	fclose(dest);
	fclose(src);		
}

void mergefiles (char* src1, char* src2, char* dest, packarch_t* pack, size_t umbral) {
	size_t bmax1 = 0;
	size_t bmax2 = 0;
	size_t i1 = 0;
	size_t i2 = 0;
	size_t i3 = 0;
	FILE* source1 = fopen(src1,"r");
	FILE* source2 = fopen(src2,"r");
	FILE* final = fopen(dest,"w");
	if (!source1) return;
	if (!source2) {
		copyfile(final,source1,pack);
		return;
	}
	char* bloque1 = calloc(1,tamanio_bloque(pack));
	fread(bloque1,tamanio_bloque(pack),1,source1);
	char* bloque2 = calloc(1,tamanio_bloque(pack));
	fread(bloque2,tamanio_bloque(pack),1,source2);
	char* bloquef = calloc(1,tamanio_bloque(pack));
	fwrite(bloquef,tamanio_bloque(pack),1,final); //Armo espacio para el bitmap
	char* bitmap1 = crear_bitmap(bloque1,tamanio_bloque(pack));
	for (size_t i = 0; i<tamanio_bloque(pack)*8; i++) if(bitmap1[i]==1) bmax1 = i;
	char* bitmap2 = crear_bitmap(bloque2,tamanio_bloque(pack));
	for (size_t i = 0; i<tamanio_bloque(pack)*8; i++) if(bitmap2[i]==1) bmax2 = i;
	char* bitmapf = calloc(8,tamanio_bloque(pack));
	fread(bloque1,tamanio_bloque(pack),1,source1); //Empiezo la lectura de la zona de datos
	fread(bloque2,tamanio_bloque(pack),1,source2); //Para ambos archivos
	size_t copiado1=0;
	size_t copiado2=0;
	size_t copiadototal =0;
	while (i1<=bmax1 && i2<=bmax2) {
		if(*(unsigned int*) (bloque1+copiado1) < *(unsigned int*) (bloque2+copiado2) && 
			*(unsigned int*) (bloque1+copiado1) !=0){
			size_t largo = medir_registro(bloque1+copiado1,registro_archivo(pack));
			if (largo > espacio_disponible(bloquef,pack)-umbral) {
				fwrite(bloquef,tamanio_bloque(pack),1,final);
				for (size_t i = 0; i<tamanio_bloque(pack); i++) bloquef[i] = 0;
				copiadototal=0;
				i3++;
			}
			bitmapf[i3]=1;	
			for (size_t i = 0; i<largo; i++) bloquef[copiadototal+i] = bloque1[copiado1+i];
			copiado1 = copiado1 +largo;
			copiadototal = copiadototal + largo;
		} else if(*(unsigned int*) (bloque1+copiado1) > *(unsigned int*) (bloque2+copiado2) && 
			*(unsigned int*) (bloque2+copiado2) !=0)	{
			size_t largo = medir_registro(bloque2+copiado2,registro_archivo(pack));
			if (largo > espacio_disponible(bloquef,pack)-umbral) {
				fwrite(bloquef,tamanio_bloque(pack),1,final);
				for (size_t i = 0; i<tamanio_bloque(pack); i++) bloquef[i] = 0;
				copiadototal=0;
				i3++;
			}
			bitmapf[i3]=1;	
			for (size_t i = 0; i<largo; i++) bloquef[copiadototal+i] = bloque2[copiado2+i];
			copiado2 = copiado2 +largo;
			copiadototal = copiadototal + largo;
		}
		while (*(unsigned int*) (bloque1+copiado1)==0 && i1<=bmax1) {
			fread(bloque1,tamanio_bloque(pack),1,source1);
			copiado1=0;
			i1++;
		}	
		while (*(unsigned int*) (bloque2+copiado2)==0 && i2<=bmax2) {
			fread(bloque2,tamanio_bloque(pack),1,source2);
			copiado2=0;
			i2++;
		}	
	}	
	while (i1<=bmax1) {
		if(*(unsigned int*) (bloque1+copiado1) !=0){
			size_t largo = medir_registro(bloque1+copiado1,registro_archivo(pack));
			if (largo > espacio_disponible(bloquef,pack)-umbral) {
				fwrite(bloquef,tamanio_bloque(pack),1,final);
				for (size_t i = 0; i<tamanio_bloque(pack); i++) bloquef[i] = 0;
				copiadototal=0;
				i3++;
			}
			bitmapf[i3]=1;	
			for (size_t i = 0; i<largo; i++) bloquef[copiadototal+i] = bloque1[copiado1+i];
			copiado1 = copiado1 +largo;
			copiadototal = copiadototal + largo;
		}
		while (*(unsigned int*) (bloque1+copiado1)==0 && i1<=bmax1) {
			fread(bloque1,tamanio_bloque(pack),1,source1);
			copiado1=0;
			i1++;
		}
	}
	while (i2<=bmax2) {
		if(*(unsigned int*) (bloque2+copiado2) !=0){
			size_t largo = medir_registro(bloque2+copiado2,registro_archivo(pack));
			if (largo > espacio_disponible(bloquef,pack)-umbral) {
				fwrite(bloquef,tamanio_bloque(pack),1,final);
				for (size_t i = 0; i<tamanio_bloque(pack); i++) bloquef[i] = 0;
				copiadototal=0;
				i3++;
			}
			bitmapf[i3]=1;	
			for (size_t i = 0; i<largo; i++) bloquef[copiadototal+i] = bloque2[copiado2+i];
			copiado2 = copiado2 +largo;
			copiadototal = copiadototal + largo;
		}
		while (*(unsigned int*) (bloque2+copiado2)==0 && i2<=bmax2) {
			fread(bloque2,tamanio_bloque(pack),1,source2);
			copiado2=0;
			i2++;
		}
	}
	fwrite(bloquef,tamanio_bloque(pack),1,final);
	for (size_t i = 0; i<tamanio_bloque(pack); i++) bloquef[i] = 0;
	fwrite(bloquef,tamanio_bloque(pack),1,final);
	free(bloque1);
	free(bloque2);
	free(bloquef);
	free(bitmap1);
	free(bitmap2);
	bloquef = guardar_bitmap(bitmapf,tamanio_bloque(pack));
	fseek(final, 0, SEEK_SET);
	fwrite(bloquef, tamanio_bloque(pack),1, final);
	free(bloquef);
	free(bitmapf);
	fclose(final);
	fclose(source1);
	fclose(source2);			
}

bool unificar(size_t umbral, size_t chunk, packarch_t* pack, char* carpeta) {
	char* temp1;
	size_t original = chunk;
	vector_t* viejo = vector_crear(free);
	vector_t* nuevo = vector_crear(NULL);
	for(size_t i = 1; i<=original; i++) {
		temp1 = calloc(1,strlen(carpeta)+20);
		sprintf(temp1, "%s/temp%i.dat", carpeta, i);
		vector_guardar(viejo,temp1);
	}
	while (original>2) {
		for(size_t i=original+1; i<=((3*original)/2)+1;i++) {
			temp1 = calloc(1,strlen(carpeta)+20);
			sprintf(temp1, "%s/temp%i.dat", carpeta, i);
			vector_guardar(nuevo,temp1);
		}
		size_t cantidad=vector_obtener_cantidad(viejo);
		for(size_t i=0; i<cantidad;i=i+2) {
			mergefiles(vector_obtener(viejo,i),vector_obtener(viejo,i+1),vector_obtener(nuevo,i/2),pack,umbral);
		}
		for(size_t i=0;i<cantidad;i++) vector_borrar_posicion(viejo,0);
		cantidad = vector_obtener_cantidad(nuevo);
		for(size_t i=0;i<cantidad;i++) vector_guardar(viejo,vector_obtener(nuevo,i));
		vector_destruir(nuevo);
		nuevo=vector_crear(NULL);
		original=original/2;
	}
	char* archfin = calloc (1,strlen(nombre_archivo(pack)) +3);
	if (!archfin) return false;
	strncpy (archfin, nombre_archivo (pack), strlen(nombre_archivo (pack))-4);
	strcat (archfin,EXT3);
	printf ("%s\n",archfin);
	mergefiles(vector_obtener(viejo,0),vector_obtener(viejo,1),archfin,pack,umbral);
	vector_destruir(viejo);
	vector_destruir(nuevo);
	free(archfin);
	for (size_t i=1; i<=((3*chunk)/2)+1; i++) {
		temp1 = calloc(1,strlen(carpeta)+20);
		sprintf(temp1, "%s/temp%i.dat", carpeta, i);
		remove(temp1);
		free(temp1);
	}
	return true;		
}	

bool reordenar (packarch_t* pack, size_t mini, size_t chunk, char* carpeta) {
	generar_archivos(pack, chunk,carpeta);
	unificar(tamanio_bloque(pack)*mini/100,chunk,pack,carpeta);
	return true;
}

char* comprimir (char* src, size_t diccionario, size_t inspeccion, size_t match) {
	size_t longitudes = ceil(log2(fmin(inspeccion,diccionario)-match+1));
	size_t posiciones = ceil(log2(diccionario-match+1));
	size_t largopar = longitudes + posiciones + 1;
	printf("%u\n", largopar);
	return NULL;
}		

int main () {
	comprimir(STRING, MEM, INSP, MATCH);
	return 0;
}	
