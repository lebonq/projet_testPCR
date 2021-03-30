#include "acquisition.h"
#include "lectureEcriture.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char** argv){

    int nbMaxBufferDemande = atoi(argv[0]);

    char** bufferDemande = (char**) malloc(sizeof(char)*nbMaxBufferDemande);
    
    for (int i = 0; i < nbMaxBufferDemande; i++){
        bufferDemande[i] = (char*) malloc(TAILLEBUF);
    }
    
    /*
    bufferDemande[0] = malloc(TAILLEBUF);
    bufferDemande[1] = malloc(TAILLEBUF);*/

    strcpy(bufferDemande[0],"test1");
    //strcpy(bufferDemande[1],"test2");

    printf("%s \n%s\n",bufferDemande[0],bufferDemande[1]);

    return 0;
}