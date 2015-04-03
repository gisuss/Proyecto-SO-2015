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
#define M 15
#endif

typedef enum {false, true} bool;

typedef struct {
	int tinta;
	int identificador;
	sem_t ocupada;
}Impresora;

Impresora Centro[M];
FILE *output;
int marca, Impresoras_libres;
pthread_mutex_t mutex_tecnico = PTHREAD_MUTEX_INITIALIZER;
sem_t mutex_impresoras_libres;
sem_t mutex_Centro;

/************  DECLARACION DE PROCEDIMIENTOS **************/

void inicializar_Centro ();
void *funcion_Centro ();
void *Presentando ();
int requerir_impresora ();
void liberar_impresora (int id_impresora);
bool verificar_tinta (int id_impresora);
void recargar_tinta (int id_impresora);

int main(void) {
	
	/************  DECLARACION DE VARIABLES **************/	
	
	pthread_t Usuarios[N];
	pthread_t Intro;
	pthread_attr_t atributos;
	int i, err;
	
	/** Inicializa todos los recursos usados por las Impresoras. **/
	inicializar_Centro ();
	srand (time(NULL));
	
	/************  CREACION DE HILO BIENVENIDA *************/
	pthread_create( &Intro , NULL , Presentando , NULL );
	
	/************  CREACION DE HILOS USUARIO  **************/
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
	fclose(output);
	pthread_join(Intro, NULL);
	system("reset");
	
	return (0);
}

void *funcion_Centro () {
	int id_Impresora;
	
	id_Impresora = requerir_impresora();
	
	if (!(verificar_tinta(id_Impresora)))
		recargar_tinta(id_Impresora);
	
	liberar_impresora(id_Impresora);
	
	pthread_exit(NULL);
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
	output = fopen("bitacora.out" , "w");
	
	fprintf(output, "*** Centro de Impresion y Tecnico Listos para empezar ***\n\n");
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
			sem_wait(&Centro[id_impre].ocupada);
		}
		i++;
	}
	sem_post(&mutex_Centro); //Libera SC Centro
	
	fprintf(output, "[Requerida] Impresora %d\n\n", id_impre);
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
	
	fprintf(output, "[Liberada] Impresora %d\n\n", id_impresora);
}

bool verificar_tinta (int id_impresora) {
	bool band2;
	
	fprintf(output, "[Verificando] Tinta Impresora %d\n", id_impresora);
	if (Centro[id_impresora].tinta > 0) {
		fprintf(output, "\tNivel de Tinta = %d \t[OK]\n\n", Centro[id_impresora].tinta);
		band2 = true;
	}else{
		fprintf(output, "\tNivel de Tinta = %d \t[NO OK]\n\n", Centro[id_impresora].tinta);
		band2 = false;
	}
	
	return (band2);
}

void recargar_tinta (int id_impresora) {
	pthread_mutex_lock(&mutex_tecnico);
	
	fprintf(output, "[Recargando] Tinta en Impresora: %d\n\t<Tecnico Trabajando> \t(...)\n\t¡Tinta Recargada.!\n\n", id_impresora);
	Centro[id_impresora].tinta = 100;
	sleep(2);
	
	pthread_mutex_unlock(&mutex_tecnico);
}

