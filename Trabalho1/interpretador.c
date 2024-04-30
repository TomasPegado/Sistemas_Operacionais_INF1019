#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/types.h>  
#include <unistd.h>

int main() {
    FILE *file;
    char path[100];
    char program[100];
    char line[200];
    int status;

    file = fopen("/home/tomas/PUC/SistemasOperacionais/Trabalho1/programas.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%s %s", path, program) == 2) {
            printf("Executing: %s %s\n", path, program);
            int pid = fork();
            if (pid != 0) { // Processo pai

                waitpid(-1, &status, 0); // espera processo filho terminar
		
		        printf("\n Processo do pai finalizado \n");
                
            } else { // Processo filho

                printf("\n Processo filho iniciado \n");
                execl(path, program, (char *)NULL);
                
                perror("execl failed");
                exit(EXIT_FAILURE);
            }
            // } else { // Error forking
            //     perror("fork failed");
            //     exit(EXIT_FAILURE);
            // }
        }
    }

    fclose(file);
    return EXIT_SUCCESS;
}
