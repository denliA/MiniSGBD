#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../DiskManager/PageId.h"



void writePageIdToPageBuffer(PageId pageId, uint8_t* buff, int first){
	PageId *deb=(PageId*)&buff[PAGEID_SIZE*first];
	*deb=pageId;

}


PageId readPageIdFromPageBuffer(uint8_t *buff, uint8_t first){
	void *debut = buff + sizeof(PageId)*first;
	PageId *ptr = (PageId*)debut;
	return *ptr;
}


PageId createHeaderPage(){
    //allocation de la page
    PageId pageId = AllocPage();
    
    //écriture sur la page
    uint8_t *headerPage = GetPage(pageId);
    //création du pageId factice valant (-1,0)
    PageId factice;
    factice.FileIdx = -1;
    factice.PageIdx = 0;
    //écriture de ce pageId factice deux fois dans la page (1er et 2e pageID)
    writePageIdToPageBuffer(factice,headerPage,0);
    writePageIdToPageBuffer(factice,headerPage,1);
    //on libère la page qui a été modifiée donc dirty à 1
    FreePage(pageId,1);
    return pageId;
}

