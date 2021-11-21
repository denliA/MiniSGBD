
#ifndef CODE_DBMANAGER_COMMAND_H_
#define CODE_DBMANAGER_COMMAND_H_

#include "BufferManager/BufferManager.h"
#include "DiskManager/PageId.h"
#include "FileManager/RelationInfo.h"


typedef struct cRC{
	char relName[100];
	uint32_t colNum;
	char ** colNames;
	ColType *colTypes;

} CreateRelationCommand;

void ExecuteRelationCommand(CreateRelationCommand *command);
CreateRelationCommand *initCreateRelationCommand(char *com);


#endif
