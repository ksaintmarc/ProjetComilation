#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "check.h"

pilePortee_t pilePortee_g;

sym_t *symNouveau(char *nom){

    int tailleNom;
    sym_t *newSym;
    CHECK(newSym = malloc(sizeof(sym_t)));

    newSym->nom = nom;
    newSym->portee = NULL;
    newSym->ligneDeclaration = 0;
    newSym->tailleTableau = 0;
    newSym->nombreArgs = 0;
    newSym->valeurRetour = 0;
    newSym->type = S_VIDE;
    
    return newSym;
}

int symHash(char *nom){

    int hash, len;
    hash = 0;
    len = strlen(nom);

    for (int i = 0; i < len; i++)
        hash += (int)nom[i];

    return hash % MAX_SYMBOLES;
}

void symAffichage(sym_t *sym){

    if (sym == NULL)
        return;

    int nomLen, porteeLen;
    nomLen = strlen(sym->nom);
    porteeLen = strlen(sym->portee);

    #if SYNTAX_DEBUG_AVANCE == 0
    // Nom du symbole
    if (nomLen > 16)
        printf("| %.16s... ", sym->nom);
    else
        printf("| %-19s ", sym->nom);
    // Portée du symbole
    if (porteeLen > 16)
        printf("| %.16s... ", sym->portee);
    else
        printf("| %-19s ", sym->portee);
    // Ligne de déclaration
    printf("| %-19d ", sym->ligneDeclaration);
    
    switch (sym->type)
    {
    case S_ENTIER:
    case S_CHAINE:
    case S_VIDE:
    case S_TABLEAU:
        // Type du symbole
        if (sym->type == S_ENTIER)
            printf("| %-19s ", "int");
        else if ((sym->type == S_CHAINE) | (sym->type == S_TABLEAU))
            printf("| %-19s ", "string");
        else 
            printf("| %-19s ", "-");
        // Taille du tableau
        if (sym->type == S_TABLEAU)
            printf("| %-19d ", sym->tailleTableau);
        else
            printf("| %-19s ", "-");
        // Nombre d'arguments et valeur de retour
        printf("| %-19s ", "-");
        printf("| %-19s ", "-");
        break;

    case S_FONCTION:
        printf("| %-19s ", "function");
        printf("| %-19s ", "-");
        printf("| %-19d ", sym->nombreArgs);
        printf("| %-19d ", sym->valeurRetour);
        break;

    default:
        printf("| %-19s ", "unknown");
        printf("| %-19s ", "-");
        printf("| %-19s ", "-");
        printf("| %-19s ", "-");
        break;
    }

    printf("|\n");
    #else
    // Si on est en mode debug avancé, on affiche que les informations nécessaires
    printf("%s ", sym->nom);
    printf("%s ", sym->portee);
    printf("%d ", sym->ligneDeclaration);

    switch (sym->type)
    {
    case S_ENTIER:
    case S_CHAINE:
    case S_VIDE:
    case S_TABLEAU:
        // Type du symbole
        if (sym->type == S_ENTIER)
            printf("%s ", "int");
        else if ((sym->type == S_CHAINE) | (sym->type == S_TABLEAU))
            printf("%s ", "string");
        else 
            printf("%s ", "-");
        // Taille du tableau
        if (sym->type == S_TABLEAU)
            printf("%d ", sym->tailleTableau);
        else
            printf("%s ", "-");
        // Nombre d'arguments et valeur de retour
        printf("%s ", "-");
        printf("%s ", "-");
        break;

    case S_FONCTION:
        printf("%s ", "function");
        printf("%s ", "-");
        printf("%d ", sym->nombreArgs);
        printf("%d ", sym->valeurRetour);
        break;

    default:
        printf("%s ", "unknown");
        printf("%s ", "-");
        printf("%s ", "-");
        printf("%s ", "-");
        break;
    }

    printf("\n");
    #endif
}

void symFree(sym_t *sym){

    if (sym == NULL)
        return;

    free(sym->nom);
    free(sym);
}

tableSym_t *tableSymNouveau(char *portee){

    int taillePortee;
    tableSym_t *newTable;
    CHECK(newTable = malloc(sizeof(tableSym_t)));

    taillePortee = strlen(portee);
    CHECK(newTable->portee = malloc((taillePortee + 1) * sizeof(char)));

    strcpy(newTable->portee, portee);
    newTable->taille = 0;
    newTable->capacite = MAX_SYMBOLES;
    newTable->precedent = -1;

    for (int i = 0; i < newTable->capacite; i++)
        newTable->symboles[i] = NULL;

    return newTable;
}

void tableSymAjout(tableSym_t *table, sym_t *sym){

    CHECK(table);
    CHECK(sym);
    
    if (table->taille >= table->capacite) {
        fprintf(stderr, "tableSymAjout : table is full\n");
        exit(EXIT_FAILURE);
    }

    int index, iteration;
    sym_t *tmp;

    index = symHash(sym->nom);
    iteration = 0;
    
    while (table->symboles[index] != NULL) {
        // Failsafe pour éviter une boucle infinie
        if (iteration >= MAX_SYMBOLES) {
            fprintf(stderr, "tableSymAjout : maximum iteration reached\n");
            exit(EXIT_FAILURE);
        }

        index = (index + 1) % MAX_SYMBOLES;
        iteration++;
    }

    sym->portee = table->portee;
    table->symboles[index] = sym;
    table->taille++;
}

