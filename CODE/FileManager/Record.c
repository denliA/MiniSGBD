#include <stdio.h>
#include <stdlib.h>

#include "RelationInfo.h"
#include "Record.h"


void RecordInit(Record* rec,RelationInfo *rel){
	rec->relInfo=rel;
	rec->values=(void**)calloc(2, rel->nbCol*sizeof(void*));

	for (int i=0;i<rel->nbCol;i++){//TODO essayer de remplacer par une enum
		if (strcmp(rel->colType[i],"int")==0){
			rec->values[i]=calloc(1,sizeof(int));
		}
		else if (strcmp(rel->colType[i],"float")==0){
			rec->values[i]=calloc(1,sizeof(float));
		}
		else if (strncmp(rel->colType[i],"string",6){
				int k=(int)strtol((rel->colType[i]+6),NULL,10);
		}
	}


	/*rec->values[0]=(char*)calloc(STRING_SIZE, sizeof(char));
    rec->values[1]=(char*)calloc(STRING_SIZE, sizeof(char));*/
}
