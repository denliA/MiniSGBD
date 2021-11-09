#include <stdio.h>
#include "../DiskManager/DiskManager.h"
#include "../DiskManager/FileList.h"
extern DBParams params;
static FileList filelist;

uint8_t* word;

PageId testAllocation(){

	printf("essai d'allocation d'une page...");
	PageId testAlloc= AllocPage();
	printf("OK, FileId:%d, PageId:%d\n",testAlloc.FileIdx,testAlloc.PageIdx);
    return testAlloc;
}

void TestEcriture(PageId testAlloc, uint8_t valeur){

	word=calloc(1,8);
	*word=valeur;
	//uint8_t wval = 31;
	//word = &wval;

    printf("essai d'ecriture sur la page FileId:%d, PageId:%d\n",testAlloc.FileIdx,testAlloc.PageIdx);
	if (WritePage(testAlloc, word)==0)	printf("OK\n");
	else printf("ECHEC\n");
}

void TestLecture(PageId testAlloc){
	printf("essai de lecture sur la page...\n");
	uint8_t* buffRead = calloc(params.pageSize,sizeof(uint8_t));

	int WriteResult=ReadPage(testAlloc, buffRead);

	if (WriteResult==-1 ||* buffRead!=*word) printf("ECHEC: %d != %d\n",*word,*buffRead);
	else printf("OK:%d\n",*buffRead);

}

PageId TestDiskManager(){
	
	printf("\nTest de DiskManager:\n");
	initDiskManager();
    
    uint8_t valeur = 3;
    PageId testAlloc = testAllocation();

    TestEcriture(testAlloc,valeur);
    TestLecture(testAlloc);

    return testAlloc;
}