sym_t *tableSymGet(tableSym_t *table, char *nom){

    int index;

    index = tableSymGetIndex(table, nom);
    if (index == -1)
        return NULL;

    return table->symboles[index];
}

int tableSymGetIndex(tableSym_t *table, char *nom){
    
    int index, iteration;

    index = symHash(nom);
    iteration = 0;

    while (table->symboles[index] != NULL) {
        // Failsafe pour éviter une boucle infinie
        if (iteration >= MAX_SYMBOLES) {
            fprintf(stderr, "tableSymGetIndex : maximum iteration reached\n");
            exit(EXIT_FAILURE);
        }

        if (strcmp(table->symboles[index]->nom, nom) == 0)
            return index;

        index = (index + 1) % MAX_SYMBOLES;
        iteration++;
    }

    return -1;
}

void tableSymAffichage(tableSym_t *table, int printTop, int printBottom){

    if (table == NULL)
        return;

    #if SYNTAX_DEBUG_AVANCE == 0
    if (printTop) {
        
        printf("Symbol table : \n");

        for (int i = 0; i < 7; i++)
            printf("+---------------------");
        
        printf("+\n");

        printf("| %-19s ", "nom variable");
        printf("| %-19s ", "portee");
        printf("| %-19s ", "ligne");
        printf("| %-19s ", "type");
        printf("| %-19s ", "taille tableau");
        printf("| %-19s ", "nombre arguments");
        printf("| %-19s ", "valeur de retour");
        printf("|\n");

        for (int i = 0; i < 7; i++)
            printf("+---------------------");
        
        printf("+\n");
    }
    #endif

    for (int i = 0; i < table->capacite; i++) {

        if (table->symboles[i] == NULL)
            continue;

        symAffichage(table->symboles[i]);
    }

    #if SYNTAX_DEBUG_AVANCE == 0
    if (printBottom) {
        for (int i = 0; i < 7; i++)
            printf("+---------------------");
        
        printf("+\n");
    }
    #endif
}

void tableSymFree(tableSym_t *table){

    for (int i = 0; i < table->capacite; i++){
        symFree(table->symboles[i]);
    }

    free(table->portee);
    free(table);
}

void pilePorteeInit(void){
    
    pilePortee_g.tables[0] = tableSymNouveau("global");
    pilePortee_g.sommet = 0;
    pilePortee_g.taille = 1;
    pilePortee_g.capacite = 1;
}

void pilePorteePush(char *portee){
    
    if (pilePortee_g.capacite >= MAX_PORTEE) {
        fprintf(stderr, "pilePorteePush : stack is full\n");
        exit(EXIT_FAILURE);
    }

    int precedent;
    precedent = pilePortee_g.sommet;

    pilePortee_g.taille++;
    pilePortee_g.capacite++;
    pilePortee_g.sommet = pilePortee_g.capacite - 1;
    pilePortee_g.tables[pilePortee_g.sommet] = tableSymNouveau(portee);
    pilePortee_g.tables[pilePortee_g.sommet]->precedent = precedent;
}

void pilePorteePop(void){

    int precedent;
    precedent = pilePortee_g.tables[pilePortee_g.sommet]->precedent;

    if (precedent == -1) {
        fprintf(stderr, "pilePorteePop : can't pop global portee\n");
        exit(EXIT_FAILURE);
    }

    pilePortee_g.sommet = precedent;
    pilePortee_g.taille--;
}

tableSym_t *pilePorteeSommet(void){

    return pilePortee_g.tables[pilePortee_g.sommet];
}

tableSym_t *pilePorteeRacine(void){

    return pilePortee_g.tables[0];
}

void pilePorteeAjout(sym_t *sym){
    
    tableSymAjout(pilePortee_g.tables[pilePortee_g.sommet], sym);
}

sym_t *pilePorteeRecherche(char *nom){
    
    int index;
    sym_t *sym;

    index = pilePortee_g.sommet;
    sym = NULL;

    while (index != -1) {
        
        sym = tableSymGet(pilePortee_g.tables[index], nom);
        if (sym != NULL)
            return sym;

        index = pilePortee_g.tables[index]->precedent;
    }

    return NULL;
}

void pilePorteeAffichage(void){

    for (int i = 0; i < pilePortee_g.capacite; i++){

        if (pilePortee_g.taille == 1)
            tableSymAffichage(pilePortee_g.tables[i], 1, 1);
        else if (i == 0)
            tableSymAffichage(pilePortee_g.tables[i], 1, 0);
        else if (i == pilePortee_g.capacite - 1)
            tableSymAffichage(pilePortee_g.tables[i], 0, 1);
        else
            tableSymAffichage(pilePortee_g.tables[i], 0, 0);
    }
}

void pilePorteeFree(void){
    
    for (int i = 0; i < pilePortee_g.capacite; i++)
        tableSymFree(pilePortee_g.tables[i]);
}
