/*****************************************************************/
/* Arquivo: globals.h                                            */
/* Tipos globais e variaveis do analisador lexico da linguagem   */
/* Domus. Deve ser incluido antes dos demais arquivos.           */
/* Estrutura baseada no analisador da linguagem TINY             */
/* (K. C. Louden, Compiler Construction), distribuido na         */
/* disciplina.                                                   */
/*****************************************************************/

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/* MAXRESERVED = quantidade de palavras reservadas da linguagem */
#define MAXRESERVED 13

/* Os 33 tipos de token da linguagem Domus. */
typedef enum {
    /* tokens de controle (2) */
    FIM_ARQUIVO, ERRO,
    /* palavras reservadas (13) */
    SENSOR, ATUADOR, PORTA, ANALOGICO, DIGITAL,
    SE, ENTAO, SENAO, FIM_SE,
    ENQUANTO, FIM_ENQUANTO,
    LER, ESCREVER,
    /* tokens multicaractere (2) */
    ID, NUM,
    /* simbolos especiais (16) */
    SOMA, SUB, MULT, DIV,
    MENOR, MENOR_IGUAL, MAIOR, MAIOR_IGUAL, IGUAL, DIFERENTE,
    ATRIBUICAO,
    PONTO, DOIS_PONTOS, VIRGULA, ABRE_PAR, FECHA_PAR
} TokenType;

extern FILE *source;  /* arquivo com o programa fonte */
extern FILE *listing; /* destino do texto de saida */

extern int lineno; /* numero da linha corrente do programa fonte */

/*****************************************************/
/***********   Flags para rastreamento    ************/
/*****************************************************/

/* EchoSource = TRUE faz o programa fonte ser copiado para a saida
   com o numero de linha durante a analise */
extern int EchoSource;

/* TraceScan = TRUE faz a informacao do token ser impressa a cada
   token reconhecido pela varredura */
extern int TraceScan;

#endif
