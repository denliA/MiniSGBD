
#include <stdio.h>
#include <stdlib.h>

#include "../BufferManager/Frame.h"
#include "../BufferManager/BufferManager.h"

void TestBufferManager(PageId id, DBParams params){
    
    printf("\nTest du BufferManager :\n");
    initBufferManager(1024);
    
    printf("Test de lecture : \n");
    uint8_t *buffer;
    buffer = GetPage(id);
    printf("La page contient : %d\n",*buffer);

    printf("Modification de la page\n");
    *buffer = 5;
    FreePage(id,1);
    
    printf("Sauvegarde\n");
    FlushAll();

    printf("Réouverture de la page\n");
    uint8_t *buffer2;
    buffer2 = GetPage(id);
    printf("La page contient : %d\n",*buffer2);
}
    


    
