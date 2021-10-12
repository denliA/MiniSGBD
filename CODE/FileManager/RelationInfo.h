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
    size_t size;
    size_t colOffset;
    PageId headerPage;
    uint32_t slotCount;
    
} RelationInfo;

void RelationInfoInit(RelationInfo* rel, char *name, uint32_t nbCol, char **colNames, Type *colTypes);
void RelationInfoFinish(void);

#endif
