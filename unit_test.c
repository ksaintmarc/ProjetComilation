#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "unit_test.h"
#include "symbol_table.h"
#include "quad.h"
#include "analyse.h"

// Exemple : suite de tests pour la fonction hello()
// BEGIN_SUITE(test_hello){
//
//     char buffer[20];
//
//     CREATE_TEST(test_hello, "hello avec cond == 1"){
//         hello_world(1, buffer);
//         CHECK_STR_EQ(buffer, "Hello World!", OK);
//     }
//
//     END();
// }

// Création d'une suite de tests pour la fonction symNouveau()
// Cette fonction crée un symbole
BEGIN_SUITE(test_symNouveau){
    
    char *nom = strdup("test");
    sym_t *sym = symNouveau(nom);

    CREATE_TEST(test_symNew, "Initialisation"){
        ASSERT_EQ(sym == NULL, FAIL);
    }
    CREATE_TEST(test_symNew, "Vérification des valeurs par défaut"){
        int isok = OK;

        if (strcmp(sym->nom, nom) != 0)
            isok = FAIL;
        if (sym->portee != NULL)
            isok = FAIL;
        if (sym->ligneDeclaration != 0)
            isok = FAIL;
        if (sym->tailleTableau != 0)
            isok = FAIL;
        if (sym->nombreArgs != 0)
            isok = FAIL;
        if (sym->valeurRetour != 0)
            isok = FAIL;
        if (sym->type != S_VIDE)
            isok = FAIL;

        CHECK_EQ(isok, OK);
    }

    symFree(sym);
    END();
}

// Création d'une suite de tests pour la fonction symHash()
// Cette fonction calcule le hash d'un symbole
// Le calcul du hash est le suivant : 
// - on additionne les valeurs ASCII de chaque caractère du nom du symbole
// - on divise le résultat par la taille de la table des symboles
// Exemple :
// - nom du symbole : test
// - taille de la table des symboles : 256
// - hash : (116 + 101 + 115 + 116) / 256 = 192
BEGIN_SUITE(test_symHash){
    
    char *nom = "test";
    int hash = symHash(nom);

    CREATE_TEST(test_symHash, "Vérification du hash"){
        CHECK_EQ(hash, 192);
    }

    END();
}

// Création d'une suite de tests pour la fonction tableSymNouveau()
// Cette fonction crée une table de symboles
BEGIN_SUITE(test_tableSymNouveau){
    
    char *nom = "main";
    tableSym_t *table = tableSymNouveau(nom);

    CREATE_TEST(test_symTableNew, "Initialisation"){
        ASSERT_EQ(table == NULL, FAIL);
    }
    CREATE_TEST(test_symTableNew, "Vérification des valeurs par défaut"){
        int isok = OK;

        if (strcmp(table->portee, "main") != 0)
            isok = FAIL;
        if (table->taille != 0)
            isok = FAIL;
        if (table->capacite != 256)
            isok = FAIL;

        CHECK_EQ(isok, OK);
    }

    tableSymFree(table);
    END();
}

// Création d'une suite de tests pour la fonction tableSymAjout()
// Cette fonction ajoute un symbole à une table de symboles
BEGIN_SUITE(test_tableSymAjout){
    
    char *nom = "main";
    tableSym_t *table = tableSymNouveau(nom);
    char *nomSym = strdup("test");
    sym_t *sym = symNouveau(nomSym);
    int index = symHash(sym->nom);

    tableSymAjout(table, sym);

    CREATE_TEST(test_symTableAdd, "Ajout d'un symbole"){
        CHECK_EQ(table->taille, 1);
    }

    CREATE_TEST(test_symTableAdd, "Placement du symbole dans la table"){
        ASSERT_EQ(table->symboles[index] == NULL, FAIL);
    }

    CREATE_TEST(test_symTableAdd, "Vérification des valeurs"){
        int isok = OK;
        sym_t *symGet = table->symboles[index];

        if (strcmp(symGet->nom, sym->nom) != 0)
            isok = FAIL;
        if (strcmp(symGet->portee, sym->portee) != 0)
            isok = FAIL;
        if (symGet->ligneDeclaration != sym->ligneDeclaration)
            isok = FAIL;
        if (symGet->tailleTableau != sym->tailleTableau)
            isok = FAIL;
        if (symGet->nombreArgs != sym->nombreArgs)
            isok = FAIL;
        if (symGet->valeurRetour != sym->valeurRetour)
            isok = FAIL;
        if (symGet->type != sym->type)
            isok = FAIL;

        CHECK_EQ(isok, OK);
    }

    tableSymFree(table);
    END();
}

