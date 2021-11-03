/*
 * Catalog.h
 *
 *  Created on: 5 oct. 2021
 *      Author: denli
 */

#ifndef CATALOG_H
#define CATALOG_H
#include "RelationInfo.h"

typedef struct _Catalog{
	RelationInfo* tab;
	int cpt;
	int sizeMax;
}Catalog;

void InitCatalog(Catalog* cat);

void FinishCatalog(Catalog* cat);

void AddRelation(RelationInfo * rel,Catalog* cat);

#endif
