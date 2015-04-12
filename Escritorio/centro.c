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

#ifndef N																/** Cantidad de Thread de Usuarios **/
#define N 300
#endif

#ifndef M																/** Cantidad de Impresoras **/
#define M 7
#endif

#ifndef MAX																/** Variable referencial **/
#define MAX 10000
#endif

typedef enum {false, true} bool;

typedef struct {
	int tinta;															/** Indica nivel de Tinta **/
	int identificador;													/** Identificador de Impresora **/
	int uso;															/** Cantidad de veces que se ha usado la Impresora **/
	sem_t ocupada;														/** Semaforo que indica si esta libre o no la Impresora **/
}Impresora;

Impresora Centro[M];													/** Arreglo de Impresoras. Representa un Recurso Critico **/
FILE *output;															/** Archivo de Salida **/
int marca, Impresoras_libres; 											/** Recurso Critico "Impresoras_libres" **/
pthread_mutex_t mutex_tecnico = PTHREAD_MUTEX_INITIALIZER;				/** Semaforo Binario, Tecnico **/
pthread_mutex_t mutex_requerido = PTHREAD_MUTEX_INITIALIZER;			/** Garantiza EM dentro de requerir_impresora **/
pthread_cond_t cond_impresoras_libres = PTHREAD_COND_INITIALIZER;		/** Semaforo Condicion que duerme miestras no hayan Impresoras disponibles **/
sem_t mutex_impresoras_libres;											/** Semaforo que garantiza la consistencia de datos **/
sem_t mutex_Centro;														/** Garantiza la EM dentro del arreglo Centro **/

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
	
	pthread_t Usuarios[N]; 	 											/** Arreglo de threads de Usuarios de tamaño N **/
	pthread_t Intro;  													/** Thread que lanza la presentacion de bienvenida **/
	pthread_attr_t atributos;  											/** Representa el cuerpo de cada Thread **/
	struct sched_param fifo_param;  									/** "sched_param" es una estructura perteneciente a <pthread.h> la cual almacena la prioridad de cada Thread **/
	int i, err;  														/** "i" se utiliza para el recorrido del ciclo for y "err" almacena codigo de error para pthread_create **/
	
	/** Inicializa todos los recursos usados por las Impresoras. **/
	inicializar_Centro (); 												/** Se encarga de incializar los recurso del centro de impresion **/ 
	srand (time(NULL)); 												/** Genera la semilla para la funcion rand() **/
	
	/************  CREACION DE HILO BIENVENIDA *************/
	pthread_create( &Intro , NULL , Presentando , NULL ); 				/** Crea y lanza el Thread Intro con el procedimiento "Presentando" **/
	
	/************  CREACION DE HILOS USUARIO  **************/
	for (i=0 ; i<N ; i++) {
		pthread_attr_init(&atributos); 									/** Esta funcion inicializa el cuerpo de cada Thread con parametros por defecto **/
		pthread_attr_setschedpolicy(&atributos, SCHED_FIFO); 			/** Modifica la politica de planificacion con FIFO **/
		fifo_param.sched_priority = (rand() % 5+1); 					/** Genera un valor random entre 1-5 y se lo asigna a la variable sched_priority que esta en la estructura "shed_param" que pertenece a <pthread.h> **/
		pthread_attr_setschedparam(&atributos, &fifo_param); 			/** Se almacena el valor "sched_priority" donde anteriormente se guardo el valor random, al cuerpo del hilo **/
		
		err = pthread_create( &Usuarios[i] , &atributos , funcion_Centro , NULL ); /** Crea el Thread, se le asignan los atributos, se almacena en el vector Usuarios, llama al procedimiento "funcion_Centro" sin parametros  **/
		if (err != 0) {
		   printf("Error creando el hilo %d\n", i);
		   abort();
		}
	}
	
	while (marca > 0);
	fclose(output); 													/** Cierra el archivo de salida **/ 
	pthread_join(Intro, NULL); 											/** Espera a que termine el Thread Intro, para continuar con la siguiente linea de instrucciones **/
	system("reset"); 													/** Resetea la pantalla **/
	
	return (0);
}

/** Procedimiento encargado de manejar la ejecucion de cada Thread **/
void *funcion_Centro () {
	int id_Impresora; 
	
	id_Impresora = requerir_impresora(); 								/** en "id_Impresora" se almacena el id de la primera impresora disponible en el arreglo Centro **/
	
	if (!(verificar_tinta(id_Impresora))) 								/** Si no hay tinta en la impresora, se llama al tecnico mediante "recargar_tinta()" **/
		recargar_tinta(id_Impresora);
	
	liberar_impresora(id_Impresora); 									/** Libera la impresora **/
	
	pthread_exit(NULL); 												/** Finaliza el Thread, mas NO lo destruye **/
}

