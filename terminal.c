/**
 * @file terminal.c
 * @author Quentin LEBON et Yanis MASSAMBA NAKAVOUA
 * @brief permet de communiquer avec les centres d'archivage
 * @version 0.1
 * @date 2021-03-23
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "terminal.h"
#include "message.h"
#include "lectureEcriture.h"
#include "alea.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Methode principal du programme
 * 
 * @param argc 
 * @param argv en 1 le descripteur de fichier de reception, en 2 le descripteur de fichier d'envoie
 * @return int 
 */
int main(int argc, char **argv){// argv[1] descripteur de fichier de lecture des réponses et argv[2] c'est ou l'on ecrit les requêtes
    
    if(argc < 3){
        printf("Il manque un descripteur de fichier\n");
        exit(0);
    }
    
    printf("Ce terminal ecrira dans le descripteur : %s\nCe terminal lira dans le descripteur : %s\n", argv[2],argv[1]);

    int argv1 = atoi(argv[1]);
    int argv2 = atoi(argv[2]);

    char *num = malloc(TAILLEBUF+1);

    while(1){
        printf("Veuillez rentrer le numero du test a verifier : \n");

        fgets(num,17,stdin);
        printf("%ld",strlen(num));
        if(strlen(num) == 16)
            validerTest(num,argv2,argv1);
        else
            printf("Vous n'avez pas rentre un numero valide.\nAppuyez sur entree pour recommencer \n");

        //Permet de vider le buffer d'entree
        fgets(num,BUFSIZ,stdin); //Sale mais ca fonctionne
    }
}

/**
 * @brief Permet de lancer la procedure de verification du test
 * 
 * @param numeroTest 
 * @param fdEnvoye 
 * @param fdRecois 
 * @return int 
 */
int validerTest(char *numeroTest, int fdEnvoye, int fdRecois){
    //Generation de la duree de validite
    char valeur[255];

    aleainit();
    sprintf(valeur,"%d",alea(1,50000));

    int err = envoyerMessage(numeroTest, valeur, fdEnvoye);
    
    if(err == 0){
        printf("Erreur dans l'envoie des messages\n");
        return err;
    }

    char *msg = malloc(TAILLEBUF+1);

    err = recevoirMessage(fdRecois, &msg);

    if(err == 0){
        printf("Erreur dans l'envoie des messages\n");
        return err;
    }

    err = affichageResultat(msg);

    if(err == 0){
        printf("Erreur dans le decoupage du message\n");
        return err;
    }

    return 1;
}

/**
 * @brief Permet d'ecrire un message de demande validation pcr, ici nous permet d'envoyer un message vers d'autre composant
 * 
 * @param numeroTest Le numero du test PCR
 * @param typeMessage 
 * @param fd Le descripteur ou l'on veut ecrire notre message
 * @return int status d'erreur de la fonction
 */
int envoyerMessage(char* numeroTest, char* dureeValidite, int fd){
    char *msg = message(numeroTest,"Demande",dureeValidite); //On cree le char* de notre message pour l'envoyer ensuite
    int err = ecritLigne(fd, msg);//On l'envoie au descripteur de fichier spécifier lors de demarrage du programme

    if(err == 0){
        return err;
    }

    free(msg);//On libere la memoire
    return err;
}

/**
 * @brief Permet de lire une ligne, cad ici de recuperer l'information envoyé à notre composant
 * 
 * @param fd Le descripteur ou l'on veut lire notre message
 * @param msg l'adresse ou l'on enregistrera notre message
 * @return int status d'erreur de la fonction
 */
int recevoirMessage(int fd, char** msg){
    *msg = litLigne(fd);

    if(*msg == NULL){
        return 0;
    }

    return 1;
}

/**
 * @brief Permet d'afficher le resultat sur la sortie standart
 * 
 * @param msg 
 * @return int 
 */
int affichageResultat(char* msg){
    char nmTest[255], typeMsg[255], valeur[255]; //ici 255 est une valeur arbitraire
    int err = decoupe(msg,nmTest,typeMsg,valeur);

    if(err == 0){
        return err;
    }

    if(atoi(valeur) == 0){
        printf("Le test %s est valide.\n",nmTest);
    }
    else{
        printf("Le test %s n'est pas valide.\n",nmTest);
    }

    enregistrerResultat(msg);

    return err;
}

/**
 * @brief Permet de faire les logs de la session du terminal
 * 
 * @param msg 
 */
void enregistrerResultat(char* msg){

    FILE* fichier = NULL;
    char nomFichier[255];
    sprintf(nomFichier,"log_terminal_%d.txt", getpid());
    fichier = fopen(nomFichier, "a");

    if (fichier != NULL)
    {
        fprintf(fichier, msg);
        fclose(fichier);
    }
    else
    {
        printf("Impossible d'ouvrir le fichier");

    }
}