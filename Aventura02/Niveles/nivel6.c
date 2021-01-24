#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
//Autores
/*
Kieran Orr Donal
Pablo Núñez Pérez
Pau Capellà Ballester
*/
//Variables
bool in_minishell = false;
char linea_buff[50];
char current_directory[100];
char *vector_tokens[10];
char *token;
char *token_export;
int npids = 0;
bool allgood = true;

//Declaraciones de los Metodos
int imprimir_prompt();
int main();
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
int is_output_redirection(char **args);

//Struct
struct info_process
{
    pid_t pid;
    char status;                  // ‘N’, ’E’, ‘D’, ‘F’
    char cmd[sizeof(linea_buff)]; // línea de comando
};

static struct info_process jobs_list[64];

int jobs_list_add(pid_t pid, char status, char *cmd)
{
    if (npids < sizeof(jobs_list))
    {
        npids++;
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

int jobs_list_find(pid_t pid)
{
    if (npids > -1)
    {
        for (int i = 0; i <= npids; i++)
        {
            if (jobs_list[i].pid == pid)
            {
                return i;
            }
        }
    }
    else
    {

        printf("No se ha encontrado el proceso con PID: %d", pid);
        return -1;
    }
}
int jobs_list_remove(int pos)
{
    if (pos <= npids && pos > -1)
    {
        pid_t auxPid = jobs_list[pos].pid;
        char staAux = jobs_list[pos].status;
        char cmdAux[sizeof(linea_buff)];
        for (int i = 0; i < sizeof(jobs_list[pos].cmd); i++)
        {
            cmdAux[i] = jobs_list[pos].cmd[i];
        }

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
        jobs_list[npids].status = staAux;
        for (int i = 0; i < sizeof(cmdAux); i++)
        {
            jobs_list[npids].cmd[i] = cmdAux[i];
        }

        npids--;
    }
    else if (pos > npids)
    {
        printf("POSICION INCORRECTA");
    }
}

//Reaper
void reaper(int sigint)
{
    //Variables
    pid_t end;
    int stat;
    
    signal(SIGCHLD, reaper);
    while ((end = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        if (end != jobs_list[0].pid)
        {
            int i = jobs_list_find(end);
            printf("\nREAPER-> Proceso hijo en background: %d (%s) finalizado con la señal: %d\n", end, jobs_list[i].cmd, WEXITSTATUS(stat));
            jobs_list_remove(i);
        }
        else
        {
            printf("\nREAPER-> Proceso hijo: %d (%s) finalizado con la señal: %d\n", end, jobs_list[0].cmd, WEXITSTATUS(stat));
            //ESTABLECEMOS ESTADO
            jobs_list[0].status = 'F';
            //ESTABLECEMOS PID
            jobs_list[0].pid = 0;
            //ESTABLECEMOS EL CAMPO DE COMANDO
            for (int i = 0; i < sizeof(linea_buff); i++)
            {
                jobs_list[0].cmd[i] = '\0'; //LLENAMOS EL CAMPO DE COMANDO CON EL VALOR NULO
            }
        }
    }
    return;
}

void ctrlc(int signum)
{
    int i = SIGTERM;

    if (jobs_list[0].pid > 0)
    {
        if (jobs_list[0].pid != getppid())
        {
            printf("\nCTRL C ===> Señal %d enviada a %d (%s) por %d\n ", i, jobs_list[0].pid, jobs_list[0].cmd, getppid());
            kill(jobs_list[0].pid, SIGTERM);
        }
        else
        {
            printf("\nSeñal %d no enviada debido a que el proceso en foreground es el shell\n", i);
        }
    }
    else
    {
        printf("\nSeñal %d no enviada debido a que no hay proceso en foreground\n", i);
    }

    return;
}

void ctrlz(int signum)
{
    int i = SIGTERM;

    if (jobs_list[0].pid > 0)
    {
        if (jobs_list[0].pid != getppid())
        {
            kill(jobs_list[0].pid, SIGSTOP);
            printf("\nCTRL Z ===> Señal %d enviada a %d (%s) por %d\n ", i, jobs_list[0].pid, jobs_list[0].cmd, getppid());
            jobs_list_add(jobs_list[0].pid, 'D', jobs_list[0].cmd);
            //ESTABLECEMOS ESTADO
            jobs_list[0].status = 'F';
            //ESTABLECEMOS PID
            jobs_list[0].pid = 0;
            //ESTABLECEMOS EL CAMPO DE COMANDO
            for (int i = 0; i < sizeof(linea_buff); i++)
            {
                jobs_list[0].cmd[i] = '\0'; //LLENAMOS EL CAMPO DE COMANDO CON EL VALOR NULO
            }
        }
        else
        {
            printf("\nSeñal %d no enviada debido a que el proceso en foreground es el shell\n", i);
        }
    }
    else
    {
        printf("\nSeñal %d no enviada debido a que no hay proceso en foreground\n", i);
    }

    return;
}

bool is_background()
{
    int i = 0;
    for (; vector_tokens[i] != NULL; i++)
    {
    }
    i--;

    if (vector_tokens[i][0] == '&')
    {
        vector_tokens[i] = NULL;

        return true;
    }
    else
    {
        return false;
    };
}

//Metodo que imprime nuestro prompt
int imprimir_prompt()
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
    printf(" __  __ _       _    _____ _          _ _ \n");
    printf("|  \\/  (_)     (_)  / ____| |        | | |\n");
    printf("| \\  / |_ _ __  _  | (___ | |__   ___| | |\n");
    printf("| |\\/| | | '_ \\| |  \\___ \\| '_ \\ / _ \\ | |\n");
    printf("| |  | | | | | | |  ____) | | | |  __/ | |\n");
    printf("|_|  |_|_|_| |_|_| |_____/|_| |_|\\___|_|_|  Nivel_06\n");
    printf("\n");

    printf(WHT "[MiniShell]");
    printf(GRN "[%s]", getenv("USER"));
    printf(BLU "[%s]" WHT ":", current_directory);
    fflush(stdout);

    return 0;
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
    signal(SIGCHLD, reaper);

    if (parse_args(vector_tokens, line) != 0)
    {

        if (check_internal(vector_tokens) == 1)
        {
            
            internal_fork(vector_tokens, is_background());

            return EXIT_SUCCESS;
        }
    }

    return EXIT_SUCCESS;
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

    for (; args[array_tokens][0] == '#'; array_tokens--)
    {
        args[array_tokens] = NULL;
    };

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

    return EXIT_SUCCESS;
}

//Internal EXPORT
int internal_export(char **args)
{

    //Variable
    int contador_export = 0;
    char *vector_export[2];

    if (args[1] == NULL)
    {
        printf("ERROR SINTAXIS\nUso: export <variable>=<valor>\n");
    }
    else if (args[1] != NULL)
    {
        token_export = strtok(args[1], "=");

        while (token_export != NULL)
        {
            vector_export[contador_export] = token_export;

            printf("TokenExport_%d:%s\n", contador_export, token_export);

            token_export = strtok(NULL, "\n");

            if (token_export != NULL)
            {
                contador_export++;
            }
        }

        if (getenv(vector_export[0]))
        {
            printf("NOMBRE:%s\n", vector_export[0]);
            printf("VALOR:%s\n", vector_export[1]);
            printf("ANTIGUO valor para %s:%s\n", vector_export[0], getenv(vector_export[0]));

            setenv(vector_export[0], vector_export[1], 1);

            printf("Comando EXPORT ejecutado\n");
            printf("NUEVO valor para %s:%s\n", vector_export[0], getenv(vector_export[0]));
            return EXIT_SUCCESS;
        }
        else
        {
            printf("ERROR - La variable especificada NO existe\n");
            return EXIT_FAILURE;
        }
    }
}

//Internal SOURCE
int internal_source(char **args)
{

    //Variable
    FILE *fp;
    char str[50];

    printf("NOMBRE FICHERO:%s\n", args[1]);

    if (args[1] == NULL)
    {
        fprintf(stderr, "ERROR - SINTAXIS\nUso: source <nombre fichero>\n");
        return EXIT_FAILURE;
    }
    else if (fopen(args[1], "r") == NULL)
    {
        fprintf(stderr, "ERROR - FICHERO NO ENCONTRADO\n");
        return EXIT_FAILURE;
    }
    else
    {
        fp = fopen(args[1], "r");

        while (fgets(str, 50, fp) != NULL)
        {
            execute_line(str);
            fflush(fp);
        }
        fclose(fp);
    }

    return EXIT_SUCCESS;
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

        kill(jobs_list[i].pid, SIGCONT);
        jobs_list[0].pid = jobs_list[i].pid;
        strcat(jobs_list[0].cmd, jobs_list[i].cmd);
        jobs_list[0].status = 'E';
        jobs_list_remove(i);
        printf("\n[internal_fg()→ Señal %d (SIGCONT) enviada a %d (%s)]\n", SIGCONT, jobs_list[0].pid, jobs_list[0].cmd);
    }
    else
    {
        jobs_list[0].pid = jobs_list[i].pid;
        strcat(jobs_list[0].cmd, jobs_list[i].cmd);
        jobs_list[0].status = 'E';
        jobs_list_remove(i);
        printf("\n%s\n", jobs_list[i].cmd);
    };

    while (jobs_list[0].status == 'E')
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
        kill(jobs_list[i].pid, SIGCONT);
        jobs_list[i].status = 'E';

        printf("\n[internal_bg()→ Señal %d (SIGCONT) enviada a %d (%s)]\n", SIGCONT, jobs_list[i].pid, jobs_list[i].cmd);
    }
    else
    {

        return EXIT_SUCCESS;
    }
}

