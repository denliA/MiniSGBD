#ifndef RELINFO_H
#define RELINFO_H

#include "DiskManager/PageId.h"

enum { T_INT, T_FLOAT, T_STRING };
typedef struct _ColType {
    uint8_t type;
    uint32_t stringSize;
} ColType;
    

typedef struct _RelInfo{
	char *name;
	uint32_t nbCol;
	char **colNames;
	ColType *colTypes;
    uint32_t size;
    uint32_t *colOffset;
    uint32_t byteBufOff;
    PageId headerPage;
    uint32_t slotCount;
    
} RelationInfo;

RelationInfo *RelationInfoInit(RelationInfo *rel, char *name, uint32_t nbCol, char **colNames, ColType *colTypesn, PageId h);
void RelationInfoFinish(RelationInfo *);

#endif
