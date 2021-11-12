#ifndef FILEUTIL_H
#define FILEUTIL_H

#define DEFAULT_BLOCK_SIZE 4096

#if defined __linux__ || defined __APPLE__ || defined __FreeBSD__
#define PATH_SEPARATOR "/"
#define ABSOLUTE_LEN 1
#elif defined _WIN32
#define PATH_SEPARATOR "\\"
#define ABSOLUTE_LEN 3
#else
#error Système d exploitation non supporté 
#endif // defined __linux__ || defined __APPLE__

#include <stddef.h>
#include <stdint.h>

// Informations sur un fichier
int exists(char *file);
int canRead(char *file);
int canWrite(char *file);
int canExecute(char *file);
int isDir(char *file);
int validPath(char *path);
long int fileSize(char *file);
int blockSize(char *file);
//int ask(char *file, char *request);


// Actions
int getBytesFromFile(char *path, uint8_t *buffer, size_t howmany);
int writeBytesToFile(char *path, uint8_t *buffer, size_t howmany);

char *tmpPath(size_t len, char *format, ...);

#endif // FILEUTIL_H
