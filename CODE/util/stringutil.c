
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stringutil.h"

StrTokens tokenize(const char *string, char *atomicSep) {
    StrTokens toks;
    
    if (string == NULL) {toks.size = 0; toks.tokens = malloc(0); return toks;}
    if (atomicSep == NULL) {toks.size = 1; toks.tokens = malloc(sizeof(void*)); toks.tokens[0] = strdup(string); return toks;}
    
    int sepSize = strlen(atomicSep), stringSize = strlen(string), maxSeps = stringSize / sepSize;
    
    char **tokens = malloc(maxSeps * sizeof(char*));
    unsigned long size = 0;
    
    const char *first = string, *last = string;
    while(*first) {
        while(*last != *atomicSep && *last)
            last++;
        if (*last == '\0') {
            tokens[size++] = strdup(first);
            first = last;
        }
        else if(strncmp(last, atomicSep, sepSize) == 0) {
            tokens[size++] = strndup2(first, last-first);
            last = first = last+sepSize;
        }
        else
            last++;
    }
    
    tokens = realloc(tokens, size*sizeof(char *));
    toks.size = size;
    toks.tokens = tokens;
    return toks;
}


void dropTokens(StrTokens toks) {
    for (int i=0; i<toks.size; i++)
        free(toks.tokens[i]);
    free(toks.tokens);
}

void printTokens(StrTokens toks) {
    putchar('[');
    for (int i=0; i<toks.size; i++) printf("\"%s\", ", toks.tokens[i]);
    putchar(']'); putchar('\n');
}

char* strndup2(char* str, int i)
{
   char* t = malloc(i+1);
   if(t)
   {
       strncpy(t, str, i);
   }

   return t;
}
/*int main(void) {
    char *test1 = "/path/to/my//file/";
    StrTokens toks = tokenize(test1, "/");
    printTokens(toks);
    return 0;
}*/
