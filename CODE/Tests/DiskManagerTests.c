#include <stdio.h>
#include "../DiskManager/DiskManager.h"
#include "../DiskManager/Filelist.h"
DBParams params;
static FileList filelist;



void TestEcriturePage(){
	params.DBPath = "../../DB"; //todo : vérifier que c'est un chemin valide
	params.pageSize = 4096;
	params.maxPagesPerFile = 4;
	params.frameCount = 2;

	printf("\nTest de DiskManager:\n");
	initDiskManager();

	printf("Test d'allocation d'une page...");
	PageId testAlloc= AllocPage();
	printf("OK, FileId:%d, PageId:%d\n",testAlloc.FileIdx,testAlloc.PageIdx);

	uint8_t* word=1;

	printf("essai d'ecriture sur la page...");

	if (WritePage(testAlloc, word)==0)	printf("OK\n");
	else printf("ECHEC\n");

	printf("essai de lecture sur la page...");
	uint8_t* buffRead;

	int WriteResult=ReadPage(testAlloc, buffRead);
	if (WriteResult==-1 || buffRead!=word) printf("ECHEC: %d != %d\n",word,buffRead);
	else printf("OK:%d\n",buffRead);


}
