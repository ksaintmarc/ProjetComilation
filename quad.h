/**
 * @file quad.h
 * @brief Quads et Quadops
 * 
 * Structure de donnée permettant de représenter les quads et les quadops.
 * Ces structures sont utilisées pour représenter les instructions 
 * MIPS afin de pouvoir les générer.
 * 
 */

#ifndef __QUAD_H__
#define __QUAD_H__

#include <stddef.h>

/**
 * @def QUADOP_ENTIER(v)
 * @brief Créer une quadop de type "entier"
 * 
 * @param v : Valeur de l'entier
 * 
 */
#define QUADOP_ENTIER(v) (struct quadop){ .type = QO_ENTIER, \
    .u = { .entier = (v) }, .i = NULL }

/**
 * @def QUADOP_CHAINE(v)
 * @brief Crée une structure quadop de type "chaîne"
 * 
 * @param v : Valeur de l'index de la chaîne
 * 
 */
#define QUADOP_CHAINE(v) (struct quadop){ .type = QO_CHAINE, \
    .u = { .indexChaine = (v) }, .i = NULL }

/**
 * @def QUADOP_ID(v)
 * @brief Crée une structure quadop de type "identifiant"
 * 
 * @param v : Valeur de l'identifiant
 * 
 */
#define QUADOP_ID(v) (struct quadop){ .type = QO_ID, \
    .u = { .id = (v) }, .i = NULL }

/**
 * @def QUADOP_ELEMENT_TABLEAU(v, index)
 * @brief Crée une structure quadop de type "élément de tableau"
 * 
 * @param v : Valeur de l'identifiant du tableau
 * @param index : Quadop d'indexation
 * 
 */
#define QUADOP_ELEMENT_TABLEAU(v, index) (struct quadop){ \
    .type = QO_ELEMENT_TABLEAU, .u = { .id = (v) }, .i = (index) }

/**
 * @def QUADOP_ARG(v)
 * @brief Crée une structure quadop de type "argument"
 * 
 * @param v : Indice de l'argument
 * 
 */
#define QUADOP_ARG(v) (struct quadop){ .type = QO_ARG, \
    .u = { .entier = (v) }, .i = NULL }

/**
 * @def QUADOP_GENERIQUE_ADRESSE(_t, v)
 * @brief Crée une structure quadop générique de type "adresse"
 * 
 * @param _t : Type de l'adresse
 * @param v : Valeur de l'adresse
 * 
 */
#define QUADOP_GENERIQUE_ADRESSE(_t, v) (struct quadop){ .type = (_t), \
    .u = { .adresse = (v) }, .i = NULL }

/**
 * @def QUADOP_VIDE
 * @brief Crée une structure quadop de type "vide"
 * @note Un quadop de type "vide" est une quadop de type "adresse" dont la valeur est 0
 * 
 */
#define QUADOP_VIDE QUADOP_GENERIQUE_ADRESSE(QO_VIDE, 0)

/**
 * @def QUADOP_ADRESSE(v)
 * @brief Crée une structure quadop de type "adresse"
 * 
 * @param v : Valeur de l'adresse
 * 
 */
#define QUADOP_ADRESSE(v) QUADOP_GENERIQUE_ADRESSE(QO_ADRESSE, (v))

/**
 * @typedef adresseQuad_t
 * @brief Type d'une adresse de quad
 * 
 */
typedef size_t adresseQuad_t;

/**
 * @enum quadopType
 * @brief Type d'un quadop
 * 
 * @var QO_ENTIER
 * @brief Quadop de type "entier"
 * 
 * @var QO_CHAINE
 * @brief Quadop de type "chaîne"
 * 
 * @var QO_ID
 * @brief Quadop de type "identifiant"
 * 
 * @var QO_ELEMENT_TABLEAU
 * @brief Quadop de type "élément de tableau"
 * 
 * @var QO_ARG
 * @brief Quadop de type "argument"
 * 
 * @var QO_ADRESSE
 * @brief Quadop de type "adresse"
 * 
 * @var QO_VIDE
 * @brief Quadop de type "vide"
 * 
 */
enum quadopType {
    QO_ENTIER,
    QO_CHAINE,
    QO_ID,
    QO_ELEMENT_TABLEAU,
    QO_ARG,
    QO_ADRESSE,
    QO_VIDE
};

