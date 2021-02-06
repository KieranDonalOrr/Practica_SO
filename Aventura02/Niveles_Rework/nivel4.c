/*
-------------------AUTORES-------------------
                Kieran Orr Donal
                Pablo Núñez Pérez
                Pau Capellà Ballester
---------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"

//Variables
bool in_minishell = false;
bool titulo = true;
char linea_buff[50];
char current_directory[100];
char *vector_tokens[10];
char *token;
char *token_export;
char *token_reaper;
char *token_ctrc;

//Declaraciones de los Metodos
char *read_line(char *line);
int execute_line(char *line);
int parse_args(char **args, char *line);
int check_internal(char **args);
int internal_cd(char **args);
int internal_export(char **args);
int internal_source(char **args);
int internal_jobs(char **args);
int internal_fg(char **args);
int internal_bg(char **args);
int internal_exit(char **args);
int internal_clear(char **args);
int internal_fork(char **args);
void imprimir_prompt();

//Struct
struct info_process
{
    pid_t pid;
    char status;                  // ‘N’, ’E’, ‘D’, ‘F’
    char cmd[sizeof(linea_buff)]; // línea de comando
};

//Array de Trabajos Global
static struct info_process jobs_list[64];

//Manejador del Reaper
void reaper(int signum)
{
    //Variables
    pid_t end;
    int wstatus;

    //Asociamos SIGCHLD al manejador reaper
    signal(SIGCHLD, reaper);

    while ((end = waitpid(-1, &wstatus, WNOHANG)) > 0)
    {

        token_reaper = strtok(jobs_list[0].cmd, "\n");

        //Devolver el estado de finalizacion del hijo
        if (WIFEXITED(wstatus))
        {
            printf("[reaper()→ Proceso hijo %d (%s) finalizado con estado: %d]\n", end, token_reaper, WEXITSTATUS(wstatus));
        }
        else if (WIFSIGNALED(wstatus))
        {
            printf("[reaper()→ Proceso hijo %d (%s) finalizado con la señal: %d]\n", end, token_reaper, WTERMSIG(wstatus));
        }
        else if (WIFSTOPPED(wstatus))
        {
            printf("[reaper()→ Proceso hijo %d (%s) finalizado con la señal: %d]\n", end, token_reaper, WIFSTOPPED(wstatus));
        }

        //Establecemos estados
        jobs_list[0].pid = 0;
        jobs_list[0].status = 'F';
        jobs_list[0].cmd[0] = '\0';
    }
}

//Manejador Ctrl C
void ctrlc(int signum)
{
    token_ctrc = strtok(jobs_list[0].cmd, "\n");
    printf("\n[ctrlc()→ Soy el proceso con PID %d, el proceso en foreground es %d (%s)]\n", getpid(), jobs_list[0].pid, token_ctrc);
    fflush(stdout);

    //Comprobamos si hay un proceso en foreground
    if (jobs_list[0].pid > 0)
    {
        if (strcmp(linea_buff, jobs_list[0].cmd) != 0)
        {
            //token_ctrc = strtok(jobs_list[0].cmd, "\n");
            printf("[ctrlc()→ Señal %d enviada a PID %d (%s) por %d]\n", SIGTERM, jobs_list[0].pid, token_ctrc, getpid());
            kill(jobs_list[0].pid, SIGTERM);
        }
        else
        {
            printf("[ctrlc()→ Señal %d no enviada por PID %d debido a que el proceso en foreground es el shell]\n", SIGTERM, getpid());
            fflush(stdout);
        }
    }
    else
    {
        printf("[ctrlc()→ Señal %d no enviada por PID %d debido a que no hay proceso en foreground]\n", SIGTERM, getpid());
        fflush(stdout);
        imprimir_prompt();
    }
}

//Metodo que imprime nuestro prompt
void imprimir_prompt()
{
    //Impresion de prueba
    /*
    printf(" [PWD]: %s\n", getenv("PWD"));
    printf(" [HOME]: %s\n", getenv("HOME"));
    printf(" [LANGUAGE]: %s\n", getenv("LANG"));
    printf(" [USER]: %s\n", getenv("USER"));
    */

    //Obtencion del directorio de trabajo
    getcwd(current_directory, 100);

    //Impresion del encabezado
    while (titulo)
    {
        printf(" __  __ _       _    _____ _          _ _ \n");
        printf("|  \\/  (_)     (_)  / ____| |        | | |\n");
        printf("| \\  / |_ _ __  _  | (___ | |__   ___| | |\n");
        printf("| |\\/| | | '_ \\| |  \\___ \\| '_ \\ / _ \\ | |\n");
        printf("| |  | | | | | | |  ____) | | | |  __/ | |\n");
        printf("|_|  |_|_|_| |_|_| |_____/|_| |_|\\___|_|_|  Nivel_04\n");
        printf("\n");

        titulo = false;
    }

    printf(WHT "[MiniShell]");
    printf(GRN "[%s]", getenv("USER"));
    printf(BLU "[%s]" WHT ":", current_directory);
    fflush(stdout);
}

