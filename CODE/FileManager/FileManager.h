#ifndef FILEMANAGER_Hcd 
#include "RelationInfo.h"
#include "Record.h"
#include "Rid.h"

typedef struct _ListRecordsIterator {
    RelationInfo *rel;
    int8_t currentList;
    PageId currentPage;
    uint8_t *buffer;
    int32_t currentSlot;
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
void DeleteRecordFromRelation(RelationInfo *rel, Rid rid);
void UpdateRecord(Record *record);
TabDeRecords GetAllRecords(RelationInfo *rel);

void printHeapFileList(PageId headerPage);

#endif