/**
 * @struct quadop
 * @brief Structure d'une quadop
 * 
 * @var quadop::type
 * @brief Type du quadop
 * 
 * @var quadop::u
 * @brief Valeur du quadop en fonction de son type
 * 
 * @var quadop::@0::entier
 * @brief Valeur du quadop de type "entier" ou "argument"
 * 
 * @var quadop::@0::indexChaine
 * @brief L'index du quadop de type "chaîne"
 * 
 * @var quadop::@0::id
 * @brief Valeur du quadop de type "identifiant", ou du tableau pour élément
 * 
 * @var quadop::@0::adresse
 * @brief Valeur du quadop de type "adresse"
 * 
 * @var quadop::i
 * @brief Quadop d'indexation pour le type "élément de tableau"
 * 
 */
struct quadop {
    enum quadopType type;
    union {
        int entier;
        size_t indexChaine;
        char* id;
        adresseQuad_t adresse;
    } u;
    struct quadop* i;
};

/**
 * @def QUAD_GENERIQUE(_t, a, b, c)
 * @brief Crée une structure quad qui est générique pour toutes les opérations
 * 
 * @param _t : Type du quad
 * @param a : Opérande 0
 * @param b : Opérande 1
 * @param c : Opérande 2
 * 
 */
#define QUAD_GENERIQUE(_t, a, b, c) (struct quad){ .type = (_t), \
    .op0 = (a), .op1 = (b), .op2 = (c) }

/**
 * @def QUAD_AFF(a, b)
 * @brief Crée une structure quad qui fait l'affectation de deux opérandes
 * 
 * @param a : Opérande 0
 * @param b : Opérande 1
 * 
 */
#define QUAD_AFF(a, b) QUAD_GENERIQUE(Q_AFF, (a), (b), QUADOP_VIDE)

/**
 * @def QUAD_ADD(a, b, c)
 * @brief Crée une structure quad qui fait l'addition
 * 
 * @param a : Opérande 0
 * @param b : Opérande 1
 * @param c : Opérande 2
 * 
 */
#define QUAD_ADD(a, b, c) QUAD_GENERIQUE(Q_ADD, (a), (b), (c))

/**
 * @def QUAD_SOUS(a, b, c)
 * @brief Crée une structure quad qui fait la soustraction
 * 
 * @param a : Opérande 0
 * @param b : Opérande 1
 * @param c : Opérande 2
 * 
 */
#define QUAD_SOUS(a, b, c) QUAD_GENERIQUE(Q_SOUS, (a), (b), (c))

/**
 * @def QUAD_MUL(a, b, c)
 * @brief Crée une structure quad qui fait la multiplication
 * 
 * @param a : Opérande 0
 * @param b : Opérande 1
 * @param c : Opérande 2
 * 
 */
#define QUAD_MUL(a, b, c) QUAD_GENERIQUE(Q_MUL, (a), (b), (c))

/**
 * @def QUAD_DIV(a, b, c)
 * @brief Crée une structure quad qui fait la division
 * 
 * @param a : Opérande 0
 * @param b : Opérande 1
 * @param c : Opérande 2
 * 
 */
#define QUAD_DIV(a, b, c) QUAD_GENERIQUE(Q_DIV, (a), (b), (c))

/**
 * @def QUAD_MOD(a, b)
 * @brief Crée une structure quad qui fait le modulo
 * 
 * @param a : Opérande 0
 * @param b : Opérande 1
 * 
 */
#define QUAD_MOD(a, b, c) QUAD_GENERIQUE(Q_MOD, (a), (b), (c))

/**
 * @def QUAD_CHAINE_EGAL(a, b)
 * @brief Crée une structure quad qui vérifie l'égalité entre les deux concatenations
 * 
 * @param a : Opérande 0
 * @param b : Opérande 1
 * 
 */
#define QUAD_CHAINE_EGAL(a, b) QUAD_GENERIQUE(Q_CHAINE_EGAL, (a), (b), QUADOP_VIDE)

/**
 * @def QUAD_CHAINE_NEGAL(a, b)
 * @brief Crée une structure quad qui vérifie l'inégalité entre les deux concatenations
 * 
 * @param a : Opérande 0
 * @param b : Opérande 1
 * 
 */
#define QUAD_CHAINE_NEGAL(a, b) QUAD_GENERIQUE(Q_CHAINE_NEGAL, (a), (b), QUADOP_VIDE)

