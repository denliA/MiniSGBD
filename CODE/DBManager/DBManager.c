#include <string.h>

#include "DBManager.h"
#include "BufferManager/BufferManager.h"
#include "CreateDropDBCommand.h"

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
        // 
    } else if (commeq(command, "DROPDB")) {
        supprimerDB();
    } else if (commeq(command, "INSERT")) {
        
    } else if (commeq(command, "BATCHINSERT")) {
    
    } else if (commeq(command, "SELECTMONO")) {
    
    } else 
        printf("Erreur: commande \"%S\" inconnue\n", command);
}
