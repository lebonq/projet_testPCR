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

//Pipe pour communiquer avec interarchive
//pipe pour communiquer avec validation
int pipeValidationAcquisition[2];
int pipeAcquisitonValidation[2];
int pipeInterArchiveAcquisiton;//lecture
int pipeAcquisitonInterArchive;//Ecriture

int main(int argc, char** argv){

    if(argc < 8){
        printf("Le programme s'utilise avec 7 arguments, ./Acquisiton tailleBuffer nomCentre idCentre nomFichierTestPcr nbTerminal fdLecteurInterArchive fdEcrivainInterArchive\n");
        exit(0);
    }

    nbMaxBufferDemande = atoi(argv[1]);
    char* nomCentre = argv[2];
    idCentre = argv[3];//On recupere l'id du centre sous forme de char*
    char* nomFichierPcr = argv[4];
    int nbTerminal = atoi(argv[5]);
    pipeInterArchiveAcquisiton = atoi(argv[6]);//pour les lire ce que interarchive nous envoye
    pipeAcquisitonInterArchive = atoi(argv[7]);//Pour ecrire

    printf("%s\n",nomCentre);

    if(strlen(idCentre) ==3){//Si il est different de 4 chars ce n;est pas bon !
        printf("Id centre invalide\n");
        exit(0);
    }

    bufferDemande = (char**)malloc(sizeof(char*)*(nbMaxBufferDemande));
    bufferDescripteur = malloc(sizeof(int)*(nbMaxBufferDemande));
    state = calloc(nbMaxBufferDemande,sizeof(int));

    for (int i = 0; i < nbMaxBufferDemande; i++){//On met un malloc dans chaque cases de notre buffer pour stocker nos code
        bufferDemande[i] = (char*)malloc(sizeof(char)*16);//16 chars suffisent car le test fait 16 caraceteres
    }

    //nos 2 tableau de pipe pour les terminaux
    int** pipeTerminalAcquisiton;
    int** pipeAcquisitionTerminal;

    pipeTerminalAcquisiton = (int**)malloc(sizeof(int*)*(nbTerminal));//on prepapre notre tableau pour nos tuyaux
    pipeAcquisitionTerminal = (int**)malloc(sizeof(int*)*(nbTerminal));
 
    for (int i = 0; i < nbTerminal; i++){//cette boucle permet de creer tout les terminaux necessaires
        pipeTerminalAcquisiton[i] = (int*)malloc(sizeof(int)*(2));
        pipeAcquisitionTerminal[i] = (int*)malloc(sizeof(int)*(2));
        
        pipe(pipeTerminalAcquisiton[i]);
        pipe(pipeAcquisitionTerminal[i]);
        pid_t terminal = fork();
        if(terminal == 0){
            char fd1[16];
            char fd2[16];
            sprintf(fd1,"%d",pipeAcquisitionTerminal[i][0]);//on converti les descripteur de fichier en char*
            sprintf(fd2,"%d",pipeTerminalAcquisiton[i][1]);
            execlp("/usr/bin/xterm", "xterm", "-e", "./Terminal", fd1, fd2, nomCentre, NULL);//on recouvre les process fils par des terminaux xterm
        }
    }

    pipe(pipeValidationAcquisition);
    pipe(pipeAcquisitonValidation);

    pid_t validation = fork();
    if(validation == 0){//ici on cree le processus validation
        char fd1[16];
        char fd2[16];
        sprintf(fd1,"%d",pipeAcquisitonValidation[0]);//on converti les descripteur de fichier en char*
        sprintf(fd2,"%d",pipeValidationAcquisition[1]);
        execlp("./Validation", "Validation", fd1, fd2, nomFichierPcr, NULL);//on recouvre le process par un 
        printf("Jamais Print\n");
        exit(0);
    }

    int fdValiderListerner= pipeValidationAcquisition[0];//Descripteur de fichier pour lire les "reponses"
    int fdInterListener = pipeInterArchiveAcquisiton;//Descripteur de fichier pour lire les "reponses"

    sem_init(&semState,0,1);//on initialise nos sémaphores
    sem_init(&nbCaseLibre,0,nbMaxBufferDemande);

    pthread_t* threadTerminal = (pthread_t*)malloc(sizeof(pthread_t)*(nbTerminal));//Un tableau qui contient tout nos threads terminaux
    pthread_t threadValider;
    pthread_t threadInter;

    int** descripteursTermimal = (int**)malloc(sizeof(int*) * (nbTerminal));//On fait un tableau qui contiendra les 4 descripteurs pour chacun de nos trheads

    for (int i = 0; i < nbTerminal; i++){//On cree autant de thread terminal que de terminal en les connectant aux bons tubes
        descripteursTermimal[i] = (int*)malloc(sizeof(int)*4);
        descripteursTermimal[i][0] = pipeTerminalAcquisiton[i][0];//Descripteur de fichier pour lire les demandes des terminaux
        descripteursTermimal[i][1] = pipeAcquisitionTerminal[i][1];//Descripteur de fichier pour ecrire les reponses dans les terminaux
        descripteursTermimal[i][2] = pipeAcquisitonValidation[1];//Le tube de validation
        descripteursTermimal[i][3]  = pipeAcquisitonInterArchive;//Le tube du serveur inter

        pthread_create(&threadTerminal[i],NULL,lireRequeteTerminal,descripteursTermimal[i]);//on cree notre thread qui gerera terminal
    }
    
    pthread_create(&threadValider,NULL,threadValidation,&fdValiderListerner);
    pthread_create(&threadInter,NULL,threadInterArchive,&fdInterListener);

    for (int i = 0; i < nbTerminal; i++){
        pthread_join(threadTerminal[i],NULL);//permet d'attendre la fin de tout les threads
    }

    pthread_join(threadValider,NULL);
    pthread_join(threadInter,NULL);

    return 0;
}

