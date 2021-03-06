
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

static int (*getOperateur(int token, int te))(union value, union value) {
    switch(token) {
	  	  case OPEQ:
	  		  return (te == T_INT ? ieq: (te==T_FLOAT ? feq : seq));
	  	  case OPSUP:
	  		  return (te == T_INT ? isup: (te==T_FLOAT ? fsup : ssup));
	  	  case OPINF:
	  	  	  return (te == T_INT ? iinf: (te==T_FLOAT ? finf : stinf));
	  	  case OPSUPEQ:
	  	  	  return (te == T_INT ? isupeq: (te==T_FLOAT ? fsupeq : ssupeq));
	  	  case OPINFEQ:
	  		  return (te == T_INT ? iinfeq: (te==T_FLOAT ? finfeq : sinfeq));
	  	  case OPNEQ:
	  		  return  (te == T_INT ? ineq:  (te==T_FLOAT ? fneq : sneq));
    }
    return NULL;
}
/********************************/


/* - Entrées: un token tok, et un type attedu parmi les types T_INT, T_FLOAT et T_STRING. 
        !!!!!! la fonction ne vérifie pas si expectedType respecte son domaine de valeurs. c'est la responsabilité de ceux qui l'utilisent
   - Sorties: true si tok est d'un type constante valide, et qui est compatible avec le type attendu 
   Si le type de la constante n'est pas égal au type attendu mais qu'il est compatible, une converision est faite
      par exemple si tok.type == INT_CONSTANT et que expectedType == STRING, tok.attr sera converti en string
*/
static int checkConstExpr(struct token *tok, struct command *comm, ColType expectedType) {
    
    switch(tok->type) {
    case INT_CONSTANT:
        if (expectedType.type == T_FLOAT) { // On doit convertir le int en float ici car setColumnTo(...) s'attendra à recevoir un float et non un int
            tok->attr.fattr = (float) tok->attr.iattr;
            tok->type = FLOAT_CONSTANT;
        } else if (expectedType.type != T_INT) { // expectedType == T_STRING
            if( comm->pos - comm->prevpos <= expectedType.stringSize )  {// Si l'écriture de ce int rentre dans le string
                sprintf(tok->attr.sattr, "%.*s", comm->pos-comm->prevpos, comm->command + comm->prevpos); // on copie l'écriture
                tok->type = STRING_CONSTANT;
            } else { // Sinon c'est une erreur
                fprintf(stderr, "Erreur: Je m'attendais à une constante string après %.*s, mais j'ai eu '%.*s'\n", comm->prevpos, comm->command, comm->pos-comm->prevpos, comm->command+comm->prevpos);
                return 0;
            }
        }
        return 1;
    case FLOAT_CONSTANT:
        if (expectedType.type == T_INT) {
            fprintf(stderr, "Erreur: Je m'attendais à une constante int après %.*s, mais j'ai eu %f\n", comm->prevpos, comm->command, tok->attr.fattr);
            return 0;
        } else if (expectedType.type != T_FLOAT) { // T_STRING 
            if( comm->pos - comm->prevpos <= expectedType.stringSize ) { // Si l'écriture de ce int rentre dans le string
                sprintf(tok->attr.sattr, "%.*s", comm->pos-comm->prevpos, comm->command + comm->prevpos); // on copie l'écriture
                tok->type = STRING_CONSTANT;
            } else { // Sinon c'est une erreur
                fprintf(stderr, "Erreur: Je m'attendais à une constante string après %.*s, mais j'ai eu '%.*s'\n", comm->prevpos, comm->command, comm->pos-comm->prevpos, comm->command+comm->prevpos);
                return 0;
            }
        }
        return 1;
    case STRING_CONSTANT:
        if (expectedType.type != T_STRING) {
            fprintf(stderr, "Erreur: Je m'attendais à une constante numérique après %.*s, mais j'ai eu '%.*s'\n", comm->prevpos, comm->command, comm->pos-comm->prevpos, comm->command+comm->prevpos);
            return 0;
        }
        return 1;
    default:
        fprintf(stderr, "Erreur: Je m'attendais à une constante après %.*s, mais j'ai eu '%.*s'\n", comm->prevpos, comm->command, comm->pos-comm->prevpos, comm->command+comm->prevpos);
        return 0;
    }
}


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


