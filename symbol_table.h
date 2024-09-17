/**
 * @file symbol_table.h
 * @brief Table des symboles
 * 
 * Structure de donnée permet de gérer une table des symboles,
 * utilisée lors de l'analyse lexicale, l'analyse syntaxique,
 * l'analyse sémantique et la génération de code.
 * 
 */

#ifndef __SYMBOL_TABLE_H__
#define __SYMBOL_TABLE_H__

#include "check.h"

/**
 * @def MAX_SYMBOLES
 * @brief Nombre maximum de symboles dans une table
 * 
 */
#define MAX_SYMBOLES 256

/**
 * @def MAX_PORTEE
 * @brief Nombre maximum de portées dans la pile
 * 
 */
#define MAX_PORTEE   256

/**
 * @enum sym_type_e
 * @brief Type d'un symbole
 * 
 * @typedef sym_type_t
 * @brief Type d'un symbole
 * 
 * @var S_ENTIER
 * @brief Symbole de type "entier"
 * 
 * @var S_CHAINE
 * @brief Symbole de type "chaîne de caractères"
 * 
 * @var S_FONCTION
 * @brief Symbole de type "fonction"
 * 
 * @var S_TABLEAU
 * @brief Symbole de type "tableau"
 * 
 * @var S_VIDE
 * @brief Symbole de type "vide"
 * 
 */
typedef enum sym_type_e {
    S_ENTIER,
    S_CHAINE,
    S_FONCTION,
    S_TABLEAU,
    S_VIDE,
} sym_type_t;

/**
 * @struct sym_s
 * @brief Structure d'un symbole
 * 
 * Cette structure est utilisée pour représenter un symbole.
 * Un symbole est une variable, une fonction ou un tableau trouvée
 * lors de l'analyse lexicale et l'analyse syntaxique.
 * 
 * @typedef sym_t
 * @brief Structure d'un symbole
 * 
 * @var sym_s::nom
 * @brief Nom du symbole
 * 
 * @var sym_s::portee
 * @brief Portée du symbole
 * 
 * @var sym_s::ligneDeclaration
 * @brief Ligne de déclaration du symbole
 * 
 * @var sym_s::tailleTableau
 * @brief Taille du tableau (si le symbole est un tableau)
 * 
 * @var sym_s::nombreArgs
 * @brief Nombre d'arguments (si le symbole est une fonction)
 * 
 * @var sym_s::valeurRetour
 * @brief Valeur de retour (si le symbole est une fonction)
 * 
 * @var sym_s::type
 * @brief Type du symbole
 * 
 */
typedef struct sym_s {
    char *nom;
    char *portee;
    int ligneDeclaration;
    int tailleTableau;
    int nombreArgs;
    int valeurRetour;
    sym_type_t type;
} sym_t;

/**
 * @struct tableSym_s
 * @brief Structure d'une table de symboles
 * 
 * Cette structure est utilisée pour représenter une table de symboles.
 * Une table de symboles est une liste de symboles qui sont dans la même
 * portée.
 * 
 * @typedef tableSym_t
 * @brief Structure d'une table de symboles
 * 
 * @var tableSym_s::symboles
 * @brief Tableau de symboles
 * 
 * @var tableSym_s::portee
 * @brief Portée de la table
 * 
 * @var tableSym_s::taille
 * @brief Nombre de symboles dans la table
 * 
 * @var tableSym_s::capacite
 * @brief Capacité de la table
 * 
 * @var tableSym_s::precedent
 * @brief Indice de la table précédente
 * @note Cette variable est utilisée pour la gestion de la pile de portée. <br>
 * On peut récupèrer la table de symboles précédente en utilisant la variable
 * globale comme ceci : `pilePortee_g.tables[tableSym.precedent]`
 */
typedef struct tableSym_s {
    sym_t *symboles[MAX_SYMBOLES];
    char *portee;
    int taille;
    int capacite;
    int precedent;
} tableSym_t;

/**
 * @struct pilePortee_s
 * @brief Structure d'une pile de portées
 * 
 * Cette structure est utilisée pour représenter une pile de portées.
 * Une pile de portées est une liste de tables de symboles qui sont dans
 * des portées différentes. <br>
 * La pile de portée est utilisée pour gérer les variables locales et
 * globales des différentes structures de contrôle et fonctions. <br>
 * Comme la structure est conçue comme un arbre, cela permet de stocker
 * en mémoire toutes les portées du programme, et qu'on désalloue pas
 * les tables de symboles lorsqu'on enlève une table de la pile. <br>
 * Cela permet de pouvoir afficher la table de symboles à la fin du programme.
 * 
 * @typedef pilePortee_t
 * @brief Structure d'une pile de portées
 * 
 * @var pilePortee_s::tables
 * @brief Tableau de tables de symboles
 * 
 * @var pilePortee_s::sommet
 * @brief Indice de la table de symboles du sommet de la pile
 * 
 * @var pilePortee_s::taille
 * @brief Nombre de tables de symboles dans la pile
 * 
 * @var pilePortee_s::capacite
 * @brief Nombre total de tables de symboles ajoutées
 * 
 */
