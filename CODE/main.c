

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "DiskManager/DBParams.h"
#include "DiskManager/DiskManager.h"
#include "DBManager/DBManager.h"

extern DBParams params;

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Utilisation: %s CHEMIN_VERS_BD (il manque un argument)\n", argv[0]);
        return -1;
    }
    params.DBPath = argv[1]; //todo : vérifier que c'est un chemin valide
    params.pageSize = 4096;
    params.maxPagesPerFile = 4;
    params.frameCount = 2;

    printf("Bonjour :)\n");
    //debut de la boucle de lecture de commandes
    InitDBM();
    
    //utilisation de getline : allocation d'un buffer de char
    char *commande;
    //toutes les comparaisons doivent etre insensibles a la casse
    do{
        
        //taille max de la commande arbitrairement choisie
        size_t nb_char_max = 250;
        //la commande est size_t parce que getline
        size_t nb_char_commande;
        
        commande = (char *)malloc(nb_char_max*sizeof(char));
        if(commande==NULL){
            fprintf(stderr, "echec de l'allocation");
            return 1;
        }
        
        //getline 
        printf("MiniSGBD>>");
        nb_char_commande = getline(&commande,&nb_char_max,stdin);
        commande[strlen(commande)-1]='\0'; // On supprime l'avant dernier caractère qui est un retour à la ligne.
        ProcessCommand(commande);
        
    }while(strncmp(commande,"EXIT",4)!=0);
    
    FinishDBM();
    free(commande);
    return 0;
}
