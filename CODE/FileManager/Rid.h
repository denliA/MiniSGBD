#ifndef RID_H
#define RID_H

#include <stdint.h>
#include "DiskManager/PageId.h"

#define equalRid(rid1,rid2) (equalPageId((rid1).pageId, (rid2).pageId))&&((rid1).slotIdx==(rid2).slotIdx))

typedef struct _rid{
    PageId pageId;
    uint32_t slotIdx;
}Rid;

#endif
