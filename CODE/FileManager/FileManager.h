#ifndef FILEMANAGER_Hcd 
#include "RelationInfo.h"
#include "Record.h"
#include "Rid.h"
#include "IndexB+Tree.h"

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


ListRecordsIterator *GetListRecordsIterator(RelationInfo *rel);
Record *GetNextRecord(ListRecordsIterator *iterator);


typedef struct _PageIterator {
    RelationInfo *rel;
    int8_t currentList;
    PageId currentPage;
    uint8_t *buffer;
} PageIterator;

typedef struct _RecordsOnPageIterator {
    RelationInfo *rel;
    uint8_t *pageBuffer;
    int32_t currentSlot;
    Record *record;
} RecordsOnPageIterator;

PageIterator GetPageIterator(RelationInfo *rel); // Crée un itérateur sur les pages de la relation rel
uint8_t *GetNextPage(PageIterator *iter); // Retourne le buffer de la prochaine page, et NULL si il n'y en a plus

RecordsOnPageIterator GetRecordsOnPageIterator(RelationInfo *rel, uint8_t *buffer);
Record *GetNextRecordOnPage(RecordsOnPageIterator *iter);

//************UTILISATION DES ITERATEURS SUR LES RECORDS ET SUR LES PAGES ************
/*
Exemple : Si on a une relation "RelationInfo *rel" et on veut itérer sur chacune des pages,
          et pour chaque page on itère sur chacun des records et on les print

PageIterator *pageiter = GetPageIterator(rel); // On initialise l'itérateur
RecordsOnPageIterator *recorditer; // On prépare un itérateur de records sur une page
uint8_t *buffer_de_page = GetNextPage(pageiter); // GetNextPage nous retourne le buffer de la prochaine page disponible
while (buffer_de_page != NULL) {
    recorditer = GetRecordsOnPageIterator(rel, buffer_de_page); // On initialise l'itérateur des records
    Record *record = GetNextRecord(recorditer); // retourne le prochian record dispo
    while( record != NULL) {
        printRecord(record); // Ici on affiche juste le record. 
        record = GetNextRecord(recorditer);
    }
    GetNextPage(pageiter);
}
*/

PageId createHeaderPage(void);
Rid InsertRecordIntoRelation(RelationInfo *rel, Record *rec);
void DeleteRecordFromRelation(RelationInfo *rel, Rid rid);
void UpdateRecord(Record *record);
TabDeRecords GetAllRecords(RelationInfo *rel);

void printHeapFileList(PageId headerPage);
void deleteHeapFile(PageId headerPage);

void createIndex(RelationInfo *rel, int column, int order);
TabDeRecords getRecordsUsingIndex(RelationInfo *rel, int index_column, int32_t key);
TabDeRecords getRecordsTabFromRIDList(ListeDeRids liste,RelationInfo *rel);

#endif
