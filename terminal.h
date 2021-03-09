int envoyerMessage(char* numeroTest, char* dureeValidite, int fd);
int recevoirMessage(int fd, char** msg);
int affichageResultat(char* msg);
int validerTest(char *numeroTest, int fdEnvoye, int fdRecois);
void enregistrerResultat(char* msg);