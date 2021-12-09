
#ifndef CODE_DBMANAGER_COMMAND_H_
#define CODE_DBMANAGER_COMMAND_H_

#include "BufferManager/BufferManager.h"
#include "DiskManager/PageId.h"
#include "FileManager/RelationInfo.h"


/***************************c***********GENERAL(utile dans plusieurs commandes)*********************************/

/******CONDITIONS (utile dans SELECT, UPDATE et DELETE; toutes celle ou y a where )*******/
union value { int32_t i; float f; char *s; }; // Union pour représenter une valeur de colonne dans un tuple

typedef struct _condition { // Structure représentant une comparaison dans le WHERE entre une colonne et une valeur;
    int colonne;
    union value val; // Constante qu'on compare à chaque tuple de l'instance
    int (*operateur)(union value val, union value colonne); // Fonction évaluant la condition
} Condition;

typedef struct _condition2 { // Structure représentant une comparaison dans le WHERE entre deux colonnes;
    int colonne1;
    int colonne2;
    int (*operateur)(union value colonne1, union value colonne2); // Fonction évaluant la condition
} Condition2;


/* Exemple: Si on a dans le where "toto >= 15", dans une relation R (hehe:integer, toto:integer, nom:string5):
    - rel vaudra 
    - colonne sera égal à 1 (la position de toto dans la relation)
    - val sera de type int et val.i vaudra 15
    - operateur sera une fonction comparant deux floats, par exemple:
    int floatsupeqfloat(union value val, union value colonne) {
        return val.f >= colonne.f;
    }

Donc, pour faire le selectmono, il faut convertir chaque condition en structure condition puis faire une fonction qui évalue chaque condition
*/

#include "util/listutil.h"
MAKEARRAYTYPE(TabDeConditions, Condition); // Crée un type tableau de taille variable de conditions
/******fin CONDITIONS *************************************************************************/

/**************************************************************************************************************/

/*************************************************** CreateRelation ******************************************/
typedef struct cRC{
	char *relName;
	uint32_t colNum;
	char ** colNames;
	ColType *colTypes;

} CreateRelationCommand;

void ExecuteRelationCommand(CreateRelationCommand *command);
CreateRelationCommand *initCreateRelationCommand(char *com);
/**************************************************************************************************************/




/************************************************** DROPDB *****************************************************/
void supprimerDB(void);
/***************************************************************************************************************/




/***************************************************INSERT*******************************************************/

#define MAX_NOM_REL = 100
#include "FileManager/Record.h"
#include "FileManager/RelationInfo.h"
#include "CommandTokenizer.h"

/** Un pointeur vers la structure de Relation à modifier,
* et le record à ajouter.
*/
typedef struct Insert{
    RelationInfo *relation;
    Record *aAjouter;
}Insert;

Insert initInsert(char* command);
void Insertion(Insert insertion);
/******************************************************************************************************************/




/**************************************************BATCHINSERT*************************************************/
typedef struct _BatchInsert{
	char *command;
	char *fileName;
	RelationInfo *relation;
}BatchInsert;

BatchInsert *initBatchInsert(char *command);
void ExecuteBatchInsert(BatchInsert *command);
/***************************************************************************************************************/



/************************************************SELECTMONO*************************************************/


typedef struct _SelectCommand {
    RelationInfo *rel;
    TabDeConditions conditions; // Tableau de conditions à satisfaire
} SelectCommand;


SelectCommand *CreateSelectCommand(char *command);
void ExecuteSelectCommand(SelectCommand *command);


/*************************************************************************************************************/




/**************************************************DELETE********************************************************/

typedef SelectCommand DeleteCommand; // La commande delete est presque la même que la commande select, on a besoin des mêmes informations.

#define CreateDeleteCommand CreateSelectCommand
void ExecuteDeleteCommand(DeleteCommand *command);
/******************************************************************************************************************/


/*************************************************************UPDATE*****************************************************************/
typedef struct _UpdateCommand {
    SelectCommand selection; // Pour la partie "nomRelation WHERE liste-de-conditions" de la commande
    // etc
} UpdateCommand;

UpdateCommand *CreateUpdateCommand(char *command);
void ExecuteUpdateCommand(UpdateCommand *command);

/************************************************************************************************************************************/

/*************************************************************JOIN*****************************************************************/
typedef struct _SelectJoinCommand {
    RelationInfo *R;
    RelationInfo *S;
    Condition2 C;
} SelectJoinCommand;

SelectJoinCommand *CreateSelectJoinCommand(char *command);
void join(RelationInfo *R,RelationInfo *S,Condition2 *c);
/**********************************************************************************************************************************/

#endif
