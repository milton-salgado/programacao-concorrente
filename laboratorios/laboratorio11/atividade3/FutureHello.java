/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Laboratório: 11 */
/* Codigo: Exemplo de uso de futures */
/* -------------------------------------------------------------------*/

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import java.util.ArrayList;
import java.util.List;


//classe callable original
class MyCallable implements Callable<Long> {
  //construtor
  MyCallable() {}

  //método para execução
  public Long call() throws Exception {
    long s = 0;
    for (long i=1; i<=100; i++) {
      s++;
    }
    return s;
  }
}

//classe callable para verificar primos em um intervalo
class PrimoCallable implements Callable<Long> {
  private final long inicio;
  private final long fim;

  //construtor - recebe o intervalo [inicio, fim]
  PrimoCallable(long inicio, long fim) {
    this.inicio = inicio;
    this.fim = fim;
  }

  //verifica se um número é primo
  private boolean ehPrimo(long n) {
    if (n <= 1) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    for (long i = 3; i <= Math.sqrt(n); i += 2) {
      if (n % i == 0) return false;
    }
    return true;
  }

  //método para execução - retorna a quantidade de primos no intervalo
  public Long call() throws Exception {
    long count = 0;
    for (long i = inicio; i <= fim; i++) {
      if (ehPrimo(i)) {
        count++;
      }
    }
    return count;
  }
}

//classe do método main
public class FutureHello  {
  private static final long N = 1000000; // Valor grande para contar primos de 1 a N
  private static final int NTHREADS = 10;

  public static void main(String[] args) {
    //cria um pool de threads (NTHREADS)
    ExecutorService executor = Executors.newFixedThreadPool(NTHREADS);
    //cria uma lista para armazenar referencias de chamadas assincronas
    List<Future<Long>> list = new ArrayList<Future<Long>>();

    // Divide o intervalo [1, N] entre as threads
    long tamanhoBloco = N / NTHREADS;

    for (int i = 0; i < NTHREADS; i++) {
      long inicio = i * tamanhoBloco + 1;
      long fim = (i == NTHREADS - 1) ? N : (i + 1) * tamanhoBloco;

      Callable<Long> worker = new PrimoCallable(inicio, fim);
      /*submit() permite enviar tarefas Callable ou Runnable e obter um objeto Future
        para acompanhar o progresso e recuperar o resultado da tarefa
       */
      Future<Long> submit = executor.submit(worker);
      list.add(submit);
    }

    System.out.println("Numero de tarefas: " + list.size());
    System.out.println("Contando primos de 1 a " + N + "...");
    //pode fazer outras tarefas enquanto as threads trabalham...

    //recupera os resultados e faz o somatório final
    long totalPrimos = 0;
    for (Future<Long> future : list) {
      try {
        totalPrimos += future.get(); //bloqueia se a computação nao tiver terminado
      } catch (InterruptedException e) {
        e.printStackTrace();
      } catch (ExecutionException e) {
        e.printStackTrace();
      }
    }
    System.out.println("Quantidade de numeros primos no intervalo [1, " + N + "]: " + totalPrimos);
    executor.shutdown();
  }
}
