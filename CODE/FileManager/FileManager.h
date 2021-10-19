#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#include "../DiskManager/PageId.h"
#include "RelationInfo.h"
#include "Record.h"
#include "Rid.h"

typedef struct _ListRecordsIterator {
    RelationInfo *rel;
    int8_t currentList;
    PageId currentPage;
    uint8_t *buffer;
    uint32_t currentSlot;
} ListRecordsIterator;

#define setRecIterState(_iter, _currentList,_currentPage, _buffer, _currentSlot) \
    { \
        iter->currentList = _currentList;\
        iter->currentPage = _currentPage;\
        iter->buffer = _buffer;\
        iter->currentSlot = _currentSlot;\
    } \

PageId createHeaderPage(void);

ListRecordsIterator *GetListRecordsIterator(RelationInfo *rel);
Record *GetNextRecord(ListRecordsIterator *iterator);


Rid InsertRecordIntoRelation(RelationInfo *rel, Record *rec);
Record *GetAllRecords(RelationInfo *rel, uint32_t *size);

#endif
