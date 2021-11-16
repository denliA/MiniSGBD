#ifndef COMMAND_TOKENIZER_H
#define COMMAND_TOKENIZER_H

#define MAX_ATTR 100

#include <stdint.h>

enum {
    INVALID_TOK=-2, ENDOFCOMMAND = -1, DEUX_POINTS, VIRGULE, WHERE, FROM, AND, OR, INTO, RECORD, OPEQ, OPSUP, OPINF, OPSUPEQ, OPINFEQ, OPNEQ, TYPE_STRING, TYPE_INT, TYPE_FLOAT, STRING_CONSTANT,
    FLOAT_CONSTANT, INT_CONSTANT, PAREN_OUVR, PAREN_FERM, ETOILE, NOM_VARIABLE
};

struct command {
    unsigned pos;
    char *command;
};

struct token {
    int type;
    union {
        char sattr[MAX_ATTR+1];
        float fattr;
        uint32_t iattr;
    } attr;
};

/*
Utilisation: 
struct command c = newCommand(command);
struct token tok;
while( nextToken(command, &tok) != ENDOFCOMMAND) {
    //faire des choses sur le token actuel : (choses précisées dans nextToken)
};
*/
struct command newCommand(char *command);


/** (ici les lettres en majuscule concernent l'énumération)
Types possiblement retournés : (qui seront à la fois dans la valeur de retour de nextToken(com, tok) et dans le champ tok->type)
    - DEUX_POINTS ou VIRGULE : indique qu'il vient de lire ":" ou "," dans la chaîne.
        --> ici le champ tok->attr doit être ignoré
    - WHERE ou FROM ou INTO ou AND ou OR: indique que le mot clé correspondant a été lu 
        --> ici le champ tok->attr doit être ignoré
    - OPEQ ou OPSUP ou OPINF ou OPSUPEQ ou OPINFEQ

*/
int nextToken(struct command com, struct token *tok);

#endif
