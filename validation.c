/**
 * @file validation.c
 * @author Quentin LEBON
 * @brief 
 * @version 0.1
 * @date 2021-03-23
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "message.h"
#include "lectureEcriture.h"
#include "validation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

int main(int argc, char **argv){
    if(argc < 4){
        printf("./Validation fdLecture fdEcriture nomFichier\n");
        exit(0);
    }

    printf("Ce process validation ecrira dans le descripteur : %s\nCe process validation lira dans le descripteur : %s\n", argv[2],argv[1]);

    int argv1 = atoi(argv[1]); //Lecture
    int argv2 = atoi(argv[2]); //Ecriture
    char* nomFichier = argv[3]; //Nom du ficher avec les resulatst

    char *demande = malloc(TAILLEBUF+1);

    while(1){
        char nmTest[255], typeMsg[255], valeur[255]; //ici 255 est une valeur arbitraire

        recevoirDemande(&demande,argv1);

        int err = decoupe(demande,nmTest,typeMsg,valeur);
        if(err == 0){
            printf("Erreur decoupe\n");
            return err;
        }

        char resValidation[255];

        int res = validerTest(nmTest,valeur,nomFichier);

        sprintf(resValidation,"%i", res); //On converti en char
        envoyerReponse(nmTest,resValidation,argv2);

    }

    return 1;
}

/**
 * @brief Permet de dire si un test est valide ou non.
 * 
 * @param numTest 
 * @param tempsValiditeTest c'est le temps en seconde pendant lequel le test est valide
 * @param numCentre 
 * @return int 
 */
int validerTest(char *numTest, char* tempsValiditeTest, char* nomFichier){

    FILE* fichier = fopen(nomFichier, "r");

    char* numerosTestFile = malloc(TAILLEBUF+1);
    char* datePrelevementFile = malloc(TAILLEBUF+1);
    char* resultatFile = malloc(TAILLEBUF+1);

    printf("Validation");

    while(fscanf(fichier,"%s %s %s",numerosTestFile,datePrelevementFile,resultatFile) != EOF){ // On recupere la ligne
        if(strcmp(numerosTestFile,numTest) == 0){ // on tests si kes 2 numeros sont les memes
            struct timeval tv;
            gettimeofday(&tv,NULL);//On recupere la date du jour en seconde ecoulee depuis 1970

            if(atoi(datePrelevementFile) + atoi(tempsValiditeTest) <= tv.tv_sec){
                fclose(fichier);
                return atoi(resultatFile);
            }
            else{
                fclose(fichier);
                return 1;
            }
        }
    }

    fclose(fichier);
    return 1;
}

/**
 * @brief Peremt d'envoyer la reponse apres la validation
 * 
 * @param numeroTest 
 * @param resulat 0 si c'est validé sinon 1
 * @param fd descripteur ou on ecrira la reponse
 * @return int 
 */
int envoyerReponse(char* numeroTest, char* resulat, int fd){
    char *msg = message(numeroTest,"Reponse",resulat);
    int err = ecritLigne(fd, msg);//On l'envoie au descripteur de fichier spécifier lors de demarrage du programme

    if(err == 0){
        return err;
    }

    free(msg);//On libere la memoire
    return err;
}

/**
 * @brief Permet de recuperer la demande et la stocker 
 * 
 * @param demande L'espace memoire pour stocker a demande
 * @param fd le descripteur de fichier pour lire la demande
 * @return int 
 */
int recevoirDemande(char** demande,int fd){
    *demande = litLigne(fd);
    if(*demande == NULL){
        return 0;
    }

    return 1;
}