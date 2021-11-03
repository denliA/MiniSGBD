#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "DiskManager/PageId.h"
#include "DiskManager/DiskManager.h"
#include "BufferManager/BufferManager.h"
#include "FileManager.h"


#define FREE_LIST 1
#define FULL_LIST 0
#define LAST_FREE 3
#define LAST_FULL 2
#define NEXT_PAGE 1
#define PREC_PAGE 0


static void writePageIdToPageBuffer(PageId pageId, uint8_t* buff, int first){
	PageId *deb=(PageId*)&buff[PAGEID_SIZE*first];
	*deb=pageId;

}


static PageId readPageIdFromPageBuffer(uint8_t *buff, uint8_t first){
	void *debut = buff + sizeof(PageId)*first;
	PageId *ptr = (PageId*)debut;
	return *ptr;
}


PageId createHeaderPage(void){
    //allocation de la page
    PageId pageId = AllocPage();
    
    //écriture sur la page
    uint8_t *headerPage = GetPage(pageId);
    //écriture du PageId de la page dans elle même (permet de construire une liste chaînée circulaire et ainsi 
    // avoir un accès plus rapide au dernier élément
    writePageIdToPageBuffer(pageId,headerPage,0); // next plein
    writePageIdToPageBuffer(pageId,headerPage,1); // next vide
    writePageIdToPageBuffer(pageId,headerPage,2); // last plein
    writePageIdToPageBuffer(pageId,headerPage,3); // last vide
    //on libère la page qui a été modifiée donc dirty à 1
    FreePage(pageId,1);
    return pageId;
}

static PageId addDataPage(RelationInfo *rel) {
    PageId newPage = AllocPage();
    uint8_t *newPageBuff = GetPage(newPage);
    uint8_t *headerBuff = GetPage(rel->headerPage);
    PageId lastVide = readPageIdFromPageBuffer(headerBuff, 2);
    uint8_t *lastVideBuff = GetPage(lastVide);
    writePageIdToPageBuffer(lastVide, newPageBuff, 0);
    writePageIdToPageBuffer(readPageIdFromPageBuffer(lastVideBuff, 1), newPageBuff, 1);
    writePageIdToPageBuffer(newPage, lastVideBuff, 1);
    writePageIdToPageBuffer(newPage, headerBuff, 3);
    FreePage(newPage, 1);
    FreePage(rel->headerPage, 1);
    FreePage(lastVide, 1);
    return newPage;
}

/* unlinkDataPage: supprome page de la liste chaînée des pages pleines ou vides (respectivement si from == FULL_LIST ou FREE_LIST) */
static void unlinkDataPage(PageId page, PageId header, int from) {
    uint8_t *pageBuff = GetPage(page);
    PageId next = readPageIdFromPageBuffer(pageBuff, NEXT_PAGE);
    PageId prec = readPageIdFromPageBuffer(pageBuff, PREC_PAGE);
    uint8_t *nextb = GetPage(next), *precb = GetPage(prec);
    
    writePageIdToPageBuffer(prec, nextb, equalPageId(next, header) ? (from == FREE_LIST ? LAST_FREE : LAST_FULL) : PREC_PAGE);
    writePageIdToPageBuffer(next, precb, equalPageId(prec, header) ? from : NEXT_PAGE);
    
    FreePage(next, 1);
    FreePage(prec, 1);
    FreePage(page, 1);
}

static void insertDataPage(PageId page, PageId header, int where) {
    uint8_t *headerBuff = GetPage(header);
    uint8_t *pageBuff = GetPage(page);
    
    PageId last = readPageIdFromPageBuffer(headerBuff, where);
    uint8_t *lastb = GetPage(last);
    
    writePageIdToPageBuffer(header, pageBuff, NEXT_PAGE);
    writePageIdToPageBuffer(last, pageBuff, PREC_PAGE);
    writePageIdToPageBuffer(page, headerBuff, where);
    writePageIdToPageBuffer(page, lastb, equalPageId(last, header) ? (LAST_FREE?FREE_LIST:FULL_LIST) : NEXT_PAGE);
    
    FreePage(header, 1);
    FreePage(page, 1);
    FreePage(last, 1);
}

static PageId getFreePageId(RelationInfo *rel) {
    uint8_t *headerBuff = GetPage(rel->headerPage);
    PageId free_page = readPageIdFromPageBuffer(headerBuff, 1);
    if (equalPageId(free_page, rel->headerPage))
        free_page = addDataPage(rel);
    FreePage(rel->headerPage, 1);
    return free_page;
}

static Rid writeRecordToDataPage(RelationInfo *rel, Record *r, PageId p) {
    uint8_t *buff = GetPage(p);
    uint8_t *bytemap = 2*PAGEID_SIZE + buff;
    uint8_t *slots = bytemap + rel->slotCount;
    uint32_t free_slot;
    Rid rid;
    
    for(free_slot=0; free_slot < rel->slotCount; free_slot++) {
        if(!bytemap[free_slot]) {
            writeToBuffer(r, slots, free_slot*rel->size);
            if (free_slot == rel->slotCount-1) {
                unlinkDataPage(p, rel->headerPage, FREE_LIST);
                insertDataPage(p, rel->headerPage, LAST_FULL);
            }
            FreePage(p, 1);
            rid.pageId = p;
            rid.slotIdx = free_slot;
            bytemap[free_slot] = 1;
            return rid;
        }
    }
    fprintf(stderr, "E: [FileManager] Trying to writeRecord to full DataPage (page=<%u,%hhu>)\n", p.FileIdx, p.PageIdx);
    exit(-1);
}

