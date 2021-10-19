#include <stdio.h>
#include <stdlib.h>
#include "RelationInfo.h"
#include "../DiskManager/DBParams.h"

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
        rel->size += colTypes[i].type == T_INT ? sizeof(int32_t) : colTypes[i].type == T_FLOAT ? sizeof(float) : colTypes[i].stringSize;
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
