#include <stdio.h>
#include <stdlib.h>
#include "time.h"

#define MAX 1000 // valor maximo de um elemento do vetor

int main(int argc, char *argv[]) {
    float *vetor1, *vetor2; // vetores que serao gerados
    long int n; // quantidade de elementos dos vetores
    float elem1, elem2; // valores gerados para incluir nos vetores
    double produto_interno = 0; // produto interno dos vetores gerados
    int fator = 1; // fator multiplicador para gerar numeros negativos
    FILE *arquivo; // descritor do arquivo de saida
    size_t ret; // retorno da funcao de escrita no arquivo

    if (argc < 3) {
        printf("--ERRO: Informe os parametros, no formato: <%s> <dimensao> <nome_arquivo_saida>\n", argv[0]);
        return 1;
    }
    n = atoi(argv[1]);

    if (!(vetor1 = (float *) calloc(n, sizeof(float)))) {
        printf("--ERRO: Erro ao alocar primeiro vetor com calloc()\n");
        return 2;
    }

    if (!(vetor2 = (float *) calloc(n, sizeof(float)))) {
        printf("--ERRO: Erro ao alocar segundo vetor com calloc()\n");
        return 3;
    }

    // preenche os vetores com valores float aleatorios
    srand(time(NULL));
    for (long int i = 0; i < n; i++) {
        elem1 = (rand() % MAX) / 3.0 * fator;
        elem2 = (rand() % MAX) / 2.5 * fator;
        vetor1[i] = elem1;
        vetor2[i] = elem2;
        produto_interno += elem1 * elem2; // acumula o produto parcial
        fator *= -1;
    }

    // abre o arquivo para escrita binaria
    if (!(arquivo = fopen(argv[2], "wb"))) {
        printf("--ERRO: Erro na abertura do arquivo binario para escrita com fopen()\n");
        return 4;
    }

    // escreve a dimensao
    ret = fwrite(&n, sizeof(long int), 1, arquivo);
    // escreve o primeiro vetor
    ret = fwrite(vetor1, sizeof(float), n, arquivo);
    if (ret < n) {
        printf("--ERRO: Erro de escrita do primeiro vetor no arquivo com fwrite()\n");
        return 5;
    }
    // escreve o segundo vetor
    ret = fwrite(vetor2, sizeof(float), n, arquivo);
    if (ret < n) {
        printf("--ERRO: Erro de escrita do segundo vetor no arquivo com fwrite()\n");
        return 6;
    }
    // escreve o produto interno
    ret = fwrite(&produto_interno, sizeof(double), 1, arquivo);

    // finaliza/limpa o uso das variaveis
    fclose(arquivo);
    free(vetor1);
    free(vetor2);

    printf("Arquivo gerado com sucesso\n");

    return 0;
}