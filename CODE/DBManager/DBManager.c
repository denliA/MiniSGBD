#include <string.h>
#include <stdio.h>

#include "DBManager.h"
#include "BufferManager/BufferManager.h"
#include "Command.h"
extern  Catalog cat;

#define commeq(big, little) ( strncmp( (big), (little), strlen(little) ) == 0 ) 

void InitDBM(void){
	InitCatalog();
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
        
    } else if (commeq(command, "BATCHINSERT")) {

    } else if (commeq(command, "SELECTMONO")) {

    } else if (!commeq(command, "exit"));
        printf("Erreur: commande \"%s\" inconnue\n", command);
}
