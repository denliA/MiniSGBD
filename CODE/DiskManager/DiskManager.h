

#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include "PageId.h"
#include "DBParams.h"

extern DBParams params;

void initDiskManager(void);
PageId AllocPage(void);
int ReadPage(PageId, uint8_t *);
int WritePage(PageId, const uint8_t *);
void DesallocPage(PageId);
void endDiskManager(void);

#endif


