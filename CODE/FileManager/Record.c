#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "RelationInfo.h"
#include "Record.h"


void RecordInit(Record* rec, RelationInfo *rel){
	rec->relInfo=rel;
	rec->values= calloc(1, rel->size);
}

void writeToBuffer(Record *rec, uint8_t *buff, uint32_t pos) {
    memcpy(buff+pos, rec->values, rec->relInfo->size);
}

void readFromBuffer(Record *rec, uint8_t *buff, uint32_t pos) {
    memcpy(rec->values, buff+pos, rec->relInfo->size);
}

void RecordFinish(Record *rec) {
    free(rec->values);
    free(rec);
}
