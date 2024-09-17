#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unit_test.h"
#include "check.h"

// Initialisation variable globale _status (a ne pas toucher !!!)
status_t _status = {
    .nbSuites = 0,
    .nbFails = 0,
};

// Fonction qui crée une suite de tests
suite_t *createSuite(char *nom, char *description){
    
    suite_t *suite;
    status_t *status = &_status;

    if (status->nbSuites >= _MAX_TESTCASES) {
        
        fprintf(stderr, "createSuite : trop de tests\n");
        exit(EXIT_FAILURE);
    }

    CHECK(suite = malloc(sizeof(suite_t)));

    suite->nom = nom;
    suite->desc = description;
    suite->nbTests = 0;
    suite->nbFails = 0;

    return suite;
}

// Fonction qui libère en mémoire une suite de tests
void freeSuite(suite_t *suite){
    
    for (int i = 0; i < suite->nbTests; i++)
        free(suite->tests[i]);

    free(suite);
}

// Fonction qui crée un test
test_t *createTest(char *description){
        
    test_t *test;
    CHECK(test = malloc(sizeof(test_t)));

    test->desc = description;

    return test;
}

// Fonction qui ajoute un test à une suite de test
void addTest(suite_t *suite, test_t *test){
    
    if (suite->nbTests >= _MAX_TESTCASES) {
        
        fprintf(stderr, "addTest : trop de tests\n");
        exit(EXIT_FAILURE);
    }

    suite->tests[suite->nbTests] = test;
    suite->nbTests++;
}

// Fonction qui execute tout les tests
int run() {
    
    int exitVal, testVal;
    status_t *status = &_status;

    for (int i = 0; i < status->nbSuites; i++) {

        suite_t *suite = status->tests[i];
        fprintf(stdout, "|=================================================================|\n");
        fprintf(stdout, " [🛠️]  TEST (%d/%d) : %s \n", i+1, status->nbSuites, suite->desc);
        fprintf(stdout, "|=================================================================|\n");
        // Execution des tests
        testVal = status->func[i](status->tests[i]);

        // Verification du test en question
        if (testVal == OK) {
            fprintf(stdout, "\n   - [✅] TEST %d OK\n", i+1);
        }
        // Si le test a échoué
        else if (testVal == FAIL) {
            fprintf(stdout, "\n   - [❌] TEST %d PAS OK : %d FAILS\n", i+1, suite->nbFails);
            status->nbFails++;
        }
        // Si le test a échoué à cause d'une assertion
        else if (testVal == EXIT) {
            fprintf(stdout, "     [❗] Assert détectée, fin des tests.\n");
            fprintf(stdout, "\n   - [❌] TEST %d PAS OK : %d FAILS\n", i+1, suite->nbFails);
            status->nbFails++;
            // On libère la mémoire avant de quitter
            for (int j = i; j < status->nbSuites; j++)
                freeSuite(status->tests[j]);

            break;
        }
        // Erreur si le test ne retourne pas OK ou FAIL
        else {
            fprintf(stderr, "%s : valeur de retour inconnu\n", suite->nom); 
            // On libère la mémoire avant de quitter
            for (int j = i; j < status->nbSuites; j++)
                freeSuite(status->tests[j]);

            exit(EXIT_FAILURE);
        }
        // Cleanup
        freeSuite(suite);
    }
    // Bilan des tests
    fprintf(stdout, "|=================================================================|\n");
    if (status->nbFails) {
        fprintf(stdout, " [❌] NB DE TESTS FAILS : %d\n", status->nbFails);
        exitVal = EXIT_FAILURE;
    }
    else {
        fprintf(stdout, " [✅] OK C'EST BON\n");
        exitVal = EXIT_SUCCESS;
    }
    fprintf(stdout, "|=================================================================|\n");

    return exitVal;
}
