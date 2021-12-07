#ifndef RELINFO_H
#define RELINFO_H

#include "DiskManager/PageId.h"

enum { T_INT=0x1000+1, T_FLOAT=0x1000+2, T_STRING=0x1000+3 };
typedef struct _ColType {
    uint32_t type;
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
    uint32_t firstSlotOff;
    PageId headerPage;
    uint32_t slotCount;
    
} RelationInfo;

RelationInfo *RelationInfoInit(RelationInfo *rel, char *name, uint32_t nbCol, char **colNames, ColType *colTypesn, PageId h);
void RelationInfoFinish(RelationInfo *);
int getTypeAtColumn(RelationInfo *relInfo, int col);
void printRelationInfo(RelationInfo *rel);
int getColumnIndex(RelationInfo *rel,char* name);

#endif