static AssignmentList parseAssignementList(struct command *comm, RelationInfo *rel) {
    AssignmentList liste; 
    initArray(liste, rel->nbCol);
    struct token tok;
    
    while(nextToken(comm, &tok) == NOM_VARIABLE) {
        Assignment ass;
        if ( (ass.colIndex = getColumnIndex(rel, tok.attr.sattr)) < 0 ) {
            fprintf(stderr, "Erreur dans SET: Pas de colonne %s dans la relation %s\n", tok.attr.sattr, rel->name); deleteArray(liste); return liste;
        }
        if( nextToken(comm, &tok) != OPEQ ) {
            fprintf(stderr, "Erreur dans SET: Je m'attendais à '=' après %.*s\n", comm->prevpos, comm->command); deleteArray(liste); return liste;
        }
        
        nextToken(comm, &tok);
        if (!checkConstExpr(&tok, comm, ass.colType=getFullTypeAtColumn(rel, ass.colIndex) ) ) {
            deleteArray(liste); return liste;
        }
        if (tok.type == INT_CONSTANT || tok.type == FLOAT_CONSTANT) 
            ass.val = *(union value *)&tok.attr;
        else
            {ass.val.s = strdup(tok.attr.sattr);}
        addElem(liste, ass);
        
        if(nextToken(comm, &tok) != VIRGULE) {
            break;
        }
    }
    pushTokenBack(comm);
    trim(liste);
    return liste;
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



//Prend en entrée une condition (colonne comparé à valeur), et l'évalue pour le record donné
int evaluerCondition(Condition *c, Record *record) {
	union value valRec;
	//valeur contenue à l'indice colonne dans le tuple
	void *valueRecord = getAtColumn(record, c->colonne);
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

//Prend en entrée une condition (colonne comparé à colonne), et l'évalue pour les deux records donnés
int evaluerCondition2(Condition2 *c, Record *record1, Record *record2) {
	union value valRec1;
	union value valRec2;
	//valeurs contenues à l'indice colonne dans le tuple
	void *valueRecord1 = getAtColumn(record1, c->colonne1);
	void *valueRecord2 = getAtColumn(record2, c->colonne2);
	//types des valeurs
	int type1 = getTypeAtRecordColumn(record1,c->colonne1);
	int type2 = getTypeAtRecordColumn(record2,c->colonne2);
	int* intValue;
	float* floatValue;
	char* stringValue;
	//valeur de colonne1
	if (type1 == T_INT){
		intValue = (int*)valueRecord1;
		valRec1.i = *intValue;
	}
	else if(type1 == T_FLOAT){
		floatValue = (float*)valueRecord1;
		valRec1.f = *floatValue;
	}
	else if(type1 == T_STRING){
		stringValue = (char*)valueRecord1;
		valRec1.s = stringValue;
	}
	//valeur de colonne2
	if (type2 == T_INT){
		intValue = (int*)valueRecord2;
		valRec2.i = *intValue;
	}
	else if(type2 == T_FLOAT){
		floatValue = (float*)valueRecord2;
		valRec2.f = *floatValue;
	}
	else if(type2 == T_STRING){
		stringValue = (char*)valueRecord2;
		valRec2.s = stringValue;
	}
	//on evalue la condition
	return c->operateur(valRec1, valRec2);
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
    nextToken(&c, &tok); {
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
        return res; // ça marchait quand même sans ce return en O0 :v
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
        deleteArray(records);
        deleteArray(resultat);
    }
}



/********************************************************************************************************************************/


/************************************************SELECTINDEX****************************************************/


CreateIndexCommand CreateCreateIndexCommand(char *command) {
    struct command com = newCommand(command);
    struct token tok;
    CreateIndexCommand ci;
    nextToken(&com, &tok); // ON
    nextToken(&com, &tok); // Nom relation
    ci.rel = findRelation(tok.attr.sattr);
    nextToken(&com, &tok); // KEY
    nextToken(&com, &tok); // = 
    nextToken(&com, &tok); // clé
    ci.column = getColumnIndex(ci.rel, tok.attr.sattr);
    nextToken(&com, &tok); // ORDER
    nextToken(&com, &tok); // = 
    nextToken(&com, &tok); // ordre
    ci.order = tok.attr.iattr;
    return ci;
}
void ExecuteCreateIndexCommand(CreateIndexCommand ci) {
    createIndex(ci.rel, ci.column, ci.order);
}

void ExecuteSelectIndexCommand(SelectCommand *command) {
    // Ici on sait que command->conditions.nelems est exactement de 1 et que command->conditions.tab[0].operateur est l'opérateur '='
    TabDeRecords records = getRecordsUsingIndex(command->rel, command->conditions.tab[0].colonne, command->conditions.tab[0].val.i);
    for (int i=0; i<records.nelems; i++) {
        printRecord(&records.tab[i]);
    }
    printf("Total records=%zu\n", records.nelems);
    deleteArray(records);
}

/*****************************************************************************************************************/


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




/*************************************************************JOIN*****************************************************************/


SelectJoinCommand *CreateSelectJoinCommand(char *command) {
    struct command com = newCommand(command); 
    struct token tok;
    SelectJoinCommand *result;
    char *nrel1, *nrel2;
    RelationInfo *rel1, *rel2;
    Condition2 c;
    
    if(nextToken(&com, &tok) != ETOILE)
        SYNTAX_ERROR(NULL, "Erreur dans SELECTJOIN: Je m'attendais à '*'\n");
    if(nextToken(&com, &tok) != FROM)
        SYNTAX_ERROR(NULL, "Erreur dans SELECTMONO: Je m'attendais à 'FROM'\n");
    
    if (nextToken(&com, &tok) != NOM_VARIABLE) {
        SYNTAX_ERROR(NULL, "Erreur dans SELECTJOIN: Je m'attendais à un nom de relation après '%.*s'\n", com.prevpos, command);
    } else if ( (rel1=findRelation(nrel1=strdup(tok.attr.sattr))) == NULL ) {
        fprintf(stderr, "Erreur dans SELECTJOIN: Relation %s non trouvée\n", nrel1); free(nrel1); return NULL;
    }
    
    if(nextToken(&com, &tok) != VIRGULE) {
        fprintf(stderr, "Erreur dans SELECTJOIN: Je m'attendais à une virgule après '%.*s'\n", com.prevpos, command); free(nrel1); return NULL;
    }
    
    if (nextToken(&com, &tok) != NOM_VARIABLE) {
        {free(nrel1); SYNTAX_ERROR(NULL, "Erreur dans SELECTJOIN: Je m'attendais à un nom de relation après '%.*s'\n", com.prevpos, command);}
    } else if ( (rel2=findRelation(nrel2=strdup(tok.attr.sattr))) == NULL ) {
        fprintf(stderr, "Erreur dans SELECTJOIN: Relation %s non trouvée\n", nrel2); free(nrel1); free(nrel2); return NULL;
    }
    
    if (nextToken(&com, &tok) != WHERE) {
        free(nrel1); free(nrel2); SYNTAX_ERROR(NULL, "Erreur dans SELECTJOIN: Je m'attendais à un nom de relation après '%.*s'\n", com.prevpos, command);
    }
    
    if (nextToken(&com, &tok) != NOM_VARIABLE) {
        free(nrel1); free(nrel2); SYNTAX_ERROR(NULL, "Erreur dans SELECTJOIN: Je m'attendais à un nom de relation après '%.*s'\n", com.prevpos, command);
    } else if (strcmp(tok.attr.sattr, nrel1)){
        fprintf(stderr, "Erreur dans SELECTJOIN: D'ou sort la relation %s ? \n", tok.attr.sattr);
    } else if (nextToken(&com, &tok) != POINT) {
        free(nrel1); free(nrel2); SYNTAX_ERROR(NULL, "Erreur dans SELECTJOIN: Je m'attendais à un point après '%.*s'\n", com.prevpos, command);
    } else if (nextToken(&com, &tok) != NOM_VARIABLE) {
        free(nrel1); free(nrel2); SYNTAX_ERROR(NULL, "Erreur dans SELECTJOIN: Je m'attendais à un nom de colonne après '%.*s'\n", com.prevpos, command);
    } else if ( (c.colonne1=getColumnIndex(rel1, tok.attr.sattr)) == -1 ) {
        fprintf(stderr, "Erreur dans SELECTJOIN: Aucune colonne de nom %s dans %s\n", tok.attr.sattr, rel1->name); free(nrel1); free(nrel2); return NULL;
    }
    
    if(nextToken(&com, &tok) < OPEQ && tok.type > OPNEQ) {
        fprintf(stderr, "Erreur dans SELECTJOIN: Je m'attendais à un opérateur de comparaison après '%.*s'\n", com.prevpos, command); free(nrel1); free(nrel2); return NULL;
    } else {
        c.operateur = getOperateur(tok.type, getTypeAtColumn(rel1, c.colonne1));
    }
    
    if (nextToken(&com, &tok) != NOM_VARIABLE) {
        free(nrel1); free(nrel2); SYNTAX_ERROR(NULL, "Erreur dans SELECTJOIN: Je m'attendais à un nom de relation après '%.*s'\n", com.prevpos, command);
    } else if (strcmp(tok.attr.sattr, nrel2)){
        fprintf(stderr, "Erreur dans SELECTJOIN: D'ou sort la relation %s ? \n", tok.attr.sattr);
    } else if (nextToken(&com, &tok) != POINT) {
        free(nrel1); free(nrel2); SYNTAX_ERROR(NULL, "Erreur dans SELECTJOIN: Je m'attendais à un point après '%.*s'\n", com.prevpos, command);
    } else if (nextToken(&com, &tok) != NOM_VARIABLE) {
        free(nrel1); free(nrel2); SYNTAX_ERROR(NULL, "Erreur dans SELECTJOIN: Je m'attendais à un nom de colonne après '%.*s'\n", com.prevpos, command);
    } else if ( (c.colonne2=getColumnIndex(rel2, tok.attr.sattr)) == -1 ) {
        fprintf(stderr, "Erreur dans SELECTJOIN: Aucune colonne de nom %s dans %s\n", tok.attr.sattr, rel2->name); free(nrel1); free(nrel2); return NULL;
    } else if (getTypeAtColumn(rel2, c.colonne2) != getTypeAtColumn(rel1, c.colonne1)) {
        fprintf(stderr, "Erreur dans SELECTJOIN: Les deux colonnes ne sont pas du même type!\n"); free(nrel1); free(nrel2); return NULL;
    }
    
    result = malloc(sizeof(SelectJoinCommand));
    result->C = c;
    result->R = rel1;
    result->S = rel2;
    free(nrel1); free(nrel2);
    return result;
}


//Page-oriented nested loop join
void join(RelationInfo *R,RelationInfo *S,Condition2 *c){
	PageIterator pageiterR = GetPageIterator(R); // On initialise l'itérateur de pages sur R
	PageIterator pageiterS; //On prépare un itérateur de pages sur S
	RecordsOnPageIterator recorditerR; // On prépare un itérateur de records sur une page
	RecordsOnPageIterator recorditerS; // On prépare un itérateur de records sur une page
	uint8_t *buffer_de_pageR = GetNextPage(&pageiterR); // GetNextPage nous retourne le buffer de la prochaine page disponible
	uint8_t *buffer_de_pageS;
	Record *recordR;
	Record *recordS;
	Record *res;
	size_t compteur = 0;
	//Pour chaque page de R
	while (buffer_de_pageR != NULL) {
		recorditerR = GetRecordsOnPageIterator(R, buffer_de_pageR); // On initialise l'itérateur des records
		Record *recordR = GetNextRecordOnPage(&recorditerR); // retourne le prochian record dispo
		while( recordR != NULL) {
			//Pour chaque tuple de R on le compare à tous les tuple de S
			//Pour chaque page de S
			pageiterS = GetPageIterator(S);
			buffer_de_pageS = GetNextPage(&pageiterS);
			while (buffer_de_pageS != NULL) {
						recorditerS = GetRecordsOnPageIterator(S, buffer_de_pageS); // On initialise l'itérateur des records
						recordS = GetNextRecordOnPage(&recorditerS); // retourne le prochain record dispo
						while( recordS != NULL) {
							//evaluer condition
							if(evaluerCondition2(c, recordR, recordS)){
								printTwoRecords(recordR,recordS);
								compteur++;
							}
							recordS = GetNextRecordOnPage(&recorditerS);
						}
						buffer_de_pageS = GetNextPage(&pageiterS);
					}
			recordR = GetNextRecordOnPage(&recorditerR);
		}
		buffer_de_pageR = GetNextPage(&pageiterR);
	}
	printf("Total records=%zu\n", compteur);
	return ;
}

/************************************************************************************************************************************/


/************************************************************UPDATE*****************************************************************/

UpdateCommand *CreateUpdateCommand(char *command) {
    UpdateCommand *res = NULL;
    struct token tok;
    struct command com = newCommand(command);
    
    SelectCommand selcom;
    
    if (nextToken(&com, &tok) != NOM_VARIABLE) {
        SYNTAX_ERROR(NULL, "Erreur dans UPDATE: Je m'attendais à un nom de relation après %.*s\n", com.prevpos, com.command);
    } else if ( (selcom.rel = findRelation(tok.attr.sattr)) == NULL ) {
        SYNTAX_ERROR(NULL, "Erreur dans le UPDATE: La relation %s n'existe pas\n", tok.attr.sattr);
    } else if (nextToken(&com, &tok) != SET) {
        SYNTAX_ERROR(NULL, "Erreur dans le UPDATE: Je m'attendais à set après %.*s\n", com.prevpos, com.command);
    }
    
    AssignmentList assliste = parseAssignementList(&com, selcom.rel);
    if (assliste.nelems == -1) {
        return NULL;
    }
    
    if(nextToken(&com, &tok) == WHERE) {
        selcom.conditions = parseConditions(selcom.rel, &com);
        if(selcom.conditions.tab == NULL) {
            deleteArray(assliste);
            return NULL;
        }
    } else if (tok.type == ENDOFCOMMAND) {
        initArray(selcom.conditions, 0);
    } else {
        SYNTAX_ERROR(NULL, "Erreur dans le UPDATE: Je m'attendais à WHERE après %.*s\n", com.prevpos, com.command);
    }
    
    res = calloc(1, sizeof(UpdateCommand));
    res->selection = selcom;
    res->nouvVals = assliste;
    return res;
}


void ExecuteUpdateCommand(UpdateCommand *command) {
    TabDeRecords allrecords = GetAllRecords(command->selection.rel);
    TabDeRecords bonsrecords = filtrerRecords(allrecords, command->selection.conditions);
    for (int i=0; i<bonsrecords.nelems; i++) {
        //printf("Old record: "); printRecord(&bonsrecords.tab[i]);
        for (int j=0; j<command->nouvVals.nelems; j++) {
            Assignment nouv = command->nouvVals.tab[j];
            setColumnTo(&bonsrecords.tab[i], nouv.colIndex, nouv.colType.type == T_STRING ? (void*)nouv.val.s : (void*)&nouv.val);
        }
        UpdateRecord(&bonsrecords.tab[i]);
        //printf("New record: "); printRecord(&bonsrecords.tab[i]);
    }
    printf("Total updated records:%zu\n", bonsrecords.nelems);
    deleteArray(allrecords); deleteArray(bonsrecords);
}

/************************************************************************************************************************************/

/*********************************************************DROP RELATION***********************************************************/
DropRelationCommand CreateDropRelationCommand(char *command) {
    DropRelationCommand rel;
    struct command com = newCommand(command);
    struct token tok;
    if (nextToken(&com, &tok) != NOM_VARIABLE) {
        SYNTAX_ERROR(-1, "Erreur dans la commande \"%s\", il faut juste un nom de relation\n", command);
    }
    rel = findRelationIndex(tok.attr.sattr);
    if(nextToken(&com, &tok) != ENDOFCOMMAND) {
        SYNTAX_ERROR(-1, "Erreur, je m'attendais à la fin de la commande après %.*s\n", com.prevpos, com.command);
    }
    return rel;
}
void ExecuteDropRelationCommand(DropRelationCommand command) {
    dropRelation(command);
}

/*********************************************************************************************************************************/


//exemple;
// CREATE RELATION S5  (C1:string2,C2:int,C3:string4,C4:float,C5:string5,C6:int,C7:int) 
//INSERT INTO S5 (A, 2, AAA, 5.7, DF, 4,4) 
/*
DROPDB
CREATE RELATION S1 (C1:string2,C2:int,C3:string4,C4:float,C5:string5,C6:int,C7:int,C8:int)
BATCHINSERT INTO S1 FROM FILE DB/S1.csv
CREATE RELATION S2 (C1:string2,C2:int,C3:string4,C4:float,C5:string5,C6:int,C7:int, TOTO:int)
BATCHINSERT INTO S2 FROM FILE DB/S1.csv
SELECTMONO * FROM S1
INSERT INTO S RECORD (a, 2, a, 2.5, a, 3, 3, 3)
SELECTMONO * FROM S1 WHERE C4=598.5 AND C7>9 
SELECTMONO * FROM S1 WHERE C4=598.5 AND C7>=9 
SELECTMONO * FROM S2 WHERE C4 = 598.5 AND C7 > 9 
DELETE FROM S1 WHERE C4 = 598.5 AND C7 > 9

CREATE RELATION R2 (C1:int,C2:string10,C3:float,C4:int,C5:int,C6:string3)
BATCHINSERT INTO R2 FROM FILE DB/R2.csv
SELECTMONO * FROM R2 WHERE C2<>egwekjqwek

CREATE RELATION  R1 (C1:int,C2:string4,C3:int)
BATCHINSERT INTO R1 FROM FILE DB/R1.csv

SELECTJOIN * FROM R1,R2 WHERE R1.C1=R2.C1
SELECTJOIN * FROM R1,R2 WHERE R1.C3=R2.C5

SELECTJOIN * FROM R1,S WHERE R1.C1=S.C2
SELECTJOIN * FROM R1,S WHERE R1.C2=S.C3
SELECTJOIN * FROM R1,S WHERE R1.C3=S.C6

CREATE RELATION T2 (C1:string3,C2:int,C3:int,C4:int,C5:string5)
BATCHINSERT INTO T2 FROM FILE T2.csv

CREATEINDEX ON TMP KEY=C2 ORDER=5
CREATEINDEX ON S1 KEY=C2 ORDER=5

*/

