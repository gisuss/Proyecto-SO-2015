/** *********************************************************************** **/
/**                           CENTRO DE IMPRESION                           **/
/**                        @Jesus Romero  V-20753800                        **/
/**                        @Sauli Quirpa  V-                                **/
/**                        @Yaemil Flores V-                                **/
/**                                                                         **/
/** *********************************************************************** **/

/** PARA COMPILAR Y EJECUTAR:
 *  gcc centro.c -o centro -pthread
 *  ./centro
 * 
 * O TAMBIEN:
 * 
 *  gcc centro.c -o centro -lpthread
 *  ./centro
 * **/
 
#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

/****************  VARIABLES Y ESTRUCTURAS GLOBALES **************/

#ifndef N
#define N 40
#endif

typedef enum {false, true} bool;

typedef struct {
	int tinta;
	int identificador;
	sem_t libre;
}Impresora;

Impresora Centro[15];
pthread_mutex_t mutex_tecnico = PTHREAD_MUTEX_INITIALIZER;
sem_t libres;

/************  DECLARACION DE PROCEDIMIENTOS **************/

void inicializar_Centro();
void *funcion_Centro();
int requerir_impresora();
void liberar_impresora();
bool verificar_tinta(); //No hace falta (hasta ahora)
void recargar_tinta();

int main(void) {
	
	/************  DECLARACION DE VARIABLES **************/	
	
	pthread_t Usuarios[N];
	int i, err;
	
	/** Inicializa todos los recursos usados por las Impresoras. **/
	inicializar_Centro();
	srand (time(NULL));

	/************  CREACION DE HILOS USUARIO **************/
	for (i=0;i<N;i++) {
		//policy = rand() % 5+1;
		err = pthread_create( &Usuarios[i] , NULL , funcion_Centro , NULL );
		if (err != 0) {
		   printf("Error creando el hilo %d", i);
		   abort();
		}
	}
	
	pthread_join(Usuarios[0], NULL);        //Espera a que los hilos terminen antes de terminar el main
	
	exit(0);
}

void inicializar_Centro() {
	int i;
	
	for (i=0;i<15;i++) {
		Centro[i].tinta = 100;
		Centro[i].identificador = i;
		sem_init(&Centro[i].libre, 0, 0);
	}
	sem_init(&libres, 0, 15);
	//pthread_mutex_init (&mutex_tecnico, NULL); 
}

int requerir_impresora() {
	return 10;
}
