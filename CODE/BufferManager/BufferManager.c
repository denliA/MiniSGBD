#include <stdlib.h>
#include <stdio.h>
#include "../DiskManager/DiskManager.h"
#include "../DiskManager/FileList.h"
#include "../DiskManager/DBParams.h"
#include "Frame.h"
#include "BufferManager.h"

static Frame *findMRU(void); // stratégie MRU. Retourne la Frame contenant la page à décharger.
static Frame *findLRU(void);
int equalPageId(PageId p1, PageId p2);

//variable globale de buffer pool
static Frame *frames;
static size_t nframes;
static unsigned long count = 0;
static Frame *lastFrame= NULL; // utilisé dans la stratégie MRU

uint8_t *GetPage(PageId pageId){
	//vérifier si la page existe en mémoire
	int i;
	int libre=-1;
	for (i=0;i<nframes;i++){
		if (equalPageId(frames[i].pageId, pageId))
			return frames[i].buffer;
		if (!frames[i].buffer)libre=i;
	}

	//lecture depuis le disque puis mise en m�moire de la page s'il y a une frame libre dispo
	if (libre!=-1){
	    frames[libre] = pageId;
	    ReadPage(pageId, frames[libre].buffer);
	    return frames[libre].buffer;
	}

	// 1 - Methode MRU
    Frame *replaced = findMRU();
    if(replaced == NULL)  { // aucune frame disponible
        return NULL;
    }
    
    if (replaced->dirty == 1)
        WritePage(replaced->pageId, replaced->buffer);
    
    replaced->pageId = pageId;
    replaced->dirty = 0;
    replaced->pin_count = 1;
    return replaced->buffer;

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
		fprintf(stderr, "Page de id <%d, %d> pas trouvee", pageId.FileIdx, pageId.PageIdx);
		return;
	}
	frames[i].pin_count--;
	if (frames[i].pin_count==0) {
		frames[i].lastUnpin=count;
		lastFrame = &frames[i];
	}
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
	}
	free(frames[0].buffer);
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

Frame *findMRU() {
    return lastFrame;
}

Frame *findLRU() {
    long unsigned min_i = 0;
    int i;
    for(i=1; i<nframes; i++) {
        if(frames[i].pin_count == 0 && frames[i].lastUnpin < frames[min_i].lastUnpin)
            min_i = i;
    }
    if (i==0 && frames[i].pin_count != 0) // pas de frame disponible
        return NULL;
    else
        return frames+i;
}
