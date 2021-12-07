
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "BufferManager/BufferManager.h"
#include "DiskManager/PageId.h"
#include "DiskManager/DiskManager.h"
#include "FileManager/RelationInfo.h"
#include "FileManager/FileManager.h"
#include "FileManager/Catalog.h"
#include "CommandTokenizer.h"
#include "Command.h"



/**********************************************************GÉNÉRAL*********************************************************/

// Pour afficher un message d'erreur puis quitter la fonction lorsqu'il y a un erreur dans la commande à parser
#define SYNTAX_ERROR(ret, ...) {\
    fprintf(stderr, __VA_ARGS__);\
    return ret;\
}

// Lorsque l'utiliateur donne une valeur du mauvais type
#define CONST_TYPE_ERROR(ret, rel, ncol, good, bad) {\
    SYNTAX_ERROR(ret, "Erreur: La colonne %d de %s est de type %s, mais j'ai eu un %s.\n", (ncol), (rel)->name, typeToString(good), typeToString(bad)); \
}

// Pour print le type d'une colonne donnée.
#define typeToString(type) ( (type) == T_INT ? "integer" : (type) == T_FLOAT ?  "float" : (type) == T_STRING ? "stringX" : "INCONNU" ) 


/*****FONCTIONS OPERATEURS*******/
static int ieq(union value v1, union value v2) { return v1.i == v2.i; }
static int ineq(union value v1, union value v2) { return v1.i != v2.i; }
static int isup(union value v1, union value v2) { return v1.i > v2.i; }
static int isupeq(union value v1, union value v2) { return v1.i >= v2.i; }
static int iinf(union value v1, union value v2) { return v1.i < v2.i; }
static int iinfeq(union value v1, union value v2) { return v1.i <= v2.i; }
static int feq(union value v1, union value v2) { return v1.f == v2.f; }
static int fneq(union value v1, union value v2) { return v1.f != v2.f; }
static int fsup(union value v1, union value v2) { return v1.f > v2.f; }
static int fsupeq(union value v1, union value v2) { return v1.f >= v2.f; }
static int finf(union value v1, union value v2) { return v1.f < v2.f; }
static int finfeq(union value v1, union value v2) { return v1.f <= v2.f; }
static int seq(union value v1, union value v2) { return strcmp(v1.s, v2.s) == 0; }
static int sneq(union value v1, union value v2) { return strcmp(v1.s, v2.s) != 0; }
static int ssup(union value v1, union value v2) { return strcmp(v1.s, v2.s) > 0; }
static int ssupeq(union value v1, union value v2) { return strcmp(v1.s, v2.s) >= 0; }
static int stinf(union value v1, union value v2) { return strcmp(v1.s, v2.s) < 0; }
static int sinfeq(union value v1, union value v2) { return strcmp(v1.s, v2.s) <= 0; }
/********************************/


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
        RecordFinish(rec); SYNTAX_ERROR(NULL,"Erreur de syntaxe: Je m'attendais à un parenthèse ouvrante\n");
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
                CONST_TYPE_ERROR(NULL,rel, i, type, T_INT);
            } break;
        case FLOAT_CONSTANT:
            if((type=getTypeAtColumn(rel, i)) != T_FLOAT) {
                RecordFinish(rec);
                CONST_TYPE_ERROR(NULL,rel, i, type, T_FLOAT);
            } break;
        case STRING_CONSTANT:
            if((type=getTypeAtColumn(rel,i)) != T_STRING) {
                RecordFinish(rec); CONST_TYPE_ERROR(NULL,rel, i, type, T_STRING);
            } else if (strlen(tok.attr.sattr) > rel->colTypes[i].stringSize) {
                RecordFinish(rec);
                SYNTAX_ERROR(NULL,"Erreur sur la colonne %d, %s fait %zu caractères, mais %s(%d) est de type string%d\n", i, tok.attr.sattr, strlen(tok.attr.sattr), rel->colNames[i], i, rel->colTypes[i].stringSize);
            } break;
         default:
            RecordFinish(rec); SYNTAX_ERROR(NULL,"Erreur: Je m'attendais à une constante pour la colonne %d de %s, tok.type == %d\n", i, rel->name, tok.type);
        }
        
        setColumnTo(rec, i, &tok.attr);
        
        nextToken(comm, &tok);
        if (i == rel->nbCol-1) {
            if( parens && tok.type != PAREN_FERM ) {
                RecordFinish(rec); SYNTAX_ERROR(NULL,"Erreur: %i colonnes lues, parenthèse fermante manquante.\n", i+1);
            }
        } else if (tok.type != VIRGULE) {
            RecordFinish(rec); SYNTAX_ERROR(NULL,"Erreur: Je m'attendais à une virgule après la colonne %d de %s. (n°token = %d)\n", i, rel->name, tok.type);
        }
    }
    return rec;
}

