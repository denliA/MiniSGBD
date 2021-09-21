#ifndef DBPARAMS_H
#define DBPARAMS_H

typedef struct {
    char* DBPath;//4096 = taille max d'un path linux
    unsigned int pageSize;
    unsigned char maxPagesPerFile;
}DBParams;

#endif
