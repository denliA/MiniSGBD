#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#include "PageId.h"

uint8_t *GetPage(PageId pageId);
void FreePage(PageId pageId, int valdirty);
void FlushAll();

#endif

