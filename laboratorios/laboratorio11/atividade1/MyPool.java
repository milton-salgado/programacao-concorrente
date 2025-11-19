/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Laboratório: 11 */
/* Codigo: Criando um pool de threads em Java */

import java.util.LinkedList;

//-------------------------------------------------------------------------------
/**
 * Classe que implementa um pool de threads personalizado.
 *
 * Gerencia um conjunto fixo de threads trabalhadoras que executam tarefas
 * de uma fila compartilhada. As tarefas são objetos Runnable adicionados
 * através do método execute().
 *
 * Funcionamento:
 * - No construtor, cria e inicia N threads que ficam aguardando tarefas
 * - As threads ficam em espera (wait) enquanto a fila está vazia
 * - Quando uma tarefa é adicionada, uma thread é notificada (notify)
 * - A thread remove a tarefa da fila e a executa
 * - O método shutdown() sinaliza o encerramento e aguarda todas as threads
 *
 * Sincronização:
 * - Usa a própria fila como monitor para sincronização
 * - wait/notify para coordenar threads trabalhadoras
 * - Flag shutdown para sinalizar encerramento gracioso
 */
class FilaTarefas {
  /** Número de threads no pool */
  private final int nThreads;
  /** Array com as threads trabalhadoras */
  private final MyPoolThreads[] threads;
  /** Fila de tarefas pendentes (FIFO) */
  private final LinkedList<Runnable> queue;
  /** Flag que indica se o pool está em processo de encerramento */
  private boolean shutdown;

  /**
   * Construtor que inicializa o pool de threads.
   * Cria e inicia imediatamente todas as threads trabalhadoras.
   *
   * @param nThreads número de threads no pool
   */
  public FilaTarefas(int nThreads) {
    this.shutdown = false;
    this.nThreads = nThreads;
    queue = new LinkedList<Runnable>();
    threads = new MyPoolThreads[nThreads];
    for (int i = 0; i < nThreads; i++) {
      threads[i] = new MyPoolThreads();
      threads[i].start();
    }
  }

  /**
   * Adiciona uma tarefa na fila para execução.
   * Notifica uma thread em espera para processar a tarefa.
   * Ignora a tarefa se o pool já estiver em shutdown.
   *
   * @param r objeto Runnable representando a tarefa a ser executada
   */
  public void execute(Runnable r) {
    synchronized (queue) {
      if (this.shutdown)
        return;
      queue.addLast(r);
      queue.notify();
    }
  }

  /**
   * Encerra o pool de threads de forma graciosa.
   * Sinaliza shutdown, acorda todas as threads e aguarda seu término.
   * As tarefas já na fila são processadas antes do encerramento.
   */
  public void shutdown() {
    synchronized (queue) {
      this.shutdown = true;
      queue.notifyAll();
    }
    for (int i = 0; i < nThreads; i++) {
      try {
        threads[i].join();
      } catch (InterruptedException e) {
        return;
      }
    }
  }

  /**
   * Classe interna que representa uma thread trabalhadora do pool.
   * Executa em loop, retirando e processando tarefas da fila compartilhada.
   */
  private class MyPoolThreads extends Thread {
    /**
     * Método principal da thread trabalhadora.
     * Loop: espera por tarefas -> retira da fila -> executa.
     * Termina quando shutdown é sinalizado e a fila está vazia.
     */
    public void run() {
      Runnable r;
      while (true) {
        synchronized (queue) {
          // Espera enquanto fila vazia e não está em shutdown
          while (queue.isEmpty() && (!shutdown)) {
            try {
              queue.wait();
            } catch (InterruptedException ignored) {
            }
          }
          // Se fila vazia após shutdown, encerra a thread
          if (queue.isEmpty())
            return;
          // Remove primeira tarefa da fila (FIFO)
          r = (Runnable) queue.removeFirst();
        }
        // Executa a tarefa fora do bloco sincronizado
        try {
          r.run();
        } catch (RuntimeException e) {
        }
      }
    }
  }
}
// -------------------------------------------------------------------------------

// --PASSO 1: cria uma classe que implementa a interface Runnable
class Hello implements Runnable {
  String msg;

  public Hello(String m) {
    msg = m;
  }

  // --metodo executado pela thread
  public void run() {
    System.out.println(msg);
  }
}

class Primo implements Runnable {
  private long numero;

  public Primo(long n) {
    this.numero = n;
  }

  private boolean ehPrimo(long n) {
    if (n <= 1)
      return false;
    if (n == 2)
      return true;
    if (n % 2 == 0)
      return false;
    for (long i = 3; i <= Math.sqrt(n); i += 2) {
      if (n % i == 0)
        return false;
    }
    return true;
  }

  public void run() {
    if (ehPrimo(numero)) {
      System.out.println(numero + " eh primo");
    } else {
      System.out.println(numero + " nao eh primo");
    }
  }
}

// Classe da aplicação (método main)
class MyPool {
  private static final int NTHREADS = 10;

  public static void main(String[] args) {
    // --PASSO 2: cria o pool de threads
    FilaTarefas pool = new FilaTarefas(NTHREADS);

    // --PASSO 3: dispara a execução dos objetos runnable usando o pool de threads
    for (int i = 0; i < 25; i++) {
      //final String m = "Hello da tarefa " + i;
      //Runnable hello = new Hello(m);
      //pool.execute(hello);
      Runnable primo = new Primo(i);
      pool.execute(primo);
    }

    // --PASSO 4: esperar pelo termino das threads
    pool.shutdown();
    System.out.println("Terminou");
  }
}
