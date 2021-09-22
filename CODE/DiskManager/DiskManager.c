
#include <stdlib.h>
#include <stdio.h>
#include "DiskManager.h"
#include "FileList.h"
#include "DBParams.h"

static uint32_t create_new_file(void);

DBParams params;
static FileList filelist;



void initDiskManager(void) {
    filelist = getList(&params);
    if(filelist.list == NULL) { // le fichier n'existe pas encore
        filelist = initList();
    }
    on_exit(endDiskManager, NULL);
}

PageId AllocPage(void) {
    char new_file_name[16] = "./F";
    FILE *f;
    PageId pid;
    uint32_t file;
    
    for (file = 0; file < filelist.nfiles; file++) {
        for(int page=0; page<4; page++) {
            if ( !IS_ALLOC(filelist, file, page) ) {
                MAKE_ALLOC(filelist, file, page);
                pid.FileIdx = file;
                pid.PageIdx = page;
                return pid;
            }
        }
    } // Si on arrive ici, c'est que toutes les pages de tous les fichiers sont déjà allouées. On doit donc créer un nouveau fichier.

    create_new_file();

    MAKE_ALLOC(filelist, file, 0);
    pid.FileIdx = file;
    pid.PageIdx = 0;
    return pid;
}

void DesallocPage(PageId pi) { // todo: ajouter warning si on désalloue un truc non alloué ou sur un fichier non existant ?
    if(pi.FileIdx < filelist.nfiles && pi.PageIdx < 4);
        MAKE_NOT_ALLOC(filelist, pi.FileIdx, pi.PageIdx);
}

void ReadPage(PageId pi, uint8_t *buffer) { //todo : gestion d'erreurs ?
    char *file_name = getFilePath(params.DBPath, pi.FileIdx);
    FILE *file = fopen(file_name, "r");
    fseek(file, pi.PageIdx * params.pageSize, SEEK_SET);
    fread(buffer, 1, params.pageSize, file);
    fclose(file);
}

void WritePage(PageId pi, const uint8_t *buffer) {
	char *file_name = getFilePath(params.DBPath, pi.FileIdx);
    FILE *file = fopen(file_name, "w");
    fseek(file, pi.PageIdx * params.pageSize, SEEK_SET);
    fwrite(buffer, 1, params.pageSize, file);
    fclose(file);
}

static uint32_t create_new_file(void) {
	uint32_t next_file_id = addFile(&filelist);
	char *file_name = getFilePath(params.DBPath, next_file_id);
	FILE *file = fopen(file_name, "w");
	void *tmp = calloc(params.pageSize, 1);
    fwrite(tmp, params.maxPagesPerFile, params.pageSize, file);
    fclose(file);
    free(tmp);
    free (file_name);
    return next_file_id;
}

void endDiskManager(int status, void *thing) {
    saveList(filelist, &params);
}
