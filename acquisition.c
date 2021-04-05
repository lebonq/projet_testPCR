/**
 * @file acquisition.c
 * @author Quentin LEBON
 * @brief 
 * @version 0.1
 * @date 2021-04-03
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "acquisition.h"
#include "lectureEcriture.h"
#include "message.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>


char** bufferDemande;//Permet de stocker les demandes sous forme de string
sem_t semBufferDemande;
int* bufferDescripteur;//Permet de stocker le descripteur de fichier en correspondance avec la demande
sem_t semBufferDescripteur;
int* state;//Permet de savoir le statut de la demande associe 
sem_t semState;
/*
 0 la case est libre
 1 la case est occupe 
*/

char* idCentre;//Pas besoin de semaphore car elle est en read-only
int nbMaxBufferDemande;//Pas besoin de semaphore car elle est en read-only

int main(int argc, char** argv){

    if(argc < 3){
        printf("Le programme s'utilise avec 2 arguments, ./creationDatabase tailleBuffer idCentre\n");
        exit(0);
    }
    nbMaxBufferDemande = atoi(argv[1]);
    idCentre = argv[2];//On recupere l'id du centre sous forme de char*

    if(strlen(idCentre) ==3){//Si il est different de 4 chars ce n;est pas bon !
        printf("Id centre invalide\n");
        exit(0);
    }

    bufferDemande = (char**)malloc(sizeof(char*)*nbMaxBufferDemande);
    bufferDescripteur = malloc(sizeof(int)*nbMaxBufferDemande);
    state = malloc(sizeof(int)*nbMaxBufferDemande);

    for (int i = 0; i < nbMaxBufferDemande; i++){//On met un malloc dans chaque cases de notre buffer pour stocker nos code
        bufferDemande[i] = (char*)malloc(sizeof(char)*16);//16 chars suffisent car le test fait 16 caraceteres
    }

    for (int i = 0; i < nbMaxBufferDemande; i++){//On initialise state a 0
        state[i] = 0;
    }

    int fdLecteur = open("txt_test_acquisition/terminal1_demande.txt",O_RDONLY);//Descripteur de fichier pour lire les demandes
    int fdEcrivain = open("txt_test_acquisition/terminal1_reponse.txt",O_WRONLY);//Descripteur de fichier pour ecrire les reponses
    int fdValider = open("txt_test_acquisition/validation1_demande.txt",O_WRONLY);//Le tube de validation
    int fdInter = open("txt_test_acquisition/inter1_demande.txt",O_WRONLY);//Le tube du serveur inter

    int descripteursTermimal[4] = {fdLecteur,fdEcrivain,fdValider,fdInter};

    int fdValiderListerner= open("txt_test_acquisition/validation1_reponse.txt",O_RDONLY);//Descripteur de fichier pour lire les "reponses"
    int fdInterListener = open("txt_test_acquisition/inter1_reponse.txt",O_RDONLY);//Descripteur de fichier pour lire les "reponses"

    sem_init(&semBufferDemande,0,1);
    sem_init(&semBufferDescripteur,0,1);
    sem_init(&semState,0,1);

    pthread_t threadTerminal;
    pthread_t threadValider;
    pthread_t threadInter;

    pthread_create(&threadTerminal,NULL,lireRequeteTerminal,descripteursTermimal);
    pthread_create(&threadValider,NULL,threadReceptionReponse,&fdValiderListerner);
    pthread_create(&threadInter,NULL,threadReceptionReponse,&fdInterListener);

    printf("Terminal\n");
    //lireRequeteTerminal(descripteursTermimal);
    printf("Valider\n");
    //threadReceptionReponse(&fdValiderListerner);
    printf("Inter Serveur\n");
    //threadReceptionReponse(&fdInterListener);

    pthread_join(threadTerminal,NULL);
    pthread_join(threadValider,NULL);
    pthread_join(threadInter,NULL);

    return 0;
}

void *lireRequeteTerminal(void* fdTermimal){
    int fdLecteur = ((int*)fdTermimal)[0];
    int fdEcrivain = ((int*)fdTermimal)[1];
    int fdValider = ((int*)fdTermimal)[2];
    int fdInter = ((int*)fdTermimal)[3];//

    char* c =  litLigne(fdLecteur);
    int i = 0;
    
    char nTest[255],type[255],valeur[255];

    while(c != NULL && i < nbMaxBufferDemande){
        sem_wait(&semState);
        if(state[i] == 0){
            state[i] = 1;//On met la case a 1 car elle va etre utilise
            decoupe(c,nTest,type,valeur);

            strcpy(bufferDemande[i],nTest);//on met le test dans le tableau on utilise STRCPY pour ne pas ecraser le pointeur
            bufferDescripteur[i] = fdEcrivain;//On met le fd correspondant

            char demandeId[255];
            decoupe_str(nTest,demandeId,0,3);//On recupere l'id du centre correspondant au test recu

            if(!strcmp(demandeId,idCentre) ){//Si c'est le meme ID on envoye a validation
                ecritLigne(fdValider,c);
            }else{//Sinon inter-archive
                ecritLigne(fdInter,c);
            }

            c =  litLigne(fdLecteur);
        }
        sem_post(&semState);
        i++;
    }

    free(c);
    return 0;
}

void *threadReceptionReponse(void* fdReponse){
    int fdReponseInt = *((int *)fdReponse);

    char* bufferReader =  malloc(TAILLEBUF-1);
    char nTest[255],type[255],valeur[255];

    read(fdReponseInt,bufferReader,TAILLEBUF+1);//On lit la premiere ligne pour lancer la boucle
    while( bufferReader != NULL){
        decoupe(bufferReader,nTest,type,valeur);
        for (int i = 0; i < nbMaxBufferDemande; i++){//On check toute les demandes
            sem_wait(&semState);
            if(state[i] == 1 && !strcmp(bufferDemande[i],nTest)){//Si la case est pleine on verifi et si le test corespond
                state[i] = 0;//On libere la case i
                write(bufferDescripteur[i],bufferReader,TAILLEBUF+1);
            }
            sem_post(&semState);
        }
        read(fdReponseInt,bufferReader,TAILLEBUF+1);//On utilise read pour etre pipe friendly
    }
    return 0;
}

/**
 * @brief Permet de recuperer une partie d'un char*
 * 
 * @param input 
 * @param output 
 * @param start 
 * @param end inclusive
 */
void decoupe_str(char* input, char * output, int start, int end){
    int j = 0;
    for ( int i = start; i <= end; ++i ) {
        output[j++] = input[i];
    }
    output[j] = 0;
}