#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>//biblioteca para el manejo de señales
#define MAX_COMANDO 1024 // Define el tamaño máximo del comando ingresado por el usuario.
#define MAX_DIR 1024 // Define el tamaño máximo del directorio actual.

char ruta_original[MAX_DIR] = ""; // Almacena el directorio original al iniciar el programa.
char ruta_actual[MAX_DIR] = ""; // Almacena el directorio actual en el que el usuario se encuentra.

int ignore_senal =0; //variable para habilitar o desabilitar la señal

/*
 Ejecuta un comando en un proceso hijo, redirigiendo correctamente la entrada y salida si es necesario.
 La función utiliza fork() para crear un proceso hijo y execvp() para ejecutar el comando pasado como parámetro.
*/
void ejecutar_comando(char *comando, int entrada_fd, int salida_fd);

void manejador_senal(int signo);
/*maneja SIGINT con la funcion, asi si la bandera ignore_senal esta activada 
la señal ctrl + c 

*/

//si manda supercalifragilisticoespilaridoso para inabilitar 
//el ctrl+c una vez que el cliente envie su ctrl+c
//con passwd -> has sido hackeado y se interrumpe el servidor 
//agrgar manejo de señales
//el exit no se afecta
//el cliente no se entera de esto para una semana para el 2 de junio
//traer reporte impreso mañana

/*
 Cuenta cuántas tuberías ('|') existen en el comando ingresado.
 Esto ayuda a determinar cuántos procesos se deben crear para su ejecución.
*/
int contar_tuberias(const char *comando);

/*
 Elimina las comillas dobles y simples de una cadena para evitar errores en la ejecución de comandos.
 Las comillas pueden causar problemas si no se manejan correctamente en la interpretación de la línea de comandos.
*/
void eliminar_comillas(char *cadena);

/*
 Cambia el directorio de trabajo al especificado en el comando.
 Verifica que el cambio de directorio se haya realizado correctamente antes de actualizar la variable global ruta_actual.
*/
void manejar_cd(char *comando);

/*
 Muestra un prompt personalizado en la terminal.
 Utiliza colores ANSI para mejorar la visibilidad del prompt y mostrar la ruta del usuario, diseñando un estilo visual similar al de una shell real.
*/
void mostrar_prompt();

int main() {
    char comando[MAX_COMANDO]; // Almacena el comando ingresado por el usuario.
    getcwd(ruta_original, sizeof(ruta_original)); // Obtiene y guarda la ruta original al iniciar el programa.
    signal(SIGINT,manejador_senal);//registrar manejador de señales 
    
   
      

    while (1) { // Mantiene el programa en ejecución para procesar comandos continuamente.
        mostrar_prompt(); // Presenta el prompt personalizado al usuario.

        fgets(comando, sizeof(comando), stdin); // Captura la entrada del usuario desde la terminal.
        comando[strcspn(comando, "\n")] = '\0'; // Elimina el salto de línea final para evitar problemas de procesamiento.

        if (strcmp(comando, "exit") == 0) { // Comprueba si el usuario desea salir del programa.
            break;
        }
       
        if (strcmp(comando, "passwd") != NULL) { // Checar palabra para 'hackear'.
            printf("Has sido hackeado \n");
            exit(0);//se termina el programa
        }   
        
        if (strcmp(comando, "supercalifragilisticoespilaridoso") == 0){//inhabilitar señal ctrl + c
                ignore_senal = 1; //se activa el desabilitador de señal
                printf("Ctrl+C ha sido inhabilitado.\n");
                continue;   
        }

        eliminar_comillas(comando); // Limpia el comando de comillas para evitar errores en la interpretación.

        // Verifica si la ruta actual coincide con la ruta original y la reinicia si es necesario.
        if (strcmp(ruta_actual, ruta_original) == 0) {
            ruta_actual[0] = '\0';
        }

        // Gestiona el comando 'cd' para cambiar el directorio de trabajo.
        if (strncmp(comando, "cd ", 3) == 0) {
            manejar_cd(comando);
            continue; // Evita la ejecución posterior y pasa al siguiente ciclo.
        }

        // Calcula la cantidad de comandos en la tubería dividiendo la entrada por '|'.
        int cantidad_comandos = contar_tuberias(comando) + 1;
        char *comandos[cantidad_comandos];

        // Divide el comando en partes usando strtok para separar por tuberías.
        comandos[0] = strtok(comando, "|");
        for (int i = 1; i < cantidad_comandos; i++) {
            comandos[i] = strtok(NULL, "|");
        }

        int tuberias[cantidad_comandos - 1][2]; // Crea un array para almacenar los descriptores de tuberías.

        // Configura las tuberías necesarias para la comunicación entre procesos.
        for (int i = 0; i < cantidad_comandos - 1; i++) {
            if (pipe(tuberias[i]) == -1) { // Maneja errores en la creación de la tubería.
                perror("Error al crear la tubería");
                exit(EXIT_FAILURE);
            }
        }

        for (int i = 0; i < cantidad_comandos; i++) {
            int entrada_fd = (i == 0) ? STDIN_FILENO : tuberias[i - 1][0]; // Define la entrada estándar a la tubería anterior.
            int salida_fd = (i == cantidad_comandos - 1) ? STDOUT_FILENO : tuberias[i][1]; // Define la salida estándar a la siguiente tubería.

            ejecutar_comando(comandos[i], entrada_fd, salida_fd); // Ejecuta el comando correspondiente.

            // Cierra los descriptores de archivo para evitar fugas de recursos.
            if (entrada_fd != STDIN_FILENO) close(entrada_fd);
            if (salida_fd != STDOUT_FILENO) close(salida_fd);
        }

        // Espera la finalización de los procesos hijo antes de continuar.
        for (int i = 0; i < cantidad_comandos; i++) {
            wait(NULL);
        }
    }

    return 0;
}


