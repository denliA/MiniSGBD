#ifndef INSERTCOMMAND_H
#define INSERTCOMMAND_H

#define MAX_NOM_REL = 100
#include "FileManager/Record.h"
#include "FileManager/RelationInfo.h"
#include "CommandTokenizer.h"

/** Un pointeur vers la structure de Relation à modifier,
* et le record à ajouter.
*/
typedef struct Insert{
    RelationInfo *relation;
    Record aAjouter;
}Insert;

Insert initInsert(char* command);
void Insertion(Insert insertion);

#endif
