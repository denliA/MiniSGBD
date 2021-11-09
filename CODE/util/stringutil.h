#ifndef STRINGUTIL_H
#define STRINGUTIL_H

typedef struct _tokenizedString {
    char **tokens;
    unsigned long size;
} StrTokens; 


StrTokens tokenize(const char *string, char *atomicSep);
void dropTokens(StrTokens toks);

#endif // STRINGUTIL_H 
