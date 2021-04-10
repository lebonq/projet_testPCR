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

int main(int argc,char** argv){

    if(argc < 2){
        printf("Le programme s'utilise avec 1 arguments, ./Acquisiton configFile\n");
        exit(0);
    }

    bufferDemande = (char**)malloc(sizeof(char*)*(nbMaxBufferDemande));
}