/** Se encarga de incializar todos los recursos del Centro de Impresion **/
void inicializar_Centro () {
	int i;
	
	for (i=0 ; i<M ; i++) {
		Centro[i].tinta = 100; 											/** Se le asigna el nivel de tinta = 100, es decir tanque lleno **/
		Centro[i].identificador = i; 									/** Se le asigna el identificador a cada impresora (0,14) **/
		Centro[i].uso = 0;												/** Se inicializa en 0 la cantidad de veces que se ha usado la impresora i **/
		sem_init(&Centro[i].ocupada, 0, 1); 							/** Se inicializa el semaforo "ocupada" de cada impresora como libre **/
	}
	sem_init(&mutex_Centro, 0, 1); 										/** Semaforo que protege el acceso al arreglo Centro **/
	sem_init(&mutex_impresoras_libres, 0, 1); 							/** Semaforo que protege las modificaciones a la cantidad de impresoras libres **/
	Impresoras_libres = M; 												/** Cantidad de impresoras libres, inicialmente M **/
	marca = N; 															/** Lleva la cuenta de cuantos usuarios hay en el Centro de Impresion **/
	output = fopen("bitacora.out" , "w"); 								/** Se crea y se abre el Archivo de salida en modo escritura **/
	
	fprintf(output, "*** Centro de Impresion y Tecnico Listos para empezar ***\n\n");
}

/** Retorna el id de la PRIMERA impresora DISPONIBLE **/
int requerir_impresora () {
	int i, id_impre, sval, j, menor = MAX;
	
	pthread_mutex_lock(&mutex_requerido);								/** Solo un Thread a la vez puede acceder a la seccion de requerir_impresora **/
	while (Impresoras_libres < 1)
		pthread_cond_wait(&cond_impresoras_libres,&mutex_requerido);	/** El semaforo "cond_impresoras_libres" se duerme mientras el semaforo "mutex_requerido" está bloqueado; mientras no hayan impresoras libres  **/
	
	pthread_mutex_unlock(&mutex_requerido);								/** Desbloquea el semaforo "mutex_requerido" **/
	pthread_cond_signal(&cond_impresoras_libres);						/** Despierta al semaforo "cond_impresoras_libres **/
	
	sem_wait(&mutex_Centro); 											/** Protege la SC del arreglo Centro, de manera que solo un Thread este buscando una impresora disponible a la vez**/
	for (i=0 ; i<M ; i++) {
		if ((sem_getvalue(&Centro[i].ocupada , &sval) == 0 ) && ( sval == 1 ) ) {	 /** Verifica si la Impresora i esta disponible **/ 
			if (Centro[i].uso <= menor) {								/** Verifica si es la Impresora i la menos usada **/
				menor = Centro[i].uso;
				j = i;
			}
		}
	}
	
	id_impre = j;														/** Actualizo la variable de Retorno **/
	sem_wait(&Centro[id_impre].ocupada); 								/** Bloquea el semaforo de la Impresora i, poniendola como ocupada **/
		
	sem_wait(&mutex_impresoras_libres); 								/** Protege el RC impresoras_libres para modificacion/lectura **/
	Impresoras_libres--;
	sem_post(&mutex_impresoras_libres); 								/** Libera el RC impresoras_libres **/
	
	Centro[id_impre].uso += 1;											/** Incremento en 1 la cantidad de veces que se ha usado la Impresora **/
	
	sem_post(&mutex_Centro); 											/** Libera la SC del arreglo Centro **/
	
	fprintf(output, "[Requerida] Impresora %d\n\n", id_impre);
	
	return (id_impre); 													/** Retorna el id de la impresora requerida anteriormente **/
}

/** Libera una Impresora **/
void liberar_impresora (int id_impresora) {
	sem_wait(&mutex_impresoras_libres);									/** Protege el RC impresoras_libres para modificacion/lectura **/
	Impresoras_libres++;
	sem_post(&mutex_impresoras_libres); 								/** Libera el RC impresoras_libres **/
	
	Centro[id_impresora].tinta -= 5;									/** Descuento 5ml de Tinta a la impresora que se esta liberando **/
	marca--;
	fprintf(output, "[Liberada] Impresora %d\n\n", id_impresora);
	sem_post(&Centro[id_impresora].ocupada); 							/** Libera el semaforo de la impresora, poniendola como disponible **/
}

bool verificar_tinta (int id_impresora) {
	bool band2;
	
	fprintf(output, "[Verificando] Tinta Impresora %d\n", id_impresora);
	if (Centro[id_impresora].tinta > 0) {
		fprintf(output, "\tNivel de Tinta = %d \t[OK]\n\n", Centro[id_impresora].tinta);
		band2 = true;													/** Si la cantidad de tinta es mayor a cero retorna verdad**/
	}else{
		fprintf(output, "\tNivel de Tinta = %d \t[NO OK]\n\n", Centro[id_impresora].tinta);
		band2 = false;													/** Si la cantidad de tinta es menor a cero retorna falso**/
	}
	
	return (band2);
}

