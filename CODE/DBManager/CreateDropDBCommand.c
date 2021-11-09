#include "CreateDropDB.h"
#include <stdio.h>
#include <stdlib.h>
#include "FileManager/Catalog.h"
#include "DiskManager/DiskManager.h"

void supprimerDB(){
    //on vide toutes les pages en mémoire
    resetDiskManager();
    //on vide le catalog
    resetCatalog();
}
