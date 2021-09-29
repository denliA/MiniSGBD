#include <stdlib.h>
#include <stdio.h>
#include "../DiskManager/DiskManager.h"
#include "../DiskManager/FileList.h"
#include "../DiskManager/DBParams.h"
#include "Frame.h"
#include "BufferManager.h"

//variable globale de buffer pool

static Frame *frames;
static size_t nframes;
static count = 0;

uint8_t *GetPage(PageId pageId){
	//vérifier si la page existe en mémoire
	int i;
	for (i=0;i<nframes;i++){
		if (equalPageId(frames[i].pageId, pageId)){
			return frames[i].buffer;
		}

	//a faire: strategie de remplacement en cas de buffer plein
	// 1 - Methode MRU


	}
}

void FreePage(PageId pageId, int valdirty){
	count++;
	int i;
	//recherche de la bonne case dans le tableau frames
	for (i=0;i<nframes;i++){
		if (equalPageId(frames[i].pageId, pageId)){
			break;
		}
	}
	if (i==nframes){ //page pas trouvÃ©e
		fprintf(stderr, "Page de id %d pas trouvee",pageId);
		return;
	}
	frames[i].pin_count--;
	if (frames[i].pin_count==0)
		frames[i].lastUnpin=count;
	//attention si l'ancienne valeur de dirty vaut 1, elle reste Ã  1
	if (frames[i].dirty==0)
		frames[i].dirty=valdirty;

}
    



void FlushAll(){
	for (int i=0;i<nframes;i++){
		if (frames[i].dirty==1){
			WritePage(frames[i].pageId,frames[i].buffer);
			frames[i].dirty=0;

		}
		frames[i].pin_count=0;
		free(frames[i].buffer);


	}
}


void initBufferManager(DBParams params, uint32_t memoire) {
    uint8_t *bpool;
    
    nframes = memoire / params.pageSize;
    frames = (Frame *) calloc(nframes, sizeof(Frame));
    bpool = (uint8_t *) malloc(params.pageSize);
    for(size_t i=0; i<nframes; i++) {
        frames[i].buffer = bpool;
        bpool += params.pageSize;
    }
}

int equalPageId(PageId p1, PageId p2){
	return ((p1.FileIdx == p2.FileIdx) && (p1.PageIdx == p2.PageIdx));
}
