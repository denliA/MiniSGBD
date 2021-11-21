
#include <stdlib.h>

#include "BufferManager/BufferManager.h"
#include "DiskManager/PageId.h"
#include "FileManager/RelationInfo.h"
#include "FileManager/FileManager.h"
#include "FileManager/Catalog.h"
#include "Command.h"


/*********************************************************CreateRelation**************************************************/
#define CRC CreateRelationCommand
#define okToken(comm, tok) (nextToken((comm), (tok))!=ENDOFCOMMAND)

void ExecuteRelationCommand(CRC *command){
	PageId page=createHeaderPage();
	RelationInfo *rel=RelationInfoInit(NULL, command->relName,command->colNum,command->colNames, command->colTypes,page);
	AddRelationInfo(rel);
}

CRC *initCreateRelationCommand(char *com){ //return NULL s'il y a une erreur
	CRC *temp=calloc(1,sizeof(CRC));
	temp->colNames=calloc(50,50*sizeof(char));
	temp->colTypes=calloc(50,sizeof(ColType));
	struct command comm=newCommand(com);

	struct token tok;

	if (okToken(comm,&tok) && (tok.type==NOM_VARIABLE)){
				temp->relName=tok.attr;
	} else{
		return NULL;
	}

	int tracker=0;

	if(okToken(comm,&tok) && tok.type==PAREN_OUVR){
		while(okToken(comm,&tok)){
			if (tok.type==NOM_VARIABLE){
				temp->colNames[tracker]=tok.attr;
			}

			if (okToken(comm,&tok) &&tok.type==DEUX_POINTS){
				nextToken((comm), (&tok));

				ColType t=calloc(1,sizeof(ColType));
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
				case PAREN_FERM:
					return temp;
				default:
					return NULL;
				}
				temp->colTypes[tracker]=t;
				temp->colNum=tracker;

			}

			if (okToken(comm,&tok) && tok.type!=VIRGULE){
				return NULL;
			}

		}
	}
	return NULL;
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
	while(nextToken(command, &tok) != ENDOFCOMMAND) {
		if(nextToken(command, &tok) == NOM_VARIABLE){
			if (strcomp(tok.attr,"BATCHINSERT")!=0){
				fprintf(stderr,"Pas la commande BATCHINSERT");
				return NULL;
			}
		}
		if(nextToken(command, &tok) != INTO){
			fprintf(stderr,"Commande mal tapee, il manque INTO");
			return NULL;
		}
		if (nextToken(command, &tok) == NOM_VARIABLE){
			temp->relationName = tok.attr;
		}
		if (nextToken(command, &tok) != FROM){
			temp->relationName = tok.attr;
		}
		if(nextToken(command, &tok) == NOM_VARIABLE){
			if (strcomp(tok.attr,"FILE")!=0){
				fprintf(stderr,"Commande mal tapee, il manque FILE");
				return NULL;
			}
		}
		if (nextToken(command, &tok) == NOM_VARIABLE){
			temp->fileName = tok.attr;
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
    Insert holacmoi = (Insert) malloc(sizeof(Insert));
    struct command c = newCommand(command);
    struct token tok;
    while( nextToken(command, &tok) != ENDOFCOMMAND) {
        if(tok.type==INTO){
            nextToken(command,&tok);
            if(tok.type==NOM_VARIABLE){
                //TODO traitement : appel chercheRelation
                holacmoi.relation = findRelation(tok.sattr);
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
}
/****************************************************************************************************************************/
