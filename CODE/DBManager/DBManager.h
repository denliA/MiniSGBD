#ifndef DBMANAGER_H
#define DBMANAGER_H
#include "../FileManager/Catalog.h"

void InitCatalog(void);

void FinishCatalog(void);

void ProcessCommand(char**);


#endif
