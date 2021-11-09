#ifndef COMMAND_TOKENIZER_H
#define COMMAND_TOKENIZER_H

#define MAX_ATTR 100

enum {
    DEUX_POINTS, VIRGULE, WHERE, FROM, AND, OR, OPEQ, OPSUP, OPINF, OPNEQ, TYPE, STRING_CONSTANT,
    FLOAT_CONSTANT, INT_CONSTANT, PAREN_OUVR, PAREN_FERM, ETOILE, ENDOFCOMMAND, NOM_VARIABLE
}

struct command {
    unsigned pos;
    char *command;
};

struct token {
    int type;
    union {
        char sattr[100];
        float fattr;
        uint32_t iattr;
    } attr
};

/*
Utilisation: 
struct command c = newCommand(char *command);
struct token tok;
while( nextToken(command, &tok) >= 0) {
    //faire des choses sur le token actuel
};

*/

struct command newCommand(char *command);
int nextToken(struct command com, struct token *tok);

#endif
