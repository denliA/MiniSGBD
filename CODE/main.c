

#include <stdio.h>

#include "DiskManager/DBParams.h"
#include "DiskManager/DiskManager.h"

extern DBParams params;

int main(int argc, char **argv) {
    params.DBPath = argv[1]; //todo : vérifier qu'un argument a bien été donné et que c'est un chemin valide
    params.pageSize = 4096;
    params.maxPagesPerFile = 4;
    initDiskManager();

    printf("Bonjour :)\n");
    return 0;
}