static int parseSelectionConditions(struct command *com, SelectCommand *res) {
    
}

// Prend en entrée une commande qui commence juste après le where
// Donne en sortie un tableau avec toutes les conditions
TabDeConditions parseConditions(RelationInfo *rel,struct command *command) {
   TabDeConditions tab;
   initArray(tab,20);
   //addElem(tab,

   struct token tok;


   while (nextToken(command,&tok)!=ENDOFCOMMAND){
	  Condition cnd;
	  if (tok.type==NOM_VARIABLE){ //
		  cnd.colonne=getColumnIndex(rel, tok.attr.sattr);
	  }
	  int te = getTypeAtColumn(rel, cnd.colonne);
	  nextToken(command,&tok);

	  if ((tok.type>=OPEQ) && (tok.type<=OPNEQ)){

		  switch(tok.type){

		  	  case OPEQ:
		  		  cnd.operateur= (te == T_INT ? ieq: (te==T_FLOAT ? feq : seq));
		  		  break;
		  	  case OPSUP:
		  		  cnd.operateur= (te == T_INT ? isup: (te==T_FLOAT ? fsup : ssup));
		  		  break;
		  	  case OPINF:
		  	  	  cnd.operateur= (te == T_INT ? iinf: (te==T_FLOAT ? finf : stinf));
		  	  	  break;
		  	  case OPSUPEQ:
		  	  	  cnd.operateur= (te == T_INT ? isupeq: (te==T_FLOAT ? fsupeq : ssupeq));
		  	  	  break;
		  	  case OPINFEQ:
		  		  cnd.operateur= (te == T_INT ? iinfeq: (te==T_FLOAT ? finfeq : sinfeq));
		  		  break;
		  	  case OPNEQ:
		  		  cnd.operateur= (te == T_INT ? ineq:  (te==T_FLOAT ? fneq : sneq));
		  		  break;
		  }


	  }else{
		  deleteArray(tab);
		  tab.tab = NULL;
		  return tab;
	  }

	  nextToken(command,&tok);
	  if (tok.type==INT_CONSTANT && te == T_INT){
		  cnd.val.i = tok.attr.iattr;
		  //cnd.val=tok.attr;
	  } else if (tok.type == INT_CONSTANT && te == T_FLOAT) {
	    cnd.val.f = (float) tok.attr.iattr;
	  } else if (tok.type == FLOAT_CONSTANT && te == T_FLOAT) {
	    cnd.val.f = tok.attr.fattr;
	  } else if (tok.type == STRING_CONSTANT && te == T_STRING) {
	    cnd.val.s = strdup(tok.attr.sattr); // LEAK TODO
	    //printf("J'ai dupliqué %s, et seq = %p et cnd.operateur = %p\n", cnd.val.s, seq, cnd.operateur);
	  } else {
	    //Erreur 
	    ; // TODO
	  }
      addElem(tab, cnd);
      
      if(nextToken(command, &tok) == AND) {
        ;
      } else if (tok.type == ENDOFCOMMAND) break;
      else {
        // Erreur TODO
      }
   }
    return tab;
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
		char* stringValue = (char*)valueRecord;
		valRec.s = stringValue;
	}
	//on evalue la condition
	return c->operateur(valRec, c->val);


}

