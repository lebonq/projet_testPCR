#include "terminal.h"
#include "message.h"
#include "lectureEcriture.h"
#include "alea.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){// argv[1] descriteru de fichier de lecture des réponses et argv[2] c'est ou l'on ecrit les requêtes
    
    if(argc < 1){
        printf("Il manque un descripteur de fichier");
        exit(0);
    }

    char valeur[255];

    aleainit();
    sprintf(valeur,"%d",alea(1,50000));
    envoyerMessage("1703582648300826", "Demande", valeur, argv[2]);

}

int envoyerMessage(char* numeroTest, char* typeMessage, char* dureeValidite, int fd){
    char *msg = message(numeroTest,typeMessage,dureeValidite); //On cree le char* de notre message pour l'envoyer ensuite
    int err = ecritLigne(fd, msg);//On l'envoie au descripteur de fichier spécifier lors de demarrage du programme

    if(err == 0){
        return err;
    }

    free(msg);//On libere la memoire
    return err;
}

int recevoirMessage(){
    return 1;
}
