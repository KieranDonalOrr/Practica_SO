#include "my_lib.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

/*-----------------------AUTORES DE LA AVENTURA_03----------------------*/
/*--------------------------Kieran Donald Orr---------------------------*/
/*--------------------------Pablo Nuñez Perez---------------------------*/
/*------------------------Pau Capella Ballester-------------------------*/

//Funcion que se encarga de dar la longitud de el "String"
size_t my_strlen(const char *str)
{
    //Variable size_t que almacenará el tamaño del String
    size_t lenght = 0;
    //Bucle while, mientras no hayamos llegado al final del string, aumentamos variable lenght
    while (*str++ != '\0')
    {
        lenght++;
    }

    return lenght;
}

//Funcion que compara dos "Strings"
int my_strcmp(const char *str1, const char *str2)
{
    //Variable entera donde guardaremos el resultado de int01 y int02
    int resultado = 0;
    //Variables enteras donde guardaremos el codigo de carácter de su string correspondiente
    int int_01 = 0;
    int int_02 = 0;
    //Bucle While, se ejecutaráa si los codigos de caracteres son iguales, y si no se ha llegado al final de algunoo de los strings
    while (int_01 == int_02 && *str1 != 0 && *str2 != 0)
    {
        //Almacenamos el contenido del string apuntado por los apuntadores str, más concretamente el carácter siguiente
        // en las variables int.
        int_01 = *str1++;
        int_02 = *str2++;
    }
    //Realizamos la diferencia entre los codigos de caracteres. Si se ha parado el bucle al final de los DOS strings, por tanto coinciden,
    //En caso contrario, los strings tienen distintos tamaños y o bien uno de los caracteres es distinto.
    resultado = int_01 - int_02;

    return resultado;
}

//Funcion que copia un "String"
char *my_strcpy(char *dest, const char *src)
{
    //Variable size_t que contiene el tamaño del string que va a ser copiado.
    size_t n = my_strlen(src);
    //Bucle for que copia el contenido del string fuente, al string destino
    for (int i = 0; i < n; i++)
    {
        dest[i] = src[i];
    }
    //Ponemos al final del string destino el carácter vacío.
    dest[n] = '\0';

    return dest;
}

//Funcion que copia los "n" caracteres de un "String"
char *my_strncpy(char *dest, const char *src, size_t n)
{
    //Variable entera se usará para iterar
    int counter;
    //Hacemos un bucle for que copia los n carácteres pasados por parámetros

    for (counter = 0; counter < n; counter++)
    {
        dest[counter] = src[counter];
    }

    return dest;
}
//Funcion que concatena dos string pasados
char *my_strcat(char *dest, const char *src)
{
    //Variables size_t que almacenan el tamaño de cada string
    size_t s = my_strlen(src);
    size_t d = my_strlen(dest);
    int counter = 0;
    //Bucle for donde copiamos cada carácter del string fuente, desde el final del string destino
    for (; counter < s + 1; counter++)
    {
        dest[counter + d] = src[counter];
    }
    //Ponemos al final de la cadena concatenada el carácter vacío
    dest[counter + d + 1] = '\0';

    return dest;
}
//Función que inicializa una pila de datos
struct my_stack *my_stack_init(int size)
{
    //Creamos un apuntador a tipos estructurados (my_stack)
    struct my_stack *stack;
    //Reservamos espacio en la dirección stack
    stack = malloc(sizeof(struct my_stack));
    //Se inicializa las variables internas de stack
    stack->top = NULL;
    stack->size = 4;

