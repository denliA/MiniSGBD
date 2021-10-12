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

