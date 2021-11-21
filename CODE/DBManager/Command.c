
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "BufferManager/BufferManager.h"
#include "DiskManager/PageId.h"
#include "DiskManager/DiskManager.h"
#include "FileManager/RelationInfo.h"
#include "FileManager/FileManager.h"
#include "FileManager/Catalog.h"
#include "Command.h"



/**********************************************************GÉNÉRAL*********************************************************/

// Pour afficher un message d'erreur puis quitter la fonction lorsqu'il y a un erreur dans la commande à parser 
#define SYNTAX_ERROR(...) {\
    fprintf(stderr, __VA_ARGS__);\
    return NULL;\
}

// Lorsque l'utiliateur donne une valeur du mauvais type
#define CONST_TYPE_ERROR(rel, ncol, good, bad) {\
    SYNTAX_ERROR("Erreur: La colonne %d de %s est de type %s, mais j'ai eu un %s.\n", (ncol), (rel)->name, typeToString(good), typeToString(bad)); \
}

// Pour print le type d'une colonne donnée.
#define typeToString(type) ( (type) == T_INT ? "integer" : (type) == T_FLOAT ?  "float" : (type) == T_STRING ? "stringX" : "INCONNU" ) 


/* Entrées: - rel : la relation que représente le tuple. 
            - command : commande commençant exactement au début du tuple, c'est à dire avec une parenthèse ouvrante 
   Sortie: record contenant le tuple représenté par command.
   
   La fonction effectue toutes les vérifications de syntaxe et de types.
*/
static Record *parseTuple(RelationInfo *rel, struct command *comm) {
    struct token tok;
    Record *rec = malloc(sizeof(Record));
    RecordInit(rec, rel);
    
    if(nextToken(comm, &tok) != PAREN_OUVR) {
        RecordFinish(rec); SYNTAX_ERROR("Erreur de syntaxe: Je m'attendais à un parenthèse ouvrante\n");
    }
    for(int i=0; i<rel->nbCol;i++) {
        int type;
        switch(nextToken(comm, &tok)) {
        case INT_CONSTANT:
            if((type=getTypeAtColumn(rel, i)) != T_INT) {
                RecordFinish(rec);
                CONST_TYPE_ERROR(rel, i, type, T_INT);
            } break;
        case FLOAT_CONSTANT:
            if((type=getTypeAtColumn(rel,i)) != T_FLOAT) {
                RecordFinish(rec);
                CONST_TYPE_ERROR(rel, i, type, T_FLOAT);
            } break;
        case STRING_CONSTANT:
            if((type=getTypeAtColumn(rel,i)) != T_FLOAT) {
                RecordFinish(rec); CONST_TYPE_ERROR(rel, i, type, T_STRING);
            } else if (strlen(tok.attr.sattr) > rel->colTypes[i].stringSize) {
                RecordFinish(rec);
                SYNTAX_ERROR("Erreur sur la colonne %d, %s fait %ld caractères, mais %s(%d) est de type string%d\n", i, tok.attr.sattr, strlen(tok.attr.sattr), rel->colNames[i], i, rel->colTypes[i].stringSize);
            } break;
         default:
            RecordFinish(rec); SYNTAX_ERROR("Erreur: Je m'attendais à une constante pour la colonne %d de %s\n", i, rel->name);
        }
        
        setColumnTo(rec, i, &tok.attr);
        
        if( i == rel->nbCol - 1 && nextToken(comm, &tok) != PAREN_FERM ) {
            RecordFinish(rec); SYNTAX_ERROR("Erreur: Je m'attendais à une virgule après la colonne %d de %s\n", i, rel->name);
        } else if ( tok.type != VIRGULE ) {
            RecordFinish(rec); SYNTAX_ERROR("Erreur: %i records lus, parenthèse fermante manquante.\n", i+1);
        }
    }
    return rec;
}




/****************************************************************************************************************************/


/*********************************************************CreateRelation**************************************************/
#define CRC CreateRelationCommand
#define okToken(comm, tok) (nextToken((comm), (tok))!=ENDOFCOMMAND)

void ExecuteRelationCommand(CRC *command){
	PageId page=createHeaderPage();
	RelationInfo *rel=RelationInfoInit(NULL, command->relName,command->colNum,command->colNames, command->colTypes,page);
	AddRelation(rel);
}

