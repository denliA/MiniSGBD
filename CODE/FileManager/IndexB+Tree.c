
#include <stdlib.h>
#include <stdio.h>
#include "IndexB+Tree.h"


BPlusTree *newBPlusNode(int32_t degre) {
    BPlusTree *node = malloc(sizeof(BPlusTree));
    node->degre = degre;
    node->nentrees = 0;
    node->indexInPere = -1;
    node->entrees = calloc(degre*2+1, sizeof(RecordEtoile));
    node->pere = node->frere_gauche = NULL;
    node->enfants = calloc(degre*2+1+1, sizeof(BPlusTree*));
    return node;
}

void splitNode(BPlusTree *node, BPlusTree **root, int not_a_leaf) {
    BPlusTree *newNode = newBPlusNode(node->degre);
    int32_t middle_key = node->entrees[node->degre].key;
    newNode->frere_gauche = node->frere_gauche;
    node->frere_gauche = newNode;
    for (int i=0; i<node->degre; i++) {
        newNode->entrees[i] = node->entrees[node->degre + i + not_a_leaf];
        if (not_a_leaf) newNode->enfants[i] = node->enfants[node->degre + i + 1];
    }
    if (not_a_leaf)
        newNode->enfants[node->degre] = node->enfants[2*node->degre + 1];
    else
        newNode->entrees[node->degre] = node->entrees[2*node->degre];
    node->nentrees = node->degre;
    newNode->nentrees = node->degre+(not_a_leaf&&1);
    insertNode(newNode, middle_key, root, node->pere, node->indexInPere+1);
}

/* Insére le noeud node comme where-ième fils du noeud father, en plaçant key comme where-1-ième clé. */ 
void insertNode(BPlusTree *node, int32_t key, BPlusTree **root, BPlusTree *father, int32_t where) {
    if (father == NULL) {
        if (where != -1) { // pas nécessaire mais pour vérifier la cohérence des données
            fprintf(stderr, "E: [insertNode] father est NULL, where devrait être égal à -1 mais where=%d\n", where); exit(-1);
        } else if (node != *root) { // pas nécessaire mais pour vérifier la cohérence des données
            fprintf(stderr, "E: [insertNode] (node)%p!=%p(*root).\n", node, *root); exit(-1);
        }
        father = newBPlusNode(node->degre);
        where = 0;
        *root = father;
    }
    for (int i=father->nentrees; i>where-1; i--) {
        father->enfants[i+1] = father->enfants[i];
        if (father->enfants[i]->indexInPere != i) { fprintf(stderr, "E: [insertNode] Incohérence représentation: father->enfants[%d]->indexInPere vaut %d (!=%d)\n", i, father->enfants[i+1]->indexInPere, i); exit(-1); }
        father->enfants[i+1]->indexInPere = i+1;
        father->entrees[i] = father->entrees[i-1];
    }
    father->entrees[where-1].key = key;
    initArray(father->entrees[where-1].rids, 0);
    father->enfants[where] = node;
    node->indexInPere = where;
    father->nentrees++;
    if (father->nentrees == father->degre*2 + 1) {
        splitNode(father, root, 1);
    } else if (father->nentrees > father->degre*2+1) {fprintf(stderr, "E: [insertnode] Incohérence représentation: father->nentrees = %d alors que degre=%d\n", father->nentrees, father->degre); exit(-1); }
    
}

BPlusTree *findEntryNode(BPlusTree *root, int32_t key) {
    BPlusTree *courant = root;
    while (courant->entrees[0].rids.tab == NULL) { // Tant que c'est un noeud interne
        int i;
        for (i=0; i<root->nentrees; i++) {
            if (courant->entrees[i].key == key) { // Cette condition est peut être même pas necessaire ?
                courant = courant->enfants[i+1];
                break;
            } else if (courant->entrees[i].key > key) {
                courant = courant->enfants[i];
                break;
            }
        }
        if(i==root->nentrees) // key est strictement plus grande que toutes les entrees
            courant = courant->enfants[i]; // On suit le dernier pointeur
    } // Ici on a trouvé la feuille qui doit contenir la bonne clé
    return courant;
}

int32_t findEntryIndex(BPlusTree *root, int32_t key) {
    for(int i=0; i<root->nentrees; i++) {
        if (root->entrees[i].key == key) {
            return i;
        }
    }
    return -1;
}

void insertRID(BPlusTree **root, Rid rid, int32_t key) {
    BPlusTree *goodNode = findEntryNode(*root, key);
    int i;
    for (i=0; i<goodNode->nentrees && goodNode->entrees[i].key >= key; i++) {
        ;
    }
    if (i < goodNode->nentrees && key == goodNode->entrees[i].key) {
        addElem(goodNode->entrees[i].rids, rid);
    } else { // On doit insérer le rid dans la ième case
        for(int j=goodNode->nentrees; j>i; j--) {
            goodNode->entrees[j] = goodNode->entrees[j-1]; // On décale les entrées pour insérer le nouveau
        }
        goodNode->entrees[i].key=key;
        initArray(goodNode->entrees[i].rids, 2);
        addElem(goodNode->entrees[i].rids, rid);
        goodNode->nentrees++;
        if (goodNode->nentrees == goodNode->degre*2+1) {
            splitNode(goodNode, root, 0);
        }
    }
}

ListeDeRids getEntryByKey(BPlusTree *root, int32_t key) {
    BPlusTree *containing_node = findEntryNode(root, key);
    int32_t index = findEntryIndex(containing_node, key);
    if(index < 0) { // La clé n'existe pas dans l'arbre
        ListeDeRids liste_vide;
        initArray(liste_vide, 0);
        return liste_vide;
    } else {
        return containing_node->entrees[index].rids;
    }
}
