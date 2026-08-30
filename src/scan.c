/*****************************************************************/
/* Arquivo: scan.c                                               */
/* Implementacao do analisador lexico da linguagem Domus.        */
/* Implementa diretamente o automato finito deterministico de    */
/* 11 estados descrito em docs/especificacao-lexica.md.          */
/*****************************************************************/

#include "globals.h"
#include "util.h"
#include "scan.h"

/* Estados do automato. Correspondem um a um aos estados da
   tabela de transicao da especificacao. */
typedef enum {
    INICIO,
    EM_NUM,
    EM_ID,
    EM_MENOR,
    EM_MAIOR,
    EM_IGUAL,
    EM_EXCLAM,
    EM_BARRA,
    EM_COMENT,
    EM_FECHA,
    FIM
} StateType;

/* Lexema do token corrente */
char tokenString[MAXTOKENLEN + 1];

/* BUFLEN = tamanho do buffer que guarda a linha corrente do fonte */
#define BUFLEN 1024

static char lineBuf[BUFLEN]; /* linha corrente do programa fonte */
static int linepos = 0;      /* posicao de leitura em lineBuf */
static int bufsize = 0;      /* quantidade de caracteres em lineBuf */
static int EOF_flag = FALSE; /* marca que o fim do arquivo foi alcancado */

/* Numero da ultima linha efetivamente lida do programa fonte.
   lineno nao serve para reportar o fim do arquivo: ele e incrementado
   antes da tentativa de leitura, entao no momento em que a leitura
   falha ele ja aponta para uma linha que nao existe. */
static int ultimaLinhaLida = 0;

/* getNextChar devolve o proximo caractere do programa fonte, lendo
   uma nova linha quando lineBuf se esgota.
   O valor e convertido para unsigned char antes de virar int: sem
   isso um byte 0xFF de um fonte acentuado seria devolvido como -1 e
   confundido com EOF. */
static int getNextChar(void)
{
    if (!(linepos < bufsize)) {
        lineno++;
        if (fgets(lineBuf, BUFLEN - 1, source)) {
            if (EchoSource)
                fprintf(listing, "%5d: %s", lineno, lineBuf);
            bufsize = strlen(lineBuf);
            linepos = 0;
            ultimaLinhaLida = lineno;
            return (unsigned char) lineBuf[linepos++];
        } else {
            EOF_flag = TRUE;
            return EOF;
        }
    } else
        return (unsigned char) lineBuf[linepos++];
}

/* devolveCaractere retrocede uma posicao em lineBuf, devolvendo a
   entrada o caractere de verificacao a frente.
   A guarda EOF_flag e obrigatoria: no fim do arquivo nao ha caractere
   a devolver, e sem ela a posicao de leitura sairia do buffer. */
static void devolveCaractere(void)
{
    if (!EOF_flag)
        linepos--;
}

/* ehLetra implementa a classe letra = [a-zA-Z_] da especificacao.
   O sublinhado precisa entrar aqui: isalpha('_') e falso, e sem esta
   funcao as palavras reservadas fim_se e fim_enquanto seriam
   quebradas em tres tokens.
   O argumento e convertido para unsigned char porque isalpha tem
   comportamento indefinido para valores negativos. */
static int ehLetra(int c)
{
    return isalpha((unsigned char) c) || (c == '_');
}

/* ehDigito implementa a classe digito = [0-9] da especificacao. */
static int ehDigito(int c)
{
    return isdigit((unsigned char) c);
}

/* Tabela das palavras reservadas da linguagem. As palavras reservadas
   nao possuem estados proprios no automato: sao reconhecidas como
   identificador e classificadas por consulta a esta tabela. */
static struct {
    char *str;
    TokenType tok;
} reservedWords[MAXRESERVED] = {
    {"sensor", SENSOR},
    {"atuador", ATUADOR},
    {"porta", PORTA},
    {"analogico", ANALOGICO},
    {"digital", DIGITAL},
    {"se", SE},
    {"entao", ENTAO},
    {"senao", SENAO},
    {"fim_se", FIM_SE},
    {"enquanto", ENQUANTO},
    {"fim_enquanto", FIM_ENQUANTO},
    {"ler", LER},
    {"escrever", ESCREVER}
};

/* reservedLookup verifica se um lexema de identificador e na verdade
   uma palavra reservada. A comparacao e sensivel a maiusculas e
   minusculas. */
static TokenType reservedLookup(char *s)
{
    int i;
    for (i = 0; i < MAXRESERVED; i++)
        if (!strcmp(s, reservedWords[i].str))
            return reservedWords[i].tok;
    return ID;
}

/***********************************/
/*  Funcao principal da varredura  */
/***********************************/

