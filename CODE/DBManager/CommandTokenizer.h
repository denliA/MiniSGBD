#ifndef COMMAND_TOKENIZER_H
#define COMMAND_TOKENIZER_H

#define MAX_ATTR 100

#include <stdint.h>
#include "FileManager/RelationInfo.h"

enum {
    INVALID_TOK=-2, ENDOFCOMMAND = -1, DEUX_POINTS, VIRGULE, POINT, WHERE, FROM, SET, AND, OR, INTO, RECORD, OPEQ, OPSUP, OPINF, OPSUPEQ, OPINFEQ, OPNEQ,
    FLOAT_CONSTANT, INT_CONSTANT, PAREN_OUVR, PAREN_FERM, ETOILE, NOM_VARIABLE, TYPE_INT=T_INT, TYPE_FLOAT=T_FLOAT, TYPE_STRING=T_STRING, STRING_CONSTANT=NOM_VARIABLE
};

struct command {
    unsigned prevpos;
    unsigned pos;
    char *command;
};

struct token {
    int type;
    union {
        char sattr[MAX_ATTR+1];
        float fattr;
        int32_t iattr;
    } attr;
};

/*
Utilisation: 
struct command c = newCommand(command);
struct token tok;
while( nextToken(&command, &tok) != ENDOFCOMMAND) {
    //faire des choses sur le token actuel : (choses précisées dans nextToken)
};
*/
struct command newCommand(char *command);


/** (ici les lettres en majuscule concernent l'énumération)
Types possiblement retournés : (qui seront à la fois dans la valeur de retour de nextToken(com, tok) et dans le champ tok->type)
    - DEUX_POINTS ou VIRGULE ou PAREN_OUVR ou PAREN_FERM ou ETOILE : indique qu'il vient de lire ":" ou "," ou "(" ou ")" ou "*" dans la chaîne.
        --> ici le champ tok->attr doit être ignoré
    - WHERE ou FROM ou INTO ou AND ou OR: indique que le mot clé correspondant a été lu 
        --> ici le champ tok->attr doit être ignoré
    - OPEQ ou OPNEQ ou OPSUP ou OPINF ou OPSUPEQ ou OPINFEQ: indique dans l'ordre les opérateurs =, <>, >, <, >=, <=
        --> ici aussi le champ tok->attr doit être ignoré 
    - TYPE_INT ou TYPE_FLOAT : pour indiquer le mot clé "int" ou "float"
        --> ici aussi le champ tok->attr doit être ignoré 
    - TYPE_STRING : pour indiquer le mot clé "stringx" avec x un entier positif
        --> on peut accéder à x avec tok->attr.iattr
    - INT_CONSTANT : pour indiquer qu'il vient de lire un nombre entier comme "35" ou "628" ou encore "0xff45d8" ou bien encore "42" ou "63" ou "-77" ou enco
        --> on peut accéder à x avec tok->attr.iattr;
    - FLOAT_CONSTANT : pour indiquer qu'il vient de lire un nombre réel comme "35.5" ou "62e8" ou encore "6.775462" ou bien ".254654654" ou "-56676.234654e45487" ou enc
        --> on peut accéder à x avec tok->attr.fattr
    - NOM_VARIABLE : Pour indiquer qu'aucun des cas décrits précédemment n'a été rencontré. Donc juste une chaîne quelconque de caractères
        --> peut représenter à la fois une valeur de stringx, OU un nom de variable
*/
int nextToken(struct command *com, struct token *tok);


void pushTokenBack(struct command *com); // Revient d'un token en arrière. (jsp si ça sera utile)

#endif
