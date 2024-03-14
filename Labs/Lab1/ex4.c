#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>



int main(void)
{

	// variaveis
	int filho;
	int status;
	printf("\n Processo pai iniciado \n");
	printf("\n pid do pai %d \n", getpid());

	filho = fork();

	if (filho != 0)
	{

		waitpid(-1, &status, 0);
		
		printf("\n Processo do pai finalizado \n");
	}
	else
	{
		printf("\n Processo filho iniciado \n");
		execl("/home/tomas/Documentos/PUC/SO/Sistemas_Operacionais_INF1019/Labs/Lab1/aloMundo", "aloMundo");

		printf("\n Processo do filho finalizado \n");
	}
	return 0;
}