//Metodo para leer la linea de comando
char *read_line(char *line)
{
    if (fgets(line, 50, stdin) == NULL)
    {
        if (feof(stdin))
        {
            printf("\nCERRAMOS MINISHELL\n");
            exit(0);
        }
    }

    //Impresion de la linea de comando
    /*
    printf("Linea Buffer:%s\n", line);
    */

    return line;
}

//Metodo para pasar la linea a parse_args y los tokens a check_internal
int execute_line(char *line)
{

    strcpy(jobs_list[0].cmd, line);

    if (parse_args(vector_tokens, line) != 0)
    {
        if (check_internal(vector_tokens) == 1)
        {
            printf("COMANDO EXTERNO\n");
            internal_fork(vector_tokens);
            return EXIT_SUCCESS;
        }

        return EXIT_SUCCESS;
    }
}

//Metodo que separa el comando en tokens
int parse_args(char **args, char *line)
{
    //Variable
    int array_tokens = 0;
    int num_tokens = 0;
    char delimiters[5] = "  \n\t\r";

    //For encargado de la limpieza de "args"
    for (int i = 0; args[i] != NULL; i++)
    {
        args[i] = NULL;
    }

    token = strtok(line, delimiters);

    while (token != NULL)
    {
        args[array_tokens] = token;

        num_tokens++;

        //Impresion de los tokens
        /*
        printf("Token_%d:%s\n", array_tokens, token);
        */

        token = strtok(NULL, delimiters);

        if (token != NULL)
        {
            array_tokens++;
        }
    }

    return num_tokens;
}

//Metodo que filtra el comando
int check_internal(char **args)
{

    if (strcmp(args[0], "cd") == 0)
    {
        printf("EJECUTANDO CD\n");
        internal_cd(vector_tokens);
    }
    else if (strcmp(args[0], "export") == 0)
    {
        printf("EJECUTANDO EXPORT\n");
        internal_export(vector_tokens);
    }
    else if (strcmp(args[0], "source") == 0)
    {
        printf("EJECUTANDO SOURCE\n");
        internal_source(vector_tokens);
    }
    else if (strcmp(args[0], "jobs") == 0)
    {
        printf("EJECUTANDO JOBS\n");
        internal_jobs(vector_tokens);
    }
    else if (strcmp(args[0], "fg") == 0)
    {
        printf("EJECUTANDO FG\n");
        internal_fg(vector_tokens);
    }
    else if (strcmp(args[0], "bg") == 0)
    {
        printf("EJECUTANDO BG\n");
        internal_bg(vector_tokens);
    }
    else if (strcmp(args[0], "exit") == 0)
    {
        printf("EJECUTANDO EXIT\n");
        printf("CERRAMOS MINISHELL\n");
        exit(0);
    }
    else if (strcmp(args[0], "clear") == 0)
    {
        printf("EJECUTANDO CLEAR\n");
        system("clear");
    }
    else
    {
        return EXIT_FAILURE;
    }
}

//Metodos de intrucciones internas

//Internal CD
int internal_cd(char **args)
{
    //Variable
    char cd_path[100];

    if (args[1] == NULL)
    {
        chdir(getenv("HOME"));
        printf("[internal_cd()→ /home/uib]\n");
        return EXIT_SUCCESS;
    }
    else if (args[1] != NULL)
    {

        if (chdir(args[1]) == 0)
        {
            getcwd(cd_path, 100);
            printf("[internal_cd()→ %s\n", cd_path);
            return EXIT_SUCCESS;
        }

        printf("ERROR - Archivo o directorio no encontrado\n");

        return EXIT_SUCCESS;
    }
}

