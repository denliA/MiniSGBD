

#include <stdio.h>

#include <DiskManager/DBParams.h>

extern DBParams params;

int main(int argc, char **argv) {
    params.DBPath = argv[1]; 
    params.pageSize = 4096;
    params.maxPagesPerFile = 4;

    printf("Bonjour :)\n");
    return 0;
}