/**
 * @def QUAD_EGAL(a, b)
 * @brief Crée une structure quad qui vérifie l'égalité entre les deux opérandes
 * 
 * @param a : Opérande 0
 * @param b : Opérande 1
 * 
 */
#define QUAD_EGAL(a, b) QUAD_GENERIQUE(Q_EGAL, (a), (b), QUADOP_VIDE)

/**
 * @def QUAD_NEGAL(a, b)
 * @brief Crée une structure quad qui vérifie l'inégalité entre les deux opérandes
 * 
 * @param a : Opérande 0
 * @param b : Opérande 1
 * 
 */
#define QUAD_NEGAL(a, b) QUAD_GENERIQUE(Q_NEGAL, (a), (b), QUADOP_VIDE)

/**
 * @def QUAD_INF(a, b)
 * @brief Crée une structure quad qui vérifie si l'opérande a 
 * est strictement inférieur à l'opérande b
 * 
 * @param a : Opérande 0
 * @param b : Opérande 1
 * 
 */
#define QUAD_INF(a, b) QUAD_GENERIQUE(Q_INF, (a), (b), QUADOP_VIDE)

/**
 * @def QUAD_INFEGAL(a, b)
 * @brief Crée une structure quad qui  vérifie si l'opérande a 
 * est inférieur ou égal à l'opérande b
 * 
 * @param a : Opérande 0
 * @param b : Opérande 1
 * 
 */
#define QUAD_INFEGAL(a, b) QUAD_GENERIQUE(Q_INFEGAL, (a), (b), QUADOP_VIDE)

/**
 * @def QUAD_SUP(a, b)
 * @brief Crée une structure quad qui vérifie si l'opérande a 
 * est strictement supérieur à l'opérande b
 * 
 * @param a : Opérande 0
 * @param b : Opérande 1
 * 
 */
#define QUAD_SUP(a, b) QUAD_GENERIQUE(Q_SUP, (a), (b), QUADOP_VIDE)

/**
 * @def QUAD_SUPEGAL(a, b)
 * @brief Crée une structure quad qui vérifie si l'opérande a 
 * est supérieur ou égal à l'opérande b
 * 
 * @param a : Opérande 0
 * @param b : Opérande 1
 * 
 */
#define QUAD_SUPEGAL(a, b) QUAD_GENERIQUE(Q_SUPEGAL, (a), (b), QUADOP_VIDE)

/**
 * @def QUAD_GOTO()
 * @brief Crée une structure quad qui fait un saut vers une autre instruction
 * 
 * @note Un quad goto n'est pas initialisé de base, elle pointe vers 
 * une autre instruction que après un appel à generationCode()
 * 
 */
#define QUAD_GOTO() QUAD_GENERIQUE(Q_GOTO, QUADOP_VIDE, QUADOP_VIDE, QUADOP_VIDE)


/**
 * @def QUAD_GOTO_ADDR(a)
 * @brief Crée une structure quad qui fait un saut vers une autre instruction
 * utilisant une adresse connue
 * 
 */
#define QUAD_GOTO_ADDR(a) QUAD_GENERIQUE(Q_GOTO, (a), QUADOP_VIDE, QUADOP_VIDE)


/**
 * @def QUAD_CONV_E_V_C(a, b)
 * @brief Crée une structure quad qui convertit un entier en chaîne de caractère
 * 
 * @param a : Opérande 0
 * @param b : Opérande 1
 * 
 */
#define QUAD_CONV_E_V_C(a, b) QUAD_GENERIQUE(Q_CONV_ENTIER_VERS_CHAINE, (a), (b), QUADOP_VIDE)

/**
 * @def QUAD_CONV_C_V_E(a, b)
 * @brief Crée une structure quad qui convertit une chaîne de caractère en entier
 * 
 * @param a : Opérande 0
 * @param b : Opérande 1
 * 
 */
#define QUAD_CONV_C_V_E(a, b) QUAD_GENERIQUE(Q_CONV_CHAINE_VERS_ENTIER, (a), (b), QUADOP_VIDE)

/**
 * @def QUAD_CONCAT(a, b, c)
 * @brief Crée une structure quad qui concatène deux chaînes de caractères
 * 
 * @param a : Opérande 0
 * @param b : Opérande 1
 * @param c : Opérande 2
 * 
 */
