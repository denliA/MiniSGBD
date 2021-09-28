#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#include "PageId.h"

void initBufferManager(DBParams params, uint32_t max_mem);
uint8_t *GetPage(PageId pageId);
void FreePage(PageId pageId, int valdirty);
void FlushAll();

#endif

