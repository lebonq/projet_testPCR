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
int* bufferDescripteur;//Permet de stocker le descripteur de fichier en correspondance avec la demande
int* state;//Permet de savoir le statut de la demande associe 
sem_t semState;//Permet de ne pas rentrer en section critique
sem_t nbCaseLibre;//Permet de savoir si il y a de la place dans le buffer
/*
 0 la case est libre
 1 la case est occupe 
*/

char* idCentre;//Pas besoin de semaphore car elle est en read-only
int nbMaxBufferDemande;//Pas besoin de semaphore car elle est en read-only

int main(int argc, char** argv){

    if(argc < 6){
        printf("Le programme s'utilise avec 2 arguments, ./Acquisiton tailleBuffer nomCentre idCentre nomFichierTestPcr nbTerminal\n");
        exit(0);
    }

    nbMaxBufferDemande = atoi(argv[1]);
    char* nomCentre = argv[2];
    idCentre = argv[3];//On recupere l'id du centre sous forme de char*
    char* nomFichierPcr = argv[4];
    int nbTerminal = atoi(argv[5]);

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

    int pipeTerminalAcquisiton[2];
    int pipeAcquisitionTerminal[2];
    int pipeValidationAcquisition[2];
    int pipeAcquisitonValidation[2];
 
    pipe(pipeTerminalAcquisiton);
    pipe(pipeAcquisitionTerminal);
    pipe(pipeValidationAcquisition);
    pipe(pipeAcquisitonValidation);

    char fd1[16];
    char fd2[16];

    //pid_t terminal = fork();
    //if(terminal == 0){
    //   execlp("/usr/bin/xterm", "xterm", "-e", "./Terminal", pipeAcquisitionTerminal[0], pipeTerminalAcquisiton[1], NULL);
    //}

    pid_t validation = fork();
    if(validation == 0){
        close(pipeAcquisitonValidation[1]);
        sprintf(fd1,"%d",pipeAcquisitonValidation[0]);
        sprintf(fd2,"%d",pipeValidationAcquisition[1]);
        execlp("./Validation", "Validation", fd1, fd2, nomFichierPcr, NULL);
        printf("Jamais Print\n");
        exit(0);
    }

    int fdLecteur = open("txt_test_acquisition/terminal1_demande.txt",O_RDONLY);//Descripteur de fichier pour lire les demandes
    int fdEcrivain = open("txt_test_acquisition/terminal1_reponse.txt",O_WRONLY);//Descripteur de fichier pour ecrire les reponses
    int fdValider = pipeAcquisitonValidation[1];//Le tube de validation
    int fdInter = open("txt_test_acquisition/inter1_demande.txt",O_WRONLY);//Le tube du serveur inter

    int descripteursTermimal[4] = {fdLecteur,fdEcrivain,fdValider,fdInter};

    int fdValiderListerner= pipeValidationAcquisition[0];//Descripteur de fichier pour lire les "reponses"
    int fdInterListener = open("txt_test_acquisition/inter1_reponse.txt",O_RDONLY);//Descripteur de fichier pour lire les "reponses"

    sem_init(&semState,0,1);
    sem_init(&nbCaseLibre,0,nbMaxBufferDemande);

    pthread_t threadTerminal;
    pthread_t threadValider;
    pthread_t threadInter;

    pthread_create(&threadTerminal,NULL,lireRequeteTerminal,descripteursTermimal);
    pthread_create(&threadValider,NULL,threadReceptionReponse,&fdValiderListerner);
    pthread_create(&threadInter,NULL,threadReceptionReponse,&fdInterListener);

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

    while(c != NULL){
        sem_wait(&semState);//Permet de ne pas ecrire dans la memoire quand elle est lu
        sem_wait(&nbCaseLibre);
        if(state[i] == 0){
            state[i] = 1;//On met la case a 1 car elle va etre utilise
            decoupe(c,nTest,type,valeur);

            strcpy(bufferDemande[i],nTest);//on met le test dans le tableau on utilise STRCPY pour ne pas ecraser le pointeur
            bufferDescripteur[i] = fdEcrivain;//On met le fd correspondant

            char demandeId[255];
            decoupe_str(nTest,demandeId,0,3);//On recupere l'id du centre correspondant au test recu

            if(!strcmp(demandeId,idCentre) ){//Si c'est le meme ID on envoye a validation
                printf("%s\n",c);
                ecritLigne(fdValider,c);
            }else{//Sinon inter-archive
                ecritLigne(fdInter,c);
            }

            c =  litLigne(fdLecteur);
        }
        sem_post(&semState);
        i++;
        if(i > nbMaxBufferDemande){
            i = 0;
        }
    }

    free(c);
    printf("Fin thread terminal\n");
    pthread_exit(NULL);
}

void *threadReceptionReponse(void* fdReponse){
    int fdReponseInt = *((int *)fdReponse);

    char* bufferReader =  malloc(TAILLEBUF-1);
    char nTest[255],type[255],valeur[255];
    bufferReader = litLigne(fdReponseInt);//On lit la premiere ligne pour lancer la boucle
    while( bufferReader != NULL){
        printf("%s\n",bufferReader);
        decoupe(bufferReader,nTest,type,valeur);
        for (int i = 0; i < nbMaxBufferDemande; i++){//On check toute les demandes
            sem_wait(&semState);
            if(state[i] == 1 && !strcmp(bufferDemande[i],nTest)){//Si la case est pleine on verifi et si le test corespond
                state[i] = 0;//On libere la case i
                sem_post(&nbCaseLibre);
                printf("%s\n",bufferReader);
                ecritLigne(bufferDescripteur[i],bufferReader);
            }
            sem_post(&semState);

        }
        bufferReader = litLigne(fdReponseInt);
    }
    printf("Fin thread validation/inter\n");
    pthread_exit(NULL);
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