#define QUAD_CONCAT(a, b, c) QUAD_GENERIQUE(Q_CONCAT, (a), (b), (c))

/**
 * @def QUAD_READ(a)
 * @brief Crée une structure quad qui lit l'opérande a
 * 
 * @param a : Opérande 0
 * 
 */
#define QUAD_READ(a) QUAD_GENERIQUE(Q_READ, (a), QUADOP_VIDE, QUADOP_VIDE)

/**
 * @def QUAD_ECHO(a)
 * @brief Crée une structure quad qui affiche les opérandes situées dans la pile
 * 
 * @param a : Nombre d'opérandes
 * 
 */
#define QUAD_ECHO(a) QUAD_GENERIQUE(Q_ECHO, (a), QUADOP_VIDE, QUADOP_VIDE)

/**
 * @def QUAD_EXIT(a)
 * @brief Crée une structure quad qui quitte le programme avec un code de retour
 * 
 * @param a : Opérande 0
 * 
 */
#define QUAD_EXIT(a) QUAD_GENERIQUE(Q_EXIT, (a), QUADOP_VIDE, QUADOP_VIDE)

/**
 * @def QUAD_EXIT_VIDE()
 * @brief Crée une structure quad qui quitte le programme
 * 
 * @note Cette macro est utilisée pour quitter le programme sans code de retour
 * 
 */
#define QUAD_EXIT_VIDE() QUAD_GENERIQUE(Q_EXIT, QUADOP_VIDE, QUADOP_VIDE, QUADOP_VIDE)

/**
 * @def QUAD_STACK(a)
 * @brief Crée une structure quad qui permet d'augmenter la taille de la pile
 * 
 * @param a : La taille à ajouter à la pile
 * 
 */
#define QUAD_STACK(a) QUAD_GENERIQUE(Q_STACK, (a), QUADOP_VIDE, QUADOP_VIDE)

/**
 * @def QUAD_CHARGE_ARG(a, b)
 * @brief Crée une structure quad qui permet de lire dans la pile un argument à une certaine position
 * 
 * @param a : La variable dans laquelle charger l'argument
 * @param b : L'index de l'argument à charger
 * 
 */
#define QUAD_CHARGE_ARG(a, b) QUAD_GENERIQUE(Q_CHARGE_ARG, (a), (b), QUADOP_VIDE)

/**
 * @def QUAD_PASSE_ARG(a, b)
 * @brief Crée une structure quad qui permet d'ajouter à la pile un argument à une certaine position
 * 
 * @param a : L'argument à passer
 * @param b : L'index de l'argument
 * 
 */
#define QUAD_PASSE_ARG(a, b) QUAD_GENERIQUE(Q_PASSE_ARG, (a), (b), QUADOP_VIDE)

/**
 * @def QUAD_PASSE_TAB(a, b, c)
 * @brief Crée une structure quad qui permet d'ajouter à la pile un tableau d'une certaine taille à une certaine position
 * 
 * @param a : Le tableau à passer en argument
 * @param b : L'index à partir duquel passer le tableau
 * @param c : La taille du tableau
 * 
 */
#define QUAD_PASSE_TAB(a, b, c) QUAD_GENERIQUE(Q_PASSE_TAB, (a), (b), (c))

/**
 * @def QUAD_ALLARGS(a)
 * @brief Crée une structure quad qui permet de créer la concaténation avec espaces des arguments
 * 
 * @param a : L'opérande résultat dans laquelle stocker la chaîne produite
 * 
 */
#define QUAD_ALLARGS(a) QUAD_GENERIQUE(Q_ALLARGS, (a), QUADOP_VIDE, QUADOP_VIDE)

/**
 * @def QUAD_CHAINE_VIDE(a)
 * @brief Crée une structure quad qui permet de tester si une chaîne est vide
 * 
 * @param a : La chaîne à tester
 * 
 */
#define QUAD_CHAINE_VIDE(a) QUAD_GENERIQUE(Q_CHAINE_VIDE, (a), QUADOP_VIDE, QUADOP_VIDE)

/**
 * @def QUAD_CHAINE_NON_VIDE(a)
 * @brief Crée une structure quad qui permet de tester si une chaîne est non vide
 * 
 * @param a : La chaîne à tester
 * 
 */
