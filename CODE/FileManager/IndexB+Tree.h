#ifndef INDEXBPTREE_H
#define INDEXBPTREE_H

#include "util/listutil.h"
#include "Rid.h"

MAKEARRAYTYPE(ListeDeRids, Rid);
typedef struct _DataEntry {
    int32_t key;
    ListeDeRids rids;
} RecordEtoile;

MAKEARRAYTYPE(BTreeEnfants, struct _BPlusTree*); 
MAKEARRAYTYPE(ListeDeRecordEtoiles, RecordEtoile);

typedef struct _BPlusTree {
    int32_t degre;
    int32_t nentrees;
    RecordEtoile *entrees;
    struct _BPlusTree *pere;
    int32_t indexInPere;
    struct _BPlusTree *frere_gauche; //droit**
    struct _BPlusTree **enfants;
} BPlusTree;

BPlusTree *newBPlusNode(int32_t degre);
void splitNode(BPlusTree *node, BPlusTree **root, int not_a_leaf);
void insertNode(BPlusTree *node, int32_t key, BPlusTree **root, BPlusTree *father, int32_t where);
BPlusTree *findEntryNode(BPlusTree *root, int32_t key);
int32_t findEntryIndex(BPlusTree *node, int32_t key);
void insertRID(BPlusTree **root, Rid rid, int32_t key);
void deleteRID(BPlusTree **root, Rid rid, int32_t key);
ListeDeRids getEntryByKey(BPlusTree *root, int32_t key);
void printTree(BPlusTree *tree, int offset);
/* Utile un jour ?
int32_t deleteByKey(BPlusTree *racine, int32_t key); 
*/

#endif