// Création d'une suite de tests pour la fonction tableSymGetIndex()
// Cette fonction récupère l'index d'un symbole à partir de son nom
BEGIN_SUITE(test_tableSymGetIndex){
    
    char *nom = "main";
    tableSym_t *table = tableSymNouveau(nom);
    char *nomSym = strdup("test");
    sym_t *sym = symNouveau(nomSym);
    int index = symHash(sym->nom);
    tableSymAjout(table, sym);

    CREATE_TEST(test_symTableGetIndex, "Récupération de l'index"){
        int indexGet = tableSymGetIndex(table, sym->nom);
        CHECK_EQ(indexGet, index);
    }

    tableSymFree(table);
    END();
}

// Création d'une suite de tests pour la fonction tableSymGet()
// Cette fonction récupère un symbole à partir de son nom
BEGIN_SUITE(test_tableSymGet){
    
    char *nom = "main";
    tableSym_t *table = tableSymNouveau(nom);
    char *nomSym = strdup("test");
    sym_t *sym = symNouveau(nomSym);
    int index = symHash(sym->nom);
    tableSymAjout(table, sym);

    CREATE_TEST(test_symTableGet, "Récupération du symbole"){
        sym_t *symGet = tableSymGet(table, sym->nom);
        ASSERT_EQ(symGet == NULL, FAIL);
    }

    CREATE_TEST(test_symTableGet, "Comparaison des symboles"){
        sym_t *symGet = tableSymGet(table, sym->nom);
        CHECK_EQ(symGet == sym, OK);
    }

    tableSymFree(table);
    END();
}

// Création d'une suite de tests pour la fonction pilePorteeInit()
// Cette fonction initialise la pile des portées
BEGIN_SUITE(test_pilePorteeInit){
    
    pilePorteeInit();
    pilePortee_t *stack = &pilePortee_g;

    CREATE_TEST(test_scopeStackInit, "Initialisation"){
        ASSERT_EQ(stack == NULL, FAIL);
    }

    CREATE_TEST(test_scopeStackInit, "Vérification des valeurs par défaut"){
        int isok = OK;

        if (strcmp(stack->tables[0]->portee, "global") != 0)
            isok = FAIL;
        if (stack->sommet != 0)
            isok = FAIL;
        if (stack->taille != 1)
            isok = FAIL;
        if (stack->capacite != 1)
            isok = FAIL;

        CHECK_EQ(isok, OK);
    }

    pilePorteeFree();
    END();
}

// Création d'une suite de tests pour la fonction pilePorteePush()
// Cette fonction ajoute une portée à la pile des portées
BEGIN_SUITE(test_pilePorteePush){
    
    pilePorteeInit();
    pilePortee_t *stack = &pilePortee_g;

    CREATE_TEST(test_scopeStackPush, "Ajout d'une portée"){
        int isok = OK;
        pilePorteePush("main");
        
        if (stack->sommet != 1)
            isok = FAIL;
        if (stack->taille != 2)
            isok = FAIL;
        if (stack->capacite != 2)
            isok = FAIL;
        if (stack->tables[1] == NULL)
            isok = FAIL;

        CHECK_EQ(isok, OK);
    }

    CREATE_TEST(test_scopeStackPush, "Ajout d'une autre portée"){
        int isok = OK;
        pilePorteePush("func");
        
        if (stack->sommet != 2)
            isok = FAIL;
        if (stack->taille != 3)
            isok = FAIL;
        if (stack->tables[2] == NULL)
            isok = FAIL;

        CHECK_EQ(isok, OK);
    }

    pilePorteeFree();
    END();
}

// Création d'une suite de tests pour la fonction pilePorteePop()
// Cette fonction retire une portée de la pile des portées