#define QUAD_CHAINE_NON_VIDE(a) QUAD_GENERIQUE(Q_CHAINE_NON_VIDE, (a), QUADOP_VIDE, QUADOP_VIDE)

/**
 * @enum quadType
 * @brief Type d'instruction pour les quads
 * 
 * @note Les quads sont des instructions intermédiaires qui sont générées
 * par le compilateur et qui sont ensuite traduites en code assembleur
 * 
 * @var Q_AFF
 * @brief Opération d'affectation
 * 
 * @var Q_ADD
 * @brief Opération d'addition
 * 
 * @var Q_SOUS
 * @brief Opération de soustraction
 * 
 * @var Q_MUL
 * @brief Opération de multiplication
 * 
 * @var Q_DIV
 * @brief Opération de division
 * 
 * @var Q_MOD
 * @brief Opération de modulo
 * 
 * @var Q_CHAINE_EGAL
 * @brief Opération de comparaison d'égalité de chaines
 * 
 * @var Q_CHAINE_NEGAL
 * @brief Opération de comparaison d'inégalité de chaines
 * 
 * @var Q_EGAL
 * @brief Opération de comparaison d'égalité d'entiers
 * 
 * @var Q_NEGAL
 * @brief Opération de comparaison d'inégalité d'entiers
 * 
 * @var Q_INF
 * @brief Opération de comparaison strictement inférieur
 * 
 * @var Q_INFEGAL
 * @brief Opération de comparaison inférieur ou égal
 * 
 * @var Q_SUP
 * @brief Opération de comparaison strictement supérieur
 * 
 * @var Q_SUPEGAL
 * @brief Opération de comparaison supérieur ou égal
 * 
 * @var Q_GOTO
 * @brief Opération de saut vers une autre instruction
 * 
 * @var Q_CONV_ENTIER_VERS_CHAINE
 * @brief Opération de conversion d'un entier en chaîne de caractère
 * 
 * @var Q_CONV_CHAINE_VERS_ENTIER
 * @brief Opération de conversion d'une chaîne de caractère en entier
 * 
 * @var Q_CONCAT
 * @brief Opération de concaténation entre deux chaînes de caractères
 * 
 * @var Q_READ
 * @brief Opération de lecture
 * 
 * @var Q_ECHO
 * @brief Opération d'affichage
 * 
 * @var Q_EXIT
 * @brief Opération de sortie du programme
 * 
 * @var Q_STACK
 * @brief Opération d'augmentation de la taille de la pile
 * 
 * @var Q_CHARGE_ARG
 * @brief Opération de chargement d'un argument depuis la pile
 * 
 * @var Q_PASSE_ARG
 * @brief Opération d'ajout d'un argument dans la pile
 * 
 * @var Q_PASSE_TAB
 * @brief Opération d'ajout d'un tableau dans la pile
 * 
 * @var Q_ALLARGS
 * @brief Opération de création de la concaténation avec espaces des arguments
 * 
 * @var Q_CHAINE_VIDE
 * @brief Opération de test si une chaîne est vide
 * 
 * @var Q_CHAINE_NON_VIDE
 * @brief Opération de test si une chaîne est non vide
 * 
 */
enum quadType {
    Q_AFF,
    Q_ADD,
    Q_SOUS,
    Q_MUL,
    Q_DIV,
    Q_MOD,
    Q_CHAINE_EGAL,
    Q_CHAINE_NEGAL,
    Q_EGAL,
    Q_NEGAL,
    Q_INF,
    Q_INFEGAL,
    Q_SUP,
    Q_SUPEGAL,
    Q_CHAINE_VIDE,
    Q_CHAINE_NON_VIDE,
    Q_GOTO,
    Q_CONV_ENTIER_VERS_CHAINE,
    Q_CONV_CHAINE_VERS_ENTIER,
    Q_CONCAT,
    Q_STACK,
    Q_CHARGE_ARG,
    Q_PASSE_ARG,
    Q_PASSE_TAB,
    Q_ALLARGS,
    Q_ECHO,
    Q_READ,
    Q_EXIT
};

/**
 * @struct quad
 * @brief Structure d'une instruction intermédiaire
 * 
 * @var quad::type
 * @brief Type de l'instruction
 * 
 * @var quad::op0
 * @brief Opérande 0
 * 
 * @var quad::op1
 * @brief Opérande 1
 * 
 * @var quad::op2
 * @brief Opérande 2
 * 
 */
