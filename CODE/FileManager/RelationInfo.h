#ifndef RELINFO_H
#define RELINFO_H

#include <stdio.h>
#include <stdlib.h>

typedef struct _RelInfo{
	char* name;
	uint8_t nbCol;
	char** colNames;
	char** colType;

}RelationInfo;

void RelationInfoInit();

#endif
