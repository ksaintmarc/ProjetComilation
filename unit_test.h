/**
 * @file unit_test.h
 * @brief Librairie de tests unitaires
 * 
 * Cette librairie permet de créer des tests unitaires.
 * On peut définir une série de tests pour une fonction donnée.
 * 
 */

#ifndef __UNIT_TEST_H__
#define __UNIT_TEST_H__

/**
 * @def _MAX_TESTCASES
 * @brief Nombre maximum de tests par suite
 * 
 */
#define _MAX_TESTCASES 100

// Override de exit()
// #define exit my_exit
// #undef exit  

/**
 * @def ERR
 * @brief Valeur de retour en cas d'erreur interne
 * 
 */
#define ERR -2

/**
 * @def EXIT
 * @brief Valeur de retour en cas d'erreur dans un test suite à un assert
 * 
 */
#define EXIT -1

/**
 * @def FAIL
 * @brief Valeur de retour en cas d'échec d'une suite de tests
 * 
 */
#define FAIL 0

/**
 * @def OK
 * @brief Valeur de retour en cas de succès d'une suite de tests
 * 
 */
#define OK 1

// Macros pour les tests
/**
 * @def BEGIN_SUITE(x)
 * @brief Définit une suite de tests
 * 
 * @param x : Nom de la suite
 * 
 */
#define BEGIN_SUITE(x) \
    int _testSuite_##x(suite_t *currSuite)

/**
 * @def END()
 * @brief Détermine la fin d'une suite de tests
 * @note Si la suite de tests est vide, La fonction de tests associé échoue.
 * 
 */
#define END() \
    if (currSuite->nbFails > 0 || currSuite->nbTests == 0) { \
        if (currSuite->nbTests == 0) { \
            fprintf(stdout, "       ❌ ERREUR : Aucun test n'a été défini ! \n"); \
            currSuite->nbFails++; \
        } \
        return FAIL; \
    } else { \
        return OK; \
    } \

/**
 * @def RUN()
 * @brief Exécute les tests
 */
#define RUN() \
    run()

/**
 * @def CREATE_SUITE(x, y)
 * @brief Crée une suite de tests associée à une fonction de test
 * 
 * @param x : Nom de la suite
 * @param y : Description de la suite
 * 
 */
#define CREATE_SUITE(x, y) \
    suite_t *_suite_##x = createSuite(#x, y); \
    _status.tests[_status.nbSuites] = _suite_##x; \
    _status.func[_status.nbSuites] = _testSuite_##x; \
    _status.nbSuites++

/**
 * @def CREATE_TEST(x, y)
 * @brief Crée un test dans la suite de tests courante
 * 
 * @param x : Nom du test
 * @param y : Description du test
 * 
 */
