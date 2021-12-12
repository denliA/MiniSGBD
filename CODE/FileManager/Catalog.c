#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "Catalog.h"
#include "DiskManager/DBParams.h"
#include "DiskManager/PageId.h"
#include "util/fileutil.h"

#define CAT_SIZE 100

extern DBParams params;

Catalog cat;


/* Format de la sauvegarde dans Catalog.def :
<NbRelations>
    <NomRelation>\n<NbColonnes><HeaderFileIdX><HeaderPageIdx>
        <NomColonne>\n<TypeColonne><TypeStringSize>
*/
void InitCatalog(void){
    FILE *file;

    
    char *path = (char *) malloc(strlen(params.DBPath)+ 1 + strlen("Catalog.def") + 1);
    path[0] = '\0';
    strcat(path, params.DBPath);
    strcat(path, "/");
    strcat(path, "Catalog.def");
    
    if(exists(path)) {
        file = fopen(path, "r");
        if(file==NULL) {
            fprintf(stderr, "E: [Catalog] Can't open %s\n", params.DBPath);
            exit(-1);
        }
    } else {
	    cat.tab=(RelationInfo*)calloc(CAT_SIZE,sizeof(RelationInfo));
	    cat.cpt=0;
	    cat.sizeMax=CAT_SIZE; //la taille max du tableau de catalog
	    return;
    }
    
    uint32_t relCount, readRels=0;
    fread(&relCount, 4, 1, file);
    
    char *lname=NULL, **lnames;
    uint32_t pageid_f=42;
    uint8_t  pageid_p=42;
    PageId pid;
    ColType *ltypes;
    uint32_t lnbCol;
    size_t tmp_size;
    
    cat.tab = (RelationInfo *) malloc((relCount+CAT_SIZE)*sizeof(RelationInfo));
    cat.cpt = relCount;
    cat.sizeMax = relCount + CAT_SIZE;
    
    while (!feof(file) && !ferror(file) && readRels < relCount) {
        if(getline(&lname, &tmp_size, file)<=1 || (fread(&lnbCol, 4, 1, file)) <= 0 || (fread(&pageid_f, 4, 1, file))<=0 || (fread(&pageid_p, 1, 1, file)) <= 0 ) {
            fprintf(stderr, "E: [Catalog] Error while loading catalog from file (in relation n°%u, line=\"%s\", nbcol=%u,fid=%u, pid=%u). Bad file format.\n", readRels, lname, lnbCol,  pageid_f, pageid_p);
            exit(-1);
        } else {
            lname[strlen(lname)-1] = '\0';
            pid.FileIdx = pageid_f;
            pid.PageIdx = pageid_p;
        }
        ltypes = (ColType *) malloc(sizeof(ColType)*lnbCol);
        lnames = (char **) calloc(lnbCol, sizeof(char *));
        for (uint32_t i=0; i<lnbCol; i++) {
            if(getline(lnames+i, &tmp_size, file)<=1 || fread(&(ltypes[i].type), 4, 1, file) <= 0 || (ltypes[i].type == T_STRING && fread(&(ltypes[i].stringSize), 4, 1, file) <= 0)) {
                fprintf(stderr, "E: [Catalog] Error while loading catalog from file (in relation n°%u, column n°%u. Bad file format.\n", readRels, i);
                exit(-1);
            } else lnames[i][strlen(lnames[i])-1] = '\0';
        }
        RelationInfoInit(cat.tab + readRels++, strdup(lname), lnbCol, lnames, ltypes, pid);
    }
    fclose(file);
}

void FinishCatalog(void){
    char *path = (char *) malloc(strlen(params.DBPath)+ 1 + strlen("Catalog.def") + 1);
    path[0] = '\0';
    strcat(path, params.DBPath);
    strcat(path, "/");
    strcat(path, "Catalog.def");
    for(int i=0; i<cat.cpt; i++) {
        //printf("Relation %d du catalog: %s\n", i+1,  cat.tab[i].name);
    }
    FILE *file = fopen(path, "w");
    fwrite(&(cat.cpt), 4, 1, file); //TODO: utiliser endianness.c
    for(uint32_t rel=0; rel<cat.cpt; rel++) {
        fprintf(file, "%s\n", cat.tab[rel].name);
        fwrite(&(cat.tab[rel].nbCol), 4, 1, file); //TODO: utiliser endianness.c
        fwrite(&(cat.tab[rel].headerPage.FileIdx), 4, 1, file);
        uint8_t tmp;
        tmp = (cat.tab[rel].headerPage.PageIdx);
        fwrite(&tmp, 1, 1, file);
        for (uint32_t col=0; col< cat.tab[rel].nbCol; col++) {
            fprintf(file, "%s\n", cat.tab[rel].colNames[col]);
            fwrite(&(cat.tab[rel].colTypes[col].type), 4, 1, file); //TODO: utiliser endianness.c
            if(cat.tab[rel].colTypes[col].type == T_STRING){
                fwrite(&(cat.tab[rel].colTypes[col].stringSize), 4, 1, file); /*printf("%u-%u: %d\n",rel, col, cat.tab[rel].colTypes[col].stringSize);*/}
        }
    }
    fclose(file);
	free(cat.tab);
}

//supprime tout dans le catalog puis le reinitialise
void resetCatalog(){
    //on vide le catalog
    FinishCatalog();
    //on supprime le catalog
    char* path = (char *) malloc( strlen(params.DBPath) + strlen("/Catalog.def") +1);
	strcpy(path, params.DBPath);
	strcat(path, "/Catalog.def");
    if(exists(path) && remove(path)<0) {
        perror("remove catalog.def");
        exit(-1);
    };
    free(path);
    //on reinitialise le catalog
    InitCatalog();
}

//TODO
//renvoie la relation du catalog portant le nom passe en argument
RelationInfo* findRelation(char nom[]){
    //on va comparer le nom cherche au nom de chaque relation
    for(uint32_t rel=0; rel<cat.cpt; rel++){
        if(strcmp(cat.tab[rel].name, nom) == 0 ){ return &cat.tab[rel];}
    }
    return NULL;
}

int findRelationIndex(char nom[]){
    //on va comparer le nom cherche au nom de chaque relation
    for(uint32_t rel=0; rel<cat.cpt; rel++){
        if(strcmp(cat.tab[rel].name, nom) == 0 ){ return rel;}
    }
    return -1;
}

void AddRelation(RelationInfo * rel){
	//realloc si le tableau est rempli
	if ((cat.cpt)>cat.sizeMax){
		//si realloc echoue, il retourne NULL
		RelationInfo* p = realloc(cat.tab,(cat.sizeMax)+CAT_SIZE);
		if (p == NULL){
			fprintf( stderr, "Echec de réallocation de cat.tab\n");
            exit(-1);
		}
		cat.tab = p;
		cat.sizeMax=(cat.sizeMax)+CAT_SIZE;
	}
	cat.tab[cat.cpt] = *rel;
	cat.cpt++;
}

void printRelations(void) {
    for(int i=0; i<cat.cpt; i++) {
        printRelationInfo(&cat.tab[i]);
    }
}
