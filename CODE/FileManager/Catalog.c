#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "Catalog.h"
#include "../DiskManager/DBParams.h"
#include "../DiskManager/PageId.h"

#define CAT_SIZE 100

extern DBParams params;

void Init(Catalog* cat){
    FILE *file;

    
    char *path = (char *) malloc(strlen(params.DBPath)+ 1 + strlen("Catalog.def") + 1);
    path[0] = '\0';
    strcat(path, params.DBPath);
    strcat(path, "/");
    strcat(path, "Catalog.def");
    
    file = fopen(path, "wx");
    if(file == NULL) {
        if(errno = EEXIST) {
            file = fopen(path, "w");
        }
        fprintf(stderr, "E: [Catalog] Can't open %s\n", params.DBPath);
        exit(-1);
    } else {
	    cat->tab=(RelationInfo*)calloc(CAT_SIZE,sizeof(RelationInfo));
	    cat->cpt=0;
	    cat->sizeMax=CAT_SIZE; //la taille max du tableau de catalog
	    return;
    }
    
    uint32_t relCount, readRels=0;
    fread(&relCount, sizeof relCount, 1, file);
    
    char *lname=NULL, **lnames;
    uint32_t pageid_f;
    uint8_t  pageid_p;
    PageId pid;
    ColType *ltypes;
    uint32_t lnbCol;
    size_t tmp_size;
    
    cat->tab = (RelationInfo *) malloc((relCount+CAT_SIZE)*sizeof(RelationInfo));
    cat->cpt = relCount;
    cat->sizeMax = relCount + CAT_SIZE;
    
    while (!feof(file) && !ferror(file) && readRels < relCount) {
        if(getline(&lname, &tmp_size, file)<=1 || fread(&lnbCol, 4, 1, file) <= 0 || fread(&pageid_f, 4, 1, file)<=0 || fread(&pageid_p, 1, 1, file) <= 0 ) {
            fprintf(stderr, "E: [Catalog] Error while loading catalog from file (in relation n°%u). Bad file format.\n", readRels);
            exit(-1);
        } else {
            lname[tmp_size-1] = '\0';
            pid.FileIdx = pageid_f;
            pid.PageIdx = pageid_p;
        }
        ltypes = (ColType *) malloc(sizeof(ColType)*lnbCol);
        lnames = (char **) calloc(lnbCol, sizeof(char *));
        for (uint32_t i=0; i<lnbCol; i++) {
            if(getline(lnames+i, &tmp_size, file)<=1 || fread(&(ltypes[i].type), 1, 1, file) <= 0 || (ltypes[i].type == T_STRING && fread(&(ltypes[i].stringSize), 4, 1, file) <= 1)) {
                fprintf(stderr, "E: [Catalog] Error while loading catalog from file (in relation n°%u, column n°%u. Bad file format.\n", readRels, i);
                exit(-1);
            } else lnames[i][tmp_size-1] = '\0';
        }
        RelationInfoInit(cat->tab, lname, lnbCol, lnames, ltypes, pid);
    }
    
}

void Finish(Catalog* cat){
    char *path = (char *) malloc(strlen(params.DBPath)+ 1 + strlen("Catalog.def") + 1);
    path[0] = '\0';
    strcat(path, params.DBPath);
    strcat(path, "/");
    strcat(path, "Catalog.def");
    
    FILE *file = fopen(path, "w");
    fwrite(&(cat->cpt), 4, 1, file);
    fprintf(file, "%s\n", cat->tab->name);
    for(uint32_t rel=0; rel<cat->cpt; rel++) {
        fprintf(file, "%s\n", cat->tab[rel].name);
        fwrite(&(cat->tab[rel].nbCol), 4, 1, file);
        for (uint32_t col=0; col< cat->tab[rel].nbCol; col++) {
            fprintf(file, "%s\n", cat->tab[rel].colNames[col]);
            fwrite(&(cat->tab[rel].colTypes[col].type), 1, 1, file);
            if(cat->tab[rel].colTypes[col].type == T_STRING)
                fwrite(&(cat->tab[rel].colTypes[col].stringSize), 4, 1, file);
        }
    }
	free(cat->tab);
	free(cat);
}

void AddRelation(RelationInfo * rel,Catalog* cat){
	//realloc si le tableau est rempli
	if ((cat->cpt)>cat->sizeMax){
		//si realloc echoue, il retourne NULL
		RelationInfo* p = realloc(cat->tab,(cat->sizeMax)+CAT_SIZE);
		if (p == NULL){
			fprintf( stderr, "Echec de réallocation de cat->tab\n");
            exit(-1);
		}
		cat->tab = p;
		cat->sizeMax=(cat->sizeMax)+CAT_SIZE;
	}
	cat->tab[cat->cpt] = *rel;
	cat->cpt++;
}