BEGIN_SUITE(test_pilePorteePop){
    
    pilePorteeInit();
    pilePortee_t *stack = &pilePortee_g;

    // On empile deux portées et on retire 1
    pilePorteePush("main");
    pilePorteePush("func");

    CREATE_TEST(test_scopeStackPop, "Retrait d'une portée"){
        int isok = OK;
        pilePorteePop();

        if (stack->sommet != 1)
            isok = FAIL;
        if (stack->taille != 2)
            isok = FAIL;
        if (stack->capacite != 3)
            isok = FAIL;

        CHECK_EQ(isok, OK);
    }

    // On ajoute une nouvelle portée
    CREATE_TEST(test_scopeStackPop, "Ajout d'une nouvelle portée"){
        int isok = OK;
        pilePorteePush("func2");

        if (stack->sommet != 3)
            isok = FAIL;
        if (stack->taille != 3)
            isok = FAIL;
        if (stack->capacite != 4)
            isok = FAIL;

        CHECK_EQ(isok, OK);
    }

    pilePorteeFree();
    END();
}

// Création d'une suite de tests pour la fonction pilePorteeSommet()
// Cette fonction récupère la portée du sommet de la pile des portées
BEGIN_SUITE(test_pilePorteeSommet){
    
    pilePorteeInit();
    pilePortee_t *stack = &pilePortee_g;

    // On empile deux portées et on retire 1
    pilePorteePush("main");
    pilePorteePush("func");
    pilePorteePop();

    CREATE_TEST(test_scopeStackTop, "Récupération de la portée"){
        int isok = OK;
        tableSym_t *table = pilePorteeSommet();

        if (table == NULL)
            isok = FAIL;
        if (strcmp(table->portee, "main") != 0)
            isok = FAIL;

        CHECK_EQ(isok, OK);
    }

    pilePorteeFree();
    END();
}

// Création d'une suite de tests pour la fonction pilePorteeAjout()
// Cette fonction ajoute un symbole à la portée du sommet de la pile des portées
BEGIN_SUITE(test_pilePorteeAjout){
    
    pilePorteeInit();
    pilePortee_t *stack = &pilePortee_g;

    // On empile deux portées et on retire 1
    pilePorteePush("main");
    pilePorteePush("func");
    pilePorteePop();

    CREATE_TEST(test_scopeStackAdd, "Ajout d'un symbole"){
        int isok = OK;
        char *nom = strdup("test");
        sym_t *sym = symNouveau(nom);
        pilePorteeAjout(sym);

        tableSym_t *table = pilePorteeSommet();
        sym_t *symGet = tableSymGet(table, sym->nom);

        if (symGet == NULL)
            isok = FAIL;
        if (symGet != sym)
            isok = FAIL;

        CHECK_EQ(isok, OK);
    }

    pilePorteeFree();
    END();
}

// Création d'une suite de tests pour la fonction pilePorteeRecherche()
// Cette fonction récupère le symbole dans la pile des portées
BEGIN_SUITE(test_pilePorteeRecherche){
    
    pilePorteeInit();
    pilePortee_t *stack = &pilePortee_g;

    // On empile deux portées et on retire 1
    char *nom = strdup("test");
    sym_t *sym = symNouveau(nom);
    pilePorteeAjout(sym);
    pilePorteePush("main");
    pilePorteePush("func");
    char *nom2 = strdup("test2");
    sym_t *sym2 = symNouveau(nom2);
    pilePorteeAjout(sym2);
    pilePorteePop();

    CREATE_TEST(test_scopeStackLookup, "Récupération d'un symbole qui existe"){
        int isok = OK;

        sym_t *symGet = pilePorteeRecherche("test");

        if (symGet == NULL)
            isok = FAIL;
        if (symGet != sym)
            isok = FAIL;

        CHECK_EQ(isok, OK);
    }

    CREATE_TEST(test_scopeStackLookup, "Récupération d'un symbole qui n'existe pas"){
        int isok = OK;

        sym_t *symGet = pilePorteeRecherche("test2");

        if (symGet != NULL)
            isok = FAIL;

        CHECK_EQ(isok, OK);
    }

    pilePorteeFree();
    END();
}

