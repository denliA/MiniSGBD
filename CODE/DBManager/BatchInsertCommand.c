/*
 * BatchInsertCommand.c
 *
 *  Created on: 16 nov. 2021
 *      Author: ij07411
 */

#include "BatchInsertCommand.h"
#include "CommandTokenizer.h"
#include <stdio.h>
#include <stdlib.h>
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
