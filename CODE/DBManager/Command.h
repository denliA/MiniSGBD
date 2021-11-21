
#ifndef CODE_DBMANAGER_COMMAND_H_
#define CODE_DBMANAGER_COMMAND_H_

#include "BufferManager/BufferManager.h"
#include "DiskManager/PageId.h"
#include "FileManager/RelationInfo.h"




/*************************************************** CreateRelation ******************************************/
typedef struct cRC{
	char *relName;
	uint32_t colNum;
	char ** colNames;
	ColType *colTypes;

} CreateRelationCommand;

void ExecuteRelationCommand(CreateRelationCommand *command);
CreateRelationCommand *initCreateRelationCommand(char *com);
/**************************************************************************************************************/




/************************************************** DROPDB *****************************************************/
void supprimerDB(void);
/***************************************************************************************************************/




/***************************************************INSERT*******************************************************/

#define MAX_NOM_REL = 100
#include "FileManager/Record.h"
#include "FileManager/RelationInfo.h"
#include "CommandTokenizer.h"

/** Un pointeur vers la structure de Relation à modifier,
* et le record à ajouter.
*/
typedef struct Insert{
    RelationInfo *relation;
    Record *aAjouter;
}Insert;

Insert initInsert(char* command);
void Insertion(Insert insertion);
/******************************************************************************************************************/




/**************************************************BATCHINSERT*************************************************/
typedef struct _BatchInsert{
	char *command;
	char *fileName;
	char *relationName;
}BatchInsert;

BatchInsert *initBatchInsert(char *command);
void ExecuteBatchInsert(BatchInsert *command);
/***************************************************************************************************************/

#endif
