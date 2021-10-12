#ifndef RECORD_H
#define RECORD_H
#include "RelationInfo.h"


typedef struct _Record{
	RelationInfo relInfo;
	void* values;

}Record;


void RecordInit(Record* rec, RelationInfo *rel);
void RecordFinish(Record *);
void writeToBuffer(Record *r, uint8_t *buff, uint32_t pos);
void readFromBuffer(Record *r, uint8_t *buff, uint32_t pos);


#endif
