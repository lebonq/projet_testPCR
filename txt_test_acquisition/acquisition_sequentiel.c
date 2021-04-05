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

int main(int argc, char** argv){

    if(argc < 3){
        printf("Le programme s'utilise avec 2 arguments, ./creationDatabase tailleBuffer idCentre\n");
        exit(0);
    }

    int nbMaxBufferDemande = atoi(argv[1]);
    char* idCentre = argv[2];//On recupere l'id du centre sous forme de char*

    if(strlen(idCentre) ==3){//Si il est different de 4 chars ce n;est pas bon !
        printf("Id centre invalide\n");
        exit(0);
    }

    char* bufferDemande[nbMaxBufferDemande];//Permet de stocker les demandes sous forme de string
    for (int i = 0; i < nbMaxBufferDemande; i++){//On met un malloc dans chaque cases de notre buffer pour stocker nos code
        bufferDemande[i] = malloc(sizeof(char)*16);//16 chars suffisent car le test fait 16 caraceteres
    }
    
    int bufferDescripteur[nbMaxBufferDemande];//Permet de stocker le descripteur de fichier en correspondance avec la demande
    int state[nbMaxBufferDemande];//Permet de savoir le statut de la demande associe 
    /**
     * 0 la case est libre
     * 1 la case est occupe 
     */
    for (int i = 0; i < nbMaxBufferDemande; i++){//On initialise state a 0
        state[i] = 0;
    }

    int fdValider = open("txt_test_acquisition/validation1_reponse.txt",O_RDONLY);//Descripteur de fichier pour lire les "reponses"
    int fdInter = open("txt_test_acquisition/inter1_reponse.txt",O_RDONLY);//Descripteur de fichier pour lire les "reponses"
    printf("Terminal\n");
    lireRequeteTerminal(bufferDemande,bufferDescripteur,state,nbMaxBufferDemande,idCentre);
    printf("Valider\n");
    threadReceptionReponse(bufferDemande,bufferDescripteur,state,nbMaxBufferDemande,fdValider);
    printf("Inter Serveur\n");
    threadReceptionReponse(bufferDemande,bufferDescripteur,state,nbMaxBufferDemande,fdInter);

    return 0;
}

/**
 * @brief Permet de lire et d'enregister les requestes envoyer par un termial
 * 
 * @param bufferDemande Notre buffer avec les codes de tests
 * @param bufferDescripteur Notre buffers avec les FD correspondant au terminal d'ou vient le code de test
 * @param state Permet de savoir d'ou vient
 * @param nbDemande 
 * @param idCentre Id du centre ou ce situ le serveur d'acquisition
 * @return int 
 */
int lireRequeteTerminal(char** bufferDemande, int* bufferDescripteur, int* state, int nbDemande,char* idCentre){
    int fdLecteur = open("txt_test_acquisition/terminal1_demande.txt",O_RDONLY);//Descripteur de fichier pour lire les demandes
    int fdEcrivain = open("txt_test_acquisition/terminal1_reponse.txt",O_WRONLY);//Descripteur de fichier pour ecrire les reponses
    int fdValider = open("txt_test_acquisition/validation1_demande.txt",O_WRONLY);//Le tube de validation
    int fdInter = open("txt_test_acquisition/inter1_demande.txt",O_WRONLY);//Le tube du serveur inter

    char* c =  litLigne(fdLecteur);
    int i = 0;
    
    char nTest[255],type[255],valeur[255];

    while(c != NULL && i < nbDemande){
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
        i++;
    }

    free(c);
    return 0;
}

/**
 * @brief Permet de lire les reponses recu dans le FD, et transmet cette reponse au FD du terminal correspondnant
 * 
 * @param bufferDemande Notre buffer avec les codes de tests
 * @param bufferDescripteur Notre buffers avec les FD correspondant au terminal d'ou vient le code de test
 * @param state Permet de savoir d'ou vient
 * @param nbMaxBufferDemande 
 * @param fdReponse fd ou l;on lit les reponses recus
 * @return int 
 */
int threadReceptionReponse(char** bufferDemande, int* bufferDescripteur, int* state ,int nbMaxBufferDemande, int fdReponse){

    char* bufferReader =  malloc(TAILLEBUF-1);
    char nTest[255],type[255],valeur[255];

    bufferReader = litLigne(fdReponse);//On lit la premiere ligne pour lancer la boucle
    while( bufferReader != NULL){
        decoupe(bufferReader,nTest,type,valeur);
        for (int i = 0; i < nbMaxBufferDemande; i++){//On check toute les demandes
            if(state[i] == 1 && !strcmp(bufferDemande[i],nTest)){//Si la case est pleine on verifi et si le test corespond
                state[i] = 0;//On libere la case i
                ecritLigne(bufferDescripteur[i],bufferReader);
            }
        }
        bufferReader = litLigne(fdReponse);
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