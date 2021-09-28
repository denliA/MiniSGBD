#include <stdlib.h>
#include <stdio.h>
#include "DiskManager.h"
#include "FileList.h"
#include "DBParams.h"
#include "Frame.h"

//variable globale de buffer pool

uint8_t *GetPage(PageId pageId){

}

void FreePage(PageId pageId, int valdirty){
	count++;
	int i;
	//recherche de la bonne case dans le tableau frames
	for (i=0;i<nframes;i++){
		if (frames[i].pageId == pageId){
			break;
		}
	}
	if (i==nframes){ //page pas trouvée
		fprintf(stderr, "Page de id %d pas trouvée",pageId);
		return;
	}
	frames[i].pin_count--;
	if (frames[i].pin_count==0)
		frames[i].lastUnpin=count;
	//attention si l'ancienne valeur de dirty vaut 1, elle reste à 1
	if (frames[i].dirty==0)
		frames[i].dirty=valdirty;

}
    



