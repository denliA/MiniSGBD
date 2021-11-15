#ifndef RID_H
#define RID_H

#include <stdint.h>
#include "DiskManager/PageId.h"

#define equalRid(rid1,rid2) (equalPageId((rid1).pageId, (rid2).pageId))&&((rid1).slotIdx==(rid2).slotIdx))
#define initDummyRID(rid)  { (rid).slotIdx = -1; }
#define isDummyRID(rid) ( (rid).slotIdx == -1 )

typedef struct _rid{
    PageId pageId;
    int32_t slotIdx;
}Rid;

#endif
