#ifndef FILELISTH
#define FILELISTH

#include <stdint.h>
#include "DBParams.h"

#define LISTNAME ".filelist"
#define DEF_ALLOC_LIST 1024

#define FLAG_ALLOC(pageid) (1 << pageid)
#define IS_ALLOC(filelist, fileid, pageid) ( (filelist.list[fileid]) & FLAG_ALLOC(pageid) )
#define MAKE_ALLOC(filelist, fileid, pageid) ( (filelist.list[fileid]) |= FLAG_ALLOC(pageid) )
#define MAKE_NOT_ALLOC(filelist, fileid, pageid) ( (filelist.list[fileid]) &= ~FLAG_ALLOC(pageid) )

typedef uint8_t PagesStatus;

typedef struct {
    PagesStatus *list;
    uint32_t nfiles;
    uint32_t size;
} FileList;

FileList getList(DBParams *p);
FileList initList(void);
uint32_t addFile(FileList *fl);
void saveList(FileList fl, DBParams *p);
char *getFilePath(char *DBPath, uint32_t FileIdx);


#endif
