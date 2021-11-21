#ifndef RECORD_H
#define RECORD_H
#include "RelationInfo.h"
#include "Rid.h"

typedef struct _Record{
	RelationInfo *relInfo;
	void* values;
    Rid rid;
}Record;


void RecordInit(Record* rec, RelationInfo *rel);
void RecordFinish(Record *);
void writeToBuffer(Record *r, uint8_t *buff, uint32_t pos);
void readFromBuffer(Record *r, uint8_t *buff, uint32_t pos);
int getTypeAtColumn(Record *r, int col);
void setColumnTo(Record *r, int col, void *value);
void *getAtColumn(Record *r, int col);

void printRecord(Record *r);

#endif