static uint32_t getRecordsInDataPage(RelationInfo *rel, PageId p, Record *list, uint32_t *size, uint32_t *offset) {
    uint32_t readrecs = 0;
    uint8_t *pb = GetPage(p);
    uint8_t *bytemap = pb + 2*PAGEID_SIZE;
    uint8_t *slots = bytemap + rel->slotCount;
    for (uint32_t slot = 0; slot < rel->slotCount; slot++) {
        if (bytemap[slot]) {
            if(*offset == *size) {
                list = (Record *) realloc(list, sizeof(Record)*((*size)+=2*rel->slotCount));
            }
            list->relInfo = rel;
            readFromBuffer(list+((*offset)++), slots, slot*rel->size);
            readrecs++;
        }
    }
    FreePage(p, 0);
    return readrecs;
}

Rid InsertRecordIntoRelation(RelationInfo *rel, Record *rec) {
    PageId page = getFreePageId(rel);
    return writeRecordToDataPage(rel, rec, page);
}
                                          
Record *GetAllRecords(RelationInfo *rel, uint32_t *size) {
    uint8_t *header = GetPage(rel->headerPage);
    Record *list = (Record *) malloc(sizeof(Record)*(*size = 2*rel->slotCount));
    uint32_t offset = 0;
    PageId next_full = readPageIdFromPageBuffer(header, FULL_LIST);
    while(!equalPageId(next_full, rel->headerPage)) {
        PageId old_full = next_full;
        uint8_t *pbuff = GetPage(next_full);
        getRecordsInDataPage(rel, next_full, list, size, &offset);
        next_full = readPageIdFromPageBuffer(pbuff, NEXT_PAGE);
        FreePage(old_full, 0);
    }
    list = (Record *) realloc(list, sizeof(Record)*offset);
    *size = offset;
    return list;
}


ListRecordsIterator *GetListRecordsIterator(RelationInfo *rel) {
    ListRecordsIterator *iter = (ListRecordsIterator *) malloc(sizeof *iter);
    iter->rel = rel;
    uint8_t *header = GetPage(rel->headerPage);
    PageId nextFull = readPageIdFromPageBuffer(header, FULL_LIST), nextFree;
    if (!equalPageId(nextFull, rel->headerPage)) {
        setRecIterState(iter, FULL_LIST, nextFull, GetPage(nextFull), -1);
    } else if (!equalPageId(nextFree = readPageIdFromPageBuffer(header, FREE_LIST), rel->headerPage)) {
        setRecIterState(iter, FREE_LIST, nextFree, GetPage(nextFree), -1);
    } else {
        iter->currentList = -1;
    }
}

static void incrementIter(ListRecordsIterator *iter) {
    iter->currentSlot++;
    
    if(iter->currentList == FULL_LIST) {
        if (iter->currentSlot < iter->rel->slotCount) {
            iter->currentSlot++;
            return;
        } else {
            PageId next = readPageIdFromPageBuffer(iter->buffer, NEXT_PAGE);
            FreePage(iter->currentPage, 0);
            if (equalPageId(next, iter->rel->headerPage)) {
                uint8_t *header = GetPage(iter->rel->headerPage);
                PageId firstFree = readPageIdFromPageBuffer(header, FREE_LIST);
                FreePage(iter->rel->headerPage, 0);
                if(equalPageId(firstFree, iter->rel->headerPage)) {
                    iter->currentList = -1;
                    return;
                }
                setRecIterState(iter, FREE_LIST, firstFree, GetPage(firstFree), 0);
            } else {
                setRecIterState(iter, FULL_LIST, next, GetPage(next), 0);
            }
        }
    }
    
    if(iter->currentList == FREE_LIST) {
        while (iter->currentList == FREE_LIST) {
            uint8_t *bytebuf = iter->buffer + iter->rel->byteBufOff; // ??? check me maybe i'm a bug
            while(iter->currentSlot < iter->rel->slotCount) {
                if(bytebuf[iter->currentSlot])
                    return;
                iter->currentSlot++;
            }
            PageId next = readPageIdFromPageBuffer(iter->buffer, NEXT_PAGE);
            FreePage(iter->currentPage, 0);
            if (equalPageId(next,iter->rel->headerPage)) {
                iter->currentList = -1;
            } else {
                setRecIterState(iter, FREE_LIST, next, GetPage(next), 0);
            }
        }
    }
}

Record *GetNextRecord(ListRecordsIterator *iter) {
    Record *rec;
    incrementIter(iter);
    if(iter->currentList == -1)
        return NULL;
    rec = (Record *) malloc(sizeof(Record));
    readFromBuffer(rec, iter->buffer, 2*PAGEID_SIZE + iter->currentSlot * iter->rel->size);
    return rec;
}

