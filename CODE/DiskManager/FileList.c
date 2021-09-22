

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "FileList.h"

FileList getList(DBParams *p) {
    FILE *file;
    FileList fl;
    
    char *path = (char *) malloc(strlen(p->DBPath) + 1 + strlen(LISTNAME) + 1);
    path[0] = '\0';
    strcat(path, p->DBPath);
    strcat(path, "/");
    strcat(path, LISTNAME);
    
    file = fopen(path, "r");
    if (file == NULL) { // if the file does not exist yet (the DB was never initialized)
        fl.list = NULL;
        return fl; 
    }
    
    unsigned char byte;
    fread(&byte, 1, 1, file);
    
    if(byte != p->maxPagesPerFile) {
        fprintf(stderr, "E: [DiskManager] expected %hhu max pages per file for the DB storage, got %hhu in path %s\n",
                p->maxPagesPerFile, byte, p->DBPath);
        exit(-1);
    }
    
    
    fl = initList();
    
    while(!feof(file)) {
        size_t status = fread(&byte, 1, 1, file);
        if(status == 0)
            continue;
        if(fl.nfiles == fl.size)
            fl.list = (PagesStatus *) realloc( fl.list, sizeof(PagesStatus) * (fl.size+=DEF_ALLOC_LIST) );
        fl.list[fl.nfiles++] = byte;
    }
    
    fclose(file);
    return fl;
}


FileList initList(void) {
    FileList fl;
    fl.list = (PagesStatus *) malloc( sizeof(PagesStatus) * DEF_ALLOC_LIST);
    fl.size = DEF_ALLOC_LIST;
    fl.nfiles = 0;
    return fl;
}

uint32_t addFile(FileList *fl) {
    if(fl->nfiles == fl->size)
            fl->list = (PagesStatus *) realloc( fl->list, sizeof(PagesStatus) * (fl->size+=DEF_ALLOC_LIST) );
    fl->list[fl->nfiles++] = 0;
    return fl->nfiles-1;
}

void saveList(FileList fl, DBParams *p) {
    FILE *file;
    
    char *path = (char *) malloc(strlen(p->DBPath)+ 1 + strlen(LISTNAME) + 1);
    path[0] = '\0';
    strcat(path, p->DBPath);
    strcat(path, "/");
    strcat(path, LISTNAME);
    
    file = fopen(path, "w");
    
    fwrite(&p->maxPagesPerFile, 1, 1, file);
    
    for(int i = 0; i<fl.nfiles; i++) {
        fwrite(&(fl.list[i]), 1, 1, file);
    }
    fclose(file);
}

char *getFilePath(char *DBPath, uint32_t FileIdx) {
	// 							  taille chemin   "/F" x ".df" '\0'
	char *path = (char *) malloc( strlen(DBPath) + 2 + 9 + 3  +  1);
	strcpy(path, DBPath);
	strcat(path, "/F");
	sprintf(path+strlen(path), "%"PRIu32, FileIdx);
	strcat(path, ".df");
	return path; 
}
