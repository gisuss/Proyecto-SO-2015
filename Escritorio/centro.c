/**                          *************************                      **/
/****************************** CENTRO DE IMPRESION **************************/
/**                          *************************                      **/
/**                          @Jesus Romero  V-20753800                      **/
/**                          @Sauli Quirpa  V-25134099                      **/
/**                          @Yaemil Flores V-23419581                      **/
/**                                                                         **/
/*****************************************************************************/

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
#include <unistd.h>

/****************  VARIABLES Y ESTRUCTURAS GLOBALES **************/

#ifndef N
#define N 300
#endif

#ifndef M
#define M 3
#endif

typedef enum {false, true} bool;

typedef struct {
	int tinta;
	int identificador;
	sem_t ocupada;
}Impresora;

Impresora Centro[M];
int marca, Impresoras_libres;
pthread_mutex_t mutex_tecnico = PTHREAD_MUTEX_INITIALIZER;
sem_t mutex_impresoras_libres;
sem_t mutex_Centro;

/************  DECLARACION DE PROCEDIMIENTOS **************/

void inicializar_Centro ();
void *funcion_Centro ();
int requerir_impresora ();
void liberar_impresora (int id_impresora);
bool verificar_tinta (int id_impresora);
void recargar_tinta (int id_impresora);

int main(void) {
	
	/************  DECLARACION DE VARIABLES **************/	
	
	pthread_t Usuarios[N];
	pthread_attr_t atributos;
	int i, err;
	
	/** Inicializa todos los recursos usados por las Impresoras. **/
	inicializar_Centro ();
	srand (time(NULL));

	/************  CREACION DE HILOS USUARIO **************/
	for (i=0;i<N;i++) {
		
		pthread_attr_init (&atributos);
		pthread_attr_setschedpolicy ( &atributos, (rand() % 5+1) );
		
		err = pthread_create( &Usuarios[i] , &atributos , funcion_Centro , NULL );
		if (err != 0) {
		   printf("Error creando el hilo %d\n", i);
		   abort();
		}
	}
	
	while (marca > 0);
	
	exit(0);
}

void *funcion_Centro () {
	int id_Impresora;
	bool band1;
	
	id_Impresora = requerir_impresora();
	band1 = verificar_tinta(id_Impresora);
	if (band1 == false) {
		printf("\tNivel de Tinta = %d \t[NO OK]\n", Centro[id_Impresora].tinta);
		recargar_tinta(id_Impresora);
	}else{
		printf("\tNivel de Tinta = %d \t[OK]\n", Centro[id_Impresora].tinta);
	}
	liberar_impresora(id_Impresora);
	
	pthread_exit((void *)"Ya esta\n");
}

void inicializar_Centro () {
	int i;
	
	for (i=0;i<M;i++) {
		Centro[i].tinta = 100;
		Centro[i].identificador = i;
		sem_init(&Centro[i].ocupada, 0, 1);
	}
	sem_init(&mutex_Centro, 0, 1);
	sem_init(&mutex_impresoras_libres, 0, 1);
	Impresoras_libres = M;
	marca = N;
	
	printf("\n*** Centro de Impresion y Tecnico Listos para empezar. ***\n\n");
}

int requerir_impresora () {
	int i = 0, id_impre, sval;
	bool band = true;
	
	while (Impresoras_libres < 1) {/** Esperar Impresora Disponible **/}
	
	sem_wait(&mutex_impresoras_libres);
	Impresoras_libres--;
	sem_post(&mutex_impresoras_libres);
	
	sem_wait(&mutex_Centro); //Proteje SC Centro
	while (band && (i >= 0) && (i < M)) {
		if ((sem_getvalue(&Centro[i].ocupada , &sval) == 0 ) && ( sval == 1 ) ) {
			band = false;
			id_impre = i;
		}
		i++;
	}
	sem_post(&mutex_Centro); //Libera SC Centro
	
	sem_wait(&Centro[id_impre].ocupada);
	printf("[Requerida] Impresora %d.\n", id_impre);
	sleep(1);
	
	return (id_impre);
}

void liberar_impresora (int id_impresora) {
	
	sem_wait(&mutex_impresoras_libres);
	Centro[id_impresora].tinta -= 5;
	marca--;
	Impresoras_libres++;
	sem_post(&mutex_impresoras_libres);
	
	sem_post(&Centro[id_impresora].ocupada);
	
	printf("[Liberada] Impresora %d.\n", id_impresora);
}

bool verificar_tinta (int id_impresora) {
	bool band2;
	
	printf("[Verificando] Tinta Impresora %d\n", id_impresora);
	if (Centro[id_impresora].tinta > 0) {
		band2 = true;
	}else{
		band2 = false;
	}
	
	return (band2);
}

void recargar_tinta (int id_impresora) {
	pthread_mutex_lock(&mutex_tecnico);
	
	printf("[Recargando] Tinta en Impresora: %d.\n\t<Tecnico Trabajando> \t(...)\n\t¡Tinta Recargada.!\n", id_impresora);
	Centro[id_impresora].tinta = 100;
	sleep(2);
	
	pthread_mutex_unlock(&mutex_tecnico);
}
