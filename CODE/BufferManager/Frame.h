#ifndef FRAME_H
#define FRAME_H

#include <PageId.h>

typedef struct _Frame{
    uint8_t *buffer;
    PageId pageId;
    int pin_count;
    unsigned dirty : 1;
} Frame;
    
#endif
