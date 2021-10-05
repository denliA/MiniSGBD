#include <stdio.h>
#include <stdlib.h>

#include "RelationInfo.h"
#include "Record.h"

#define STRING_SIZE 50

void RecordInit(Record* rec,RelationInfo *rel){
	rec->relInfo=rel;
	rec->values=(char**)calloc(STRING_SIZE, sizeof(char*));

	rec->values[0]=(char*)calloc(STRING_SIZE, sizeof(char));
    rec->values[1]=(char*)calloc(STRING_SIZE, sizeof(char));
}
