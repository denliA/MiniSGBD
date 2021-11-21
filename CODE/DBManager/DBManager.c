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

    } else if (commeq(command, "SELECTMONO")) {

    } else if (commeq(command, "LIST RELATIONS")) {
        printRelations();
    } else if (!commeq(command, "exit"))
        printf("Erreur: commande \"%s\" inconnue\n", command);
}
