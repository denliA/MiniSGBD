
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
static Record *parseTuple(RelationInfo *rel, struct command *comm, int parens) {
    struct token tok;
    Record *rec = malloc(sizeof(Record));
    RecordInit(rec, rel);
    
    if(parens && nextToken(comm, &tok) != PAREN_OUVR) {
        RecordFinish(rec); SYNTAX_ERROR("Erreur de syntaxe: Je m'attendais à un parenthèse ouvrante\n");
    }
    for(int i=0; i<rel->nbCol;i++) {
        int type;
        switch(nextToken(comm, &tok)) {
        case INT_CONSTANT:
            if ((type=getTypeAtColumn(rel,i)) == T_FLOAT) { // On doit convertir le int en float ici car setColumnTo(...) s'attendra à recevoir un float et non un int
                tok.attr.fattr = (float) tok.attr.iattr;
            }
            else if((type=getTypeAtColumn(rel, i)) != T_INT) {
                RecordFinish(rec);
                CONST_TYPE_ERROR(rel, i, type, T_INT);
            } break;
        case FLOAT_CONSTANT:
            if((type=getTypeAtColumn(rel, i)) != T_FLOAT) {
                RecordFinish(rec);
                CONST_TYPE_ERROR(rel, i, type, T_FLOAT);
            } break;
        case STRING_CONSTANT:
            if((type=getTypeAtColumn(rel,i)) != T_STRING) {
                RecordFinish(rec); CONST_TYPE_ERROR(rel, i, type, T_STRING);
            } else if (strlen(tok.attr.sattr) > rel->colTypes[i].stringSize) {
                RecordFinish(rec);
                SYNTAX_ERROR("Erreur sur la colonne %d, %s fait %ld caractères, mais %s(%d) est de type string%d\n", i, tok.attr.sattr, strlen(tok.attr.sattr), rel->colNames[i], i, rel->colTypes[i].stringSize);
            } break;
         default:
            RecordFinish(rec); SYNTAX_ERROR("Erreur: Je m'attendais à une constante pour la colonne %d de %s, tok.type == %d\n", i, rel->name, tok.type);
        }
        
        setColumnTo(rec, i, &tok.attr);
        
        nextToken(comm, &tok);
        if (i == rel->nbCol-1) {
            if( parens && tok.type != PAREN_FERM ) {
                RecordFinish(rec); SYNTAX_ERROR("Erreur: %i colonnes lues, parenthèse fermante manquante.\n", i+1);            
            }
        } else if (tok.type != VIRGULE) {
            RecordFinish(rec); SYNTAX_ERROR("Erreur: Je m'attendais à une virgule après la colonne %d de %s. (n°token = %d)\n", i, rel->name, tok.type);        
        }
    }
    return rec;
}


// Prend en entrée une commande qui commence juste après le where
// Donne en sortie un tableau avec toutes les conditions
TabDeConditions parseConditions(struct command *command) {

}


//Prend en entrée une condition, et l'évalue pour le record donné
int evaluerCondition(Condition *c, Record *record) {
	union value valRec;
	//valeur contenue à l'indice colonne dans le tuple
	void *valueRecord = getAtColumn(record, c->colonne);;
	//type de la valeur
	int type = getTypeAtRecordColumn(record,c->colonne);
	if (type == T_INT){
		int* intValue = (int*)valueRecord;
		valRec.i = *intValue;
	}
	else if(type == T_FLOAT){
		float* floatValue = (float*)valueRecord;
		valRec.f = *floatValue;
	}
	else if(type == T_STRING){
		char** stringValue = (char**)valueRecord;
		valRec.s = *stringValue;
	}
	//on evalue la condition
	return c->operateur(c->val,valRec);


}

// Retourne le résultat de l'évaluation d'un ensemble de conditions avec AND entre elles. Utilise evaluerCondition
int evaluerAndConditions(TabDeConditions conditions, Record *record) {
    //TODO
}

// Prend en entrée un tableau de reltions, et un tableau de conditions, et retourne un tableau de record respectant toutes les conditions
// Utilise: evaluerAndConditions() 
Record *filtrerRecords(Record *tous, TabDeConditions conditions) {

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
	{
		if(nextToken(&comm, &tok) == NOM_VARIABLE){
			if (strcmp(tok.attr.sattr,"BATCHINSERT")!=0){
				fprintf(stderr,"Pas la commande BATCHINSERT");
				return NULL;
			}
		}
		if(nextToken(&comm, &tok) != INTO){
			fprintf(stderr,"Commande mal tapee, il manque INTO, tok=%d", tok.type);
			return NULL;
		}
		if (nextToken(&comm, &tok) == NOM_VARIABLE){
			temp->relation = findRelation(tok.attr.sattr);
			if(temp->relation == NULL) {
			    fprintf(stderr, "E: [Insertion] Commande invalide, pas de nom de relation\n");
			    return NULL;
			}
		}
		if (nextToken(&comm, &tok) != FROM){
			fprintf(stderr,"Commande mal tapee, il manque FROM\n");
			return NULL;
		}
		if(nextToken(&comm, &tok) != NOM_VARIABLE || strcmp(tok.attr.sattr, "FILE") != 0) {
		    fprintf(stderr, "Commande mal tapee, il manque FILE: %d!=%d ou  %.100s != FILE\n", NOM_VARIABLE, tok.type, tok.attr.sattr);
		    return NULL;
		}
		if (nextToken(&comm, &tok) == NOM_VARIABLE){
			temp->fileName = strdup(tok.attr.sattr);
		}
	}
	return temp;
}

