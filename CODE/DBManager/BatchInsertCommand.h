/*
 * BatchInsertCommand.h
 *
 *  Created on: 16 nov. 2021
 *      Author: ij07411
 */

#ifndef CODE_DBMANAGER_BATCHINSERTCOMMAND_H_
#define CODE_DBMANAGER_BATCHINSERTCOMMAND_H_

typedef struct _BatchInsert{
	char *command;
	char *fileName;
	char *relationName;
}BatchInsert;

BatchInsert *initBatchInsert(char *command);

 void ExecuteBatchInsert(char *command);



#endif