//Internal Fork
int internal_fork(char **args, bool background)
{
    //Variable

    int status = 'E';
    pid_t childpid;
    childpid = fork();

    if (childpid == 0)
    {
        if (background)
        {
            signal(SIGTSTP, SIG_IGN);
            signal(SIGINT, SIG_IGN);
            signal(SIGCHLD, SIG_DFL);
            
        }

        else
        {

            signal(SIGTSTP, SIG_IGN);
            signal(SIGINT, SIG_IGN);
            signal(SIGCHLD, SIG_DFL);
            jobs_list[0].status = 'E';
            jobs_list[0].pid = getpid();

           

            for (int i = 0; args[i] != NULL; i++)
            {
                strcat(jobs_list[0].cmd, args[i]);
                strcat(jobs_list[0].cmd, " ");
            }
        }

        is_output_redirection(vector_tokens);

        if (execvp(args[0], args) < 0)
        {
           

            exit(0);
        }
        exit(0);
        return EXIT_SUCCESS;
    }
    else
    {

        
        if (background)
        {
            printf("Adding background\n");
            char linea[50];
            printf("%s\n", linea);
            jobs_list_add(childpid, 'E', jobs_list[0].cmd);
            return EXIT_SUCCESS;
        }
        else
        {

            jobs_list[0].status = 'E';
            jobs_list[0].pid = childpid;
            while (jobs_list[0].status == 'E')
            {
                pause();
            }
            return EXIT_SUCCESS;
        }
    }
}

//Redireccionamiento de la salida
int is_output_redirection(char **args)
{
    //Variables
    bool encontrado = false;
    int fp;

    for (int i = 0; args[i] != NULL; i++)
    {

        if (encontrado)
        {
            if (fp = open(args[i], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR))
            {
                dup2(fp, 1);
                execvp(args[i], &args[0]);
                close(fp);
                return EXIT_SUCCESS;
            }
        }
        else if (args[i][0] == '>')
        {
            args[i] = NULL;
            encontrado = true;
        }
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