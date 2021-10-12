#include <stdio.h>
#include "BufferManagerTests.c"
#include "DiskManagerTests.c"

 extern    DBParams params;

int main(){
    
    params.DBPath = "DB"; //todo : vérifier que c'est un chemin valide
	params.pageSize = 108;
	params.maxPagesPerFile = 4;
	params.frameCount = 2;

	printf("hello world!");

	PageId id = TestDiskManager();

    TestBufferManager(id,params);
}
