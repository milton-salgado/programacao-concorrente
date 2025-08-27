#include <stdio.h>
#include <stdlib.h>
#include "timer.h"

int main(int argc, char *argv[]) {
    float *vetor1, *vetor2; // vetores de entrada
    long int n; // dimensao dos vetores
    FILE *arquivo; // arquivo de entrada
    size_t ret; // retorno da funcao de leitura
    double produto_original; // produto interno registrado no arquivo
    double produto_sequencial = 0; // resultado do calculo sequencial
    double tempo_inicio, tempo_fim, tempo_processamento;

    if (argc < 2) {
        printf("--ERRO: Informe os parametros, no formato: <%s> <arquivo_entrada>\n", argv[0]);
        return 1;
    }

    if (!(arquivo = fopen(argv[1], "rb"))) {
        printf("--ERRO: Erro na abertura do arquivo para leitura com fopen()\n");
        return 2;
    }

    ret = fread(&n, sizeof(long int), 1, arquivo);
    if (!ret) {
        printf("--ERRO: Erro de leitura das dimensoes dos vetores no arquivo\n");
        return 3;
    }

    if (!(vetor1 = (float *) malloc(sizeof(float) * n))) {
        printf("--ERRO: Erro ao alocar primeiro vetor com malloc()\n");
        return 4;
    }

    if (!(vetor2 = (float *) malloc(sizeof(float) * n))) {
        printf("--ERRO: Erro ao alocar segundo vetor com malloc()\n");
        return 5;
    }

    ret = fread(vetor1, sizeof(float), n, arquivo);
    if (ret < n) {
        printf("--ERRO: Erro de leitura do primeiro vetor no arquivo\n");
        return 6;
    }

    ret = fread(vetor2, sizeof(float), n, arquivo);
    if (ret < n) {
        printf("--ERRO: Erro de leitura do segundo vetor no arquivo\n");
        return 7;
    }

    // inicio da medicao do tempo de processamento
    GET_TIME(tempo_inicio);

    // calculo sequencial do produto interno
    for (long int i = 0; i < n; i++)
        produto_sequencial += vetor1[i] * vetor2[i];

    // fim da medicao do tempo de processamento
    GET_TIME(tempo_fim);
    tempo_processamento = tempo_fim - tempo_inicio;

    // le o produto interno registrado no arquivo
    ret = fread(&produto_original, sizeof(double), 1, arquivo);

    // calcula a variacao relativa
    double variacao_relativa = (produto_original != 0) ?
        ((produto_sequencial - produto_original) / produto_original) : 0;

    // impressao dos resultados
    printf("%ld\t1\t%.6f\t%.6lf\t%.6lf\t%.6lf\n",
        n,                      // Dimensao dos vetores
        tempo_processamento,    // Tempo de processamento
        produto_sequencial,     // Produto interno calculado
        produto_original,       // Produto interno original
        variacao_relativa);     // Variacao relativa

    // desaloca os espacos de memoria
    free(vetor1);
    free(vetor2);
    fclose(arquivo);

    return 0;
}