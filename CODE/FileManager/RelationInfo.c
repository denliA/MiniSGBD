#include <stdio.h>
#include <stdlib.h>
#include "RelationInfo.h"

#define CHAR_SIZE 51
#define COLUMN_AMOUNT 256

void RelationInfoInit(RelationInfo* rel){
	rel->name=(char*)calloc(CHAR_SIZE,sizeof(char));
	rel->colNames=(char**)calloc(COLUMN_AMOUNT,sizeof(char*));
	rel->colType=(char**)calloc(COLUMN_AMOUNT,sizeof(int*));

	for (int i=0;i<COLUMN_AMOUNT;i++){
		rel->colNames[i]=(char*)calloc(CHAR_SIZE,sizeof(char));
		rel->colType[i]=(int*)calloc(1,sizeof(int));
	}

}

void RelationInfoFinish(RelationInfo* rel){
	for (int i=0;i< COLUMN_AMOUNT;i++){
		free(rel->colNames[i]);
		free(rel->colType[i]);
	}
	free(rel->name);
}
