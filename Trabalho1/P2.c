#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ROW1 2
#define COL1 3
#define ROW2 3
#define COL2 2

void multiplyMatrices(int first[][COL1], int second[][COL2], int result[][COL2])
{
    for (int i = 0; i < ROW1; i++)
    {
        for (int j = 0; j < COL2; j++)
        {
            result[i][j] = 0;
            for (int k = 0; k < COL1; k++)
            {
                result[i][j] += first[i][k] * second[k][j];
            }
        }
    }
}

void printMatrix(int matrix[][COL2])
{
    for (int i = 0; i < ROW1; i++)
    {
        for (int j = 0; j < COL2; j++)
        {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}

int main()
{
    int first[ROW1][COL1] = {
        {1, 2, 3},
        {4, 5, 6}};
    int second[ROW2][COL2] = {
        {7, 8},
        {9, 10},
        {11, 12}};
    int result[ROW1][COL2];

    int n = 5;
    printf("Calculando Resultado da Multiplicação em P2...\n");
    while (n)
    {
        sleep(1);
        n--;
    }

    multiplyMatrices(first, second, result);
    printf("Resultado da multiplicação de matriz de P2:\n");
    printMatrix(result);

    return 0;
}
