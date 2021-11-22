
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "DiskManager.h"
#include "FileList.h"
#include "util/fileutil.h"

#define fatal(...)   { fprintf(stderr, __VA_ARGS__); exit(EXIT_FAILURE); }

static uint32_t create_new_file(void);

DBParams params;
static FileList filelist;

void initDiskManager(void) {
    
    filelist = getList(&params);
    if(filelist.list == NULL) { // le fichier n'existe pas encore
        filelist = initList();
    }
    
    if(getInt32End() == UNRECOGNIZED_ENDIAN) fatal("E: [initDiskManager] La représentation d'entiers de cette machine n'est pas gérée\n");
    if(getFloatEnd() == UNRECOGNIZED_ENDIAN) fatal("E: [initDiskManager] La représentation de réels de cette machine n'est aps gérée\n");
    
    char *endpath = tmpPath(strlen(params.DBPath)+strlen("/.endianness"), "%s%s", params.DBPath, "/.endianness");
    if(exists(endpath)) {
        getBytesFromFile(endpath, params.saveEndianness, 2);
    } else {
        params.saveEndianness[0] = getInt32End();
        params.saveEndianness[1] = getFloatEnd();
    }
    
    atexit(endDiskManager);
}

void resetDiskManager(void){
    
    char * path;
    uint32_t file;
   
    //pour chaque file dans filelist
    for (file = 0; file < filelist.nfiles; file++){
        //on recupere son adresse
        path = getFilePath(params.DBPath, file);
        //on le supprime
        remove(path);
        free(path);
    }

    //on supprime filelist aussi
    path = (char *) malloc( strlen(params.DBPath) + strlen("/.filelist") +1);
	strcpy(path, params.DBPath);
	strcat(path, "/.filelist");
    remove(path);
    free(path);

    //on reinitialise parce que c'est un reset
    initDiskManager();
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
    FILE *file = fopen(file_name, "r+"); // avec "w" le fichier est toujours écrasé :( avec "a" fseek ne marche plus... alors il faut utiliser "r+" 
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

void endDiskManager(void) {
    saveList(filelist, &params);
    writeBytesToFile(tmpPath(strlen(params.DBPath)+strlen("/.endianness"), "%s%s", params.DBPath, "/.endianness"), params.saveEndianness, 2);
}
