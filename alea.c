#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

#include "alea.h"

/**
 * Initialisation du g�n�rateur al�atoire
 */
void aleainit()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);

    srand( (unsigned)time( NULL ) + getpid() + tv.tv_usec);//Ajout par Quentin Lebon des micros secondes du temps actuel pour un tirage aleatoire plus "rapide"
}

/**
 * Retourne un nombre al�atoire en min et max (bornes comprises)
 */
int alea(int min, int max)
{
  int lgr;

  lgr = max-min+1;
  return (int)((double)rand()/RAND_MAX*lgr+min);
}

