#ifndef FRAME_H
#define FRAME_H

#include "PageId.h"

typedef struct _Frame{
    //adresse du buffer contenant la page dans le BufferPool
    uint8_t *buffer;
    //Id de la page voulue dans la DB 
    PageId pageId;
    int pin_count;
    //instant du dernier free
    unsigned long lastUnpin;
    unsigned dirty : 1;  //il fait un seul bit
} Frame;
    
#endif
