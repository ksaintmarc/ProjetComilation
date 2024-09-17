#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "chaines_litterales.h"
#include "check.h"

static void agrandissementListeChaineLitterale(struct listeChaineLitterale* l, size_t delta){

    if (l->taille + delta <= l->tailleMax)
        return;

    if(l->tailleMax == 0)
        l->tailleMax = 2;
    else
        l->tailleMax *= 2;
    CHECK((l->chaines = realloc(l->chaines, l->tailleMax * sizeof(char*))) != NULL);
}

size_t ajoutListeChaineLitterale(struct listeChaineLitterale* l, char* chaine){

    agrandissementListeChaineLitterale(l, 1);
    l->chaines[l->taille] = chaine;
    return l->taille++;
}

void listeChaineLitteraleFree(struct listeChaineLitterale* l){

    for (size_t i = 0; i < l->taille; i++)
        free(l->chaines[i]);
    free(l->chaines);
    l->taille = l->tailleMax = 0;
    l->chaines = NULL;
}
