/**
 * @file acquisition.h
 * @author Quentin LEBON
 * @brief 
 * @version 0.1
 * @date 2021-04-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */
void *lireRequeteTerminal(void* fdTermimal);
void *threadValidation(void* fdReponse);
void *threadInterArchive(void* unUse);
void decoupe_str(char* input, char * output, int start, int end);