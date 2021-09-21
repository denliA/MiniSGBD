

#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include "PageId.h"

void initDiskManager(void);
PageId AllocPage(void);
void ReadPage(PageId, uint8_t *);
void WritePage(PageId, const uint8_t *);
void DesallocPage(PageId);

#endif
