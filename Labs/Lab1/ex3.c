#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Função de comparação
int compare(const void *a, const void *b)
{
	return (*(int *)a - *(int *)b);
}

int main(void)
{

	// variaveis
	int filho;
	int status;
	int vetor[10] = {1, 4, 3, 5, 2, 8, 0, 7, 9};
	printf("\n Processo pai iniciado \n");
	printf("\n pid do pai %d \n", getpid());
	for (int i = 0; i < 10; i++)
	{
		printf("\n Posicao %d = %d \n", i, vetor[i]);
	}

	filho = fork();

	if (filho != 0)
	{

		waitpid(-1, &status, 0);
		for (int i = 0; i < 10; i++)
		{
			printf("\n Posicao %d = %d \n", i, vetor[i]);
		}
		printf("\n Processo do pai finalizado \n");
	}
	else
	{
		printf("\n Processo filho iniciado \n");
		qsort(vetor, 10, sizeof(int), compare);
		for (int i = 0; i < 10; i++)
		{
			printf("%d \n", vetor[i]);
		}

		printf("\n Processo do filho finalizado \n");
	}
	return 0;
}
