
#include "stringutil.h"
#include "fileutil.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>

/* *************************************************************************
 * Macros pour généraliser les fonctions de manipulation de fichiers afin
 * de pouvoir le faire indépendemment du système
 ***************************************************************************/
#if defined(__linux__) || defined(__apple__)
    typedef struct stat MyStatStruct;
    #include <unistd.h>
    #define mystat stat
    #define myaccess access
    #define CANGETBLOCKSIZE 1
#elif defined(__WIN32) || defined (__win32)
    #include <io.h>
    typedef struct _stat MyStatStruct;
    #define mystat _stat
    #define myaccess _access_s
    #ifndef R_OK
    #define F_OK 0
    #define R_OK 1
    #define W_OK 2
    #define X_OK 4
    #endif
    #ifndef S_IFDIR
    #define S_IFDIR _S_IFDIR
    #endif
    #define CANGETBLOCKSIZE 0
#endif
/************************************************************************/


static int isAbsolute(char *path) {
    #if defined(__WIN32) || defined(__win32)
    return isalpha(path[0]) && path[1] == ':' && path[2] == '\\';
    #else
    return path[0] == '/';
    #endif
}

/***********************************************************************
 * Fonctions pour tester l'existence d'un fichier et les droits
 * d'accès dessus ainsi que le type d'un fichier (fichier, dossier) et
 * sa taille
 ************************************************************************/
int exists(char *file) {
    return myaccess(file, F_OK) == 0;
}

int canRead(char *file) {
    return myaccess(file, R_OK) == 0;
}

int canWrite(char *file) {
    return myaccess(file, W_OK) == 0;
}

int canExecute(char *file) {
    return myaccess(file, X_OK) == 0;
}

int isDir(char *file) {
    MyStatStruct buf;
    mystat(file, &buf);
    return buf.st_mode & S_IFDIR;
}

long int fileSize(char *file) {
    MyStatStruct buf;
    mystat(file, &buf);
    return buf.st_size;
}
/************************************************************************/


/************************************************************************
 * Fonction pour déterminer la taille d'un secteur disque
 * (ne marque que sur UNIX pour l'instant)
 * à partir de : https://stackoverflow.com/a/15947809
 ************************************************************************/
int blockSize(char *file) {
    if(!CANGETBLOCKSIZE)
        return DEFAULT_BLOCK_SIZE;
    // TODO : à remplacer par une utilisation de ioctl pour avoir la vraie taille physique.
    MyStatStruct buf;
    mystat(file, &buf);
    return buf.st_blksize;  
}
/************************************************************************/


#define VALID_FILE 1
#define VALID_DIR 2

int validPath(char *path) {
    #define _validPathCheck_(subpath, isLast)  { \
        if(!exists(subpath)) {\
            fprintf(stderr, "E: le fichier %s n'existe pas, ou n'est pas accessible. (pour path=%s)\n", (subpath), path); \
            dropTokens(subs); free(tmp);\
            return 0; \
        }\
        if(!(isLast) && !canExecute(subpath)) {\
            fprintf(stderr, "E: impossible d'accéder à %s : manque le droit d'execution sur %s\n", path, (subpath));\
            dropTokens(subs); free(tmp);\
            return 0;\
        }\
        if(!(isLast) && !isDir(subpath)) {\
            fprintf(stderr, "E: dans le chemin \"%s\", \"%s\" n'est pas un dossier.", path, subpath);\
            dropTokens(subs); free(tmp);\
            return 0;\
        }\
    }
    
    char *p = path;
    char *tmp = malloc(strlen(path)*sizeof(char));
    tmp[0] = '\0';
    int abs = isAbsolute(path);
    StrTokens subs = tokenize(path + (abs? ABSOLUTE_LEN : 0), PATH_SEPARATOR);
    
    if(abs) {
        strncat(tmp, path, ABSOLUTE_LEN);
        if (subs.size == 0)
            _validPathCheck_(tmp, 0);
    }
    for (int i=0; i<subs.size; i++) {
        strcat(tmp, i==0 && !abs ? "" : PATH_SEPARATOR);
        strcat(tmp, subs.tokens[i]);
        _validPathCheck_(tmp, i == subs.size - 1);
    }
    dropTokens(subs);
    free(tmp);
    return isDir(path) ? VALID_DIR : VALID_FILE;
}


int getBytesFromFile(char *path, uint8_t *buffer, size_t howmany) {
    FILE *f = fopen(path, "r");
    if(f==NULL) return errno;
    size_t res;
    if(res = fread(buffer, 1, howmany, f) < 0)
        return errno;
    fclose(f);
    return res;
}

int writeBytesToFile(char *path, uint8_t *buffer, size_t howmany) {
    FILE *f = fopen(path, "w");
    if(f==NULL) return errno;
    size_t res;
    if((res=fwrite(buffer, 1, howmany, f)) < howmany)
        return errno;
    fclose(f);
    return res;
}

char *tmpPath(size_t len, char *format, ...) {
    static char *string;
    if(string != NULL) {
        free(string);
    }
    if((string = malloc(len))==NULL)
        return NULL;
    va_list p;
    va_start(p, format);
    vsprintf(string, format, p);
    va_end(p);
    return string;
}

/*int main(int argc, char **argv) {
    if(argc != 2) exit(1);
    printf("%d\n", validPath(argv[1]));
    return 0;
}*/
