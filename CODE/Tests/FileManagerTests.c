#include <stdio.h>
#include <stdlib.h>

#include "FileManager/Catalog.h"
#include "FileManager/Record.h"
#include "FileManager/RelationInfo.h"

void TestFileManager(){

}

void TestAddRelation() {
    char *nom = "testRelation";
    uint32_t nbCol = 4;
    char *nomsColonnes[] = {"testInt", "testFloat", "testString5", "testInt2"};
    ColType colTypesn[] = {T_INT, 0, T_FLOAT, 0, T_STRING, 5, T_INT, 0 };
    PageId pid = createHeaderPage();
    RelationInfo *rel = RelationInfoInit(NULL, nom, nbCol, nomsColonnes, colTypesn, pid);
    
}
