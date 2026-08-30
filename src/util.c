/*****************************************************************/
/* Arquivo: util.c                                               */
/* Utilitarios do analisador lexico da linguagem Domus.          */
/*****************************************************************/

#include "globals.h"
#include "util.h"

/* imprimeToken escreve a categoria do token e, quando existe, o seu
   lexema. Cada token ocupa uma linha da saida. */
void imprimeToken(TokenType token, const char *lexema)
{
    switch (token) {

    /* palavras reservadas (13) */
    case SENSOR:
    case ATUADOR:
    case PORTA:
    case ANALOGICO:
    case DIGITAL:
    case SE:
    case ENTAO:
    case SENAO:
    case FIM_SE:
    case ENQUANTO:
    case FIM_ENQUANTO:
    case LER:
    case ESCREVER:
        fprintf(listing, "palavra reservada: %s\n", lexema);
        break;

    /* tokens multicaractere (2) */
    case ID:
        fprintf(listing, "identificador: %s\n", lexema);
        break;
    case NUM:
        /* O valor lido e impresso junto do token: o enunciado exige
           exibir o valor/atributo do token quando houver. */
        fprintf(listing, "numero: %s\n", lexema);
        break;

    /* simbolos especiais (16) */
    case SOMA:
    case SUB:
    case MULT:
    case DIV:
    case MENOR:
    case MENOR_IGUAL:
    case MAIOR:
    case MAIOR_IGUAL:
    case IGUAL:
    case DIFERENTE:
    case ATRIBUICAO:
    case PONTO:
    case DOIS_PONTOS:
    case VIRGULA:
    case ABRE_PAR:
    case FECHA_PAR:
        fprintf(listing, "simbolo: %s\n", lexema);
        break;

    /* tokens de controle (2) */
    case ERRO:
        fprintf(listing, "ERRO: lexema encontrado: %s\n", lexema);
        break;
    case FIM_ARQUIVO:
        fprintf(listing, "fim de arquivo\n");
        break;

    default: /* nao deve acontecer */
        fprintf(listing, "Token desconhecido: %d\n", token);
        break;
    }
}
