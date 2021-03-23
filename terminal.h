/**
 * @file terminal.h
 * @author Quentin LEBON
 * @brief 
 * @version 0.1
 * @date 2021-03-23
 * 
 * @copyright Copyright (c) 2021
 * 
 */

int envoyerMessage(char* numeroTest, char* dureeValidite, int fd);
int recevoirMessage(int fd, char** msg);
int affichageResultat(char* msg);
int validerTest(char *numeroTest, int fdEnvoye, int fdRecois);
void enregistrerResultat(char* msg);