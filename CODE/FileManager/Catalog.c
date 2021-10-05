/*
 * Catalog.c
 *
 *  Created on: 5 oct. 2021
 *      Author: denli
 */

#include <stdio.h>
#include <sdtlib.h>
#include <RelationInfo.h>
#define CAT_SIZE 50;

typedef struct _Catalog{
	RelationInfo** tab;
	int cpt;
}Catalog;

void Init(Catalog* cat){
	cat->tab=(RelationInfo**)calloc(CAT_SIZE,sizeof(*RelationInfo));
	cat->cpt=0;
}

void Finish(Catalog* cat){
	free(cat->tab);
	free(cat);
}

void AddRelation(RelationInfo * rel,Catalog* cat){
	//
	cat->tab[cat->cpt] = rel;
	cat->cpt++;
}

