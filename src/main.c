/*****************************************************************/
/* Arquivo: main.c                                               */
/* Programa de teste do analisador lexico da linguagem Domus.    */
/* Uso: analisador_domus arquivo_fonte                           */
/*****************************************************************/

#include "globals.h"
#include "util.h"
#include "scan.h"

int lineno = 0;   /* linha corrente do programa fonte */
FILE *source;     /* arquivo com o programa fonte */
FILE *listing;    /* destino do texto de saida */

int EchoSource = FALSE;
int TraceScan = TRUE;

int main(int argc, char *argv[])
{
    TokenType token;

    if (argc != 2) {
        fprintf(stderr, "Uso: %s arquivo_fonte\n", argv[0]);
        return 1;
    }

    listing = stdout;

    source = fopen(argv[1], "r");
    if (source == NULL) {
        fprintf(stderr, "Erro ao abrir arquivo: \"%s\"\n", argv[1]);
        return 1;
    }

    /* getToken e chamada repetidamente ate alcancar o fim do arquivo. */
    do {
        token = getToken();
    } while (token != FIM_ARQUIVO);

    fclose(source);
    return 0;
}