// Retourne le résultat de l'évaluation d'un ensemble de conditions avec AND entre elles. Utilise evaluerCondition
int evaluerAndConditions(TabDeConditions conditions, Record *record) {
    for(int i=0; i<conditions.nelems;i++)
        if(!evaluerCondition(&conditions.tab[i], record))
            return 0;
    return 1;
}

// Prend en entrée un tableau de reltions, et un tableau de conditions, et retourne un tableau de record respectant toutes les conditions
// Utilise: evaluerAndConditions() 
TabDeRecords filtrerRecords(TabDeRecords tous, TabDeConditions conditions) {
    //initialiser le tableau de records respectant les conditions
    TabDeRecords res;
    initArray(res,10);
    //parcourir les record dans tous : 
    for(int i = 0; i<tous.nelems; i++){
        if(evaluerAndConditions(conditions,&tous.tab[i])){
            addElem(res,tous.tab[i]);
        }
    }
    trim(res);
    return res;
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
		SYNTAX_ERROR(NULL,"Erreur dans la commande: Je m'attendais à un nom de relaton.\n");
	}

	int tracker=0;

	if(okToken(&comm,&tok) && tok.type==PAREN_OUVR){
		for( nextToken(&comm, &tok);;nextToken(&comm, &tok)){
		    if(tracker == sizeTabs) {
		        temp->colNames = realloc(temp->colNames, (sizeTabs+5)*sizeof(char*));
		        temp->colTypes = realloc(temp->colTypes, (sizeTabs+5)*sizeof(ColType));
		        sizeTabs+=5;
		    }
		    
			if (tok.type==NOM_VARIABLE){
				temp->colNames[tracker]=strdup(tok.attr.sattr);
			} else { SYNTAX_ERROR(NULL,"Erreur de syntaxe: Je m'attendais à un nom de colonne.\n"); }

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
					SYNTAX_ERROR(NULL,"Erreur: Je m'attendais à un type\n");
				}
				temp->colTypes[tracker]=t;
				temp->colNum=tracker;

			} else { SYNTAX_ERROR(NULL,"Erreur de syntaxe: Je m'attendais à deux points.\n"); }
            nextToken(&comm,&tok);
            if (tok.type == PAREN_FERM) {
                tracker++; break;
            } else if (tok.type!=VIRGULE){
				SYNTAX_ERROR(NULL,"Erreur de syntaxe: Je m'attendais à une virgule ou à une parenthèse fermante\n");
			}
			tracker++;
		}
	} else {
	SYNTAX_ERROR(NULL,"Erreur dans la commande: Je m'attendais à une parenthèse ouvrante, j'ai eu %d\n", tok.type);
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
	if(fich==NULL) {
	    fprintf(stderr, "Erreur dans BATCHINSERT: Le fichier %s n'a pas pu être ouvert\n", command->fileName);
	    return;
	}
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
        
        if(nextToken(&c, &tok) != RECORD) {
            fprintf(stderr, "E: [Insersion] Commande invalide, Utilisation: INSERT INTO nomRelation RECORD tuple\n");
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
/******************************************Utilisée pour les commandes SELECTMONO, UPDATE et DELETE***************************/
SelectCommand *CreateSelectCommand(char *command) {
    SelectCommand *res;
    RelationInfo *rel;
    TabDeConditions tab;
    
    struct command comm = newCommand(command);
    struct token tok;
    
    // SELECTMONO * FROM nomRelation OU DELETE FROM nomRelation OU UPDATE nomRelation
    // Si on lit un *, on avance mais si on lit autre chose on revient d'un pas et on laisse le reste lire. Même chose pour si on lit un FROM
    if (nextToken(&comm, &tok) != ETOILE) {
        pushTokenBack(&comm);
    } if (nextToken(&comm, &tok) != FROM) {
        pushTokenBack(&comm);
    }
    
    // On lit la relation et on la cherche
    if(nextToken(&comm, &tok) != NOM_VARIABLE) {
        SYNTAX_ERROR(NULL,"Erreur de syntaxe: Je m'attendais à un nom de relation.\n");
    }
    rel = findRelation(tok.attr.sattr);
    if (rel == NULL) {
        SYNTAX_ERROR(NULL,"Erreur : Relation %s non trouvée. \n", tok.attr.sattr);
    }
    
    if(nextToken(&comm, &tok) == ENDOFCOMMAND) {
        res = malloc(sizeof(SelectCommand));
        res->rel = rel;
        initArray(res->conditions, 0);
    } else if (tok.type == WHERE) {
        res = malloc(sizeof(SelectCommand));
        res->rel = rel;
        res->conditions = parseConditions(rel, &comm);
        if (res->conditions.tab == NULL)
            return NULL;
        return res;
    } else {
        SYNTAX_ERROR(NULL,"Erreur: Je m'attendais à un WHERE\n");
    }
    
}

void ExecuteSelectCommand(SelectCommand *command) {
    if(command->conditions.nelems == 0) {
        /*TabDeRecords records = GetAllRecords(command->rel);
        for( int i=0; i<records.nelems;i++) {
            printRecord(&records.tab[i]);
        }*/
        ListRecordsIterator *iter = GetListRecordsIterator(command->rel);
        Record *r;
        int i=0;
        for (r = GetNextRecord(iter); r; r = GetNextRecord(iter), i++)
            printRecord(r);
        printf("Total records=%d\n", i);
    } else {
        TabDeRecords records = GetAllRecords(command->rel);
        TabDeRecords resultat = filtrerRecords(records, command->conditions);
        for (int i=0; i<resultat.nelems;i++)
            printRecord(&resultat.tab[i]);
        printf("Total records=%zu\n", resultat.nelems);
    }
}



/********************************************************************************************************************************/


/***********************************************************DELETE*****************************************************************/

// CreateDeleteCommand est juste CreateSelectCommand. Pas besoin de recoder!

void ExecuteDeleteCommand(DeleteCommand *command) {
    TabDeRecords records = GetAllRecords(command->rel);
    TabDeRecords resultat = filtrerRecords(records, command->conditions);
    for (int i=0; i<resultat.nelems; i++) {
        DeleteRecordFromRelation(resultat.tab[i].relInfo, resultat.tab[i].rid);
    }
    printf("Total deleted records=%zu\n", resultat.nelems);
    deleteArray(records);
    deleteArray(resultat);
}

/**********************************************************************************************************************************/


/*************************************************************UPDATE*****************************************************************/


// Fonctions utiles : FileManager.c:UpdateRecord(Record *rec)
UpdateCommand *CreateUpdateCommand(char *command) {

}

void ExecuteUpdateCommand(UpdateCommand *command);

/************************************************************************************************************************************/

//exemple;
// CREATE RELATION S5  (C1:string2,C2:int,C3:string4,C4:float,C5:string5,C6:int,C7:int) 
//INSERT INTO S5 (A, 2, AAA, 5.7, DF, 4,4) 
/*
DROPDB
CREATE RELATION S (C1:string2,C2:int,C3:string4,C4:float,C5:string5,C6:int,C7:int, C8:int)
BATCHINSERT INTO S FROM FILE DB/S1.csv
CREATE RELATION S2 (C1:string2,C2:int,C3:string4,C4:float,C5:string5,C6:int,C7:int, TOTO:int)
BATCHINSERT INTO S2 FROM FILE DB/S1.csv
SELECTMONO * FROM S
INSERT INTO S RECORD (a, 2, a, 2.5, a, 3, 3, 3)
SELECTMONO * FROM S WHERE C4 = 598.5 AND C7 > 9 
SELECTMONO * FROM S2 WHERE C4 = 598.5 AND C7 > 9 
DELETE FROM S WHERE C4 = 598.5 AND C7 > 9

CREATE RELATION R2(C1:int, C2:string10, C3:float, C4:int, C5:int, C6:string3)
BATCHINSERT INTO R2 FROM FILE DB/R2.csv
SELECTMONO * FROM R2 WHERE C2 <> egwekjqwek

*/

