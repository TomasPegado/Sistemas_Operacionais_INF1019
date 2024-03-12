#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void bubbleSort(int vetor[], int n)
{
	int i, j, temp;
	for (i = 0; i < n - 1; i++)
	{
		// Passa pelo vetor de 0 a n-i-1
		// Os últimos i elementos já estão no lugar
		for (j = 0; j < n - i - 1; j++)
		{
			// Troca se o elemento encontrado é maior
			// que o próximo elemento
			if (vetor[j] > vetor[j + 1])
			{
				temp = vetor[j];
				vetor[j] = vetor[j + 1];
				vetor[j + 1] = temp;
			}
		}
	}
}

int main(void)
{

	// variaveis
	int vetor[10] = {1, 4, 3, 5, 2, 8, 0, 7, 9};
	pid_t pidfilho, pidpai;
	int status;

	pidfilho = fork(); // retorna o pid do filho'
	pidpai = getpid(); // retorna o pid do pai

	if (pidfilho != 0)
	{
		printf("\n Processo pai iniciado \n");
		printf("\n pid do pai %d \n", pidpai);
		for (int i = 0; i < 10; i++)
		{
			printf("\n Posicao %d = %d \n", i, vetor[i]);
		}
		waitpid(-1, &status, 0);
		printf("\n Pos waitpid \n");
		for (int i = 0; i < 10; i++)
		{
			printf("\n Posicao %d = %d \n", i, vetor[i]);
		}
		printf("\n Processo do pai finalizado \n");
	}
	else
	{
		printf("\n Processo filho iniciado \n");
		bubbleSort(vetor, 10);
		for (int i = 0; i < 10; i++)
		{
			printf("%d ", vetor[i]);
		}

		printf("\n pid do pai %d \n", pidpai);
		printf("\n pid do filho %d \n", pidfilho);
		printf("\n Processo do filho finalizado \n");
	}
	return 0;
}
