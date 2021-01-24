#include "my_lib.c"
#include <pthread.h>

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
    struct my_stack *stack_prueba;
    struct my_stack *stack_salida;
    struct my_stack_node *pointer;
    stack_prueba = my_stack_read(args);
    void *data;

    printf("DATO: %p\n", data);

    my_stack_write(stack_prueba, "s2-6el");

    stack_salida = my_stack_read("s2-6el");

    for (int i = 0; i < my_stack_len(stack_salida); i++)
    {
        printf("Elemento_%d: %p", i, my_stack_pop(stack_salida));
    }
    //CASO EN EL QUE NO HAY FICHERO
    // my_stack = my_stack_init(sizeof(int));
}

void *worker(void *ptr)
{
    for (int i = 0; i < iterations; i++)
    {
        pthread_mutex_lock(&lock);
        void *data = my_stack_pop(my_stack);
        pthread_mutex_unlock(&lock);
        data++;
        pthread_mutex_lock(&lock);
        my_stack_push(my_stack, data);
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
    }
    else if (argc > 2)
    {
        printf("ERROR SYNTAX - Número de argumentos invalidos - Uso: ./av3 <file>\n");
    }
    else
    {
        printf("ERROR SYNTAX - Uso: ./av3 <file>\n");
    }
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