/* getToken retorna o proximo token do programa fonte. */
TokenType getToken(void)
{
    /* indice de escrita em tokenString */
    int tokenStringIndex = 0;
    /* token reconhecido nesta chamada */
    TokenType currentToken = ERRO;
    /* estado corrente do automato */
    StateType estado = INICIO;
    /* indica se o caractere lido faz parte do lexema */
    int save;

    while (estado != FIM) {
        int c = getNextChar();
        save = TRUE;
        switch (estado) {

        case INICIO:
            if (ehDigito(c))
                estado = EM_NUM;
            else if (ehLetra(c))
                estado = EM_ID;
            else if ((c == ' ') || (c == '\t') || (c == '\r') || (c == '\n'))
                save = FALSE;
            else if (c == '<')
                estado = EM_MENOR;
            else if (c == '>')
                estado = EM_MAIOR;
            else if (c == '=')
                estado = EM_IGUAL;
            else if (c == '!')
                estado = EM_EXCLAM;
            else if (c == '/')
                estado = EM_BARRA;
            else {
                estado = FIM;
                switch (c) {
                case EOF:
                    save = FALSE;
                    currentToken = FIM_ARQUIVO;
                    break;
                case '+':
                    currentToken = SOMA;
                    break;
                case '-':
                    currentToken = SUB;
                    break;
                case '*':
                    currentToken = MULT;
                    break;
                case '.':
                    currentToken = PONTO;
                    break;
                case ':':
                    currentToken = DOIS_PONTOS;
                    break;
                case ',':
                    currentToken = VIRGULA;
                    break;
                case '(':
                    currentToken = ABRE_PAR;
                    break;
                case ')':
                    currentToken = FECHA_PAR;
                    break;
                default:
                    currentToken = ERRO;
                    break;
                }
            }
            break;

        case EM_NUM:
            if (!ehDigito(c)) {
                devolveCaractere();
                save = FALSE;
                estado = FIM;
                currentToken = NUM;
            }
            break;

        case EM_ID:
            /* Em Domus o identificador aceita digitos depois da
               primeira letra, entao a condicao de saida testa as duas
               classes. Testar apenas letra quebraria a1b2 em quatro
               tokens. */
            if (!ehLetra(c) && !ehDigito(c)) {
                devolveCaractere();
                save = FALSE;
                estado = FIM;
                currentToken = ID;
            }
            break;

        case EM_MENOR:
            estado = FIM;
            if (c == '=')
                currentToken = MENOR_IGUAL;
            else if (c == '-')
                currentToken = ATRIBUICAO;
            else {
                devolveCaractere();
                save = FALSE;
                currentToken = MENOR;
            }
            break;

        case EM_MAIOR:
            estado = FIM;
            if (c == '=')
                currentToken = MAIOR_IGUAL;
            else {
                devolveCaractere();
                save = FALSE;
                currentToken = MAIOR;
            }
            break;

        case EM_IGUAL:
            estado = FIM;
            if (c == '=')
                currentToken = IGUAL;
            else {
                devolveCaractere();
                save = FALSE;
                currentToken = ERRO;
            }
            break;

        case EM_EXCLAM:
            estado = FIM;
            if (c == '=')
                currentToken = DIFERENTE;
            else {
                devolveCaractere();
                save = FALSE;
                currentToken = ERRO;
            }
            break;

        case EM_BARRA:
            if (c == '*') {
                /* A barra ja tinha sido acumulada no lexema, quando
                   ainda nao se sabia se era divisao ou abertura de
                   comentario. Agora precisa ser descartada, senao
                   vazaria para o inicio do proximo token. */
                save = FALSE;
                tokenStringIndex = 0;
                estado = EM_COMENT;
            } else {
                devolveCaractere();
                save = FALSE;
                estado = FIM;
                currentToken = DIV;
            }
            break;

        case EM_COMENT:
            save = FALSE;
            if (c == EOF) {
                estado = FIM;
                currentToken = FIM_ARQUIVO;
            } else if (c == '*')
                estado = EM_FECHA;
            break;

        case EM_FECHA:
            save = FALSE;
            if (c == EOF) {
                estado = FIM;
                currentToken = FIM_ARQUIVO;
            } else if (c == '/')
                estado = INICIO;
            else if (c != '*')
                /* Um asterisco seguido de outro asterisco mantem o
                   automato aqui: numa sequencia de varios asteriscos
                   antes da barra final, o comentario ainda pode
                   fechar no proximo caractere. */
                estado = EM_COMENT;
            break;

        case FIM:
        default: /* nao deve acontecer */
            fprintf(listing, "Erro interno do analisador: estado %d\n", estado);
            estado = FIM;
            currentToken = ERRO;
            break;
        }

        if (save && (tokenStringIndex < MAXTOKENLEN))
            tokenString[tokenStringIndex++] = (char) c;

        if (estado == FIM) {
            tokenString[tokenStringIndex] = '\0';
            if (currentToken == ID)
                currentToken = reservedLookup(tokenString);
        }
    }

    if (TraceScan) {
        /* O fim do arquivo e reportado na ultima linha que existe de
           fato no fonte, e nao na posicao do contador de leitura. */
        fprintf(listing, "%5d: ",
                (currentToken == FIM_ARQUIVO) ? ultimaLinhaLida : lineno);
        imprimeToken(currentToken, tokenString);
    }
    return currentToken;
} /* fim de getToken */
