#ifndef PAGEID_H
#define PAGEID_H

#include <stdint.h>

#define equalPageId(p1, p2) (((p1).FileIdx == (p2).FileIdx) && ((p1).PageIdx == (p2).PageIdx))

typedef struct _PageId {
    uint32_t FileIdx;
    unsigned PageIdx:2;
} PageId;


#define PAGEID_SIZE 5
 
#endif
