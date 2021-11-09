

#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include "PageId.h"

void initDiskManager(void);
PageId AllocPage(void);
int ReadPage(PageId, uint8_t *);
int WritePage(PageId, const uint8_t *);
void DesallocPage(PageId);
void endDiskManager(void);
void resetDiskManager(void);

#endif
