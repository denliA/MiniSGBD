#ifndef PAGEID_H
#define PAGEID_H

#include <stdint.h>

typedef struct _PageId {
    uint32_t FileIdx;
    unsigned PageIdx:2;
} PageId;

#endif