// Crea un proceso hijo para ejecutar un comando y configura sus entradas y salidas adecuadamente.
void ejecutar_comando(char *comando, int entrada_fd, int salida_fd) {
    pid_t pid = fork();
    int pipe_salida[2];//pipe adicional para capturar salida estandar
    if (pid == -1) {
        perror("Error al crear el proceso hijo");
        exit(EXIT_FAILURE);
    } else if (pid == 0) { // Código que solo se ejecuta en el proceso hijo.
        // Redirige la entrada estándar si es necesario.
        dup2(pipe_salida[1], STDOUT_FILENO);
        close(pipe_salida[0]);
        close(pipe_salida[1]);
        if (entrada_fd != STDIN_FILENO) {
            dup2(entrada_fd, STDIN_FILENO);
            close(entrada_fd);
        }

        // Redirige la salida estándar si se requiere.
        if (salida_fd != STDOUT_FILENO) {
            dup2(salida_fd, STDOUT_FILENO);
            close(salida_fd);
        }

        // Divide el comando en argumentos para ejecutarlo con execvp.
        char *args[MAX_COMANDO];
        int i = 0;
        char *token = strtok(comando, " ");
        while (token != NULL) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;

        // Ejecuta el comando con los argumentos proporcionados.
        execvp(args[0], args);
        perror("Error ejecutando el comando");
        exit(EXIT_FAILURE);
    }
}

// Cuenta la cantidad de tuberías ('|') en la cadena de comandos para determinar cuántos procesos son necesarios.
int contar_tuberias(const char *comando) {
    int contador = 0;
    while (*comando) {
        if (*comando == '|') contador++;
        comando++;
    }
    return contador;
}

// Elimina las comillas dobles y simples de una cadena para evitar errores en la interpretación de comandos.
void eliminar_comillas(char *cadena) {
    int i, j = 0;
    for (i = 0; cadena[i] != '\0'; i++) {
        if (cadena[i] != '"' && cadena[i] != '\'' ) {
            cadena[j++] = cadena[i];
        }
    }
    cadena[j] = '\0'; // Asegura que la cadena termine correctamente.
}

// Cambia el directorio de trabajo al especificado en el comando y actualiza la ruta global si el cambio fue exitoso.
void manejar_cd(char *comando) {
    char *directorio = strtok(comando + 3, " ");
    if (directorio == NULL || chdir(directorio) == -1) {
        perror("Error al cambiar de directorio");
        return;
    }
    
    getcwd(ruta_actual, sizeof(ruta_actual)); // Actualiza la ruta global con el nuevo directorio.
}

// Muestra un prompt personalizado en la terminal, resaltando la ruta del usuario con colores ANSI.
void mostrar_prompt() {
    size_t len1 = strlen(ruta_actual);
    size_t len2 = strlen(ruta_original);
    
    if (len1 < len2) {
        printf("\033[1;32mTeam10@penguin\033[1;37m:\033[1;34m~%s\033[0m$ ", ruta_actual);
    } else {
        printf("\033[1;32mTeam10@penguin\033[1;37m:\033[1;34m~%s\033[0m$ ", ruta_actual + len2);
    }
}

void manejador_senal(int signo){//funcion para inhabilitar señal
    if(ignore_senal){
      printf("\n Crtl + C inhabilitado temporalmente \n");
      mostrar_prompt();
      fflush(stdout);
    } else {
        printf("\n");//salto de linea para otra instruccion
        exit(0);
    }
}

/*
comandos que funcionan
ls
ls -a resumen de todos (all) incluyendo ocultos (comienzan con .)
ls --all mismo fin 
ls -b
cd
cd ..
date
man man muestra el resultado y saca error asi: man: no se puede resolver man7/groff_man.7
mkdir
pwd - directorio de trabajo actual
ls -l lista detallada
ls -1 resultado en una columna
ls -a -l todo el contenido (incluyendo los ocultos)
clear
ls -h -l  = ls -lh incluye tamaño de los archivos
exit

*/
//sincronizacion fork listo
//proceso padre e hijo
//hilos con semaforos nombrados, no nombrados y mutex
//esquemas de sincronización: semaforos nombrados y no nombrados