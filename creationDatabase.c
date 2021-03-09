#include "lectureEcriture.h"
#include "alea.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
    
    if(argc < 3){
        printf("Le programme s'utilise avec 2 arguments, ./creationDatabase nbTest idCentre\n");
        exit(0);
    }

    char fileName[255];

    sprintf(fileName, "resulats_centre_%s.txt", argv[2]);

    FILE* fd = fopen(fileName,"w");

    //char test[255];
    long unsigned int nbTest = 100000000000;

    for(int i = 0; i <= atoi(argv[1]); i++){
        aleainit();
        nbTest += i*alea(1,150);

        fprintf(fd, "%s%lu %s %s\n",argv[2], nbTest, fileName, fileName);
    }

    fclose(fd);
}