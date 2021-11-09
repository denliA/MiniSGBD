#define cRC createRelationCommand

#include <stdlib.h>

#include "BufferManager/BufferManager.h"
#include "DiskManager/PageId.h"
#include "FileManager/RelationInfo.h"
#include "FileManager/FileManager.h"
#include "FileManager/Catalog.h"
#include "Command.h"

#define okToken(comm, tok) (nextToken((comm), (tok))!=ENDOFCOMMAND)

void ExecuteRelationCommand(cRC *command){
	PageId page=createHeaderPage();
	RelationInfo *rel=RelationInfoInit(NULL, command->relName,command->colNum,command->colNames, command->colTypes,page);
	AddRelationInfo(rel);
}

cRC *initCreateRelationCommand(char *com){ //return NULL s'il y a une erreur
	cRC *temp=calloc(1,sizeof(cRC));
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
