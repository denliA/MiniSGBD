
#include "CommandTokenizer.h"

#include <ctype.h>

struct command newCommand(char *command) {
    struct command nouv;
    nouv.command = command;
    nouv.pos = 0;
    return nouv;
}

int nextToken(struct command com, struct token *tok) {
    while(com.command[com.pos] != '\0' && isblank(com.command[com.pos]))
        com.pos++;
    switch(com.command[com.pos]) {
        case '(':  pos++; return tok.type = PAREN_OUVR;
        case ')':  pos++; return tok.type = PAREN_FERM;
        case ',':  pos++; return tok.type = VIRGULE;
        case ':':  pos++; return tok.type = DEUX_POINTS;
        case '*':  pos++; return tok.type = ETOILE;
    }
    
    
}
