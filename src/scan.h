/*****************************************************************/
/* Arquivo: scan.h                                               */
/* Interface do analisador lexico da linguagem Domus.            */
/*****************************************************************/

#ifndef _SCAN_H_
#define _SCAN_H_

/* MAXTOKENLEN = comprimento maximo do lexema de um token */
#define MAXTOKENLEN 40

/* tokenString armazena o lexema do token reconhecido */
extern char tokenString[MAXTOKENLEN + 1];

/* Retorna o proximo token do programa fonte. Deve ser chamada
   repetidamente ate devolver FIM_ARQUIVO. */
TokenType getToken(void);

#endif
