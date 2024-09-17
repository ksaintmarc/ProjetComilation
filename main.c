#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "symbol_table.h"

extern int yyparse(void);
extern void yyerror(const char* msg);
extern void lexFree();

// Variable globales
extern FILE *yyin;
extern FILE *yyout;
extern int yydebug;

int modeTos_g = 0;

// Fonction qui lit les arguments en entrée de programme
void readOpt(int argc, char **argv){
    // On définit 5 options possibles
    // --output : Indique le chemin de sortie
    // --input : Indique le chemin d'entrée
    // --debug : Active le mode debug
    // --tos : Affiche la table des symboles
    // --version : Affiche la version du programme
    struct option longOptions[] = {
        {"output", required_argument, 0, 'o'},
        {"input", required_argument, 0, 'i'},
        {"debug", no_argument, 0, 'd'},
        {"tos", no_argument, 0, 't'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };

    int opt;
    int optionIndex = 0;

    while ((opt = getopt_long(argc, argv, "+o:i:tv", \
        longOptions, &optionIndex)) != -1) 
    {
        switch (opt)
        {
        case 'i':
            yyin = fopen(optarg, "r");
            if (yyin == NULL) {
                fprintf(stderr, "%s : Erreur d'ouverture du fichier %s\n",\
                    argv[0], optarg);
                exit(EXIT_FAILURE);
            }

            break;

        case 'o':
            yyout = fopen(optarg, "w");
            if (yyout == NULL) {
                fprintf(stderr, "%s : Erreur d'ouverture du fichier %s\n",\
                    argv[0], optarg);
                exit(EXIT_FAILURE);
            }

            break;

        case 'd':
            yydebug = 1;
            break;
        
        case 't':
            modeTos_g = 1;
            break;
        
        case 'v':
            fprintf(stdout, "SoS : AUBRY Sébastien, PHAM Van, SAINT-MARC Kelana, TORTEROTOT Thomas\n");
            break;

        case '?':
            fprintf(stderr, "SoS : Unknown option: %c \n", optopt);
            exit(1);
        }
    }
}

// Fonction d'initialisation du programme
void initSos(int argc, char **argv){
 
    readOpt(argc, argv);
    pilePorteeInit();
}

// Fonction qui libère la mémoire
void freeSos(void){
    
    lexFree();
    pilePorteeFree();

    if (yyin != NULL)
        CHECK(!fclose(yyin));

    if (yyout != NULL)
        CHECK(!fclose(yyout));
}

int main(int argc, char **argv){

    initSos(argc, argv);
    
    int return_val = yyparse();

    if (modeTos_g)
        pilePorteeAffichage();
    
    freeSos();

    return return_val;
}
