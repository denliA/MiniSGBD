
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
        if (not_a_leaf) {
            newNode->enfants[i] = node->enfants[node->degre + i + 1];
            newNode->enfants[i]->pere = newNode; newNode->enfants[i]->indexInPere = i; // on met à jour le père des enfants déplacés
        }
    }
    if (not_a_leaf) {
        newNode->enfants[node->degre] = node->enfants[2*node->degre + 1];
        newNode->enfants[node->degre]->pere = newNode; newNode->enfants[node->degre]->indexInPere = node->degre;
    }
    else
        newNode->entrees[node->degre] = node->entrees[2*node->degre];
    node->nentrees = node->degre;
    newNode->nentrees = node->degre+(!not_a_leaf);
    insertNode(newNode, middle_key, root, node->pere, node->indexInPere+1);
    if(node->pere == NULL) { // Si on a une nouvelle racine, on insere node dans la case 0 du pere
        node->pere = newNode->pere;
        node->indexInPere = 0;
        node->pere->enfants[0] = node;
    }
}

/* Insére le noeud node comme where-ième fils du noeud father, en plaçant key comme where-1-ième clé. */ 
void insertNode(BPlusTree *node, int32_t key, BPlusTree **root, BPlusTree *father, int32_t where) {
    if (father == NULL) {
        if (where != 0) { // pas nécessaire mais pour vérifier la cohérence des données
            fprintf(stderr, "E: [insertNode] father est NULL, where devrait être égal à 0 mais where=%d\n", where); exit(-1);
        } else if (node != *root) { // pas nécessaire mais pour vérifier la cohérence des données
            //fprintf(stderr, "E: [insertNode] (node)%p!=%p(*root).\n", node, *root); exit(-1);
        }
        father = newBPlusNode(node->degre);
        where = 1; // à 0 on insére l'ancienne node
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
    node->pere = father;
    father->nentrees++;
    if (father->nentrees == father->degre*2 + 1) {
        splitNode(father, root, 1);
    } else if (father->nentrees > father->degre*2+1) {fprintf(stderr, "E: [insertnode] Incohérence représentation: father->nentrees = %d alors que degre=%d\n", father->nentrees, father->degre); exit(-1); }
    
}

BPlusTree *findEntryNode(BPlusTree *root, int32_t key) {
    BPlusTree *courant = root;
    while (courant->enfants[0] != NULL) { // Tant que c'est un noeud interne
        int i;
        int32_t exentrees = courant->nentrees;
        //printf("courant:%p",courant); puts("");
        for (i=0; i<courant->nentrees; i++) {
            if (courant->entrees[i].key == key) { // Cette condition est peut être même pas necessaire ?
                courant = courant->enfants[i+1];
                break;
            } else if (courant->entrees[i].key > key) {
                courant = courant->enfants[i];
                break;
            }
        }
        if(i==exentrees) // key est strictement plus grande que toutes les entrees
            courant = courant->enfants[i]; // On suit le dernier pointeur
    } // Ici on a trouvé la feuille qui doit contenir la bonne clé
    //printf("Entry Node = %p\n", courant);
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
    //printf("Inserting %d\n[", key);
    //for (int j=0; j<goodNode->nentrees; j++) printf("%d:%d, ", j, goodNode->entrees[j].key);
    //printf("]\n");
    int i;
    for (i=0; i<goodNode->nentrees && goodNode->entrees[i].key < key; i++) {
        ;
    }
    //printf("Found i=%d\n\n", i);
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

void deleteRID(BPlusTree **root, Rid rid, int32_t key) {
    BPlusTree *containing_node = findEntryNode(*root, key);
    int32_t index = findEntryIndex(containing_node, key);
    if(index == -1) {fprintf(stderr, "E: [deleteRid] no key %d found\n", key); exit(-1);}
    ListeDeRids *rids = &containing_node->entrees[index].rids;
    for (int i=0; i<rids->nelems; i++) {
        if( equalRid(rids->tab[i], rid) ) {
            for (int j=i; j<rids->nelems-1; j++) {
                rids->tab[j] = rids->tab[j+1];
            }
            rids->nelems--;
            return;
        }
        //TODO: Merge qand on supprime trop. Pour le moment une entrée n'est pas supprimée même si elle a 0 rids.
    }
    printf("E: [deleteRid] rid <<%d,%d>,%d> not found with key %d\n", rid.pageId.FileIdx, rid.pageId.PageIdx, rid.slotIdx, key); exit(-1);
}

void printTree(BPlusTree *tree, int offset) {
    if (!tree) return;
    for (int i=0; i<offset; i++) putchar(' ');
    printf("%p:  [", tree);
    for (int i=0; i<tree->nentrees; i++) {
        printf("p%d(%p),%d,", i, tree->enfants[i], tree->entrees[i].key);
    } printf("p%d(%p)]  --> %p ||^%d(%p)\n",  tree->nentrees, tree->enfants[tree->nentrees], tree->frere_gauche, tree->indexInPere, tree->pere);
    for (int i=0; i<=tree->nentrees; i++) {
        printTree(tree->enfants[i], offset+2);
    }
}