// Création d'une suite de tests pour la fonction ajouteListeQuad()
// Cette fonction ajoute un quadruplet à la liste des quadruplets
BEGIN_SUITE(test_ajoutListeQuad){

    struct quadop op0 = { .type = QO_ENTIER, .u.entier = 2 };
    struct quadop op1 = { .type = QO_ENTIER, .u.entier = 3 };
    struct quadop op2 = { .type = QO_ENTIER, .u.entier = 4 };
    struct quadop op3 = { .type = QO_ENTIER, .u.entier = 5 };
    struct quadop op4 = { .type = QO_ENTIER, .u.entier = 6 };
    struct quadop op5 = { .type = QO_ENTIER, .u.entier = 7 };
    struct quad q1 = QUAD_AFF(op0, op1);
    struct quad q2 = QUAD_SOUS(op2, op3, op4);
    struct quad q3 = QUAD_MUL(op4, op5, op0);
    struct quad q4 = QUAD_MOD(op0, op1, op2);
    struct quad q5 = QUAD_EGAL(op2, op3);
    struct listeQuad quads = { .taille = 0, .tailleMax = 0, .qs = NULL };

    CREATE_TEST(test_ajoutListeQuad, "Vérification de l'ajout d'un quadruplet à la liste"){
        int isok = OK;
        ajoutListeQuad(&quads, q1);
        // On vérifie si le quadruplet a bien été ajouté
        if (quads.taille != 1)
            isok = FAIL;
        if (quads.tailleMax != 1)
            isok = FAIL;
        if (quads.qs == NULL)
            isok = FAIL;

        ASSERT_EQ(isok, OK);
    }

    CREATE_TEST(test_ajoutListeQuad, "Vérification des valeurs dans la liste lors de l'ajout"){
        int isok = OK;
        struct quad qGet = quads.qs[0];
        // On vérifie les valeurs du quadruplet dans la liste
        if (qGet.type != Q_AFF)
            isok = FAIL;
        if (qGet.op0.type != QO_ENTIER)
            isok = FAIL;
        if (qGet.op0.u.entier != q1.op0.u.entier)
            isok = FAIL;
        if (qGet.op1.type != QO_ENTIER)
            isok = FAIL;
        if (qGet.op1.u.entier != q1.op1.u.entier)
            isok = FAIL;
        if (qGet.op2.type != QO_VIDE)
            isok = FAIL;

        CHECK_EQ(isok, OK);
    }

    CREATE_TEST(test_ajoutListeQuad, "Vérification de la taille lors de l'ajout de plusieurs quadruplets"){
        struct quad qGet;
        int isok = OK;
        ajoutListeQuad(&quads, q2);
        ajoutListeQuad(&quads, q3);
        ajoutListeQuad(&quads, q4);
        ajoutListeQuad(&quads, q5);
        // On vérifie si la taille de la liste a bien été augmentée
        if (quads.taille != 5)
            isok = FAIL;
        if (quads.tailleMax != 8)
            isok = FAIL;

        CHECK_EQ(isok, OK);
    }

    CREATE_TEST(test_ajoutListeQuad, "Vérification des valeurs dans la liste lors de l'ajout de plusieurs quadruplets"){
        int isok = OK;
        struct quad qGet;
        qGet = quads.qs[1];
        // On vérifie les valeurs du quadruplet dans la liste
        if (qGet.type != Q_SOUS)
            isok = FAIL;
        if (qGet.op0.type != QO_ENTIER)
            isok = FAIL;
        if (qGet.op0.u.entier != q2.op0.u.entier)
            isok = FAIL;
        if (qGet.op1.type != QO_ENTIER)
            isok = FAIL;
        if (qGet.op1.u.entier != q2.op1.u.entier)
            isok = FAIL;
        if (qGet.op2.type != QO_ENTIER)
            isok = FAIL;

        qGet = quads.qs[2];
        if (qGet.type != Q_MUL)
            isok = FAIL;
        if (qGet.op0.type != QO_ENTIER)
            isok = FAIL;
        if (qGet.op0.u.entier != q3.op0.u.entier)
            isok = FAIL;
        if (qGet.op1.type != QO_ENTIER)
            isok = FAIL;
        if (qGet.op1.u.entier != q3.op1.u.entier)
            isok = FAIL;
        if (qGet.op2.type != QO_ENTIER)
            isok = FAIL;
        if (qGet.op2.u.entier != q3.op2.u.entier)
            isok = FAIL;

        qGet = quads.qs[3];
        if (qGet.type != Q_MOD)
            isok = FAIL;
        if (qGet.op0.type != QO_ENTIER)
            isok = FAIL;
        if (qGet.op0.u.entier != q4.op0.u.entier)
            isok = FAIL;
        if (qGet.op1.type != QO_ENTIER)
            isok = FAIL;
        if (qGet.op1.u.entier != q4.op1.u.entier)
            isok = FAIL;
        if (qGet.op2.type != QO_ENTIER)
            isok = FAIL;

        qGet = quads.qs[4];
        if (qGet.type != Q_EGAL)
            isok = FAIL;
        if (qGet.op0.type != QO_ENTIER)
            isok = FAIL;
        if (qGet.op0.u.entier != q5.op0.u.entier)
            isok = FAIL;
        if (qGet.op1.type != QO_ENTIER)
            isok = FAIL;
        if (qGet.op1.u.entier != q5.op1.u.entier)
            isok = FAIL;
        if (qGet.op2.type != QO_VIDE)
            isok = FAIL;
        
        CHECK_EQ(isok, OK);
    }

    listeQuadFree(&quads);
    END();
}

