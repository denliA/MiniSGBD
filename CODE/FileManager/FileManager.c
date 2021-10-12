#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../DiskManager/PageId.h"

#ifndef PAGEID_SIZE
#define PAGEID_SIZE 5


void writePageIdToPageBuffer(PageId pageId, uint8_t* buff, int first){
	PageId *deb=(PageId*)&buff[PAGEID_SIZE*first];
	*deb=pageId;

}

#endif
