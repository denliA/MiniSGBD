#include <stdio.h>
#include <stdlib.h>
#include "InsertCommand.h"
#include "FileManager/Catalog.h"

Insert initInsert(char* command){
    Insert holacmoi = (Insert) malloc(sizeof(Insert));
    struct command c = newCommand(command);
    struct token tok;
    while( nextToken(command, &tok) != ENDOFCOMMAND) {
        if(tok.type==INTO){
            nextToken(command,&tok);
            if(tok.type==NOM_VARIABLE){
                //TODO traitement : appel chercheRelation
                holacmoi.relation = findRelation(tok.sattr);
            }
            else fprintf(stderr, "E: [Insertion] Commande invalide, pas de nom de relation\n");
        }
    };
}
