#ifndef RELINFO_H
#define RELINFO_H

#include "../DiskManager/PageId.h"

typedef struct _ColType {
    enum { T_INT, T_FLOAT, T_STRING } type;
    uint32_t stringSize;
} ColType;
    

typedef struct _RelInfo{
	char *name;
	uint32_t nbCol;
	char **colNames;
	ColType *colTypes;
    uint32_t size;
    uint32_t colOffset;
    PageId headerPage;
    uint32_t slotCount;
    
} RelationInfo;

RelationInfo *RelationInfoInit(char *name, uint32_t nbCol, char **colNames, Type *colTypes);
void RelationInfoFinish(RelationInfo *);

#endif