void *Presentando () {
	printf("\e[43;1m/**                          *************************                      **/\e[m\n");
	printf("\e[43;1m/****************************** CENTRO DE IMPRESION **************************/\e[m\n");
	printf("\e[43;1m/**                          *************************                      **/\e[m\n");
	printf("\e[43;1m/**                          @Jesus Romero  V-20753800                      **/\e[m\n");
	printf("\e[43;1m/**                          @Sauli Quirpa  V-25134099                      **/\e[m\n");
	printf("\e[43;1m/**                          @Yaemil Flores V-23419581                      **/\e[m\n");
	printf("\e[43;1m/**                                                                         **/\e[m\n");
	printf("\e[43;1m/**      Toda Informacion, Estructuras, Funciones/Procedimientos            **/\e[m\n");
	printf("\e[43;1m/**       y Lineas de Codigo contenidas en este proyecto, estan             **/\e[m\n");
	printf("\e[43;1m/**    protegidas mediante derechos de autor. Cualquier distribucion,       **/\e[m\n");
	printf("\e[43;1m/**       reproduccion y modificacion para fines personales sin el          **/\e[m\n");
	printf("\e[43;1m/**          debido consentimiento de los respectivos miembros              **/\e[m\n");
	printf("\e[43;1m/**                  y compañia, sera objeto de sancion.                    **/\e[m\n");
	printf("\e[43;1m/**                                                                         **/\e[m\n");
	printf("\e[43;1m/**     Copyright © 2014-2015 SISTEMAS OPERATIVOS™, All Rights Reserved.    **/\e[m\n");
	printf("\e[43;1m/*****************************************************************************/\e[m\n");
	
	sleep(5);
	system("reset");
	
	if (M == 15) {
		
		printf("\n\n\n\n\n\n\n\t\t\t             \e[36m<\e[m \e[42;1mC\e[m A R G A N D O \e[36m>\e[m           \n\n");
		sleep(3);
		system("clear");
		printf("\n\n\n\n\n\n\n\t\t\t           \e[36m< <\e[m \e[42;1mC A\e[m R G A N D O \e[36m> >\e[m         \n\n");
		sleep(3);
		system("clear");
		printf("\n\n\n\n\n\n\n\t\t\t         \e[36m< < <\e[m \e[42;1mC A R\e[m G A N D O \e[36m> > >\e[m       \n\n");
		sleep(3);
		system("clear");
		printf("\n\n\n\n\n\n\n\t\t\t       \e[36m< < < <\e[m \e[42;1mC A R G\e[m A N D O \e[36m> > > >\e[m     \n\n");
		sleep(3);
		system("clear");
		printf("\n\n\n\n\n\n\n\t\t\t     \e[36m< < < < <\e[m \e[42;1mC A R G A\e[m N D O \e[36m> > > > >\e[m   \n\n");
		sleep(3);
		system("clear");
		printf("\n\n\n\n\n\n\n\t\t\t   \e[36m< < < < < <\e[m \e[42;1mC A R G A N\e[m D O \e[36m> > > > > >\e[m \n\n");
		sleep(3);
		system("clear");
		printf("\n\n\n\n\n\n\n\t\t\t \e[36m< < < < < < <\e[m \e[42;1mC A R G A N D O\e[m \e[36m> > > > > > >\e[m \n\n");
		sleep(4);
		
	}else if (M > 0 && M <= 5) {
	
		printf("\n\n\n\n\n\n\n\t\t\t             \e[36m<\e[m \e[42;1mC\e[m A R G A N D O \e[36m>\e[m           \n\n");
		sleep(12);
		system("clear");
		printf("\n\n\n\n\n\n\n\t\t\t           \e[36m< <\e[m \e[42;1mC A\e[m R G A N D O \e[36m> >\e[m         \n\n");
		sleep(12);
		system("clear");
		printf("\n\n\n\n\n\n\n\t\t\t         \e[36m< < <\e[m \e[42;1mC A R\e[m G A N D O \e[36m> > >\e[m       \n\n");
		sleep(12);
		system("clear");
		printf("\n\n\n\n\n\n\n\t\t\t       \e[36m< < < <\e[m \e[42;1mC A R G\e[m A N D O \e[36m> > > >\e[m     \n\n");
		sleep(12);
		system("clear");
		printf("\n\n\n\n\n\n\n\t\t\t     \e[36m< < < < <\e[m \e[42;1mC A R G A\e[m N D O \e[36m> > > > >\e[m   \n\n");
		sleep(12);
		system("clear");
		printf("\n\n\n\n\n\n\n\t\t\t   \e[36m< < < < < <\e[m \e[42;1mC A R G A N\e[m D O \e[36m> > > > > >\e[m \n\n");
		sleep(12);
		system("clear");
		printf("\n\n\n\n\n\n\n\t\t\t   \e[36m< < < < < < <\e[m \e[42;1mC A R G A N D\e[m O \e[36m> > > > > > >\e[m \n\n");
		sleep(12);
		system("clear");
		printf("\n\n\n\n\n\n\n\t\t\t \e[36m< < < < < < <\e[m \e[42;1mC A R G A N D O\e[m \e[36m> > > > > > >\e[m \n\n");
		sleep(13);
		
	}else if (M > 5 && M < 15) {
		
		printf("\n\n\n\n\n\n\n\t\t\t             \e[36m<\e[m \e[42;1mC\e[m A R G A N D O \e[36m>\e[m           \n\n");
		sleep(9);
		system("clear");
		printf("\n\n\n\n\n\n\n\t\t\t           \e[36m< <\e[m \e[42;1mC A\e[m R G A N D O \e[36m> >\e[m         \n\n");
		sleep(9);
		system("clear");
		printf("\n\n\n\n\n\n\n\t\t\t         \e[36m< < <\e[m \e[42;1mC A R\e[m G A N D O \e[36m> > >\e[m       \n\n");
		sleep(9);
		system("clear");
		printf("\n\n\n\n\n\n\n\t\t\t       \e[36m< < < <\e[m \e[42;1mC A R G\e[m A N D O \e[36m> > > >\e[m     \n\n");
		sleep(9);
		system("clear");
		printf("\n\n\n\n\n\n\n\t\t\t     \e[36m< < < < <\e[m \e[42;1mC A R G A\e[m N D O \e[36m> > > > >\e[m   \n\n");
		sleep(9);
		system("clear");
		printf("\n\n\n\n\n\n\n\t\t\t   \e[36m< < < < < <\e[m \e[42;1mC A R G A N\e[m D O \e[36m> > > > > >\e[m \n\n");
		sleep(9);
		system("clear");
		printf("\n\n\n\n\n\n\n\t\t\t \e[36m< < < < < < <\e[m \e[42;1mC A R G A N D O\e[m \e[36m> > > > > > >\e[m \n\n");
		sleep(7);
	}
	
	pthread_exit(NULL);
}
