#include  <stdio.h>
#include  <string.h>
#include  <sys/types.h>
#include  <sys/wait.h>
#include <unistd.h>

int main(void){

	// variaveis
	int var;
    int filho;
	int status;
	

	var = 1;
	printf("\n Processo pai iniciado \n");
	printf("\n pid do pai %d \n", getpid());
	printf("\n Variável = %d \n", var);
	filho = fork();


	if( filho != 0 ){
		
		waitpid(-1, &status, 0);
		printf("\n Variável = %d \n",var);
		printf("\n Processo do pai finalizado \n");
	}
	else {
		printf("\n Processo filho iniciado \n");
		var = 5;
		printf("\n Variável = %d \n",var);
		printf("\n Processo do filho finalizado \n");
	}
	return 0;
}
    