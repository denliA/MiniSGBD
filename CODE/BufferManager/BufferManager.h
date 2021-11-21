#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#include "DiskManager/PageId.h"
#include "DiskManager/DBParams.h"

void initBufferManager(uint32_t max_mem);
uint8_t *GetPage(PageId pageId);
void FreePage(PageId pageId, int valdirty);
void FlushAll();
void resetBufferManager(void);

#endif

