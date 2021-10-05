#include <stdio.h>
#include "../DiskManager/DiskManager.h"
#include "../DiskManager/FileList.h"
DBParams params;
static FileList filelist;



void TestEcriturePage(){
	params.DBPath = "../DB"; //todo : vérifier que c'est un chemin valide
	params.pageSize = 108;
	params.maxPagesPerFile = 4;
	params.frameCount = 2;

	printf("\nTest de DiskManager:\n");
	initDiskManager();

	printf("essai d'allocation d'une page...");
	PageId testAlloc= AllocPage();
	printf("OK, FileId:%d, PageId:%d\n",testAlloc.FileIdx,testAlloc.PageIdx);

	uint8_t* word=calloc(1,8);
	*word=3;
	//uint8_t wval = 31;
	//word = &wval;

	printf("essai d'ecriture sur la page...");

	if (WritePage(testAlloc, word)==0)	printf("OK\n");
	else printf("ECHEC\n");

	printf("essai de lecture sur la page...");
	uint8_t* buffRead = calloc(params.pageSize,sizeof(uint8_t));

	int WriteResult=ReadPage(testAlloc, buffRead);

	if (WriteResult==-1 ||* buffRead!=*word) printf("ECHEC: %d != %d\n",*word,*buffRead);
	else printf("OK:%d\n",*buffRead);


}
