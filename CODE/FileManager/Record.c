#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "RelationInfo.h"
#include "Record.h"
#include "DiskManager/DBParams.h"
#include "util/endianness.h"

extern DBParams params;

void RecordInit(Record* rec, RelationInfo *rel){
	rec->relInfo=rel;
	rec->values= calloc(1, rel->size);
	initDummyRID(rec->rid);
}

void writeToBuffer(Record *rec, uint8_t *buff, uint32_t pos) {
    if(getInt32End() == params.saveEndianness[0] && getFloatEnd() == params.saveEndianness[1])
        memmove(buff+pos, rec->values, rec->relInfo->size);
    else {
        buff+=pos;
        void *ptr = rec->values;
        ColType *types = rec->relInfo->colTypes;
        for (int i=0; i<rec->relInfo->nbCol; i++) {
            if(types[i].type == T_INT) {
                writeInt32InBuffer(*(int32_t*) ptr, buff, params.saveEndianness[0]);
                buff+=4; ptr+=4;
            } else if(types[i].type == T_FLOAT) {
                writeFloatInBuffer(*(float*) ptr, buff, params.saveEndianness[1]);
                buff+=4; ptr+=4;
            } else if(types[i].type == T_STRING) {
                memmove(buff, ptr, types[i].stringSize);
                buff+= (types[i].stringSize + (4-types[i].stringSize%4));
                ptr+= (types[i].stringSize + (4-types[i].stringSize%4));
            } else {
                fprintf(stderr, "E: [readFromBuffer] Error in type (%d) of column %d (%.10s) of relation %.10s \n", types[i].type, i, rec->relInfo->colNames[i], rec->relInfo->name);
                exit(EXIT_FAILURE);
            }
        }
    }
}

void readFromBuffer(Record *rec, uint8_t *buff, uint32_t pos) {
    if(getInt32End() == params.saveEndianness[0] && getFloatEnd() == params.saveEndianness[1])
        memmove(rec->values, buff+pos, rec->relInfo->size);
    else {
        buff+=pos;
        void *ptr = rec->values;
        ColType *types = rec->relInfo->colTypes;
        for (int i=0; i<rec->relInfo->nbCol; i++) {
            if(types[i].type == T_INT) {
                uint32_t *pi = (int32_t *) ptr;
                *pi = readInt32FromBuffer(buff, params.saveEndianness[0]);
                buff+=4; ptr+=4;
            } else if(types[i].type == T_FLOAT) {
                float *pf = (float *) ptr;
                *pf = readFloatFromBuffer(buff, params.saveEndianness[1]);
                buff+=4; ptr+=4;
            } else if(types[i].type == T_STRING) {
                memmove(ptr, buff, types[i].stringSize);
                ((uint8_t*)ptr)[types[i].stringSize] = '\0';
                buff+= (types[i].stringSize + (4-types[i].stringSize%4));
                ptr+= (types[i].stringSize + (4-types[i].stringSize%4));
            } else {
                fprintf(stderr, "E: [readFromBuffer] Error in type (%d) of column %d (%.10s) of relation %.10s \n", types[i].type, i, rec->relInfo->colNames[i], rec->relInfo->name);
                exit(EXIT_FAILURE);
            }
        }
    }
}

void RecordFinish(Record *rec) {
    free(rec->values);
    free(rec);
}

int getTypeAtRecordColumn(Record *r, int col) {
    return getTypeAtColumn(r->relInfo, col);
}

void setColumnTo(Record *r, int col, void *value) {
    if (r->relInfo->nbCol <= col || col < 0) { fprintf(stderr, "E: [setColumnTo] bad col(%d) with nbCols=%d\n", col, r->relInfo->nbCol); exit(EXIT_FAILURE); }
    void *position = r->values + r->relInfo->colOffset[col];
    switch(r->relInfo->colTypes[col].type) {
    case T_INT:
        *(int32_t*)position = *(int32_t*) value;
        break;
    case T_FLOAT:
        *(float*)position = *(float*) value;
        break;
    case T_STRING:
        if(strnlen(value, r->relInfo->colTypes[col].stringSize + 1) > r->relInfo->colTypes[col].stringSize){
            fprintf(stderr, "E: [setColumnTo] setting column %d of tuple <<%d, %d>, %d> of relation %s to too long string\n", col, r->rid.pageId.FileIdx, r->rid.pageId.PageIdx, r->rid.slotIdx, r->relInfo->name);
            exit(EXIT_FAILURE);
        }
        strcpy(position, value);
        break;
    }
}

void *getAtColumn(Record *r, int col) {
    if (r->relInfo->nbCol <= col || col < 0) { fprintf(stderr, "E: [setColumnTo] bad col(%d) with nbCols=%d\n", col, r->relInfo->nbCol); exit(EXIT_FAILURE); }
    return r->values + r->relInfo->colOffset[col];
}
void printRecord(Record *r) {
    RelationInfo *rel = r->relInfo;
    for(int i=0; i< rel->nbCol; i++) {
        void *val = (r->values+rel->colOffset[i]);
        if (rel->colTypes[i].type == T_INT)
            printf("%d", *(int*)val);
        else if (rel->colTypes[i].type == T_FLOAT)
            printf("%f", *(float*)val);
        else if (rel->colTypes[i].type == T_STRING) {
            printf("%.*s", rel->colTypes[i].stringSize, (char*) val);
        } else
            printf("INVALID_COL%d_TYPE", i);
        if(i!=rel->nbCol - 1)
            printf(" ; ");
    }
    putchar('\n');
}
