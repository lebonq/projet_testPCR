/**
 * @file interArchive.c
 * @author Quentin LEBON
 * @brief 
 * @version 0.1
 * @date 2021-04-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "interArchive.h"
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

int nbMaxBufferDemande;//Pas besoin de semaphore car elle est en read-only
char** idCentres;
int* descripteurCentre;
int nbServerAcquisition;

//les tableaux qui contiendrons nos pipes
int** pipeInterArchiveAcquisiton;
int** pipeAcquisitionInterArchive;

int main(int argc,char** argv){

    if(argc < 2){
        printf("Le programme s'utilise avec 1 arguments, ./Acquisiton configFile nbAcquisiton tailleBuffer\n");
        exit(0);
    }

    char* fileConfig = argv[1];
    nbServerAcquisition = atoi(argv[2]);
    nbMaxBufferDemande = atoi(argv[3]);

    bufferDemande = (char**)malloc(sizeof(char*)*(nbMaxBufferDemande));
    bufferDescripteur = malloc(sizeof(int)*(nbMaxBufferDemande));
    state = malloc(sizeof(int)*(nbMaxBufferDemande));
    descripteurCentre = malloc(sizeof(int)*(nbMaxBufferDemande));

    sem_init(&semState,0,1);
    sem_init(&nbCaseLibre,0,nbMaxBufferDemande);

    for (int i = 0; i < nbMaxBufferDemande; i++){//On met un malloc dans chaque cases de notre buffer pour stocker nos code
        bufferDemande[i] = (char*)malloc(sizeof(char)*16);//16 chars suffisent car le test fait 16 caraceteres
        state[i] = 0;//On initialise state a 0
    }

    for (int i = 0; i < nbServerAcquisition; i++){
        idCentres[i] = (char*)malloc(sizeof(char)*16);
        descripteurCentre[i] = 0;
    }

    pthread_t* threadAcquisition = (pthread_t*)malloc(sizeof(pthread_t)*(nbServerAcquisition));//Un tableau qui contient tout nos threads acquisitoon

    pipeAcquisitionInterArchive = (int**)malloc(sizeof(int*)*(nbServerAcquisition));
    pipeInterArchiveAcquisiton = (int**)malloc(sizeof(int*)*(nbServerAcquisition));

    char nomFilePcr[255], nbTerminaux[255], idCentre[255];
    int i = 0;
    FILE* config = fopen(fileConfig,"r");
    while(fscanf(config,"%s %s %s",nomFilePcr,idCentre,nbTerminaux) != EOF){//on lit le fichier de config
        pipeAcquisitionInterArchive[i] = (int*)malloc(sizeof(int)*(2));
        pipeInterArchiveAcquisiton[i] = (int*)malloc(sizeof(int)*(2));
        
        pipe(pipeAcquisitionInterArchive[i]);
        pipe(pipeInterArchiveAcquisiton[i]);

        idCentres[i] = idCentre;

        pid_t Acqusition = fork();
        if(Acqusition == 0){
            char fd1[16];
            char fd2[16];
            sprintf(fd1,"%d",pipeInterArchiveAcquisiton[i][0]);
            sprintf(fd2,"%d",pipeAcquisitionInterArchive[i][1]);
            char* nbMaxBufferDemandeChar = argv[3];
            char nomCentre[512];
            sprintf(nomCentre,"Centre %s",idCentre);
            execlp("/usr/bin/xterm", "xterm", "-e", "./Acquisition", nbMaxBufferDemandeChar, idCentres, nomFilePcr, nbTerminaux, fd1 , fd2 , NULL);
        }
        int** descripteursTermimal = (int**)malloc(sizeof(int*) * (nbServerAcquisition));//On fait un tableau qui contiendra les 4 descripteurs pour chacun de nos trheads
        descripteursTermimal[i] = (int*)malloc(sizeof(int)*4);
        descripteursTermimal[i][0] = pipeAcquisitionInterArchive[i][0];//Descripteur de fichier pour lire
        descripteursTermimal[i][1] = pipeInterArchiveAcquisiton[i][1];//Descripteur de fichier pour ecrire les reponses 
        pthread_create(&threadAcquisition[i],NULL,threadInter,descripteursTermimal[i]);//todo
        i++;
    }
    
}

void* threadInter(void* fd){//todo
    int fdLecteur   =    ((int*)fd)[0];
    int fdEcrivain  =    ((int*)fd)[1];

    char* bufferReader =  malloc(TAILLEBUF-1);
    char nTest[255],type[255],valeur[255];
    bufferReader = litLigne(fdLecteur);//On lit la premiere ligne pour lancer la boucle
    while( bufferReader != NULL){
        decoupe(bufferReader,nTest,type,valeur);
        if(strcmp(type,"Demande")){//on regarde si le message recu est une reponse ou une demande
            sem_wait(&nbCaseLibre);
            sem_wait(&semState);//Permet de ne pas ecrire dans la memoire quand elle est lu

            char demandeId[255];
            decoupe_str(nTest,demandeId,0,3);//On recupere l'id du centre correspondant au test recu

            for (int i = 0; i < nbMaxBufferDemande; i++){//On check toute les demandes
                if(state[i] == 0){
                    state[i] = 1;
                    strcpy(bufferDemande[i],nTest);//on met le test dans le tableau on utilise STRCPY pour ne pas ecraser le pointeur
                    bufferDescripteur[i] = fdEcrivain;//On met le fd correspondant
                    for(int j = 0; j < nbServerAcquisition; j++){
                        if(strcmp(idCentres[j],demandeId)){//On transfert au bon serveur acquisition
                            printf("Transfert au bon serveur acquisition\n");
                            ecritLigne(pipeInterArchiveAcquisiton[j][1],bufferReader);//on donne le tuyaux corrspond au centre de l'id de la demande
                        }
                    }
                }
            }

            sem_post(&semState);
        }
        else{//ici reponse donc on transmet le message au terminal

            for (int i = 0; i < nbMaxBufferDemande; i++){//On check toute les demandes
                sem_wait(&semState);//on rentre en section critique
                if(state[i] == 1 && !strcmp(bufferDemande[i],nTest)){//Si la case est pleine on verifi et si le test corespond
                    state[i] = 0;//On libere la case i
                    sem_post(&nbCaseLibre);//On libere une case dans notre semaphore
                    ecritLigne(bufferDescripteur[i],bufferReader);//On ecrit la ligne dans le descipteur correspondant
                }
                sem_post(&semState);
            }
        }

        bufferReader = litLigne(fdLecteur);
    }

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