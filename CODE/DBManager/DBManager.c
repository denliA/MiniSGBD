#include <string.h>
#include <stdio.h>

#include "DBManager.h"
#include "BufferManager/BufferManager.h"
#include "DiskManager/DiskManager.h"
#include "Command.h"
#include "FileManager/Catalog.h"
#include "FileManager/RelationInfo.h"

extern  Catalog cat;

#define commeq(big, little) ( strncmp( (big), (little), strlen(little) ) == 0 ) 

void InitDBM(void){
    initDiskManager();
	InitCatalog();
	initBufferManager(4096*2); // deux pages
}

void FinishDBM(void){
	FinishCatalog();
	FlushAll();
    endDiskManager();
}



void ProcessCommand(char* command){
    if(commeq(command, "CREATE RELATION")) {

    	CreateRelationCommand *crc = initCreateRelationCommand(command+strlen("CREATE RELATION"));
    	if (crc) ExecuteRelationCommand(crc);

    } else if (commeq(command, "DROPDB")) {
        supprimerDB();
    } else if (commeq(command, "INSERT")) {
        Insert holactjrsmoi = initInsert(command+strlen("INSERT"));
        if(holactjrsmoi.relation != NULL) Insertion(holactjrsmoi);
    } else if (commeq(command, "BATCHINSERT")) {
        BatchInsert *b= initBatchInsert(command);
        if(b) ExecuteBatchInsert(b);
    } else if (commeq(command, "SELECTMONO")) {
        SelectCommand *s = CreateSelectCommand(command+strlen("SELECTMONO"));
        if(s && s->rel) ExecuteSelectCommand(s);
    } else if (commeq(command, "LIST RELATIONS")) {
        printRelations();
    } else if (commeq(command, "DELETE")) {
        DeleteCommand *d = CreateDeleteCommand(command+strlen("DELETE"));
        if(d->rel) ExecuteDeleteCommand(d);
    } else if (commeq(command, "SELECTJOIN")) {
        SelectJoinCommand *sj = CreateSelectJoinCommand(command+strlen("SELECTJOIN"));
        if(sj) join(sj->R, sj->S, &sj->C);
    } else if (!commeq(command, "EXIT"))
        printf("Erreur: commande \"%s\" inconnue\n", command);
}
