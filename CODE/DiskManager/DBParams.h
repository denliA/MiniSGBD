#ifndef DBPARAMS_H
#define DBPARAMS_H

#include "util/endianness.h"

typedef struct {
    char* DBPath;//4096 = taille max d'un path linux
    unsigned int pageSize;
    unsigned char maxPagesPerFile;
    unsigned int frameCount;
    uint8_t saveEndianness[2];
}DBParams;

#endif