typedef struct pilePortee_s {
    tableSym_t *tables[MAX_PORTEE];
    int sommet;
    int taille;
    int capacite;
} pilePortee_t;

/**
 * @var pilePortee_g
 * @brief Variable globale contenant toutes les tables de symboles. 
 * @note La portée globale a comme indice 0 et son précédent -1 dans la pile de portées. <br>
 * Pour récupèrer la table de symboles précédente, on peut utiliser
 * la variable `precedent` de la structure `tableSym_t`. Cela permet de
 * récupèrer la table de symboles pré   cédente en utilisant la variable
 * globale comme ceci : `pilePortee_g.tables[tableSym.precedent]`.
 * @see tableSym_t
 * 
 */
extern pilePortee_t pilePortee_g;

// Opérations sur les symboles
/**
 * @brief Crée un nouveau symbole
 * 
 * @param nom : Nom du symbole
 * @return Un symbole nouvellement créé
 * 
 */
sym_t *symNouveau(char *nom);

/**
 * @brief Renvoie le hash associé à un nom de symbole
 * 
 * @param nom : Nom du symbole
 * @return Le hash associé au nom
 * 
 */
int symHash(char *nom);

/**
 * @brief Affiche un symbole
 * 
 * @param sym : Symbole à afficher
 * 
 */
void symAffichage(sym_t *sym);

/**
 * @brief Libère en mémoire un symbole
 * 
 * @param sym : Symbole à libérer
 * 
 */
void symFree(sym_t *sym);

// Opérations sur la table des symboles
/**
 * @brief Crée une nouvelle table de symboles
 * 
 * @param portee : Nom de la portée de la table
 * @return Une table de symboles nouvellement créée
 * 
 */
tableSym_t *tableSymNouveau(char *portee);

/**
 * @brief Ajoute un symbole à une table de symboles
 * 
 * @param table : Table de symboles
 * @param sym : Symbole à ajouter
 * 
 */
void tableSymAjout(tableSym_t *table, sym_t *sym);

/**
 * @brief Recherche un symbole dans une table de symboles
 * 
 * @param table : Table de symboles
 * @param nom : Nom du symbole à rechercher
 * @return Le symbole trouvé ou NULL si non trouvé
 * 
 */
sym_t *tableSymGet(tableSym_t *table, char *nom);

/**
 * @brief Recherche un symbole dans une table de symboles
 * 
 * @param table : Table de symboles
 * @param nom : Nom du symbole à rechercher
 * @return L'indice du symbole trouvé ou -1 si non trouvé
 * 
 */
int tableSymGetIndex(tableSym_t *table, char *nom);

/**
 * @brief Affiche une table de symboles
 * 
 * @param table : Table de symboles à afficher
 * @param printTop : Affiche le haut de la table
 * @param printBottom : Affiche le bas de la table
 * 
 */
void tableSymAffichage(tableSym_t *table, int printTop, int printBottom);

/**
 * @brief Libère en mémoire une table de symboles
 * 
 * @param table : Table de symboles à libérer
 * 
 */
void tableSymFree(tableSym_t *table);

// Opérations sur les piles de portées
/**
 * @brief Initialise la pile de portée
 * 
 */
void pilePorteeInit(void);

/**
 * @brief Ajoute une nouvelle portée à la pile
 * 
 * @param nom : Nom de la portée
 * 
 */
void pilePorteePush(char *nom);

/**
 * @brief Enlève la portée du sommet de la pile
 * 
 */
void pilePorteePop(void);

/**
 * @brief Renvoie la table de symboles du sommet de la pile
 * 
 * @return La table de symboles du sommet de la pile
 * 
 * @note Si la seule portée présente est la portée globale, la fonction
 * échoue et quitte le programme. <br>
 * La portée globale a comme indice 0 et son précédent -1 dans la pile de portée.
 * 
 */
tableSym_t *pilePorteeSommet(void);

/**
 * @brief Renvoie la table de symboles à la racine de la pile (globale)
 * 
 * @return La table de symboles à la racine de la pile (globale)
 * 
 */
tableSym_t *pilePorteeRacine(void);

/**
 * @brief Ajoute un symbole à la table de symboles du sommet de la pile
 * 
 * @param sym : Symbole à ajouter
 * 
 */
void pilePorteeAjout(sym_t *sym);

/**
 * @brief Recherche un symbole dans la pile de portée
 * 
 * @param nom : Nom du symbole à rechercher
 * @return Le symbole trouvé ou NULL si non trouvé
 * 
 */
sym_t *pilePorteeRecherche(char *nom);

/**
 * @brief Affiche l'ensemble des tables à l'état actuel de son éxecution.
 * 
 * @note Contrairement au nom de la fonction, cette fonction n'affiche
 * pas la l'état de la pile de portée lors de la compilation. <br>
 * Elle affiche toutes les tables (portées) qu'elle a enregistré, grâce à une implémentation
 * sous forme d'un arbre, malgré qu'elle soit utilisée comme une pile.
 * 
 */
void pilePorteeAffichage(void);

/**
 * @brief Libère en mémoire la pile de portée
 * 
 */
void pilePorteeFree(void);

#endif