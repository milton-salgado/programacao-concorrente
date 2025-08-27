# Makefile para os codigos criados
# Uso: make [target]

CC = gcc
CFLAGS = -Wall
LDFLAGS = -lpthread

# Executaveis
EXECUTAVEIS = gera_vetores_produto produto_interno_sequencial produto_interno_concorrente

# Arquivos fonte
GERADOR_SRC = gera_vetores_produto.c
SEQUENCIAL_SRC = produto_interno_sequencial.c
CONCORRENTE_SRC = produto_interno_concorrente.c

# Configuracoes de teste
DIMENSOES = 100000 500000 1000000
THREADS = 1 2 4 8

.PHONY: all clean test benchmark help setup

# Compila todos os executaveis
all: $(EXECUTAVEIS)

# Regras de compilacao
gera_vetores_produto: $(GERADOR_SRC)
	$(CC) $(CFLAGS) -o $@ $<

produto_interno_sequencial: $(SEQUENCIAL_SRC)
	$(CC) $(CFLAGS) -o $@ $<

produto_interno_concorrente: $(CONCORRENTE_SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Cria estrutura de diretorios
setup:
	@echo "Criando diretorios..."
	@mkdir -p dados_produto resultados
	@echo "Estrutura criada!"

# Teste rapido com 1, 2 e 4 threads
test: all setup
	@echo "=== TESTE RAPIDO - PRODUTO INTERNO ==="
	@echo "Gerando vetores de teste (100.000 elementos)..."
	@./gera_vetores_produto 100000 dados_produto/teste.bin
	@echo
	@echo "Dimensao    Threads    Tempo(s)    Produto_Calc    Produto_Orig    Var_Relativa"
	@echo "=========================================================================="
	@echo "Testando versao sequencial..."
	@./produto_interno_sequencial dados_produto/teste.bin
	@echo "Testando com 2 threads..."
	@./produto_interno_concorrente dados_produto/teste.bin 2
	@echo "Testando com 4 threads..."
	@./produto_interno_concorrente dados_produto/teste.bin 4
	@echo "Teste concluido!"

# Gera vetores de teste de varios tamanhos
vetores-teste: gera_vetores_produto setup
	@echo "Gerando vetores de teste..."
	@./gera_vetores_produto 50000 dados_produto/pequeno.bin
	@./gera_vetores_produto 200000 dados_produto/medio.bin
	@./gera_vetores_produto 1000000 dados_produto/grande.bin
	@./gera_vetores_produto 5000000 dados_produto/muito_grande.bin
	@echo "Vetores gerados em dados_produto/"

# Teste especifico com parametros definidos pelo proprio usuario
teste-especifico: all
	@echo "Uso: make teste-especifico DIMENSAO=<tam> THREADS=<num>"
	@echo "Exemplo: make teste-especifico DIMENSAO=500000 THREADS=4"
ifndef DIMENSAO
	@echo "ERRO: Especifique DIMENSAO=<tamanho>"
	@exit 1
endif
ifndef THREADS
	@echo "ERRO: Especifique THREADS=<numero>"
	@exit 1
endif
	@echo "Gerando vetor com DIMENSAO=$(DIMENSAO)..."
	@./gera_vetores_produto $(DIMENSAO) dados_produto/custom.bin
	@echo "Executando sequencial..."
	@./produto_interno_sequencial dados_produto/custom.bin
	@echo "Executando concorrente com $(THREADS) threads..."
	@./produto_interno_concorrente dados_produto/custom.bin $(THREADS)

# Comparacao sequencial vs concorrente
comparacao: all setup
	@echo "=== COMPARACAO SEQUENCIAL VS CONCORRENTE ==="
	@echo "Dimensao utilizada: 1000000 elementos"
	@./gera_vetores_produto 1000000 dados_produto/comparacao.bin > /dev/null
	@echo
	@echo "Executando 5 vezes cada configuracao..."
	@echo "Configuracao       Exec1     Exec2     Exec3     Exec4     Exec5     Media"
	@echo "========================================================================"
	@printf "%-15s " "Sequencial"
	@soma=0; \
	for i in 1 2 3 4 5; do \
		tempo=$$(./produto_interno_sequencial dados_produto/comparacao.bin | cut -f3); \
		printf "%8s  " $$tempo; \
		soma=$$(echo "$$soma + $$tempo" | bc -l 2>/dev/null || echo "$$soma $$tempo" | awk '{print $$1+$$2}'); \
	done; \
	media=$$(echo "scale=6; $$soma / 5" | bc -l 2>/dev/null || echo "$$soma 5" | awk '{print $$1/$$2}'); \
	printf "%8s\n" $$media
	@for threads in 2 4 8; do \
		printf "%-15s " "$$threads threads"; \
		soma=0; \
		for i in 1 2 3 4 5; do \
			tempo=$$(./produto_interno_concorrente dados_produto/comparacao.bin $$threads | cut -f3); \
			printf "%8s  " $$tempo; \
			soma=$$(echo "$$soma + $$tempo" | bc -l 2>/dev/null || echo "$$soma $$tempo" | awk '{print $$1+$$2}'); \
		done; \
		media=$$(echo "scale=6; $$soma / 5" | bc -l 2>/dev/null || echo "$$soma 5" | awk '{print $$1/$$2}'); \
		printf "%8s\n" $$media; \
	done

# Analise de variacao relativa
variacao: all setup
	@echo "=== ANALISE DE VARIACAO RELATIVA ==="
	@echo "Testando diferentes dimensoes..."
	@echo "Dimensao    Sequencial    2 Threads     4 Threads     8 Threads"
	@echo "=============================================================="
	@for dim in 100000 500000 1000000; do \
		printf "%8d    " $$dim; \
		./gera_vetores_produto $$dim dados_produto/temp_$$dim.bin > /dev/null; \
		var_seq=$$(./produto_interno_sequencial dados_produto/temp_$$dim.bin | cut -f6); \
		printf "%10s    " $$var_seq; \
		for threads in 2 4 8; do \
			var_conc=$$(./produto_interno_concorrente dados_produto/temp_$$dim.bin $$threads | cut -f6); \
			printf "%10s    " $$var_conc; \
		done; \
		echo; \
		rm -f dados_produto/temp_$$dim.bin; \
	done

# Limpeza
clean:
	@echo "Limpando arquivos..."
	@rm -f $(EXECUTAVEIS)
	@rm -f *.txt
	@rm -rf dados_produto resultados
	@echo "Limpeza concluida!"

# Limpeza suave (mantem os dados gerados)
clean-build:
	@echo "Removendo apenas executaveis..."
	@rm -f $(EXECUTAVEIS)

# Ajuda
help:
	@echo "Makefile para Produto Interno - Targets disponiveis:"
	@echo "  all              - Compila todos os programas"
	@echo "  setup            - Cria estrutura de diretorios"
	@echo "  test             - Executa teste rapido"
	@echo "  benchmark        - Executa benchmark completo"
	@echo "  vetores-teste    - Gera vetores de varios tamanhos"
	@echo "  teste-especifico - Teste com parametros especificos"
	@echo "                     Uso: make teste-especifico DIMENSAO=X THREADS=Y"
	@echo "  comparar         - Compara sequencial vs concorrente"
	@echo "  comparar-v2      - Versao alternativa mais robusta"
	@echo "  variacao         - Analise de variacao relativa"
	@echo "  variacao-v2      - Versao alternativa mais robusta"
	@echo "  clean            - Remove tudo"
	@echo "  clean-build      - Remove apenas executaveis"
	@echo "  help             - Mostra esta ajuda"