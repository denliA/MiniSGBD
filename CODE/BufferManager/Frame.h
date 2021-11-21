#ifndef FRAME_H
#define FRAME_H

#include "DiskManager/PageId.h"

typedef struct _Frame{
    //adresse du buffer contenant la page dans le BufferPool
    uint8_t *buffer;
    //Id de la page voulue dans la DB 
    PageId pageId;
    int pin_count;
    //instant du dernier free
    struct _unpinned_frame_list *unp;
    unsigned dirty : 1;  //il fait un seul bit
} Frame;

typedef struct _unpinned_frame_list {
    struct _unpinned_frame_list *prec;
    Frame *frame;
    struct _unpinned_frame_list *next;
} UnpFrame;

UnpFrame *initReplacementList(void);
UnpFrame *lastElem(UnpFrame *list);
UnpFrame *firstElem(UnpFrame *list);
UnpFrame *insertUnpAfter(UnpFrame *origin, Frame *f);
void delete_unp(UnpFrame *unp);
void freeReplacementList(UnpFrame *list);

#define isListEmpty(list) ( lastElem(list) == (list) )

#endif
