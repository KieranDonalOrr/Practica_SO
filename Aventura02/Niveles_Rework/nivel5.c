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
char *token_ctrz;

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
int internal_fork(char **args, bool background);
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
int npids = 0;

//Metodo de añadir un proceso al jobs list
int jobs_list_add(pid_t pid, char status, char *cmd)
{
    //SI el numero de procesos activos, es menor al tamaño de jobs  list
    if (npids < sizeof(jobs_list))
    {
        //Aumentamos npids
        npids++;
        //Establecemos las variables del proceso
        jobs_list[npids].pid = pid;
        jobs_list[npids].status = status;
        for (int i = 0; i < sizeof(cmd); i++)
        {
            jobs_list[npids].cmd[i] = cmd[i];
        }
    }
    else
    {
        printf("NO HAY ESPACIOS EN JOBS LISTS");
    }
}
//Metodo de busqueda de un proceso
int jobs_list_find(pid_t pid)
{
    //Si el numero de procesos es mayor a -1
    if (npids > -1)
    {
        //Iteramos en jobs lists hasta encontrar un proceso con el mismo pid pasado por parametro
        for (int i = 0; i <= npids; i++)
        {
            if (jobs_list[i].pid == pid)
            {
                //Return indice
                return i;
            }
        }
        //Caso en el que no se encuentre el proceso con el pid especificado
        printf("No se ha encontrado el proceso con PID: %d", pid);
        return -1;
    }
    //No hay procesos
    else
    {

        printf("No se ha encontrado el proceso con PID: %d", pid);
        return -1;
    }
}
int jobs_list_remove(int pos)
{
    //Si la posicion es menor o igual al numero de procesos y además es mayor que -1
    if (pos <= npids && pos > -1)
    {
        //Variables auxiliares
        pid_t auxPid = jobs_list[pos].pid;
        char staAux = jobs_list[pos].status;
        char cmdAux[sizeof(linea_buff)];
        for (int i = 0; i < sizeof(jobs_list[pos].cmd); i++)
        {
            cmdAux[i] = jobs_list[pos].cmd[i];
        }
        //Reordenamos jobs_list
        for (int i = pos; pos <= npids; pos++)
        {
            jobs_list[pos].pid = jobs_list[pos + 1].pid;
            jobs_list[pos].status = jobs_list[pos + 1].status;
            for (int i = 0; i < sizeof(jobs_list[pos + 1].cmd); i++)
            {
                jobs_list[pos].cmd[i] = jobs_list[pos + 1].cmd[i];
            }
        }
        jobs_list[npids].pid = auxPid;
        jobs_list[npids].status = 'F';
        for (int i = 0; i < sizeof(cmdAux); i++)
        {
            jobs_list[npids].cmd[i] = cmdAux[i];
        }

        npids--;
    }
    //Posicion invalida
    else if (pos > npids)
    {
        printf("POSICION INCORRECTA");
    }
}

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
        if (end == jobs_list[0].pid)
        {

            token_reaper = strtok(jobs_list[0].cmd, "\n");

            //Devolver el estado de finalizacion del hijo
            if (WIFEXITED(wstatus))
            {
                printf("\n[reaper()→ Proceso hijo %d (%s) finalizado en foreground con estado: %d]\n", end, token_reaper, WEXITSTATUS(wstatus));
            }
            else if (WIFSIGNALED(wstatus))
            {
                printf("\n[reaper()→ Proceso hijo %d (%s) finalizado en foreground con la señal: %d]\n", end, token_reaper, WTERMSIG(wstatus));
            }
            else if (WIFSTOPPED(wstatus))
            {
                printf("\n[reaper()→ Proceso hijo %d (%s) finalizado en foreground con la señal: %d]\n", end, token_reaper, WIFSTOPPED(wstatus));
            }

            //Establecemos estados
            jobs_list[0].pid = 0;
            jobs_list[0].status = 'F';
            jobs_list[0].cmd[0] = '\0';
        }
        else
        {
            int i = jobs_list_find(end);
            if (WIFEXITED(wstatus))
            {
                printf("\n[reaper()→ Proceso hijo %d (%s) finalizado en background con estado: %d]\n", end, jobs_list[i].cmd, WEXITSTATUS(wstatus));
            }
            else if (WIFSIGNALED(wstatus))
            {
                printf("\n[reaper()→ Proceso hijo %d (%s) finalizado en background con la señal: %d]\n", end, jobs_list[i].cmd, WTERMSIG(wstatus));
            }
            else if (WIFSTOPPED(wstatus))
            {
                printf("\n[reaper()→ Proceso hijo %d (%s) finalizado en background con la señal: %d]\n", end, jobs_list[i].cmd, WIFSTOPPED(wstatus));
            }
            jobs_list_remove(i);
        }
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

