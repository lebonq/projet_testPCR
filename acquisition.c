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

    int nbMaxBufferDemande = atoi(argv[1]);
    char* idCentre = argv[2];

    char bufferDemande[nbMaxBufferDemande][TAILLEBUF];//Permet de stocker les demandes sous forme de string

    int bufferDescripteur[nbMaxBufferDemande];//Permet de stocker le descripteur de fichier en correspondance avec la demande

    int state[nbMaxBufferDemande];//Permet de savoir le statut de la demande associe 
    /**
     * 0 la case est libre
     * 1 la case est occupe 
     */

    lireRequeteTerminal(bufferDemande,bufferDescripteur,state,nbMaxBufferDemande,idCentre);
    valider(bufferDemande,bufferDescripteur,state,nbMaxBufferDemande);

    /*while(1){
        //Lire les terminaux
        //Envoyer les requetes au validation
        //Lire les reponses validation
        //Envoyer reponse a terminal
        //Envoyer interarchive
        //Lire reponses inter
        //Envoyer reponse a terminal
    }*/

    return 0;
}

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

            bufferDemande[i] = nTest;//on met le test
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

int valider(char** bufferDemande, int* bufferDescripteur, int* state ,int nbMaxBufferDemande){
    int fdLecteur = open("txt_test_acquisition/validation1_reponse.txt",O_RDONLY);//Descripteur de fichier pour lire les "reponses"

    char* bufferReader =  malloc(TAILLEBUF-1);
    char nTest[255],type[255],valeur[255];

    while( bufferReader != NULL){
        bufferReader = litLigne(fdLecteur);
        decoupe(bufferReader,nTest,type,valeur);

        for (int i = 0; i < nbMaxBufferDemande; i++){//On check toute les demandes
            if(state[i] == 1 && !strcmp(bufferDemande[i],bufferReader)){//Si la case est pleine on verifi et si le test corespond
                state[i] = 0;//On libere la case i
                printf("Envoye reponse");
            }
        }
    } 

    return 0;
}

int validerViaInterArchive(){
    return 0;
}

void decoupe_str(char* input, char * output, int start, int end){
    int j = 0;
    for ( int i = start; i <= end; ++i ) {
        output[j++] = input[i];
    }
    output[j] = 0;
}