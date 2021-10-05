/*
 * Catalog.c
 *
 *  Created on: 5 oct. 2021
 *      Author: denli
 */

#include <stdio.h>
#include <stdlib.h>
#include <RelationInfo.h>
#define CAT_SIZE 100

typedef struct _Catalog{
	RelationInfo* tab;
	int cpt;
	int sizeMax;
}Catalog;

void Init(Catalog* cat){
	cat->tab=(RelationInfo*)calloc(CAT_SIZE,sizeof(RelationInfo));
	cat->cpt=0;
	cat->sizeMax=CAT_SIZE; //la taille max du tableau de catalog
}

void Finish(Catalog* cat){
	free(cat->tab);
	free(cat);
}

void AddRelation(RelationInfo * rel,Catalog* cat){
	//realloc si le tableau est rempli
	if ((cat->cpt)>cat->sizeMax){
		//si realloc echoue, il retourne NULL
		RelationInfo* p = realloc(cat->tab,cat->sizeMax*2);
		if (p == NULL){
			fprintf( stderr, "Echec de réallocation de cat->tab");
		}
		cat->tab = p;
		cat->sizeMax=(cat->sizeMax)*2;
	}
	cat->tab[cat->cpt] = rel;
	cat->cpt++;
}

