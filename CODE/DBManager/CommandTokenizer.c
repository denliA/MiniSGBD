
#include "CommandTokenizer.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static char specials[] = ",:*><=()";

#define strbegstr(big, little) ( strncmp( (big), (little), strlen(little) ) == 0 ) 

struct command newCommand(char *command) {
    struct command nouv;
    nouv.command = command;
    nouv.pos = 0;
    return nouv;
};

int nextToken(struct command com, struct token *tok) {
    while(com.command[com.pos] != '\0' && isblank(com.command[com.pos]))
        com.pos++;
    if(com.command[com.pos] == '\0') 
        return tok->type = ENDOFCOMMAND;
    switch(com.command[com.pos]) {
        case '(':  com.pos++; return tok->type = PAREN_OUVR;
        case ')':  com.pos++; return tok->type = PAREN_FERM;
        case ',':  com.pos++; return tok->type = VIRGULE;
        case ':':  com.pos++; return tok->type = DEUX_POINTS;
        case '*':  com.pos++; return tok->type = ETOILE;
        case '=':  com.pos++; return tok->type = OPEQ;
        case '>':  case '<':
            if(com.command[com.pos+1] == '=') {
                com.pos+=2;
                return tok->type = com.command[com.pos] == '>' ? OPSUPEQ : OPINFEQ;
            }
            com.pos++;
            return tok->type = com.command[com.pos] == '>' ? OPSUP : OPINF;
    }
    if(strbegstr(com.command+com.pos, "WHERE")) {
        com.pos += strlen("WHERE");
        return tok->type = WHERE;
    } else if (strbegstr(com.command+com.pos, "FROM")) {
        com.pos+=strlen("FROM");
        return tok->type = FROM;
    } else if (strbegstr(com.command+com.pos, "AND")) {
        com.pos+= strlen("AND");
        return tok->type = AND;
    } else if(strbegstr(com.command+com.pos, "OR")) {
        com.pos += strlen("OR");
        return tok->type = OR;
    } else {
        int i;
        for (i=0; i<MAX_ATTR; i++) {
            tok->attr.sattr[i] = com.command[com.pos];
            com.pos++;
            if (strchr(specials,com.command[com.pos])) {
                tok->attr.sattr[i+1] = '\0';
                return tok->type = NOM_VARIABLE;
            }
        }
        tok->attr.sattr[i] = '\0';
        fprintf(stderr, "Erreur dans la commande \"%s\", le nom \"%s...\" est trop long\n", com.command, tok->attr.sattr);
        return tok->type = INVALID_TOK;
    }
}
