#include "DBManager.h"
#include "BufferManager/BufferManager.h"

extern  Catalog cat;

void InitDBM(void){
	InitCatalog();
}

void FinishDBM(void){
	FinishCatalog();
	FlushAll();
}

//affiche
void ProcessCommand(char* command){

}
