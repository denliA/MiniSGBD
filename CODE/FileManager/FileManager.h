#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#include "../DiskManager/PageId.h"
#include "RelationInfo.h"
#include "Rid.h"

Rid InsertRecordIntoRelation(RelationInfo *rel, Record *rec);
Record *GetAllRecords(RelationInfo *rel, uint32_t *size);

#endif
