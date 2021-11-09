
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "DiskManager.h"
#include "FileList.h"
#include "DBParams.h"
#include "util/fileutil.h"

static uint32_t create_new_file(void);
static void check_endianness(void);

DBParams params;
static FileList filelist;
char ordre_ordi, ordre_save;


void initDiskManager(void) {
    filelist = getList(&params);
    if(filelist.list == NULL) { // le fichier n'existe pas encore
        filelist = initList();
    }
    check_endianness();
    atexit(endDiskManager);
}

PageId AllocPage(void) {
    FILE *f;
    PageId pid;
    uint32_t file;
    
    for (file = 0; file < filelist.nfiles; file++) {
        for(int page=0; page<4; page++) {
            if ( !IS_ALLOC(filelist, file, page) ) {
                MARK_ALLOC(filelist, file, page);
                pid.FileIdx = file;
                pid.PageIdx = page;
                return pid;
            }
        }
    } // Si on arrive ici, c'est que toutes les pages de tous les fichiers sont déjà allouées. On doit donc créer un nouveau fichier.

    create_new_file();

    MARK_ALLOC(filelist, file, 0);
    pid.FileIdx = file;
    pid.PageIdx = 0;
    return pid;
}

void DesallocPage(PageId pi) { // todo: ajouter warning si on désalloue un truc non alloué ou sur un fichier non existant ?
    if(pi.FileIdx < filelist.nfiles && pi.PageIdx < 4){
        MARK_NOT_ALLOC(filelist, pi.FileIdx, pi.PageIdx);
    }
}

int ReadPage(PageId pi, uint8_t *buffer) { //todo : gestion d'erreurs ?
    if (!IS_ALLOC(filelist, pi.FileIdx, pi.PageIdx)) {
        fprintf(stderr, "E: [DiskManager] Demande de lecture d'une page non allouee (File %u, Page %u)\n", pi.FileIdx, pi.PageIdx);
        return -1;
    }
    char *file_name = getFilePath(params.DBPath, pi.FileIdx);
    FILE *file = fopen(file_name, "r");
    fseek(file, pi.PageIdx * params.pageSize, SEEK_SET);
    fread(buffer, sizeof(uint8_t), params.pageSize, file);
    fclose(file);
    return 0;
}

int WritePage(PageId pi, const uint8_t *buffer) {
    if (!IS_ALLOC(filelist, pi.FileIdx, pi.PageIdx)) {
        fprintf(stderr, "E: [DiskManager] Demande d'écriture dans une page non allouee (File %u, Page %u)\n", pi.FileIdx, pi.PageIdx);
        return -1;
    }
	char *file_name = getFilePath(params.DBPath, pi.FileIdx);
    FILE *file = fopen(file_name, "w");
    fseek(file, pi.PageIdx * params.pageSize, SEEK_SET);
    fwrite(buffer, sizeof(uint8_t), params.pageSize, file);
    fclose(file);
    return 0;
}

static uint32_t create_new_file(void) {
    uint32_t next_file_id = addFile(&filelist);
    char *file_name = getFilePath(params.DBPath, next_file_id);
    FILE *file = fopen(file_name, "w");
    void *tmp = calloc(params.pageSize, 4);
    fwrite(tmp, params.maxPagesPerFile, params.pageSize, file);
    fclose(file);
    free(tmp);
    free (file_name);
    return next_file_id;
}

static void check_endianness(void) {
    int test = 1;
    char *petite = (char*) &test;
    ordre_ordi = (*petite == 1) ? LSF : MSF;
    char *tmp = malloc(strlen(params.DBPath)+strlen("/.endianness")+1); strcpy(tmp, params.DBPath); strcat(tmp, "/.endianness");
    if(exists(tmp)) {
        FILE *f = fopen(tmp, "r");
        fread(&ordre_save, 1, 1, f);
        fclose(f);
    } else {
        ordre_save = ordre_ordi;
    }
    free(tmp);
}

static void saveEndianness(void) {
    FILE *f;
    char *tmp = malloc(strlen(params.DBPath)+strlen("/.endianness")+1); strcpy(tmp, params.DBPath); strcat(tmp, "/.endianness");
    f = fopen(tmp, "w");
    fwrite(&ordre_save, 1, 1, f);
    free(tmp);
}

void endDiskManager(void) {
    saveList(filelist, &params);
    saveEndianness();
}
