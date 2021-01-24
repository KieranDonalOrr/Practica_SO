#include "my_lib.c"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

/*-----------------------AUTORES DE LA AVENTURA_03----------------------*/
/*--------------------------Kieran Donald Orr---------------------------*/
/*--------------------------Pablo Nuñez Perez---------------------------*/
/*------------------------Pau Capella Ballester-------------------------*/

//Variables relacionadas con hilos
int nthreads = 10;
int iterations = 100000;
pthread_t hilos[10];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
struct my_stack *my_stack;
struct my_stack *stack_prueba;


//Declaraciones de los Metodos
//PASO 1
int inicializacion(char *args);
//PASO 2
void crearHilos();
//PASO 3
void *worker(void *ptr);
//PASO 4
//PASO 5

int inicializacion(char *args)
{
    //Primero haremos la comprobacion de si el fichero existe con el metodo my_stack_read

    struct my_data *data1;
    struct my_stack *stack_salida;
    struct my_stack_node *pointer;
    //stack_prueba = my_stack_read(args);
    void *data;
    stack_prueba = my_stack_init(4);

    //printf("DATO: %p\n", data);

    //my_stack_write(stack_prueba, "fichero.txt");
    for (int i = 0; i < 10; i++)
    {
        my_stack_push(stack_prueba, 0);
        printf("Elemento_%d: %d \n", i, 0);
    }

    //    my_stack_write(stack_prueba,"fichero.txt");

    //    stack_prueba = my_stack_read("fichero.txt");

    //     for (int i = 0; i < 10; i++)
    //     {
    //         int s = 0;
    //         printf("Elemento_%d: %c \n", i, my_stack_pop(stack_prueba));
    //     }
    //CASO EN EL QUE NO HAY FICHERO
    // my_stack = my_stack_init(sizeof(int));
}

void *worker(void *ptr)
{
    for (int i = 0; i < iterations; i++)
    {

        pthread_mutex_lock(&lock);

        void *data = my_stack_pop(stack_prueba);
        pthread_mutex_unlock(&lock);
        data++;

        pthread_mutex_lock(&lock);

        my_stack_push(stack_prueba, data);
        pthread_mutex_unlock(&lock);
    }
    pthread_exit(NULL);
}

//MAIN
int main(int argc, char *argv[])
{
    if (argc == 2)
    {
        printf("El argumento proporcionado es: %s\n", argv[1]);
        inicializacion(argv[1]);
        printf("Hevuelto de inicializar \n");
        for (int i = 0; i < 10; i++)
        {
            pthread_create(&hilos[i], NULL, worker, NULL);
        }
        for (int i = 0; i < 10; i++)
        {
            pthread_join(hilos[i], NULL);
        }

        int i = my_stack_write(stack_prueba,"fichero.txt");
        printf("Elementos escritos: %d \n", i);
        *stack_prueba = *my_stack_read("fichero.txt");

        for (int i = 0; i < 10; i++)
        {

            printf("Elemento_%d: %d \n", i, my_stack_pop(stack_prueba));
        }
    }
    else if (argc > 2)
    {
        printf("ERROR SYNTAX - Número de argumentos invalidos - Uso: ./av3 <file>\n");
    }
    else
    {
        printf("ERROR SYNTAX - Uso: ./av3 <file>\n");
    }
    return NULL;
}
//MÉTODO HACEDOR DE HILOS
/*
void crearHilos()
{
    int resultado;   
    for (int i = 0; i < 10; i++)
    {
        resultado=pthread_create(&hilos[i], NULL, worker, NULL);
    }
    if  (resultado==0){
        printf("Éxito en la creación de los hilos");
    }else{
        printf("Ha habido un error en la creación");
    }
}
*/