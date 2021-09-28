#include <stdlib.h>
#include <stdio.h>
#include "DiskManager.h"
#include "FileList.h"
#include "DBParams.h"
#include "Frame.h"
#include "BufferManager.h"

//variable globale de buffer pool

static Frame *frames;
static size_t nframes;
static count;

uint8_t *GetPage(PageId pageId) {

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
