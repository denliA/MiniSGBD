#include "DBManager.h"
#include "BufferManager/BufferManager.h"

extern  Catalog cat;

void InitDBM(){
	InitCatalog(&cat);
}

void FinishDBM(){
	FinishCatalog(&cat);
	FlushAll();
}

//affiche
void ProcessCommand(char* command){

}
