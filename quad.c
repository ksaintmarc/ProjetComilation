#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "quad.h"
#include "check.h"


static size_t puissance2Suivante(size_t taille){

    int i;
    if (!taille)
        return 0;
    --taille;
    for (i = 0; taille; taille >>= 1, ++i);
    return 1 << i;
}

static void agrandissementListeQuad(struct listeQuad* l, size_t delta){

    size_t nouvelleTaille = puissance2Suivante(l->taille + delta);
    if (nouvelleTaille <= l->tailleMax)
        return;

    l->tailleMax = nouvelleTaille;
    CHECK((l->qs = realloc(l->qs, l->tailleMax * sizeof(struct quad))) != NULL);
}

struct listeQuad* ajoutListeQuad(struct listeQuad* l, struct quad q){
    
    agrandissementListeQuad(l, 1);
    l->qs[l->taille++] = q;
    return l;
}

void listeQuadFree(struct listeQuad* l){

    free(l->qs);
    l->taille = l->tailleMax = 0;
}

struct listeAdresseQuad* nouvelleListeAdresseQuad(struct listeAdresseQuad* l){

    l->valeurs = NULL;
    l->taille = l->tailleMax = 0;
    return l;
}

static struct listeAdresseQuad* agrandissementListeAdresseQuad(
        struct listeAdresseQuad* l, size_t delta){

    size_t nouvelleTaille = puissance2Suivante(l->taille + delta);
    if (nouvelleTaille <= l->tailleMax)
        return l;
    l->tailleMax = nouvelleTaille;
    CHECK((l->valeurs = realloc(l->valeurs, l->tailleMax * sizeof(adresseQuad_t))) != NULL);
    return l;
}

struct listeAdresseQuad* ajoutListeAdresseQuad(
        struct listeAdresseQuad* l, adresseQuad_t adresse){
    
    l = agrandissementListeAdresseQuad(l, 1);
    l->valeurs[l->taille++] = adresse;
    return l;
}

struct listeAdresseQuad* listeAdresseQuadSeul(struct listeAdresseQuad* l, adresseQuad_t adresse){
    return ajoutListeAdresseQuad(nouvelleListeAdresseQuad(l), adresse);
}

struct listeAdresseQuad* concatListeAdresseQuadFree(
        struct listeAdresseQuad* l0, struct listeAdresseQuad* l1, struct listeAdresseQuad* lResultat){
    
    lResultat = nouvelleListeAdresseQuad(lResultat);
    if (l0->taille + l1->taille == 0)
        return lResultat;
    lResultat = agrandissementListeAdresseQuad(lResultat, l0->taille + l1->taille);
    memcpy(lResultat->valeurs, l0->valeurs, l0->taille * sizeof(adresseQuad_t));
    memcpy(lResultat->valeurs + l0->taille, l1->valeurs, l1->taille * sizeof(adresseQuad_t));
    lResultat->taille = l0->taille + l1->taille;
    listeAdresseQuadFree(l0);
    listeAdresseQuadFree(l1);
    return lResultat;
}

struct listeAdresseQuad* concat3ListeAdresseQuadFree(
        struct listeAdresseQuad* l0, struct listeAdresseQuad* l1,
        struct listeAdresseQuad* l2, struct listeAdresseQuad* lResultat){

    lResultat = nouvelleListeAdresseQuad(lResultat);
    if (l0->taille + l1->taille + l2->taille == 0)
        return lResultat;
    lResultat = agrandissementListeAdresseQuad(lResultat, l0->taille + l1->taille + l2->taille);
    memcpy(lResultat->valeurs, l0->valeurs, l0->taille * sizeof(adresseQuad_t));
    memcpy(lResultat->valeurs + l0->taille, l1->valeurs, l1->taille * sizeof(adresseQuad_t));
    size_t taille01 = l0->taille + l1->taille;
    memcpy(lResultat->valeurs + taille01, l2->valeurs, l2->taille * sizeof(adresseQuad_t));
    lResultat->taille = taille01 + l2->taille;
    listeAdresseQuadFree(l0);
    listeAdresseQuadFree(l1);
    listeAdresseQuadFree(l2);
    return lResultat;
}

void afficherListeAdresseQuad(struct listeAdresseQuad* l){
    printf("%lu (%lu) [", l->taille, l->tailleMax);
    for (size_t i = 0; i < l->taille; ++i)
        printf(i ? ", %lu" : " %lu", l->valeurs[i]);
    printf(" ]\n");
}

void listeAdresseQuadFree(struct listeAdresseQuad* l){
    free(l->valeurs);
}
