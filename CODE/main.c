

#include <stdio.h>

typedef struct {
    char* DBPath;//4096 = taille max d'un path linux
    unsigned int pageSize;
    char maxPagesPerFile;
}DBParams;

DBParams params;

int main(int argc, char **argv) {
    params.DBPath = argv[1]; 
    params.pageSize = 4096;
    params.maxPagesPerFile = 4;

    printf("Bonjour :)\n");
    return 0;
}