void ctrlz(int signum)
{

    int i = SIGSTOP;

        token_ctrz = strtok(jobs_list[0].cmd, "\n");
    
   
    printf("\n[ctrlz()→ Soy el proceso con PID %d, el proceso en foreground es %d (%s)]\n", getpid(), jobs_list[0].pid, token_ctrz);
    fflush(stdout);

    if (jobs_list[0].pid > 0)
    {
        if (strcmp(linea_buff, jobs_list[0].cmd) != 0)
        {

            printf("\nCTRL Z ===> Señal %d enviada a %d (%s) por %d\n ", i, jobs_list[0].pid, jobs_list[0].cmd, getpid());
            jobs_list_add(jobs_list[0].pid, 'D', jobs_list[0].cmd);
            //ESTABLECEMOS ESTADO
            jobs_list[0].status = 'F';
            //ESTABLECEMOS PID
            jobs_list[0].pid = 0;
            kill(jobs_list[npids].pid, SIGSTOP);
            //ESTABLECEMOS EL CAMPO DE COMANDO
            for (int i = 0; i < sizeof(linea_buff); i++)
            {
                jobs_list[0].cmd[i] = '\0'; //LLENAMOS EL CAMPO DE COMANDO CON EL VALOR NULO
            }
        }
        else
        {
            printf("[ctrlz()→ Señal %d no enviada por PID %d debido a que el proceso en foreground es el shell]\n", SIGSTOP, getpid());
            fflush(stdout);
        }
    }
    else
    {
        printf("[ctrlz()→ Señal %d no enviada por PID %d debido a que no hay proceso en foreground]\n", SIGSTOP, getpid());
        fflush(stdout);
    }
    return;
}

bool is_background()
{
    //Variable indice para navegar por el vector de tokens
    int i = 0;
    for (; vector_tokens[i] != NULL; i++)
    {
    }
    //Decrementamos i para posicionarnos en el ultimo token
    i--;
    //Si es &, es un proceso en background
    if (vector_tokens[i][0] == '&')
    {
        vector_tokens[i] = NULL;

        return true;
    }
    else
    {
        printf("No es background");
        return false;
    };
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
            internal_fork(vector_tokens, is_background());
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

        token = strtok(NULL, delimiters);

        if (token != NULL)
        {
            array_tokens++;
        }
    }

    //Quitamos el token comentario
    if (array_tokens != 0)
    {
        for (; args[array_tokens][0] == '#'; array_tokens--)
        {
            args[array_tokens] = NULL;
        };
    }

    return num_tokens;
}

