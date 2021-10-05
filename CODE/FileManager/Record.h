#ifndef RECORD_H
#define RECORD_H
#include "RelationInfo.h"


typedef struct _Record{
	RelationInfo relInfo;
	char** values;

}Record;


void RecordInit();
void RecordFinish();



#endif