    return stack;
}
//Funcion PUSH
int my_stack_push(struct my_stack *stack, void *data)
{
    //Se crea una variable apuntadora a tipos estructurados (my_stack)
    struct my_stack_node *nou_node;
    //Reservamos espacio en la dirección nou_node
    nou_node = malloc(sizeof(struct my_stack_node));
    //Inicializamos el nodo
    nou_node->data = data;
    nou_node->next = stack->top;
    //El top pasa a apuntar al nuevo nodo creado "nou_node"
    stack->top = nou_node;

    return EXIT_SUCCESS;
}
//Funcion POP
void *my_stack_pop(struct my_stack *stack)
{
    if (stack->top != NULL)
    {
        //Declaramos un apuntador del ultimo nodo creado
        struct my_stack_node *nou_node;
        //Asignamos la dirección de top al ultimo nodo
        nou_node = stack->top;
        //El top pasa a apuntar el siguiente nodo
        stack->top = nou_node->next;
        //Devolvemos el "data" y liberamos el nodo con la funcion "free"
        return (nou_node->data);
        free(nou_node);
    }
    else
    {
        printf("No hay elementos en la pila \n");
    }

    return EXIT_SUCCESS;
}
//Funcion que devuelve la longitud de la pila que se pasa por parametro
int my_stack_len(struct my_stack *stack)
{
    int numNodos = 0;
    //Comprobamos que el "top" no sea NULL para saber si hay elementos en la pila
    if (stack->top != NULL)
    {

        //Inicializamos un nodo auxiliar para recorrer la pila
        struct my_stack_node *auxiliar = stack->top;
        //Mientras el nodo no apunte al final de la pila la recorremos aunmentado la cantidad de elementos
        while (auxiliar->next != NULL)
        {
            auxiliar = auxiliar->next;
            numNodos++;
        }
        //Incremetamos el valor
        numNodos++;
    }
    //Devolvemos la cantidad de elementos de la pila
    return numNodos;
}
//Funcion que se encarga de vaciar completamente la pila pasada por parametro
int my_stack_purge(struct my_stack *stack)
{
    //Contador
    int liberado = 0;
    //Obtenemos la cantidad de nodos que borraremos con la funcion "my_stack_len"
    int nodos_para_borrar = my_stack_len(stack);
    //Inicializamos un nodo auxiliar
    struct my_stack_node *auxiliar;
    //Reservamos memoria con la funcion "malloc"
    auxiliar = malloc(sizeof(struct my_stack_node));

    //Recorremos la pila para liberar los cada uno de los elementos
    for (int counter = 0; counter < nodos_para_borrar; counter++)
    {

        liberado = liberado + sizeof(*auxiliar);
        liberado = liberado + stack->size;
        free(my_stack_pop(stack));
    }

    //Liberamos la memoria que contenia la pila con la funcion "free"
    free(stack);
    liberado = liberado + sizeof(struct my_stack);
    return liberado;
}

//Funcion que se encarga de escibir los elementos de una pila en un fichero destino
int my_stack_write(struct my_stack *stack, char *filename)
{
    //Realizamos las comparaciones para saber si hay informacion de la pila
    if (stack == NULL)
    {
        printf("No hay pila");
        return -1;
    }
    else if (stack->top == NULL)
    {
        printf("Top NULL");
    }
    else
    {
        //Obtenemos el largo de la pila con la funcion "my_stack_len"
        int lenght = my_stack_len(stack);
        printf("Stack lenght: %d", lenght);
        //Inicializamos una pila auxiliar en la que volcaremos los datos
        struct my_stack *aux_stack = my_stack_init(lenght);
        //Inicializamos un puntero para recorrer la pila
        struct my_stack_node *pointer = malloc(sizeof(struct my_stack_node));
        pointer = stack->top;

        //Obtenemos los elementos y los volcamos en la pila auxiliar mediante la funcion "my_stack_push"
        for (int i = 0; i < lenght; i++)
        {
            my_stack_push(aux_stack, pointer->data);
            pointer = pointer->next;
        }

        //Obtenemos la direccion del fichero con la funcion "open" y declaramos los parametros y permisos necesarios
        int fichero = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        //Inicializamos el "buffer" necesario para la escritura
        void *buff = &stack->size;
        int contador_elementos = 0;
        int longitud_aux = my_stack_len(aux_stack);

        //Escribimos la cantidad de elementos que se escribiran en el fichero
        buff=4;
        printf("EScribiendo: %d \n",buff);
        
        write(fichero, buff, 4);

        //Recorremos la pila auxiliar escribiendo los datos en el fichero en el orden correcto
        for (int j = 0; j < longitud_aux; j++)
        {
            buff = my_stack_pop(aux_stack);
            printf("EScribiendo: %d \n",buff);
            write(fichero, buff, 4);
            contador_elementos++;
        }
        //Liberamos la memoria ocupada por la pila auxiliar
        free(aux_stack);
        //Cerramos el fichero con la funcion "close"
        close(fichero);

        //Devolvemos la cantidad de elementos que se han escrito en el fichero
        return contador_elementos;
    }

    return EXIT_SUCCESS;
}

//Funcion que se encarga de leer los elementos de un fichero e introduccirlos en la pila
struct my_stack *my_stack_read(char *filename)
{
    //Obtenemos la direccion del fichero con la funcion "open"
    int fichero = open(filename, O_RDONLY, S_IRUSR);

    if (fichero == -1)
    {
        printf("ERROR - El archivo no existe\n");
    }
    else
    {
        printf("Estoy leyendo \n");
        void *buff;
        int size = 4;

        //Leemos el primer dato del fichero que es la cantidad de elementos que deberemos leer
        read(fichero, &size, sizeof(int));

        //Inicializamos la pila y reservamos espacio en memoria con la funcion "malloc"
        struct my_stack *stack = my_stack_init(size);
        buff = malloc(size);
        int contador = 0;

        //Recorremos el fichero y vamos escribiendo los elementos en la pila con la funcion "my_stack_push"
        printf("Voy a empezar a leer");
        while (read(fichero, buff, size) != NULL)
        {
            printf("%d",buff);
            my_stack_push(stack, buff);
            buff = malloc(size);
            contador++;
        }
        //Cerramos el fichero
        close(fichero);

        //Devolvemos la pila generada de la lectura del fichero
        return stack;
    }
}