/**
 * @brief Permet de lire les requetes envoyer la terminaux et de les router au bon endroit
 * 
 * @param fdTermimal 
 * @return void* 
 */
void *lireRequeteTerminal(void* fdTermimal){
    int fdLecteur   =    ((int*)fdTermimal)[0];
    int fdEcrivain  =    ((int*)fdTermimal)[1];
    int fdValider   =    ((int*)fdTermimal)[2];
    int fdInter     =    ((int*)fdTermimal)[3];

    char* c =  litLigne(fdLecteur);
    int i = 0;
    
    char nTest[255],type[255],valeur[255];

    while(c != NULL){//La boucle d'effectue tant que le tube n'est pas vide
        sem_wait(&nbCaseLibre);
        sem_wait(&semState);//Permet de ne pas ecrire dans la memoire quand elle est lu
        if(state[i] == 0){
            state[i] = 1;//On met la case a 1 car elle va etre utilise
            if(0 == decoupe(c,nTest,type,valeur)){
                printf("ERROR DECOUPE\n");
                exit(0);//si erreur decoupe on coupe ke programme en le notifiant
            }

            strcpy(bufferDemande[i],nTest);//on met le test dans le tableau on utilise STRCPY pour ne pas ecraser le pointeur
            bufferDescripteur[i] = fdEcrivain;//On met le fd correspondant

            char demandeId[255];
            decoupe_str(nTest,demandeId,0,3);//On recupere l'id du centre correspondant au test recu

            if(!strcmp(demandeId,idCentre) ){//Si c'est le meme ID on envoye a validation
                printf("Transfert a validation\n");
                ecritLigne(fdValider,c);
            }else{//Sinon inter-archive
                printf("Transfert a inter archive\n");
                ecritLigne(fdInter,c);
            }
            sem_post(&semState);//On sort de la section critique
            c =  litLigne(fdLecteur);//On attends un nouvel input
        }

        i++;
        if(i > nbMaxBufferDemande){//si le i est egale a la taille max du buffer on le remte a 0
            i = 0;
        }
    }

    free(c);
    printf("Fin thread terminal\n");
    pthread_exit(NULL);
}

/**
 * @brief Permet de router les réponses envoyer par validation
 * 
 * @param fdReponse 
 * @return void* 
 */
void *threadValidation(void* fdReponse){
    int fdReponseInt = *((int *)fdReponse);

    char* bufferReader =  malloc(TAILLEBUF-1);
    char nTest[255],type[255],valeur[255];
    bufferReader = litLigne(fdReponseInt);//On lit la premiere ligne pour lancer la boucle
    while( bufferReader != NULL){//La boucle s'effectue tant que le buffer de lecture n'est pas vide
        decoupe(bufferReader,nTest,type,valeur);
        for (int i = 0; i < nbMaxBufferDemande; i++){//On check toute les demandes
            sem_wait(&semState);//on rentre en section critique
            if(state[i] == 1 && !strcmp(bufferDemande[i],nTest)){//Si la case est pleine on verifi et si le test corespond
                state[i] = 0;//On libere la case i
                sem_post(&nbCaseLibre);//On libere une case dans notre semaphore
                ecritLigne(bufferDescripteur[i],bufferReader);//On ecrit la ligne dans le descipteur correspondant
            }
            sem_post(&semState);

        }
        bufferReader = litLigne(fdReponseInt);
    }
    printf("Fin thread validation/inter\n");
    pthread_exit(NULL);
}

/**
 * @brief Permet de router les information qui nous parviennent de inter archive
 * 
 * @param unUse 
 * @return void* 
 */
void *threadInterArchive(void* unUse){

    char* bufferReader =  malloc(TAILLEBUF-1);
    char nTest[255],type[255],valeur[255];
    bufferReader = litLigne(pipeInterArchiveAcquisiton);//On lit la premiere ligne pour lancer la boucle
    while( bufferReader != NULL){//La boucle s'effectue tant que le buffer de lecture n'est pas vide
        printf("%s\n",bufferReader);
        if( 0 == decoupe(bufferReader,nTest,type,valeur)){
            printf("ERROR DECOUPE\n");
            exit(0);
        }
        if(!strcmp(type,"Demande")){//on regarde si le message recu est une reponse ou une demande
            sem_wait(&nbCaseLibre);//on regarde si une case est vide
            sem_wait(&semState);//Permet de ne pas ecrire dans la memoire quand elle est lu

            for (int i = 0; i < nbMaxBufferDemande; i++){//On check toute les demandes
                if(state[i] == 0){
                    state[i] = 1;
                    strcpy(bufferDemande[i],nTest);//on met le test dans le tableau on utilise STRCPY pour ne pas ecraser le pointeur
                    bufferDescripteur[i] = pipeAcquisitonInterArchive;//On met le fd correspondant a inter archive comme ca le thread validation l'enverra directement a interarchive
                    printf("Transfert a depuis acquisiton thread validation\n");
                    ecritLigne(pipeAcquisitonValidation[1],bufferReader);
                    break;
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
                    printf("Envoye reponse interarchve a termina\n");
                    ecritLigne(bufferDescripteur[i],bufferReader);//On ecrit la ligne dans le descipteur correspondant
                }
                sem_post(&semState);
            }
        }

        bufferReader = litLigne(pipeInterArchiveAcquisiton);
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