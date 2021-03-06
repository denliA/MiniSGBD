#ifndef CATALOG_H
#define CATALOG_H
#include "RelationInfo.h"

typedef struct _Catalog{
	RelationInfo* tab;
	int cpt;
	int sizeMax;
}Catalog;

void InitCatalog(void);
void FinishCatalog(void);
void AddRelation(RelationInfo * rel);
RelationInfo* findRelation(char nom[]);
int findRelationIndex(char nom[]);
void resetCatalog(void);
void printRelations(void);
void dropRelation(int relationIndex);

#endif