//Metodo que filtra el comando
int check_internal(char **args)
{

    if (strcmp(args[0], "cd") == 0)
    {

        internal_cd(vector_tokens);
    }
    else if (strcmp(args[0], "export") == 0)
    {

        internal_export(vector_tokens);
    }
    else if (strcmp(args[0], "source") == 0)
    {

        internal_source(vector_tokens);
    }
    else if (strcmp(args[0], "jobs") == 0)
    {

        internal_jobs(vector_tokens);
    }
    else if (strcmp(args[0], "fg") == 0)
    {

        internal_fg(vector_tokens);
    }
    else if (strcmp(args[0], "bg") == 0)
    {

        internal_bg(vector_tokens);
    }
    else if (strcmp(args[0], "exit") == 0)
    {
        //printf("EJECUTANDO EXIT\n");
        //printf("CERRAMOS MINISHELL\n");
        exit(0);
    }
    else if (strcmp(args[0], "clear") == 0)
    {

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
    if (npids == 0)
    {
        printf("No hay nada en background\n");
    }
    else
    {
        for (int i = 1; i <= npids; i++)
        {
            printf("[%d] %d   %c   %s \n", i, jobs_list[i].pid, jobs_list[i].status, jobs_list[i].cmd);
        }
    }
    return EXIT_SUCCESS;
}

//Internal FG
int internal_fg(char **args)
{
    //Comprobamos que la sintaxis
    if (args[1] == NULL)
    {

        printf("\nSINTAX ERROR: FG < elemento a poner en foreground>\n");
        return EXIT_SUCCESS;
    }
    else
        ;

    int i = atoi(args[1]);
    if (jobs_list[i].pid == 0)
    {
        printf("\nERROR NO HAY PROCESSO\n");
        return EXIT_SUCCESS;
    }
    for (int i = 0; i < sizeof(jobs_list[0].cmd); i++)
    {
        jobs_list[0].cmd[i] = '\0';
    }

    if (jobs_list[i].status == 'D')
    {

        jobs_list[0].pid = jobs_list[i].pid;
        strcat(jobs_list[0].cmd, jobs_list[i].cmd);
        jobs_list[0].status = 'E';
        jobs_list_remove(i);
        printf("\n[internal_fg()→ Señal %d (SIGCONT) enviada a %d (%s)]\n", SIGCONT, jobs_list[0].pid, jobs_list[0].cmd);
        kill(jobs_list[i].pid, SIGCONT);
    }
    else
    {
        if (i <= npids)
        {
            jobs_list[0].pid = jobs_list[i].pid;
            strcat(jobs_list[0].cmd, jobs_list[i].cmd);
            jobs_list[0].status = 'E';
            jobs_list_remove(i);
            printf("\n%s\n", jobs_list[i].cmd);
        }
        else
        {
            printf("\nNo hay ningun processo en esta posicion\n");
        }
    };

    while (jobs_list[0].pid > 0)
    {
        pause();
    }

    return EXIT_SUCCESS;
}

//Internal BG
int internal_bg(char **args)
{
    if (args[1] == NULL)
    {

        printf("\nSINTAX ERROR: FG < elemento a poner en foreground>\n");
        return EXIT_SUCCESS;
    }
    else
        ;

    int i = atoi(args[1]);
    if (jobs_list[i].pid == 0)
    {
        printf("\nERROR NO HAY PROCESSO\n");
        return EXIT_SUCCESS;
    }
    for (int i = 0; i < sizeof(jobs_list[0].cmd); i++)
    {
        jobs_list[0].cmd[i] = '\0';
    }

    if (jobs_list[i].status == 'D')
    {

        jobs_list[i].status = 'E';

        printf("\n[internal_bg()→ Señal %d (SIGCONT) enviada a %d (%s)]\n", SIGCONT, jobs_list[i].pid, jobs_list[i].cmd);
        kill(jobs_list[i].pid, SIGCONT);
    }
    else
    {

        return EXIT_SUCCESS;
    }
}

//Internal Fork
int internal_fork(char **args, bool background)
{
    //Variables
    int pid_fork;
    int wstatus;

    pid_fork = fork();
    signal(SIGTSTP, SIG_IGN);
        signal(SIGINT, SIG_IGN);
        signal(SIGCHLD, SIG_DFL);

    for (int i = 0; args[i] != NULL; i++)
    {
        strcat(jobs_list[0].cmd, args[i]);
        strcat(jobs_list[0].cmd, " ");
    }

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
        if (background)
        {
            strcat(jobs_list[0].cmd, " ");
            strcat(jobs_list[0].cmd, "&");
            jobs_list_add(pid_fork, 'E', jobs_list[0].cmd);
            printf("[%d] %d   %c   %s \n", npids, jobs_list[npids].pid, jobs_list[npids].status, jobs_list[npids].cmd);
            return EXIT_SUCCESS;
        }
        else
        {
            jobs_list[0].status = 'E';
            jobs_list[0].pid = pid_fork;

            while (jobs_list[0].pid > 0)
            {
                //Pausamos al Padre
                pause();
            }
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
    signal(SIGTSTP, ctrlz);

    //Bucle infinito Leer y Ejecutar (Añadimos la impresión del encabezado)
    while (in_minishell != false)
    {
        usleep(10000);
        imprimir_prompt();

        if (read_line(linea_buff))
        {
            execute_line(linea_buff);
        }
    }
}