void ExecuteBatchInsert(BatchInsert *command){
	FILE* fich = fopen(command->fileName,"r");
	char *res=NULL;
	size_t taille;
	int count = 1;
	while( getline(&res, &taille, fich) > 0) {
	    struct command c = newCommand(res);
	    Record *r = parseTuple(command->relation, &c, 0);
	    if(r)
	        InsertRecordIntoRelation(command->relation, r);
        else
            printf("Erreur batchinsert %s into %s : je n'ai pas pu parser le tuple en ligne %d\n", command->fileName, command->relation->name, count);
	    count++;
	} 
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
                if(holacmoi.relation == NULL) { // Mauvais nom de relation
                    fprintf(stderr, "E: [Insersion] La relation %s n'existe pas.\n", tok.attr.sattr);
                    return holacmoi;
                }
            } else { // Pas de nom de relation
                fprintf(stderr, "E: [Insertion] Commande invalide, pas de nom de relation\n");
                holacmoi.relation = NULL; return holacmoi; // Pour indiquer qu'il y a eu une erreur
            }
        } else { // pas de  INTO
            fprintf(stderr, "E: [Insertion] Commande invalide, INSERT est suivi de INTO\n");
            holacmoi.relation = NULL; return holacmoi;
        }
        
        Record *rec = parseTuple(holacmoi.relation, &c, 1);
        if (rec == NULL) { // Erreur dans le parsing du tuple
            holacmoi.relation = NULL; return holacmoi; // pas besoin de print d'erreur ici, parseTuple() s'en occupe.
        }
        
        holacmoi.aAjouter = rec;
        if(nextToken(&c,&tok) != ENDOFCOMMAND) {
            fprintf(stderr, "E: [Insertion] Commande invalide, je m'attendais à une fin de commande.\n");
            holacmoi.relation = NULL; return holacmoi;
        }
        return holacmoi; 
    }
}

void Insertion(Insert insertion) {
    InsertRecordIntoRelation(insertion.relation, insertion.aAjouter);
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

/*************************************************************SELECTMONO*******************************************************/
SelectCommand *CreateSelectCommand(char *command) {
    SelectCommand *res;
    RelationInfo *rel;
    TabDeConditions tab;
    
    struct command comm = newCommand(command);
    struct token tok;
    
    // SELECTMONO * FROM
    if (nextToken(&comm, &tok) != ETOILE) {
        SYNTAX_ERROR("Erreur de syntaxe: Je m'attendais à un * après le SELECTMONO\n");
    } if (nextToken(&comm, &tok) != FROM) {
        SYNTAX_ERROR("Erreur de syntaxe: Je m'attendais à un FROM après SELECTMONO *\n");
    }
    
    // On lit la relation et on la cherche
    if(nextToken(&comm, &tok) != NOM_VARIABLE) {
        SYNTAX_ERROR("Erreur de syntaxe: Je m'attendais à un nom de relation.\n");
    }
    rel = findRelation(tok.attr.sattr);
    if (rel == NULL) {
        SYNTAX_ERROR("Erreur : Relation %s non trouvée. \n", tok.attr.sattr);
    }
    
    if(nextToken(&comm, &tok) == ENDOFCOMMAND) {
        res = malloc(sizeof(SelectCommand));
        res->rel = rel;
        initArray(res->conditions, 0);
    } else if (tok.type == WHERE) {
        return NULL; // TODO : gestion du WHERE en utilisant TabDeConditions parseConditions(command *com);
    } else {
        SYNTAX_ERROR("Erreur: Je m'attendais à un WHERE\n");
    }
    
}

void ExecuteSelectCommand(SelectCommand *command) {
    uint32_t s;
    if(command->conditions.nelems == 0) {
        Record *records = GetAllRecords(command->rel, &s);
        for( int i=0; i<s;i++) {
            printRecord(&records[i]);
        }
    } else { 
        // A FAIRE TODO
        // // Utilise : Record *GetAllRecords(RelationInfo *rel, uint32_t *size) de FileManager.c qui prend en entrée une relation et retourne un tableau 
        // Utilise : filtrerRecords défini plus haut dans ce fichier!
    }
}
/********************************************************************************************************************************/

//exemple;
// CREATE RELATION S5  (C1:string2,C2:int,C3:string4,C4:float,C5:string5,C6:int,C7:int) 
//INSERT INTO S5 (A, 2, AAA, 5.7, DF, 4,4) 
/*
DROPDB
CREATE RELATION S (C1:string2,C2:int,C3:string4,C4:float,C5:string5,C6:int,C7:int, C8:int)
INSERT INTO S (a, 2, a, 2.5, a, 3, 3, 3)
BATCHINSERT INTO S FROM FILE DB/S1.csv
*/