struct quad {
    enum quadType type;
    struct quadop op0;
    struct quadop op1;
    struct quadop op2;
};

/**
 * @struct listeQuad
 * @brief Structure d'une liste de quads
 * 
 * @var listeQuad::taille
 * @brief Nombre d'éléments dans la liste
 * 
 * @var listeQuad::tailleMax
 * @brief Taille maximale de la liste
 * 
 * @var listeQuad::qs
 * @brief Tableau de quads
 * 
 */
struct listeQuad {
    size_t taille;
    size_t tailleMax;
    struct quad* qs;
};

/**
 * @struct listeAdresseQuad
 * @brief Structure d'une liste d'adresses de quads
 * 
 * @var listeAdresseQuad::taille
 * @brief Nombre d'éléments dans la liste
 * 
 * @var listeAdresseQuad::tailleMax
 * @brief Taille maximale de la liste
 * 
 * @var listeAdresseQuad::valeurs
 * @brief Tableau d'adresses de quads
 * 
 */
struct listeAdresseQuad {
    size_t taille;
    size_t tailleMax;
    adresseQuad_t* valeurs;
};

/**
 * @brief Ajoute un quad à la liste des quads
 * 
 * @param l : Liste de quads
 * @param q : Quad à ajouter
 * @return La liste de quads
 */
struct listeQuad* ajoutListeQuad(struct listeQuad* l, struct quad q);

/**
 * @brief Libère en mémoire la liste des quads
 * 
 * @param l : Liste de quads
 * 
 */
void listeQuadFree(struct listeQuad* l);

/**
 * @brief Crée une nouvelle liste d'adresses de quads
 * 
 * @param l : Liste d'adresses de quads à copier
 * @return Une nouvelle liste d'adresses de quads
 * 
 */
struct listeAdresseQuad* nouvelleListeAdresseQuad(struct listeAdresseQuad* l);

/**
 * @brief Ajoute une adresse de quad à la liste d'adresses de quads
 * 
 * @param l : Liste d'adresses de quads
 * @param adresse : Adresse de quad à ajouter
 * @return La liste d'adresses de quads
 * 
 */
struct listeAdresseQuad* ajoutListeAdresseQuad(
        struct listeAdresseQuad* l, adresseQuad_t adresse);

/**
 * @brief Ajoute une adresse de quad à la liste d'adresses de quads
 * 
 * @param l : Liste d'adresses de quads
 * @param adresse : Adresse de quad à ajouter
 * @return La liste d'adresses de quads
 * 
 */
struct listeAdresseQuad* listeAdresseQuadSeul(struct listeAdresseQuad* l, adresseQuad_t adresse);

/**
 * @brief Concatène deux listes d'adresses de quads puis libère les deux listes
 * 
 * @param l0 : Première liste d'adresses de quads
 * @param l1 : Deuxième liste d'adresses de quads
 * @param lResultat : Une liste d'adresses de quads
 * @return La liste d'adresses de quads lResultat plus les deux listes concaténées
 *
 */
struct listeAdresseQuad* concatListeAdresseQuadFree(
        struct listeAdresseQuad* l0, struct listeAdresseQuad* l1,
        struct listeAdresseQuad* lResultat);

/**
 * @brief Concatène trois listes d'adresses de quads puis libère les trois listes
 * 
 * @param l0 : Première liste d'adresses de quads
 * @param l1 : Deuxième liste d'adresses de quads
 * @param l2 : Troisième liste d'adresses de quads
 * @param lResultat  : Une liste d'adresses de quads
 * @return La liste d'adresses de quads lResultat plus les trois listes concaténées
 *
 */
struct listeAdresseQuad* concat3ListeAdresseQuadFree(
        struct listeAdresseQuad* l0, struct listeAdresseQuad* l1,
        struct listeAdresseQuad* l2, struct listeAdresseQuad* lResultat);

/**
 * @brief Affiche une liste d'adresses de quads
 * 
 * @param l : Liste d'adresses de quads
 * 
 */
void afficherListeAdresseQuad(struct listeAdresseQuad* l);

/**
 * @brief Libère en mémoire la liste d'adresses de quads
 * 
 * @param l : Liste d'adresses de quads
 * 
 */
void listeAdresseQuadFree(struct listeAdresseQuad* l);

#endif
