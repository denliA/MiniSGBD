#include <stdio.h>
#include <stdlib.h>
#include "RelationInfo.h"

#define CHAR_SIZE 51
#define COLUMN_AMOUNT 256

extern DBParams params;

RelationInfo *RelationInfoInit(char *name, uint32_t nbCol, char **colNames, Type *colTypes, PageId headerPage) {
	RelationInfo *rel = (RelationInfo *) malloc(sizeof(RelationInfo));
	
	rel->name=name;
	rel->nbCol = nbCol;
	rel->colNames=colNames;
	rel->colTypes = colTypes;
    rel->headerPage = headerPage;
    
    rel->size = 0;
    rel->colOffset = (size_t *) malloc( (sizeof *colOffset) * nbCol );
    for(int i = 0; i<nbCol; i++) {
        rel->colOffset[i] = size;
        size += colTypes[i].type == T_INT ? sizeof(int32_t) : colTypes[i].type == T_FLOAT ? sizeof(float) : colTypes[i].stringSize;
    }
    
    unsigned int pageSize = params.pageSize;
    pageSize -= 2*PAGEID_SIZ;
    
    rel->slotCount = pageSize / (rel->size + 1);
}

void RelationInfoFinish(RelationInfo* rel){
	for (int i=0;i< rel->size;i++){
		free(rel->colNames[i]);
	}
	free(rel->name);
	free(rel->colTypes);
	free(rel->colOffset);
}