CRC *initCreateRelationCommand(char *com){ //return NULL s'il y a une erreur
	CRC *temp=calloc(1,sizeof(CRC));
	temp->colNames=calloc(5,5*sizeof(char*));
	temp->colTypes=calloc(5,sizeof(ColType));
	size_t sizeTabs = 5;
	struct command comm=newCommand(com);

	struct token tok;

	if (okToken(&comm,&tok) && (tok.type==NOM_VARIABLE)){
				temp->relName=strdup(tok.attr.sattr);
	} else{
		SYNTAX_ERROR("Erreur dans la commande: Je m'attendais à un nom de relaton.\n");
	}

	int tracker=0;

	if(okToken(&comm,&tok) && tok.type==PAREN_OUVR){
		while(okToken(&comm,&tok) && tok.type != PAREN_FERM){
		    if(tracker == sizeTabs) {
		        temp->colNames = realloc(temp->colNames, (sizeTabs+5)*sizeof(char*));
		        temp->colTypes = realloc(temp->colTypes, (sizeTabs+5)*sizeof(ColType));
		        sizeTabs+=5;
		    }
		    
			if (tok.type==NOM_VARIABLE){
				temp->colNames[tracker]=strdup(tok.attr.sattr);
			} else { SYNTAX_ERROR("Erreur de syntaxe: Je m'attendais à un nom de colonne.\n"); }

			if (okToken(&comm,&tok) && tok.type==DEUX_POINTS){
				nextToken((&comm), (&tok));

				ColType t;
				switch (tok.type){

				case TYPE_INT:
					t.type=T_INT;
					break;
				case TYPE_FLOAT:
					t.type=T_FLOAT;
					break;
				case TYPE_STRING:
					t.type=T_STRING;
					t.stringSize=tok.attr.iattr;
					break;
				default:
					SYNTAX_ERROR("Erreur: Je m'attendais à un type\n");
				}
				temp->colTypes[tracker]=t;
				temp->colNum=tracker;

			} else { SYNTAX_ERROR("Erreur de syntaxe: Je m'attendais à deux points.\n"); }

			if (okToken(&comm,&tok) && tok.type!=VIRGULE && tok.type!=PAREN_FERM){
				SYNTAX_ERROR("Erreur de syntaxe: Je m'attendais à une virgule ou à une parenthèse fermante\n");
			}
			tracker++;
		}
	} else {
	SYNTAX_ERROR("Erreur dans la commande: Je m'attendais à une parenthèse ouvrante, j'ai eu %d\n", tok.type);	
	}
	temp->colNum = tracker;
	return temp;
}
/***************************************************************************************************************************/




/****************************************************BATCHINSERT************************************************************/
#define NB 20

BatchInsert *initBatchInsert(char *command){
	char chaine[NB];
	BatchInsert *temp = calloc(1,sizeof(BatchInsert));
	temp->command = command;
	struct command comm  = newCommand(command);
	struct token tok;
	while(nextToken(&comm, &tok) != ENDOFCOMMAND) {
		if(nextToken(&comm, &tok) == NOM_VARIABLE){
			if (strcmp(tok.attr.sattr,"BATCHINSERT")!=0){
				fprintf(stderr,"Pas la commande BATCHINSERT");
				return NULL;
			}
		}
		if(nextToken(&comm, &tok) != INTO){
			fprintf(stderr,"Commande mal tapee, il manque INTO");
			return NULL;
		}
		if (nextToken(&comm, &tok) == NOM_VARIABLE){
			temp->relationName = strdup(tok.attr.sattr);
		}
		if (nextToken(&comm, &tok) != FROM){
			if (strcmp(tok.attr.sattr,"FILE")!=0){
				fprintf(stderr,"Commande mal tapee, il manque FILE");
				return NULL;
			}
		}
		if (nextToken(&comm, &tok) == NOM_VARIABLE){
			temp->fileName = strdup(tok.attr.sattr);
		}
	};
	return temp;
}

void ExecuteBatchInsert(BatchInsert *command){
	FILE* fich = fopen(command->fileName,"r");
}
/****************************************************************************************************************************/



/*****************************************************INSERT*****************************************************************/
Insert initInsert(char* command){
    Insert holacmoi;
    struct command c = newCommand(command);
    struct token tok;
    while( nextToken(&c, &tok) != ENDOFCOMMAND) {
        if(tok.type==INTO){
            nextToken(&c,&tok);
            if(tok.type==NOM_VARIABLE){
                //TODO traitement : appel chercheRelation
                holacmoi.relation = findRelation(tok.attr.sattr);
            }
            else fprintf(stderr, "E: [Insertion] Commande invalide, pas de nom de relation\n");
        }
    };
}
/***************************************************************************************************************************/


/***********************************************************DROPDB**********************************************************/
void supprimerDB(void){
    //on vide toutes les pages en mémoire
    resetDiskManager();
    //on vide le catalog
    resetCatalog();
    //on vide le BufferManager
    resetBufferManager();
}
/****************************************************************************************************************************/
