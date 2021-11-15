#ifndef LISTUTIL_H
#define LISTUTIL_H

#include <stddef.h>
#include <stdlib.h>

/* Définissions génériques pour un tableau de taille variable  */
/*
Utilisation : Si on veut par exemple manipuler un tableau d'objets de type int
    1- Les fichiers qui manipulent le tableau doivent avoir une définition de celui ci, donc dans un header commun on met:
        MAKEARRAYTYPE(TabEntiers, int);
    2- Pour l'utiliser en général : 
        TabEntiers montableau;
        initArray(montableau, 5); // à chaque fois que le tableau manque d'espace, on rajoute 5 à sa taille
        addElem(montableau, 42); // montableau contient désormais [42]
        addElem(montableau, -1); // montableau contien désormais [42, -1]
        int elem = montableau.tab[1]; // elem vaut -1
*/


#define MAKEARRAYTYPE(name, type) typedef struct name { type *tab;  size_t nelems; size_t maxelems; size_t increment;} name;
#define initArray(array, def_increment) { array.nelems = 0; array.tab = malloc( (sizeof array[0]) * def_increment); array.maxelems = array.increment = def_increment; }
#define addElem(array, elem) { if(array.nelems >= array.maxelems) { array.tab = realloc((sizeof array[0])*(array.maxelems += array.increment)); } array.tab[nelems++] = elem; }
#define deleteArray(array) { free(array.tab); }



#endif // LISTUTIL_H

