#include <stdlib.h>
#include <stdio.h>
#include "DiskManager/DiskManager.h"
#include "DiskManager/FileList.h"
#include "DiskManager/DBParams.h"
#include "Frame.h"
#include "BufferManager.h"

static Frame *findMRU(void); // stratégie MRU. Retourne la Frame contenant la page à décharger.
static Frame *findLRU(void);

//variable globale de buffer pool
static Frame *frames;
static size_t nframes;
static size_t loaded_frames;
static unsigned long count = 0;
static UnpFrame *replacement_list;

uint8_t *GetPage(PageId pageId){
	//vérifier si la page existe en mémoire
	int i;
	int libre=-1;
	for (i=0;i<loaded_frames;i++){
		if (equalPageId(frames[i].pageId, pageId)) {
			frames[i].pin_count++;
			if(frames[i].pin_count == 1)
			    delete_unp(frames[i].unp);
			return frames[i].buffer;
		}
	}

	//lecture depuis le disque puis mise en mémoire de la page s'il y a une frame libre dispo
	if (loaded_frames < nframes){
	    frames[loaded_frames].pageId = pageId;
	    frames[loaded_frames].pin_count = 1;
	    ReadPage(pageId, frames[loaded_frames].buffer);
	    return frames[loaded_frames++].buffer;
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
    if (i==nframes){ //page pas trouvée
		fprintf(stderr, "Page de id <%d, %d> pas trouvee", pageId.FileIdx, pageId.PageIdx);
		return;
	} else if (frames[i].pin_count<=0) {
	    fprintf(stderr, "Page de id <%d, %d> a été free alors qu'elle n'était pas utilisée", pageId.FileIdx, pageId.PageIdx);
	}
	frames[i].pin_count--;
	if (frames[i].pin_count==0) {
		frames[i].unp = insertUnpAfter(lastElem(replacement_list), &frames[i]);
	}
	//attention si l'ancienne valeur de dirty vaut 1, elle reste à  1
	if (frames[i].dirty==0)
		frames[i].dirty=valdirty;

}
    



void FlushAll(){
	for (int i=0;i<nframes;i++){
		if (frames[i].dirty==1){
			WritePage(frames[i].pageId,frames[i].buffer);
			frames[i].dirty=0;
		}
		//frames[i].pin_count=0;
	}
	//free(frames[0].buffer);
}


void initBufferManager(DBParams params, uint32_t memoire) {
    uint8_t *bpool;
    
    nframes = memoire / params.pageSize;
    frames = (Frame *) calloc(nframes, sizeof(Frame));
    bpool = (uint8_t *) malloc(nframes*params.pageSize);
    for(size_t i=0; i<nframes; i++) {
        frames[i].buffer = bpool;
        bpool += params.pageSize;
    }
    loaded_frames = 0;
    replacement_list = initReplacementList();
}


Frame *findMRU() {

    if(isListEmpty(replacement_list)) {
        return NULL;
    }
    UnpFrame *unp = lastElem(replacement_list);
    Frame *ret = unp->frame;
    delete_unp(unp);
    return ret;
}

Frame *findLRU() {
    if(isListEmpty(replacement_list)) {
        return NULL;
    }
    UnpFrame *unp = firstElem(replacement_list);
    Frame *ret = unp->frame;
    delete_unp(unp);
    return ret;
}
