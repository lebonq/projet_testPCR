int lireRequeteTerminal(char** bufferDemande, int* bufferDescripteur, int* state, int nbDemande,char* idCentre);
int threadReceptionReponse(char** bufferDemande, int* bufferDescripteur, int* state, int idCentre, int fdReponse);
void decoupe_str(char* input, char * output, int start, int end);