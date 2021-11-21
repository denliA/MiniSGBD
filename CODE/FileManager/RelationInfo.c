#include <stdio.h>
#include <stdlib.h>
#include "RelationInfo.h"
#include "DiskManager/DBParams.h"

#define CHAR_SIZE 51
#define COLUMN_AMOUNT 256

extern DBParams params;

RelationInfo *RelationInfoInit(RelationInfo *rel, char *name, uint32_t nbCol, char **colNames, ColType *colTypes, PageId headerPage) {
    if(rel == NULL)
        rel = (RelationInfo *) malloc(sizeof(RelationInfo));
	
    rel->name=name;
    rel->nbCol = nbCol;
    rel->colNames=colNames;
    rel->colTypes = colTypes;
    rel->headerPage = headerPage;
    
    rel->size = 0;
    rel->colOffset = (uint32_t *) malloc( (sizeof *rel->colOffset) * nbCol );
    for(int i = 0; i<nbCol; i++) {
        rel->colOffset[i] = rel->size;
        // Pour garder les int et les float alignés en mémoire, on arrondit la taille d'un string au multiple de 4 supérieur.
        rel->size += colTypes[i].type == T_INT ? sizeof(int32_t) : colTypes[i].type == T_FLOAT ? sizeof(float) : 
                        colTypes[i].type == T_STRING 
                            ? (colTypes[i].stringSize+1)%4 == 0 ? (colTypes[i].stringSize+1) : (colTypes[i].stringSize+1) + (4 - (colTypes[i].stringSize+1)%4)
                            : 0; // TODO: générer proproment une erreur si le type n'est pas valide
    }
    
    unsigned int pageSize = params.pageSize;
    pageSize -= 2*PAGEID_SIZE;
    
    rel->slotCount = pageSize / (rel->size + 1);
    rel->byteBufOff = 2 * PAGEID_SIZE + rel->slotCount * rel->size;
    return rel;
}

void RelationInfoFinish(RelationInfo* rel){
	for (int i=0;i< rel->size;i++){
		free(rel->colNames[i]);
	}
	free(rel->name);
	free(rel->colTypes);
	free(rel->colOffset);
}

int getTypeAtColumn(RelationInfo *relInfo, int col) {
    if(!relInfo || relInfo->nbCol <= col || col<0) 
        return -1;
    return relInfo->colTypes[col].type;
}

void printRelationInfo(RelationInfo *rel) {
    printf("%s ", rel->name);
    putchar('(');
    for(int i=0; i<rel->nbCol;i++) {
        printf("%s", rel->colNames[i]);
        putchar(':');
        switch(rel->colTypes[i].type) {
        case T_INT: printf("int"); break;
        case T_FLOAT: printf("float"); break;
        case T_STRING:
            printf("string");
            printf("%d", rel->colTypes[i].stringSize);
            break;
        default:
            printf("UNKNOWN");
        }
        if (i != rel->nbCol -1)
            putchar(',');
        else
            puts(")"); 
    }
}