#define CREATE_TEST(x, y) \
    do { \
        test_t *_test_##x = createTest(y); \
        addTest(currSuite, _test_##x); \
        fprintf(stdout, "   - %s", _test_##x->desc); \
    } while (0);

/**
 * @def CHECK_EQ(x, y)
 * @brief Vérifie l'égalité de deux valeurs de types quelconques
 * 
 * @param x : Valeur 1
 * @param y : Valeur 2
 * 
 */
#define CHECK_EQ(x, y) \
    do { \
        typeof(x) _x = x; \
        typeof(y) _y = y; \
        if (_x == _y) { \
            fprintf(stdout, "        ✅\n"); \
            currSuite->exitStatus = 0; \
        } else { \
            fprintf(stdout, "       ❌ ERREUR : %d != %d\n", x, y); \
            currSuite->exitStatus = 0; \
            currSuite->nbFails++; \
        } \
    } while (0) \

/**
 * @def CHECK_STR_EQ(x, y, z)
 * @brief Vérifie l'égalité de deux chaînes de caractères avec le résultat attendu
 * 
 * @param x : Chaîne 1
 * @param y : Chaîne 2
 * @param z : Valeur de retour attendue de la fonction strcmp()
 * 
 */
#define CHECK_STR_EQ(x, y, z) \
    do { \
        int _tmp = (strcmp(x, y) == 0); \
        if (_tmp == z) { \
            fprintf(stdout, "       ✅\n"); \
            currSuite->exitStatus = 0; \
        } else { \
            fprintf(stdout, "       ❌ ERREUR : %d != %d\n", _tmp, z); \
            currSuite->exitStatus = 0; \
            currSuite->nbFails++; \
        } \
    } while (0) \

/**
 * @def ASSERT_EQ(x, y)
 * @brief Vérifie l'égalité de deux valeurs de types quelconques 
 * et arrête le test en cas d'échec
 * 
 * @param x : Valeur 1
 * @param y : Valeur 2
 * 
 */
#define ASSERT_EQ(x, y) \
    do { \
        int _currNbFails = currSuite->nbFails; \
        CHECK_EQ(x, y); \
        if (_currNbFails != currSuite->nbFails) { \
            return EXIT; \
        } \
    } while (0) \

/**
 * @def ASSERT_STR_EQ(x, y, z)
 * @brief Vérifie l'égalité de deux chaînes de caractères avec 
 * le résultat attendu et arrête le test en cas d'échec
 * 
 * @param x : Chaîne 1
 * @param y : Chaîne 2
 * @param z : Valeur de retour attendue de la fonction strcmp()
 * 
 */
#define ASSERT_STR_EQ(x, y, z) \
    do { \
        int _currNbFails = currSuite->nbFails; \
        CHECK_STR_EQ(x, y, z); \
        if (_currNbFails != currSuite->nbFails) { \
            return EXIT; \
        } \
    } while (0) \


// Si vraiment c'est nécéssaire, à voir pour une implémentation
// des signaux pour vérifier les sorties exit() et isolation des tests.
// Par contre, la mise en place de ce système est assez complexe.
// #define CHECK_EXIT(x, y, z)

/**
 * @struct test_s
 * @brief Structure d'un test
 * 
 * Cette structure contient les informations relatives à un test. <br>
 * Elle contient la description du test.
 * 
 * @typedef test_t
 * @brief Structure d'un test
 * 
 * @var test_s::desc 
 * @brief Description du test
 * 
 */
typedef struct test_s {
    char *desc;
} test_t;

/**
 * @struct suite_s
 * @brief Structure d'une suite de tests
 * 
 * Cette structure contient les informations relatives à une suite de tests. <br>
 * Elle contient le nom de la suite, la description de la suite, le nombre de tests,
 * le nombre de tests en échec, le statut de sortie de la suite et un tableau contenant
 * les tests de la suite.
 * 
 * @typedef suite_t
 * @brief Structure d'une suite de tests
 * 
 * @var suite_s::nom 
 * @brief Nom de la suite
 * 
 * @var suite_s::desc 
 * @brief Description de la suite
 * 
 * @var suite_s::nbTests 
 * @brief Nombre de tests dans la suite
 * 
 * @var suite_s::nbFails
 * @brief Nombre de tests en échec dans la suite
 * 
 * @var suite_s::exitStatus
 * @brief Statut de sortie de la suite
 * 
 * @var suite_s::tests
 * @brief Tableau des tests de la suite
 * 
 */
typedef struct suite_s {
    char *nom;
    char *desc;
    int nbTests;
    int nbFails;
    int exitStatus;
    test_t *tests[_MAX_TESTCASES];
} suite_t;

/**
 * @struct status_s
 * @brief Structure d'état des tests
 * 
 * Cette structure contient les informations relatives à l'état des tests. <br>
 * Elle contient le nombre de suites de tests, le nombre de tests en échec,
 * un tableau contenant les suites de tests et un tableau contenant les fonctions de tests.
 * 
 * @typedef status_t
 * @brief Structure d'état des tests
 * 
 * @var status_s::nbSuites
 * @brief Nombre de suites de tests
 * 
 * @var status_s::nbFails
 * @brief Nombre de tests en échec
 * 
 * @var status_s::tests
 * @brief Tableau des suites de tests
 * 
 * @var status_s::func
 * @brief Tableau des fonctions de tests
 * 
 */
typedef struct status_s {
    int nbSuites;
    int nbFails;
    suite_t *tests[_MAX_TESTCASES];
    int (*func[_MAX_TESTCASES])(suite_t *);
} status_t;

/**
 * @var _status
 * @brief Variable globale contenant l'état des tests
 * 
 */
extern status_t _status;

// Opérations sur les suites de tests
/**
 * @brief Crée une suite de tests
 * 
 * @param nom : Le nom de la suite
 * @param description : La description de la suite
 * @return La suite de tests créée
 * 
 */
suite_t *createSuite(char *nom, char *description);

/**
 * @brief Libère en mémoire une suite de tests
 * 
 * @param suite : La suite de tests à libérer 
 * 
 */
void freeSuite(suite_t *suite);

/**
 * @brief Crée un test
 * 
 * @param description : La description du test
 * @return Le test créé
 * 
 */
test_t *createTest(char *description);

/**
 * @brief Ajoute un test à une suite de tests
 * 
 * @param suite : La suite de tests
 * @param test : Le test à ajouter
 * 
 */
void addTest(suite_t *suite, test_t *test);

/**
 * @brief Exécute toutes les suites de tests
 * 
 * @return EXIT_SUCCESS si tous les tests ont réussi, EXIT_FAILURE sinon
 * 
 */
int run(void);

#endif