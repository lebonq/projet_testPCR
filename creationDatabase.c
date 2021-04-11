/**
 * @file creationDatabase.c
 * @author Quentin LEBON
 * @brief Permet de creer une fausse base de données pour notre centre d'archivage
 * @version 0.1
 * @date 2021-03-23
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "lectureEcriture.h"
#include "alea.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int main(int argc, char **argv){
    
    if(argc < 3){
        printf("Le programme s'utilise avec 2 arguments, ./creationDatabase nbTest idCentre\n");
        exit(0);
    }

    char fileName[255];

    sprintf(fileName, "resultats_centre_%s.txt", argv[2]);

    FILE* fd = fopen(fileName,"w");//On cree le fichier

    long unsigned int nbTest = 100000000000; //Permet de genere un code aleatoire

    struct timeval tv;
    gettimeofday(&tv,NULL);//On recupere la date du jour en seconde ecoulee depuis 1970

    for(long unsigned int i = 0; i <= atoi(argv[1]); i++){
        aleainit();
        nbTest += i;//*alea(1,150);

        long int timeTest = tv.tv_sec - alea(14400,172800); //Ici on retire entre 4 et 48 heures a l'heures actuelle pour simuler un prelevement qui a ete fait entre 4 et 96 avant le vol

        int resultatTest = alea(0,100);

        if(resultatTest < 93){ //93% de negatif
            resultatTest = 0; //negatif
        }
        else{ //7% de positif
            resultatTest = 1;
        }

        fprintf(fd, "%s%lu %ld %d\n",argv[2], nbTest, timeTest, resultatTest);
    }

    fclose(fd);
}