// Création d'une suite de tests pour la fonction nouvelleListeAdresseQuad()
// Cette fonction crée une nouvelle liste d'adresses de quadruplets
BEGIN_SUITE(test_nouvelleListeAdresseQuad){

    struct listeAdresseQuad laq;

    CREATE_TEST(test_nouvelleListeAdresseQuad, "Initialisation d'une liste d'adresses de quadruplets"){
        int isok = OK;
        nouvelleListeAdresseQuad(&laq);
     
        if (laq.taille != 0)
            isok = FAIL;
        if (laq.tailleMax != 0)
            isok = FAIL;
        if (laq.valeurs != NULL)
            isok = FAIL;

        CHECK_EQ(isok, OK);
    }

    END();
}

// Création d'une suite de tests pour la fonction ajouteListeAdresseQuad()
// Cette fonction ajoute une adresse de quadruplet à la liste des adresses de quadruplets
BEGIN_SUITE(test_ajoutListeAdresseQuad){
    struct listeAdresseQuad laq;
    adresseQuad_t aq = 4;


    CREATE_TEST(test_ajoutListeAdresseQuad, "Ajout d'une adresse de quadruplet dans la liste"){
        int isok = OK;
        nouvelleListeAdresseQuad(&laq);
        ajoutListeAdresseQuad(&laq, aq);
     
        if (laq.taille != 1)
            isok = FAIL;
        if (laq.tailleMax != 1)
            isok = FAIL;
        if (laq.valeurs == NULL)
            isok = FAIL;
        if (laq.valeurs[0] != aq)
            isok = FAIL;

        CHECK_EQ(isok, OK);
    }

    CREATE_TEST(test_ajoutListeAdresseQuad, "Ajout de plusieurs adresses de quadruplets dans la liste"){
        int isok = OK;
        nouvelleListeAdresseQuad(&laq);
        ajoutListeAdresseQuad(&laq, aq);
        ajoutListeAdresseQuad(&laq, aq + 4);
     
        if (laq.taille != 2)
            isok = FAIL;
        if (laq.tailleMax != 2)
            isok = FAIL;
        if (laq.valeurs == NULL)
            isok = FAIL;
        if (laq.valeurs[0] != aq)
            isok = FAIL;
        if (laq.valeurs[1] != aq + 4)
            isok = FAIL;

        CHECK_EQ(isok, OK);
    }

    END();
}

// Création d'une suite de tests pour la fonction listeAdresseQuadSeul()
// Cette fonction retourne une liste d'adresses de quadruplets contenant une seule adresse
BEGIN_SUITE(test_listeAdresseQuadSeul){
    struct listeAdresseQuad laq;
    adresseQuad_t aq = 4;

    CREATE_TEST(test_listeAdresseQuadSeul, "Ajout d'une adresse de quadruplet seule dans la liste"){
        int isok = OK;
        nouvelleListeAdresseQuad(&laq);
        listeAdresseQuadSeul(&laq, aq);
     
        if (laq.taille != 1)
            isok = FAIL;
        if (laq.tailleMax != 1)
            isok = FAIL;
        if (laq.valeurs == NULL)
            isok = FAIL;
        if (laq.valeurs[0] != aq)
            isok = FAIL;

        CHECK_EQ(isok, OK);
    }

    END();
}

