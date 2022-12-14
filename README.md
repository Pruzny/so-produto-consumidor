# Produtor Consumidor
Repositório para a atividade da disciplina Sistemas Operacionais em 22.2 

# O Projeto: Objetivo

O objetivo do projeto era utilizar solução baseada em semáforo para o problema produtor-consumidor, às especificações do projeto estarão após os exemplos e como rodá-lo

# O Projeto: Feito em C e o gerador_matriz feito em Python

Utilize o [Python](https://www.python.org/downloads/) 3.9+

# O Projeto: Como instalar e rodar

## Instalação

git clone https://github.com/pruzny/so-produtor-consumidor
cd so-produtor-consumidor

## Utilizando

Primeiro, temos que gerar as arrays, já que, nosso produtor consumidor é baseado em multiplicações e somas de arrays e vectores. Para gerá-las

### Flags

 Flags

| Flag   | Função                     |
|--------|----------------------------|
| -min   | Indica o valor mínimo do valor que será gerado aleatoriamente EX: [-min -1 | será gerado um valor até -1, incluindo o mesmo]| 
| -max   | Indica o valor máximo do valor que será gerado aleatoriamente EX: [-max 1 | será gerado um valor até 1, incluindo o mesmo] |

## Rodando o gerador de matriz
```
py gerador_matriz.py -min -1 -max 1

```
## Rodando o produtor consumidor
```

gcc -o produtor-consumidor produtor-consumidor.c
./produtor-consumidor

```

# Saída

A saída estará no arquivo saida.out e será produzida pelo Consumidor.
o modelo da saída para cada matriz:

```
================================
Entrada: Nome;
——————————–
A (formatado com linhas e colunas, usando espa ̧cos para separar elementos em uma linha)
——————————–
B (formatado com linhas e colunas, usando espa ̧cos para separar elementos em uma linha)
——————————–
C (formatado com linhas e colunas, usando espa ̧cos para separar elementos em uma linha)
——————————–
V (cada elemento em uma linha)
——————————–
E
================================

```

# Especificações do Projeto

```
P - Thread produtora - Lê um arquivo (entrada.in) contendo uma lista de 50 arquivos de entrada (um nome de arquivo por linha), cada um contendo duas matrizes quadradas e ordem 10 de doubles. A cada arquivo lida, a thread produtora cria dinamicamente uma estrutura S, preenche o nome do arquivo de entrada, além de A e B e coloca o ponteiro para a estrutura S em shared[0].buffer[in] para ser processada pela etapa seguinte. Existem 1 instância desta thread.

CP1 - Thread Consumidora & Produtora 1 - Move shared[0]→buffer[out] para um ponteiro temporário, calcula C=A*B no elemento temporário e move o ponteiro temporário para shared[1]→buffer[in]. Existem 5 instâncias desta thread.

CP2 - Thread Consumidora & Produtora 2 - Move shared[1]→buffer[out] para um ponteiro temporário, calcula V como a soma das colunas de C. e move o ponteiro temporário para shared[2]→buffer[in]. Existem 4 instâncias dessa thread.

CP3 - Thread Consumidora & Produtora 3 - Move shared[2]→buffer[out],  para um ponteiro temporário, calcula E como a soma do elementos do vetor V. e move o ponteiro temporário para shared[3]→buffer[in]. Existem 3 instâncias dessa thread.

C - Thread consumidora - Escreve um arquivo (saida.out) contendo Nome, A, B, C, V e E para cada um dos arquivos de entrada, obedecendo o seguinte formato. Existem 1 instância desta thread:
```