void recargar_tinta (int id_impresora) {
	pthread_mutex_lock(&mutex_tecnico); 								/** Bloquea al Tecnico, poniendolo como OCUPADO **/
	
	fprintf(output, "[Recargando] Tinta en Impresora: %d\n\t<Tecnico Trabajando> \t(...)\n\t¡Tinta Recargada.!\n\n", id_impresora);
	Centro[id_impresora].tinta = 100; 									/** Tecnico restaura el nivel de tinta de la Impresora **/
	
	pthread_mutex_unlock(&mutex_tecnico); 								/** Libera al Tecnico, poniendolo como DISPONIBLE **/
}

void *Presentando () {
	
	printf("\n\n");
	printf("                \e[44;1m11111111\e[m  \e[44;1m11111111\e[m  \e[44;1m111\e[m     \e[44;1m11\e[m  \e[44;1m11111111\e[m  \e[44;1m11111111\e[m  \e[44;1m11111111\e[m\n");
	printf("                \e[44;1m11\e[m        \e[44;1m11\e[m    \e[44;1m11\e[m  \e[44;1m1111\e[m    \e[44;1m11\e[m     \e[44;1m11\e[m     \e[44;1m11\e[m   \e[44;1m111\e[m  \e[44;1m11\e[m    \e[44;1m11\e[m\n");
	printf("                \e[44;1m11\e[m        \e[44;1m11\e[m   \e[44;1m11\e[m   \e[44;1m11\e[m \e[44;1m11\e[m   \e[44;1m11\e[m     \e[44;1m11\e[m     \e[44;1m11111111\e[m  \e[44;1m11\e[m    \e[44;1m11\e[m\n");
 	printf("                \e[44;1m11\e[m         \e[44;1m1111\e[m     \e[44;1m11\e[m  \e[44;1m11\e[m  \e[44;1m11\e[m     \e[44;1m11\e[m     \e[44;1m11\e[m \e[44;1m11\e[m     \e[44;1m11\e[m    \e[44;1m11\e[m\n");
   	printf("                \e[44;1m11\e[m           \e[44;1m111\e[m    \e[44;1m11\e[m   \e[44;1m11\e[m \e[44;1m11\e[m     \e[44;1m11\e[m     \e[44;1m11\e[m  \e[44;1m11\e[m    \e[44;1m11\e[m    \e[44;1m11\e[m\n");
	printf("                \e[44;1m11111111\e[m      \e[44;1m1111\e[m  \e[44;1m11\e[m    \e[44;1m1111\e[m     \e[44;1m11\e[m     \e[44;1m11\e[m   \e[44;1m111\e[m  \e[44;1m11111111\e[m\n\n");
	
	printf("                                 \e[43;1m111111111111111111111111111\e[m\n");
	printf("                                 \e[43;1m1\e[m                         \e[43;1m1\e[m\n");
	printf("                                 \e[43;1m1\e[m   \e[41;1m11111\e[m      \e[41;1m11111111\e[m   \e[43;1m1\e[m\n");
	printf("                                 \e[43;1m1\e[m   \e[41;1m11\e[m  \e[41;1m111\e[m    \e[41;1m11\e[m    \e[41;1m11\e[m   \e[43;1m1\e[m\n");
	printf("\e[43;1m1111111111111111111111111111111111\e[m   \e[41;1m11\e[m   \e[41;1m111\e[m   \e[41;1m11\e[m   \e[41;1m11\e[m    \e[43;1m1111111111111111111111111111111111\e[m\n");
	printf("\e[43;1m1111111111111111111111111111111111\e[m   \e[41;1m11\e[m   \e[41;1m111\e[m    \e[41;1m1111\e[m      \e[43;1m1111111111111111111111111111111111\e[m\n");
	printf("                                 \e[43;1m1\e[m   \e[41;1m11\e[m  \e[41;1m111\e[m       \e[41;1m111\e[m     \e[43;1m1\e[m\n");
	printf("                                 \e[43;1m1\e[m   \e[41;1m11111\e[m          \e[41;1m1111\e[m   \e[43;1m1\e[m\n");
	printf("                                 \e[43;1m1\e[m                         \e[43;1m1\e[m\n");
	printf("                                 \e[43;1m111111111111111111111111111\e[m\n\n");
	                     
	printf("\e[44;1m11111111\e[m  \e[44;1m1111\e[m   \e[44;1m1111\e[m  \e[44;1m11111111\e[m  \e[44;1m11111111\e[m  \e[44;1m11111111\e[m  \e[44;1m11111111\e[m \e[44;1m11111111\e[m  \e[44;1m11111111\e[m  \e[44;1m111\e[m     \e[44;1m11\e[m\n");
	printf("   \e[44;1m11\e[m     \e[44;1m11\e[m \e[44;1m11\e[m \e[44;1m11\e[m \e[44;1m11\e[m  \e[44;1m11\e[m    \e[44;1m11\e[m  \e[44;1m11\e[m   \e[44;1m111\e[m  \e[44;1m11\e[m    \e[44;1m11\e[m  \e[44;1m11\e[m          \e[44;1m11\e[m     \e[44;1m11\e[m    \e[44;1m11\e[m  \e[44;1m1111\e[m    \e[44;1m11\e[m\n");
	printf("   \e[44;1m11\e[m     \e[44;1m11\e[m  \e[44;1m111\e[m  \e[44;1m11\e[m  \e[44;1m11111111\e[m  \e[44;1m11111111\e[m  \e[44;1m11\e[m   \e[44;1m11\e[m   \e[44;1m111111\e[m      \e[44;1m11\e[m     \e[44;1m11\e[m    \e[44;1m11\e[m  \e[44;1m11\e[m \e[44;1m11\e[m   \e[44;1m11\e[m\n");
	printf("   \e[44;1m11\e[m     \e[44;1m11\e[m       \e[44;1m11\e[m  \e[44;1m11\e[m        \e[44;1m11\e[m \e[44;1m11\e[m      \e[44;1m1111\e[m         \e[44;1m11\e[m      \e[44;1m11\e[m     \e[44;1m11\e[m    \e[44;1m11\e[m  \e[44;1m11\e[m  \e[44;1m11\e[m  \e[44;1m11\e[m\n");
	printf("   \e[44;1m11\e[m     \e[44;1m11\e[m       \e[44;1m11\e[m  \e[44;1m11\e[m        \e[44;1m11\e[m  \e[44;1m11\e[m       \e[44;1m111\e[m        \e[44;1m11\e[m      \e[44;1m11\e[m     \e[44;1m11\e[m    \e[44;1m11\e[m  \e[44;1m11\e[m   \e[44;1m11\e[m \e[44;1m11\e[m\n");
	printf("\e[44;1m11111111\e[m  \e[44;1m11\e[m       \e[44;1m11\e[m  \e[44;1m11\e[m        \e[44;1m11\e[m   \e[44;1m111\e[m      \e[44;1m1111\e[m  \e[44;1m111111\e[m   \e[44;1m11111111\e[m  \e[44;1m11111111\e[m  \e[44;1m11\e[m    \e[44;1m1111\e[m\n\n");
	
	sleep(5);
	system("reset");
	
	printf("\n\n");
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
	
	sleep(8);
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
		
		printf("\n\n\n\n\n\n\n\t\t\t                 \e[36m<\e[m \e[42;1mC\e[m A R G A N D O \e[36m>\e[m           \n\n");
		sleep(10);
		system("clear");
		printf("\n\n\n\n\n\n\n\t\t\t               \e[36m< <\e[m \e[42;1mC A\e[m R G A N D O \e[36m> >\e[m         \n\n");
		sleep(10);
		system("clear");
		printf("\n\n\n\n\n\n\n\t\t\t             \e[36m< < <\e[m \e[42;1mC A R\e[m G A N D O \e[36m> > >\e[m       \n\n");
		sleep(10);
		system("clear");
		printf("\n\n\n\n\n\n\n\t\t\t           \e[36m< < < <\e[m \e[42;1mC A R G\e[m A N D O \e[36m> > > >\e[m     \n\n");
		sleep(10);
		system("clear");
		printf("\n\n\n\n\n\n\n\t\t\t         \e[36m< < < < <\e[m \e[42;1mC A R G A\e[m N D O \e[36m> > > > >\e[m   \n\n");
		sleep(10);
		system("clear");
		printf("\n\n\n\n\n\n\n\t\t\t       \e[36m< < < < < <\e[m \e[42;1mC A R G A N\e[m D O \e[36m> > > > > >\e[m \n\n");
		sleep(10);
		system("clear");
		printf("\n\n\n\n\n\n\n\t\t\t     \e[36m< < < < < < <\e[m \e[42;1mC A R G A N D\e[m O \e[36m> > > > > > >\e[m \n\n");
		sleep(10);
		system("clear");
		printf("\n\n\n\n\n\n\n\t\t\t   \e[36m< < < < < < < <\e[m \e[42;1mC A R G A N D O\e[m \e[36m> > > > > > > >\e[m \n\n");
		sleep(7);
		
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