//Internal EXPORT
int internal_export(char **args)
{

    //Variable
    int contador_export = 0;
    char *vector_export[2];

    if (args[1] == NULL)
    {
        printf("ERROR SINTAXIS - Uso: export <variable>=<valor>\n");

        return EXIT_SUCCESS;
    }
    else if (args[1] != NULL)
    {
        token_export = strtok(args[1], "=");

        while (token_export != NULL)
        {
            vector_export[contador_export] = token_export;

            //Impresion de token export
            /*
            printf("TokenExport_%d:%s\n", contador_export, token_export);
            */

            token_export = strtok(NULL, "\n");

            if (token_export != NULL)
            {
                contador_export++;
            }
        }

        if (contador_export == 0)
        {
            printf("ERROR SINTAXIS - Uso: export <variable>=<valor>\n");

            return EXIT_SUCCESS;
        }

        if (getenv(vector_export[0]))
        {
            printf("NOMBRE:%s\n", vector_export[0]);
            printf("VALOR:%s\n", vector_export[1]);
            printf("ANTIGUO valor para %s:%s\n", vector_export[0], getenv(vector_export[0]));

            setenv(vector_export[0], vector_export[1], 1);

            printf("NUEVO valor para %s:%s\n", vector_export[0], getenv(vector_export[0]));
            return EXIT_SUCCESS;
        }
        else
        {
            printf("ERROR - La variable especificada NO existe\n");
            return EXIT_SUCCESS;
        }
    }
}

//Internal SOURCE
int internal_source(char **args)
{

    //Variables
    FILE *fp;
    char *buff;

    if (args[1] == NULL)
    {
        printf("ERROR SINTAXIS - Uso: source <nombre fichero>\n");

        return EXIT_SUCCESS;
    }
    else
    {
        fp = fopen(args[1], "r");

        if (fp == NULL)
        {

            printf("ERROR - Fichero no encontrado\n");

            return EXIT_SUCCESS;
        }
        else
        {
            buff = malloc(1024);

            buff = fgets(buff, 50, fp);

            while (buff != NULL)
            {
                printf("LINEA: %s", buff);
                execute_line(buff);
                buff = fgets(buff, 50, fp);
                fflush(fp);
            }

            fclose(fp);
            free(buff);

            return EXIT_SUCCESS;
        }
    }
}

//Internal JOBS
int internal_jobs(char **args)
{
    printf("Comando JOBS ejecutado\n");
    return EXIT_SUCCESS;
}

//Internal FG
int internal_fg(char **args)
{
    printf("Comando FG ejecutado\n");
    return EXIT_SUCCESS;
}

//Internal BG
int internal_bg(char **args)
{
    printf("Comando BG ejecutado\n");
    return EXIT_SUCCESS;
}

//Internal Fork
int internal_fork(char **args)
{
    //Variables
    int pid_fork;
    int wstatus;

    printf("EJECUTANDO FORK\n");

    pid_fork = fork();

    jobs_list[0].pid = pid_fork;

    if (pid_fork == 0)
    {

        printf("[execute_line()→ PID hijo: %d]\n", getpid());

        if (execvp(args[0], args) < 0)
        {
            printf("ERROR - No exite ningun comando externo\n");

            exit(1);
        }

        return EXIT_SUCCESS;
    }
    else if (pid_fork > 0)
    {
        printf("[execute_line()→ PID padre: %d]\n", getpid());

        while (jobs_list[0].pid > 0)
        {

            //Pausamos al Padre
            pause();
        }

        return EXIT_SUCCESS;
    }
    else
    {
        printf("ERROR\n");

        return EXIT_SUCCESS;
    }
}

//Main de nuestro MiniShell (Blucle infinito de 2 acciones: Leer linea y ejecutarla)
int main()
{
    in_minishell = true;

    signal(SIGCHLD, reaper);
    signal(SIGINT, ctrlc);

    //Bucle infinito Leer y Ejecutar (Añadimos la impresión del encabezado)
    while (in_minishell != false)
    {
        imprimir_prompt();

        if (read_line(linea_buff))
        {
            execute_line(linea_buff);
        }
    }
}
