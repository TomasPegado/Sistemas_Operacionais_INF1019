#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_ACESSOS 100
#define NUM_PROCESSOS 3
#define NUM_PAGINAS 16

void gerar_acessos(const char *nome_arquivo, int seed_offset)
{
    FILE *arquivo = fopen(nome_arquivo, "w");
    if (arquivo == NULL)
    {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL) + seed_offset);

    for (int i = 0; i < NUM_ACESSOS; i++)
    {
        int pagina = rand() % NUM_PAGINAS;
        char operacao = (rand() % 2) == 0 ? 'R' : 'W';
        fprintf(arquivo, "%02d %c\n", pagina, operacao);
    }

    fclose(arquivo);
}

int main(void)
{
    const char *arquivos[NUM_PROCESSOS] = {"acessos_P1.txt", "acessos_P2.txt", "acessos_P3.txt"};

    for (int i = 0; i < NUM_PROCESSOS; i++)
    {
        gerar_acessos(arquivos[i], i);
    }

    printf("Arquivos de acesso gerados com sucesso.\n");
    return 0;
}