// Création d'une suite de tests pour la fonction conversionChaineVersEntier;
// Cette fonction retourne une liste d'adresses de quadruplets contenant une seule adresse
BEGIN_SUITE(test_conversionChaineVersEntier){

    CREATE_TEST(test_conversionChaineVersEntier, "Conversion de la chaine \"42\" en entier"){
        char* entier = "42";
        int converti = conversionChaineVersEntier(entier, 0);
     
        CHECK_EQ(converti == 42, OK);
    }

    CREATE_TEST(test_conversionChaineVersEntier, "Conversion de la chaine \"-42\" en entier"){
        char* entier = "-42";
        int estNegatif = entier[0]=='-';
        if(estNegatif)
            entier++;
        int converti = conversionChaineVersEntier(entier, estNegatif);
     
        CHECK_EQ(converti == -42, OK);
    }

    CREATE_TEST(test_conversionChaineVersEntier, "Conversion de la chaine \"2147483647\" en entier"
            " équivalent à INT_MAX"){
        char* entier = "2147483647";
        int estNegatif = entier[0]=='-';
        if(estNegatif)
            entier++;
        int converti = conversionChaineVersEntier(entier, estNegatif);
     
        CHECK_EQ(converti == INT_MAX, OK);
    }

    CREATE_TEST(test_conversionChaineVersEntier, "Conversion de la chaine \"-2147483648\" en entier"
            " équivalent à INT_MIN"){
        char* entier = "-2147483648";
        int estNegatif = entier[0]=='-';
        if(estNegatif)
            entier++;
        int converti = conversionChaineVersEntier(entier, estNegatif);

        CHECK_EQ(converti == INT_MIN, OK);
    }

    END();
}



int main(void){

    // CREATE_SUITE(test_hello, "Test de la fonction hello(), qui affiche Hello World!");
    CREATE_SUITE(test_symNouveau, \
        "Test de la fonction symNouveau(), qui crée un symbole");
    
    CREATE_SUITE(test_symHash, \
        "Test de la fonction symHash(), qui calcule le hash d'un symbole");
    
    CREATE_SUITE(test_tableSymNouveau, \
        "Test de la fonction tableSymNouveau(), qui crée une table de symboles");
    
    CREATE_SUITE(test_tableSymAjout, \
        "Test de la fonction tableSymAjout(), qui ajoute un symbole à une table de symboles");
    
    CREATE_SUITE(test_tableSymGetIndex, \
        "Test de la fonction tableSymGetIndex(), qui récupère l'index d'un symbole à partir de son nom");
    
    CREATE_SUITE(test_tableSymGet, \
        "Test de la fonction tableSymGet(), qui récupère un symbole à partir de son nom");
    
    CREATE_SUITE(test_pilePorteeInit, \
        "Test de la fonction pilePorteeInit(), qui initialise la pile de portées");
    
    CREATE_SUITE(test_pilePorteePush, \
        "Test de la fonction pilePorteePush(), qui ajoute une table de symboles à la pile de portées");
    
    CREATE_SUITE(test_pilePorteePop, \
        "Test de la fonction pilePorteePop(), qui retire une table de symboles à la pile de portées");
    
    CREATE_SUITE(test_pilePorteeSommet, \
        "Test de la fonction pilePorteeSommet(), qui récupère la table de symboles du sommet de la pile de portées");
    
    CREATE_SUITE(test_pilePorteeAjout, \
        "Test de la fonction pilePorteeAjout(), qui ajoute un symbole "
        "à la table de symboles du sommet de la pile de portées");
    
    CREATE_SUITE(test_pilePorteeRecherche, \
        "Test de la fonction pilePorteeRecherche(), qui récupère un symbole "
        "à partir de son nom dans la pile de portées");
    
    CREATE_SUITE(test_ajoutListeQuad, \
        "Test de la fonction ajoutListeQuad(), qui ajoute un quadruplet à la liste de quadruplets");
    
    CREATE_SUITE(test_nouvelleListeAdresseQuad, \
        "Test de la fonction nouvelleListeAdresseQuad(), qui crée une nouvelle liste d'adresses de quadruplets");

    CREATE_SUITE(test_ajoutListeAdresseQuad, \
        "Test de la fonction ajoutListeAdresseQuad(), qui ajoute une adresse de quadruplet à la liste d'adresses de quadruplets");

    CREATE_SUITE(test_listeAdresseQuadSeul, \
        "Test de la fonction listeAdresseQuadSeul(), qui vérifie si une liste d'adresses de quadruplets ne contient qu'une seule adresse");

    CREATE_SUITE(test_conversionChaineVersEntier, \
        "Test de la fonction conversionChaineVersEntier(), qui vérifie que la valeur retourné est bien la conversion en entier la chaine"
            " passé en paramètre");

    return RUN();
}