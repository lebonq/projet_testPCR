/**
 * @file validation.h
 * @author Quentin LEBON
 * @brief 
 * @version 0.1
 * @date 2021-03-23
 * 
 * @copyright Copyright (c) 2021
 * 
 */

int validerTest(char *numTest, char* tempsValiditeTest, char* numCentre);
int recevoirDemande(char** demande,int fd);
int envoyerReponse(char* numeroTest, char* resulat, int fd);
void enregistrerResultat(char* msg);