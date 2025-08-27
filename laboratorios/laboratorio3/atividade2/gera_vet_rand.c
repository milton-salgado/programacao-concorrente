#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX 1000 // valor maximo de um elemento do vetor
// descomentar o define abaixo caso deseje imprimir uma versao do vetor gerado no formato texto
// #define LOG 

int main(int argc, char *argv[]) {
   float *vetor; // vetor que sera gerado
   long int n; // quantidade de elementos do vetor
   float elem; // valor gerado para incluir no vetor
   double soma = 0; // soma total dos elementos gerados
   int fator = 1; // fator multiplicador para gerar numeros negativos
   FILE *arquivo; // descritor do arquivo de saida
   size_t ret; // retorno da funcao de escrita no arquivo de saida

   // recebe os argumentos de entrada
   if (argc < 3) {
      printf("--ERRO: Informe os parametros, no formato: <%s> <dimensao> <nome_arquivo_saida>\n", argv[0]);
      return 1;
   }
   n = atoi(argv[1]);

   if (!(vetor = (float *) calloc(n, sizeof(float)))) {
      printf("--ERRO: Erro ao alocar vetor de numeros com calloc()\n");
      return 2;
   }

   // preenche o vetor com valores float aleatorios
   srand(time(NULL));
   for (long int i = 0; i < n; i++) {
      elem = (rand() % MAX) / 3.0 * fator;
      vetor[i] = elem;
      soma += elem; // acumula o elemento na soma total
      fator *= -1;
   }

   // imprimir na saida padrao o vetor gerado
#ifdef LOG
   printf("%ld\n", n);
   for (long int i = 0; i < n; i++)
      printf("%f ", vetor[i]);
   printf("\n");
   printf("%lf\n", soma);
#endif

   // escreve o vetor no arquivo
   // abre o arquivo para escrita binaria
   if (!(arquivo = fopen(argv[2], "wb"))) {
      printf("--ERRO: Erro na abertura do arquivo binario para escrita com fopen()\n");
      return 3;
   }

   // escreve a dimensao
   ret = fwrite(&n, sizeof(long int), 1, arquivo);
   // escreve os elementos do vetor
   ret = fwrite(vetor, sizeof(float), n, arquivo);
   if (ret < n) {
      printf("--ERRO: Erro de escrita no arquivo com fwrite()\n");
      return 4;
   }

   // escreve o somatorio
   ret = fwrite(&soma, sizeof(double), 1, arquivo);

   // finaliza/limpa o uso das variaveis
   fclose(arquivo);
   free(vetor);

   printf("Arquivo gerado com sucesso\n");

   return 0;
}