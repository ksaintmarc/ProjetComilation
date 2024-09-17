#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "analyse.h"
#include "check.h"
#include "chaines_litterales.h"
#include "symbol_table.h"
#include "mips.h"

extern int yylineno;

struct listeQuad quads = { .taille = 0, .tailleMax = 0, .qs = NULL };
struct listeChaineLitterale chainesLitterales = { .taille = 0, .tailleMax = 0, .chaines = NULL };
int tailleListeOperandes = 0;

void arretRegleNonImplementee(char* msg){
    fprintf(stderr, "La règle : %s, n'a pas encore été implémentée.\n"
            "Arrêt de la compilation. (Ligne %d)\n", msg, yylineno);
    exit(EXIT_FAILURE);
}

void arretSymboleNonDefini(char* sym){
    fprintf(stderr, "Le symbole : %s, n'est pas défini.\n"
            "Arrêt de la compilation. (Ligne %d)\n", sym, yylineno);
    exit(EXIT_FAILURE);
}

void arretSymboleDejaDefini(char* sym){
    fprintf(stderr, "Le symbole : %s, est déjà défini.\n"
            "Arrêt de la compilation. (Ligne %d)\n", sym, yylineno);
    exit(EXIT_FAILURE);
}

void arretSymboleDeMauvaisType(char* sym, char* attendu){
    fprintf(stderr, "Le symbole : %s, était supposé être de type %s, "
            "mais est d'un autre type.\n"
            "Arrêt de la compilation. (Ligne %d)\n", sym, attendu, yylineno);
    exit(EXIT_FAILURE);
}

int conversionChaineVersEntier(char* chaine, int estNegatif){
    long x = 0;
    char* msgError = "Attention, il y a un overflow pour l'entier %c%s !\n%s";
    char* msgError2 = "Erreur sémantique\nEchec de la compilation\n";
    for (int i = 0; chaine[i] != '\0'; ++i){
        x *= 10;
        x += chaine[i] - '0';
        if (x > (long)INT_MAX + 1){
            if(estNegatif)
                fprintf(stderr, msgError, '-', chaine, msgError2);    
            else
                fprintf(stderr, msgError, ' ', chaine, msgError2);   
            exit(1);
        }
    }
    if (estNegatif)
        x = -x;
    if (x > INT_MAX || x < INT_MIN){
        if(estNegatif)
            fprintf(stderr, msgError, '-', chaine, msgError2);    
        else
            fprintf(stderr, msgError, ' ', chaine, msgError2); 
        exit(1);
    }
    return (int)x;
}

size_t generationChaine(char* chaine){
    return ajoutListeChaineLitterale(&chainesLitterales, chaine);
}

char* nouveauTemporaire(){
    static unsigned int n = 0;
    char tmp[16];
    snprintf(tmp, 16, "%%%d", n++);
    char* res;
    CHECK((res = strdup(tmp)) != NULL);
    return res;
}

void generationCode(struct quad q){
    ajoutListeQuad(&quads, q);
}

inline static void completeFree(struct listeAdresseQuad* l,
        adresseQuad_t adresse){

    for (size_t i = 0; i < l->taille; ++i){
        struct quad* q = &quads.qs[l->valeurs[i]];
        //Cette fonction ne sera utilisée qu'avec des quadruplets faisant un goto
        if (q->type == Q_GOTO)
            q->op0 = QUADOP_ADRESSE(adresse);
        else if (q->type == Q_CHAINE_VIDE || q->type == Q_CHAINE_NON_VIDE)
            q->op1 = QUADOP_ADRESSE(adresse);
        else if (q->type == Q_STACK)
            q->op0 = QUADOP_ENTIER((int)adresse);
        else
            q->op2 = QUADOP_ADRESSE(adresse);
    }

    listeAdresseQuadFree(l);
}

void yyerror(const char* msg){

    fprintf(stderr, "Erreur syntaxique : %s (Ligne %d)\n", msg, yylineno);
}

inline static void debugBasique(char* msg){

    (void)msg;
    #if SYNTAX_DEBUG != 0
    printf("%s\n", msg);
    #endif
}

int nombreDeChiffre(int x){

    int i = 1;
    x = x/10;
    while (x>=1){

        x = x/10;
        i++;
    }
    return i;
}

//Index de l'expression pour le debug
int exprDebugIndex = 0;

char* debugAvanceMsgConcatenation(char* debugMsgs[], size_t taille){

    size_t tailleTotal = 1;
    for (size_t i = 0; i < taille; i++){
        tailleTotal += strlen(debugMsgs[i]);
    }

    char* debugMsg = malloc(sizeof(char)*tailleTotal);
    if (debugMsg == NULL){
        perror("Erreur de malloc");
        exit(EXIT_FAILURE);
    }

    size_t position = 0;
    for (size_t i = 0; i < taille; i++){

        strcpy(debugMsg + position, debugMsgs[i]);
        position += strlen(debugMsgs[i]);
    }
    debugMsg[tailleTotal-1] = '\0';
    return debugMsg;
}

char* debugAvanceMsgCreation(){

    char* debugMsg = malloc(sizeof(char)*(nombreDeChiffre(exprDebugIndex)+8));
    if (debugMsg == NULL){
        perror("Erreur de malloc");
        exit(EXIT_FAILURE);
    }
    sprintf(debugMsg, "exprId %d", exprDebugIndex++);
    return debugMsg;
}

char* copieChaineDeChar(char* source){
    size_t taille = strlen(source);
    char* destination = malloc(sizeof(char)*(taille +1));
    if (destination == NULL){
        perror("Erreur de malloc");
        exit(EXIT_FAILURE);
    }
    strcpy(destination, source);
    return destination;
}

// Crée un quadop pour l'élément d'un tableau
// La mémoire allouée est libérée lors de la génération du code MIPS
inline static struct quadop quadopElementTableau(
        char* id, struct quadop qo){

    sym_t* symId = pilePorteeRecherche(id);
    if (symId == NULL)
        arretSymboleNonDefini(id);
    if (symId->type != S_TABLEAU)
        arretSymboleDeMauvaisType(id, "tableau");
    struct quadop* index;
    CHECK((index = malloc(sizeof(struct quadop))) != NULL);
    *index = qo;
    return QUADOP_ELEMENT_TABLEAU(symId->nom, index);
}

adresseQuad_t miseEnPlaceListeOperandes(){

    adresseQuad_t addr = quads.taille;
    generationCode(QUAD_STACK(QUADOP_ENTIER(0)));

    tailleListeOperandes = 0;

    return addr;
}

adresseQuad_t miseEnPlaceFor(char* id, int estForIn){

    // On doit dupliquer id car il faudrait autrement faire remonter
    // l'information de s'il faut ou non free jusqu'au for, ce qui
    // nécessiterait de retourner une structure de données dédiée ici,
    // ce qui introduit une complexité que je juge excessive.

    sym_t* symId = pilePorteeRecherche(id);
    if (symId == NULL){
        symId = symNouveau(strdup(id));
        symId->type = S_CHAINE;
        pilePorteeAjout(symId);
    }

    char* nomCompteur = nouveauTemporaire();
    sym_t *symCompteur = symNouveau(nomCompteur);
    symCompteur->type = S_ENTIER;
    pilePorteeAjout(symCompteur);

    // Initialisation du compteur
    generationCode(QUAD_AFF(QUADOP_ID(nomCompteur), QUADOP_ENTIER(0)));

    // Récupération de nextquad, le premier quadruplet de la boucle
    adresseQuad_t res = quads.taille;

    // Condition de boucle
    generationCode(QUAD_SUPEGAL(QUADOP_ID(nomCompteur),
            QUADOP_ENTIER(tailleListeOperandes)));

    // id := operandes[compteur]
    generationCode(QUAD_CHARGE_ARG(
            QUADOP_ID(symId->nom), QUADOP_ID(nomCompteur)));

    // compteur++
    generationCode(QUAD_ADD(QUADOP_ID(nomCompteur),
            QUADOP_ID(nomCompteur), QUADOP_ENTIER(1)));

    // id libéré dans rInstructionFor...

    return res;
}

valExpr_t rProgrammeListeInstructions(valExpr_t listeInstructions){

    debugBasique("programme -> liste_instructions");
    (void)listeInstructions;
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    completeFree(&(listeInstructions.suivant), quads.taille);

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "programme( ";
    debugMsgs[1] = listeInstructions.debugMsg;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(listeInstructions.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    free(valExpr.debugMsg);
    #endif

    quadsAMips(quads.qs, quads.taille,
            chainesLitterales.chaines, chainesLitterales.taille);

    listeQuadFree(&quads);
    listeChaineLitteraleFree(&chainesLitterales);

    return valExpr;
}

valExpr_t rListeInstructionsListeInstructionsPvInstruction(valExpr_t listeInstructions, adresseQuad_t marqueur,
        valExpr_t instruction){

    debugBasique("liste_instructions -> liste_instructions ; instruction");
    completeFree(&(listeInstructions.suivant), marqueur);
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.suivant = instruction.suivant;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 5;
    char* debugMsgs[taille];
    debugMsgs[0] = "liste_instructions( ";
    debugMsgs[1] = listeInstructions.debugMsg;
    debugMsgs[2] = " ; ";
    debugMsgs[3] = instruction.debugMsg;
    debugMsgs[4] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(instruction.debugMsg);
    free(listeInstructions.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rListeInstructionsInstruction(valExpr_t instruction){

    debugBasique("liste_instructions -> instruction");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.suivant = instruction.suivant;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "liste_instructions( ";
    debugMsgs[1] = instruction.debugMsg;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(instruction.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rInstructionIdEgConcatenation(char* id, valExpr_t concatenation){

    int freeId = 1;
    debugBasique("instruction -> ID = concatenation");
    sym_t* symId = pilePorteeRecherche(id);
    if (symId == NULL){
        symId = symNouveau(id);
        symId->type = S_CHAINE;
        pilePorteeAjout(symId);
        freeId = 0;
    }
    struct quadop resultat = QUADOP_ID(symId->nom);
    generationCode(QUAD_AFF(resultat, concatenation.resultat));
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.resultat = resultat;
    nouvelleListeAdresseQuad(&valExpr.suivant);

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 5;
    char* debugMsgs[taille];
    debugMsgs[0] = "instruction( ";
    debugMsgs[1] = id;
    debugMsgs[2] = " = ";
    debugMsgs[3] = concatenation.debugMsg;
    debugMsgs[4] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(concatenation.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    if (freeId)
        free(id);

    return valExpr;
}

valExpr_t rInstructionIdCoOperandeEntierCfEgConcatenation(char* id, valExpr_t operandeEntier, 
        valExpr_t concatenation){

    debugBasique("instruction -> ID [ operande_entier ] = concatenation");
    valExpr_t valExpr;
    valExpr.resultat = quadopElementTableau(id, operandeEntier.resultat);
    generationCode(QUAD_AFF(valExpr.resultat, concatenation.resultat));
    valExpr.debugMsg = NULL;
    nouvelleListeAdresseQuad(&valExpr.suivant);

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 7;
    char* debugMsgs[taille];
    debugMsgs[0] = "instruction( ";
    debugMsgs[1] = id;
    debugMsgs[2] = " [ ";
    debugMsgs[3] = operandeEntier.debugMsg;
    debugMsgs[4] = " ] = ";
    debugMsgs[5] = concatenation.debugMsg;
    debugMsgs[6] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(operandeEntier.debugMsg);
    free(concatenation.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    free(id);

    return valExpr;
}

valExpr_t rInstructionDeclareIdCoEntierCf(char* id, char* entier){

    debugBasique("instruction -> DECLARE ID [ ENTIER ]");
    sym_t* symId = pilePorteeRecherche(id);
    if (symId != NULL)
        arretSymboleDejaDefini(id);
    symId = symNouveau(id);
    symId->type = S_TABLEAU;
    symId->tailleTableau = conversionChaineVersEntier(entier, 0);
    pilePorteeAjout(symId);
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    nouvelleListeAdresseQuad(&valExpr.suivant);

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 5;
    char* debugMsgs[taille];
    debugMsgs[0] = "instruction( declare ";
    debugMsgs[1] = id;
    debugMsgs[2] = " [ ";
    debugMsgs[3] = entier;
    debugMsgs[4] = " ]) ";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    free(entier);

    return valExpr;
}

valExpr_t rInstructionIfTestBlocThenListeInstructionsElsePartFi(valExpr_t testBloc, 
        adresseQuad_t marqueurM1, valExpr_t listeInstructions, adresseQuad_t marqueurG,
        adresseQuad_t marqueurM2, valExpr_t elsePart){

    debugBasique("instruction -> IF test_bloc THEN liste_instructions else_part FI");
    completeFree(&(testBloc.vrai), marqueurM1);
    completeFree(&(testBloc.faux), marqueurM2);
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    struct listeAdresseQuad result, G;
    listeAdresseQuadSeul(&G, marqueurG);
    concat3ListeAdresseQuadFree(&(listeInstructions.suivant), &G, &(elsePart.suivant), &result);
    valExpr.suivant = result;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 7;
    char* debugMsgs[taille];
    debugMsgs[0] = "instruction( if ";
    debugMsgs[1] = testBloc.debugMsg;
    debugMsgs[2] = " then ";
    debugMsgs[3] = listeInstructions.debugMsg;
    debugMsgs[4] = " ";
    debugMsgs[5] = elsePart.debugMsg;
    debugMsgs[6] = " fi)";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(testBloc.debugMsg);
    free(elsePart.debugMsg);
    free(listeInstructions.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rInstructionForIdDoListeInstructionsDone(char* id, valExpr_t listeInstructions){

    arretRegleNonImplementee("instruction -> FOR ID DO liste_instructions DONE");
    debugBasique("instruction -> FOR ID DO liste_instructions DONE");
    (void)id;
    (void)listeInstructions;
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 5;
    char* debugMsgs[taille];
    debugMsgs[0] = "instruction( for ";
    debugMsgs[1] = id;
    debugMsgs[2] = " do ";
    debugMsgs[3] = listeInstructions.debugMsg;
    debugMsgs[4] = " done)";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(listeInstructions.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    free(id);

    return valExpr;
}

valExpr_t rInstructionForIdInListeOperandesDoListeInstructionsDone(
        char* id, adresseQuad_t addrQuadStack,
        valExpr_t listeOperandes, adresseQuad_t premierQuad,
        valExpr_t listeInstructions){

    (void)id; // traité par miseEnPlaceFor

    debugBasique("instruction -> FOR ID IN liste_operandes DO liste_instructions DONE");

    generationCode(QUAD_GOTO_ADDR(QUADOP_ADRESSE(premierQuad)));

    struct listeAdresseQuad addrExtStack;
    listeAdresseQuadSeul(&addrExtStack, addrQuadStack);
    completeFree(&addrExtStack, (adresseQuad_t)tailleListeOperandes);

    struct listeAdresseQuad addrRetrStack;
    listeAdresseQuadSeul(&addrRetrStack, premierQuad);
    completeFree(&addrRetrStack, quads.taille);

    generationCode(QUAD_STACK(QUADOP_ENTIER(-tailleListeOperandes)));

    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    nouvelleListeAdresseQuad(&valExpr.suivant);

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 7;
    char* debugMsgs[taille];
    debugMsgs[0] = "instruction( for ";
    debugMsgs[1] = id;
    debugMsgs[2] = " in ";
    debugMsgs[3] = listeOperandes.debugMsg;
    debugMsgs[4] = " do ";
    debugMsgs[5] = listeInstructions.debugMsg;
    debugMsgs[6] = " done)";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(listeOperandes.debugMsg);
    free(listeInstructions.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    free(id);

    return valExpr;
}

valExpr_t rInstructionWhileTestBlocDoListeInstructionsDone(adresseQuad_t marqueurM1,
        valExpr_t testBloc, adresseQuad_t marqueurM2, valExpr_t listeInstructions){

    debugBasique("instruction -> WHILE test_bloc DO liste_instructions DONE");
    // voir moodle cours5.pdf, page 194
    completeFree(&(testBloc.vrai), marqueurM2);
    generationCode(QUAD_GOTO_ADDR(QUADOP_ADRESSE(marqueurM1)));
    completeFree(&(listeInstructions.suivant), marqueurM1);
    
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.suivant = testBloc.faux;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 5;
    char* debugMsgs[taille];
    debugMsgs[0] = "instruction( while ";
    debugMsgs[1] = testBloc.debugMsg;
    debugMsgs[2] = " do ";
    debugMsgs[3] = listeInstructions.debugMsg;
    debugMsgs[4] = " done)";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(testBloc.debugMsg);
    free(listeInstructions.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rInstructionUntilTestBlocDoListeInstructionsDone(adresseQuad_t marqueurM1,
        valExpr_t testBloc, adresseQuad_t marqueurM2, valExpr_t listeInstructions){

    debugBasique("instruction -> UNTIL test_bloc DO liste_instructions DONE");
    // voir moodle cours5.pdf, page 194
    completeFree(&(testBloc.faux), marqueurM2);
    generationCode(QUAD_GOTO_ADDR(QUADOP_ADRESSE(marqueurM1)));
    completeFree(&(listeInstructions.suivant), marqueurM1);
    
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.suivant = testBloc.vrai;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 5;
    char* debugMsgs[taille];
    debugMsgs[0] = "instruction( until ";
    debugMsgs[1] = testBloc.debugMsg;
    debugMsgs[2] = " do ";
    debugMsgs[3] = listeInstructions.debugMsg;
    debugMsgs[4] = " done)";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(testBloc.debugMsg);
    free(listeInstructions.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rInstructionCaseOperandeInListeCasEsac(valExpr_t operande, valExpr_t listeCas){

    arretRegleNonImplementee("instruction -> CASE operande IN liste_cas ESAC");
    debugBasique("instruction -> CASE operande IN liste_cas ESAC");
    (void)operande;
    (void)listeCas;
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 5;
    char* debugMsgs[taille];
    debugMsgs[0] = "instruction( case ";
    debugMsgs[1] = operande.debugMsg;
    debugMsgs[2] = " in ";
    debugMsgs[3] = listeCas.debugMsg;
    debugMsgs[4] = " esac)";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(operande.debugMsg);
    free(listeCas.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rInstructionEchoListeOperandes(adresseQuad_t addrQuadStack, valExpr_t listeOperande){

    debugBasique("instruction -> ECHO liste_operandes");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    generationCode(QUAD_ECHO(QUADOP_ENTIER(tailleListeOperandes)));
    generationCode(QUAD_STACK(QUADOP_ENTIER(-tailleListeOperandes)));
    struct listeAdresseQuad addrExtStack;
    listeAdresseQuadSeul(&addrExtStack, addrQuadStack);
    completeFree(&addrExtStack, (adresseQuad_t)tailleListeOperandes);
    nouvelleListeAdresseQuad(&valExpr.suivant);

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "instruction( echo ";
    debugMsgs[1] = listeOperande.debugMsg;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(listeOperande.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rInstructionReadId(char* id){

    int freeId = 1;
    debugBasique("instruction -> READ ID");
    sym_t* symId = pilePorteeRecherche(id);
    if (symId == NULL){
        symId = symNouveau(id);
        symId->type = S_CHAINE;
        pilePorteeAjout(symId);
        freeId = 0;
    }
    struct quadop resultat = QUADOP_ID(symId->nom);
    generationCode(QUAD_READ(resultat));
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.resultat = resultat;
    nouvelleListeAdresseQuad(&valExpr.suivant);

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "instruction( read ";
    debugMsgs[1] = id;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    if (freeId)
        free(id);

    return valExpr;
}

valExpr_t rInstructionReadIdCoOperandeEntierCf(char* id, valExpr_t operandeEntier){

    debugBasique("instruction -> READ ID [ operande_entier ]");
    valExpr_t valExpr;
    valExpr.resultat = quadopElementTableau(id, operandeEntier.resultat);
    generationCode(QUAD_READ(valExpr.resultat));
    valExpr.debugMsg = NULL;
    nouvelleListeAdresseQuad(&valExpr.suivant);

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 5;
    char* debugMsgs[taille];
    debugMsgs[0] = "instruction( read ";
    debugMsgs[1] = id;
    debugMsgs[2] = " [ ";
    debugMsgs[3] = operandeEntier.debugMsg;
    debugMsgs[4] = " ])";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(operandeEntier.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    free(id);

    return valExpr;
}

valExpr_t rInstructionDeclarationDeFonction(valExpr_t declarationDeFonction){

    arretRegleNonImplementee("instruction -> declaration_de_fonction");
    debugBasique("instruction -> declaration_de_fonction");
    (void)declarationDeFonction;
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "instruction( ";
    debugMsgs[1] = declarationDeFonction.debugMsg;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(declarationDeFonction.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rInstructionAppelDeFonction(valExpr_t appelDeFonction){

    arretRegleNonImplementee("instruction -> appel_de_fonction");
    debugBasique("instruction -> appel_de_fonction");
    (void)appelDeFonction;
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "instruction( ";
    debugMsgs[1] = appelDeFonction.debugMsg;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(appelDeFonction.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rInstructionReturn(){

    arretRegleNonImplementee("instruction -> RETURN");
    debugBasique("instruction -> RETURN");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 1;
    char* debugMsgs[taille];
    debugMsgs[0] = "instruction( return )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rInstructionReturnOperandeEntier(valExpr_t operandeEntier){

    arretRegleNonImplementee("instruction -> RETURN operande_entier");
    debugBasique("instruction -> RETURN operande_entier");
    (void)operandeEntier;
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "instruction( return ";
    debugMsgs[1] = operandeEntier.debugMsg;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(operandeEntier.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rInstructionExit(){

    debugBasique("instruction -> EXIT");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    struct quad quadExit = QUAD_EXIT_VIDE();
    generationCode(quadExit);
    nouvelleListeAdresseQuad(&valExpr.suivant);

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 1;
    char* debugMsgs[taille];
    debugMsgs[0] = "instruction( exit )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rInstructionExitOperandeEntier(valExpr_t operandeEntier){

    debugBasique("instruction -> EXIT operande_entier");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    struct quad quadExit = QUAD_EXIT(operandeEntier.resultat);
    generationCode(quadExit);
    nouvelleListeAdresseQuad(&valExpr.suivant);

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "instruction( exit ";
    debugMsgs[1] = operandeEntier.debugMsg;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(operandeEntier.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rElsePartElifTestBlocThenListeInstructionsElsePart(valExpr_t testBloc,
        adresseQuad_t marqueurM1, valExpr_t listeInstructions, adresseQuad_t marqueurG,
        adresseQuad_t marqueurM2, valExpr_t elsePart){

    debugBasique("else_part -> ELIF test_bloc THEN liste_instructions else_part");
    completeFree(&(testBloc.vrai), marqueurM1);
    completeFree(&(testBloc.faux), marqueurM2);
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    struct listeAdresseQuad result, G;
    listeAdresseQuadSeul(&G, marqueurG);
    concat3ListeAdresseQuadFree(&(listeInstructions.suivant), &G, &(elsePart.suivant), &result);
    valExpr.suivant = result;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 7;
    char* debugMsgs[taille];
    debugMsgs[0] = "else_part( elif ";
    debugMsgs[1] = testBloc.debugMsg;
    debugMsgs[2] = " then ";
    debugMsgs[3] = listeInstructions.debugMsg;
    debugMsgs[4] = " ";
    debugMsgs[5] = elsePart.debugMsg;
    debugMsgs[6] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(testBloc.debugMsg);
    free(listeInstructions.debugMsg);
    free(elsePart.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rElsePartElseListeInstructions(valExpr_t listeInstructions){

    debugBasique("else_part -> ELSE liste_instructions");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.suivant = listeInstructions.suivant;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "else_part( else ";
    debugMsgs[1] = listeInstructions.debugMsg;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(listeInstructions.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rElsePartEmpty(){

    debugBasique("else_part -> %empty");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    struct listeAdresseQuad result = {.taille=0, .tailleMax=0, .valeurs=NULL};
    valExpr.suivant = result;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 1;
    char* debugMsgs[taille];
    debugMsgs[0] = "else_part( %empty )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rListeCasListeCasFiltrePfListeInstructionsDpv(valExpr_t listeCas, 
        valExpr_t filtre, valExpr_t listeInstructions){

    arretRegleNonImplementee("liste_cas -> liste_cas filtre ) liste_instructions ;;");
    debugBasique("liste_cas -> liste_cas filtre ) liste_instructions ;;");
    (void)listeCas;
    (void)filtre;
    (void)listeInstructions;
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 7;
    char* debugMsgs[taille];
    debugMsgs[0] = "liste_cas( ";
    debugMsgs[1] = listeCas.debugMsg;
    debugMsgs[2] = " ";
    debugMsgs[3] = filtre.debugMsg;
    debugMsgs[4] = " ) ";
    debugMsgs[5] = listeInstructions.debugMsg;
    debugMsgs[6] = " ;;)";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(listeCas.debugMsg);
    free(filtre.debugMsg);
    free(listeInstructions.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rListeCasFiltrePfListeInstructionsDpv(valExpr_t filtre, valExpr_t listeInstructions){

    arretRegleNonImplementee("liste_cas -> filtre ) liste_instructions ;;");
    debugBasique("liste_cas -> filtre ) liste_instructions ;;");
    (void)filtre;
    (void)listeInstructions;
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 5;
    char* debugMsgs[taille];
    debugMsgs[0] = "liste_cas( ";
    debugMsgs[1] = filtre.debugMsg;
    debugMsgs[2] = " ) ";
    debugMsgs[3] = listeInstructions.debugMsg;
    debugMsgs[4] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(filtre.debugMsg);
    free(listeInstructions.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rFiltreFiltreTerm(valExpr_t filtreTerm){

    arretRegleNonImplementee("filtre -> filtre_term");
    debugBasique("filtre -> filtre_term");
    (void)filtreTerm;
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "filtre( ";
    debugMsgs[1] = filtreTerm.debugMsg;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(filtreTerm.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rFiltreFiltreOuFiltreTerm(valExpr_t filtre, valExpr_t filtreTerm){

    arretRegleNonImplementee("filtre -> filtre | filtre_term");
    debugBasique("filtre -> filtre | filtre_term");
    (void)filtre;
    (void)filtreTerm;
    valExpr_t valExpr;
    valExpr.debugMsg = "";

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 5;
    char* debugMsgs[taille];
    debugMsgs[0] = "filtre( ";
    debugMsgs[1] = filtre.debugMsg;
    debugMsgs[2] = " | ";
    debugMsgs[3] = filtreTerm.debugMsg;
    debugMsgs[4] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(filtre.debugMsg);
    free(filtreTerm.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rFiltreFois(){

    arretRegleNonImplementee("filtre -> *");
    debugBasique("filtre -> *");
    valExpr_t valExpr;
    valExpr.debugMsg = "";

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 1;
    char* debugMsgs[taille];
    debugMsgs[0] = "filtre(*)";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rFiltreTermMot(char* mot){

    arretRegleNonImplementee("filtre_term -> mot");
    debugBasique("filtre_term -> mot");
    (void)mot;
    valExpr_t valExpr;
    valExpr.debugMsg = "";

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "filtre_term(";
    debugMsgs[1] = mot;
    debugMsgs[2] = ")";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    free(mot);

    return valExpr;
}

valExpr_t rFiltreTermGchaine(char* gchaine){

    arretRegleNonImplementee("filtre_term -> GCHAINE");
    debugBasique("filtre_term -> GCHAINE");
    (void)gchaine;
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "filtre_term( ";
    debugMsgs[1] = gchaine;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    free(gchaine);

    return valExpr;
}

valExpr_t rFiltreTermApchaine(char* apchaine){

    arretRegleNonImplementee("filtre_term -> APCHAINE");
    debugBasique("filtre_term -> APCHAINE");
    (void)apchaine;
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "filtre_term( ";
    debugMsgs[1] = apchaine;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    free(apchaine);

    return valExpr;
}

char* rMotAutremot(char* autremot){

    debugBasique("mot -> AUTREMOT");
    return autremot;
}

char* rMotId(char* id){

    debugBasique("mot -> ID");
    return id;
}

char* rMotEntier(char* entier){

    debugBasique("mot -> ENTIER");
    return entier;
}

valExpr_t rListeOperandesListeOperandesOperande(valExpr_t listeOperandes, valExpr_t operande){

    debugBasique("liste_operandes -> liste_operandes operande");
    (void)listeOperandes;
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    struct quad quadPasseArg = QUAD_PASSE_ARG(operande.resultat,
            QUADOP_ENTIER(tailleListeOperandes++));
    generationCode(quadPasseArg);

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 5;
    char* debugMsgs[taille];
    debugMsgs[0] = "liste_operandes( ";
    debugMsgs[1] = listeOperandes.debugMsg;
    debugMsgs[2] = " ";
    debugMsgs[3] = operande.debugMsg;
    debugMsgs[4] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(listeOperandes.debugMsg);
    free(operande.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rListeOperandesOperande(valExpr_t operande){

    debugBasique("liste_operandes -> operande");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    struct quad quadPasseArg = QUAD_PASSE_ARG(operande.resultat,
            QUADOP_ENTIER(tailleListeOperandes++));
    generationCode(quadPasseArg);

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "liste_operandes( ";
    debugMsgs[1] = operande.debugMsg;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(operande.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rListeOperandesDollarAoIdCoFoisCfAf(char* id){

    debugBasique("liste_operandes -> $ { ID [ * ]}");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    sym_t* symId = pilePorteeRecherche(id);
    if (symId == NULL)
        arretSymboleNonDefini(id);
    if (symId->type != S_TABLEAU)
        arretSymboleDeMauvaisType(id, "tableau");
    struct quad quadPasseTab = QUAD_PASSE_TAB(QUADOP_ID(symId->nom),
            QUADOP_ENTIER(tailleListeOperandes),
            QUADOP_ENTIER(symId->tailleTableau));
    generationCode(quadPasseTab);
    tailleListeOperandes += symId->tailleTableau;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "liste_operandes( $ { ";
    debugMsgs[1] = id;
    debugMsgs[2] = " [*]})";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    free(id);

    return valExpr;
}

valExpr_t rConcatenationConcatenationOperande(valExpr_t concatenation, valExpr_t operande){

    debugBasique("concatenation -> concatenation operande");
    char* nom = nouveauTemporaire();
    struct quadop resultat = QUADOP_ID(nom);
    sym_t *sym = symNouveau(nom);
    sym->type = S_CHAINE;
    pilePorteeAjout(sym);
    struct quad quad;
    quad = QUAD_CONCAT(resultat, concatenation.resultat, operande.resultat);
    generationCode(quad);
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.resultat = resultat;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 5;
    char* debugMsgs[taille];
    debugMsgs[0] = "concatenation( ";
    debugMsgs[1] = concatenation.debugMsg;
    debugMsgs[2] = " ";
    debugMsgs[3] = operande.debugMsg;
    debugMsgs[4] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(concatenation.debugMsg);
    free(operande.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rConcatenationOperande(valExpr_t operande){

    debugBasique("concatenation -> operande");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.resultat = operande.resultat;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "concatenation( ";
    debugMsgs[1] = operande.debugMsg;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(operande.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rTestBlocTestTestExpr(valExpr_t testExpr){

    debugBasique("test_bloc -> TEST test_expr");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.vrai = testExpr.vrai;
    valExpr.faux = testExpr.faux;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "test_bloc( test ";
    debugMsgs[1] = testExpr.debugMsg;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(testExpr.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rTestExprTestExprOTestExpr2(valExpr_t testExpr, adresseQuad_t marqueur, valExpr_t testExpr2){

    debugBasique("test_expr -> test_expr -o test_expr2");
    completeFree(&(testExpr.faux), marqueur);
    struct listeAdresseQuad result;
    concatListeAdresseQuadFree(&(testExpr.vrai),&(testExpr2.vrai), &result);
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.faux = testExpr2.faux;
    valExpr.vrai = result;


    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 5;
    char* debugMsgs[taille];
    debugMsgs[0] = "test_expr( ";
    debugMsgs[1] = testExpr.debugMsg;
    debugMsgs[2] = " -o ";
    debugMsgs[3] = testExpr2.debugMsg;
    debugMsgs[4] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(testExpr.debugMsg);
    free(testExpr2.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rTestExprTestExpr2(valExpr_t testExpr2){

    debugBasique("test_expr -> test_expr2");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.vrai = testExpr2.vrai;
    valExpr.faux = testExpr2.faux;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "test_expr( ";
    debugMsgs[1] = testExpr2.debugMsg;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(testExpr2.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rTestExpr2TestExpr2ATestExpr3(valExpr_t testExpr2, adresseQuad_t marqueur, valExpr_t testExpr3){

    debugBasique("test_expr2 -> test_expr2 -a test_expr3");
    completeFree(&(testExpr2.vrai),marqueur);
    struct listeAdresseQuad result;
    concatListeAdresseQuadFree(&(testExpr2.faux),&(testExpr3.faux),&result);
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.vrai = testExpr3.vrai;
    valExpr.faux = result;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 5;
    char* debugMsgs[taille];
    debugMsgs[0] = "test_expr2( ";
    debugMsgs[1] = testExpr2.debugMsg;
    debugMsgs[2] = " -a ";
    debugMsgs[3] = testExpr3.debugMsg;
    debugMsgs[4] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(testExpr3.debugMsg);
    free(testExpr2.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rTestExpr2TestExpr3(valExpr_t testExpr3){

    debugBasique("test_expr2 -> test_expr3");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.vrai = testExpr3.vrai;
    valExpr.faux = testExpr3.faux;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "test_expr2( ";
    debugMsgs[1] = testExpr3.debugMsg;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(testExpr3.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rTestExpr3PoTestExprPf(valExpr_t testExpr){

    debugBasique("test_expr3 -> ( test_expr )");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.vrai = testExpr.vrai;
    valExpr.faux = testExpr.faux;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "test_expr3( ( ";
    debugMsgs[1] = testExpr.debugMsg;
    debugMsgs[2] = " ) )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(testExpr.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rTestExpr3ExcPoTestExprPf(valExpr_t testExpr){

    debugBasique("test_expr3 -> ! ( test_expr )");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.vrai = testExpr.faux;
    valExpr.faux = testExpr.vrai;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "test_expr3( !( ";
    debugMsgs[1] = testExpr.debugMsg;
    debugMsgs[2] = " ) )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(testExpr.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rTestExpr3TestInstruction(valExpr_t testInstruction){

    debugBasique("test_expr3 -> test_instruction");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.vrai = testInstruction.vrai;
    valExpr.faux = testInstruction.faux;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "test_expr3( ";
    debugMsgs[1] = testInstruction.debugMsg;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(testInstruction.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rTestExpr3ExcTestInstruction(valExpr_t testInstruction){

    debugBasique("test_espr3 -> ! test_instruction");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.vrai = testInstruction.faux;
    valExpr.faux = testInstruction.vrai;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "test_expr3( ! ";
    debugMsgs[1] = testInstruction.debugMsg;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(testInstruction.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rTestInstructionConcatenationEgConcatenation(valExpr_t concatenation1,
        valExpr_t concatenation2){

    debugBasique("test_instruction -> concatenation = concatenation");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    struct listeAdresseQuad res1, res2;
    struct quad q;

    q = QUAD_CHAINE_EGAL(concatenation1.resultat,concatenation2.resultat);
    listeAdresseQuadSeul(&res1,quads.taille);
    valExpr.vrai = res1;
    generationCode(q);

    listeAdresseQuadSeul(&res2,quads.taille);
    valExpr.faux = res2;
    generationCode(QUAD_GOTO());

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 5;
    char* debugMsgs[taille];
    debugMsgs[0] = "test_instruction( ";
    debugMsgs[1] = concatenation1.debugMsg;
    debugMsgs[2] = " = ";
    debugMsgs[3] = concatenation2.debugMsg;
    debugMsgs[4] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(concatenation2.debugMsg);
    free(concatenation1.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rTestInstructionConcatenationExcEgConcatenation(valExpr_t concatenation1,
        valExpr_t concatenation2){

    debugBasique("test_instruction -> concatenation != concatenation");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    struct listeAdresseQuad res1, res2;
    struct quad q;

    q = QUAD_CHAINE_NEGAL(concatenation1.resultat,concatenation2.resultat);
    listeAdresseQuadSeul(&res1,quads.taille);
    valExpr.vrai = res1;
    generationCode(q);
    
    listeAdresseQuadSeul(&res2,quads.taille);
    valExpr.faux = res2;
    generationCode(QUAD_GOTO());

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 5;
    char* debugMsgs[taille];
    debugMsgs[0] = "test_instruction( ";
    debugMsgs[1] = concatenation1.debugMsg;
    debugMsgs[2] = " != ";
    debugMsgs[3] = concatenation2.debugMsg;
    debugMsgs[4] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(concatenation2.debugMsg);
    free(concatenation1.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rTestInstructionOperateur1Concatenation(char operateur1, valExpr_t concatenation){

    debugBasique("test_instruction -> operateur1 concatenation");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    struct listeAdresseQuad res1, res2;
    struct quad q;

    if (operateur1 == 'n')
        q = QUAD_CHAINE_NON_VIDE(concatenation.resultat);
    else
        q = QUAD_CHAINE_VIDE(concatenation.resultat);

    listeAdresseQuadSeul(&res1,quads.taille);
    valExpr.vrai = res1;
    generationCode(q);
    
    listeAdresseQuadSeul(&res2,quads.taille);
    valExpr.faux = res2;
    generationCode(QUAD_GOTO());

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 5;
    char* debugMsgs[taille];
    char debugOperateur1[2];
    debugOperateur1[0] = operateur1;
    debugOperateur1[1] = '\0';
    debugMsgs[0] = "test_instruction( -";
    debugMsgs[1] = debugOperateur1;
    debugMsgs[2] = " ";
    debugMsgs[3] = concatenation.debugMsg;
    debugMsgs[4] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(concatenation.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rTestInstructionOperandeOperateur2Operande(valExpr_t operande1, enum relopType operateur2,
        valExpr_t operande2){

    debugBasique("test_instruction -> operande operateur2 operande");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    struct listeAdresseQuad res1, res2;
    struct quad q;

    char* debugOperateur2 = "";
    switch (operateur2){

    case RO_EGAL:
        q = QUAD_EGAL(operande1.resultat,operande2.resultat);
        debugOperateur2 = " -eq ";
        break;

    case RO_NEGAL:
        q = QUAD_NEGAL(operande1.resultat,operande2.resultat);
        debugOperateur2 = " -ne ";
        break;

    case RO_SUP:
        q = QUAD_SUP(operande1.resultat,operande2.resultat);
        debugOperateur2 = " -gt ";
        break;

    case RO_SUPEGAL:
        q = QUAD_SUPEGAL(operande1.resultat,operande2.resultat);
        debugOperateur2 = " -ge ";
        break;

    case RO_INF:
        q = QUAD_INF(operande1.resultat,operande2.resultat);
        debugOperateur2 = " -lt ";
        break;

    case RO_INFEGAL:
        q = QUAD_INFEGAL(operande1.resultat,operande2.resultat);
        debugOperateur2 = " -le ";
        break;
    }

    listeAdresseQuadSeul(&res1,quads.taille);
    valExpr.vrai = res1;
    generationCode(q);
    
    listeAdresseQuadSeul(&res2,quads.taille);
    valExpr.faux = res2;
    generationCode(QUAD_GOTO());

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 5;
    char* debugMsgs[taille];
    debugMsgs[0] = "test_instruction( ";
    debugMsgs[1] = operande1.debugMsg;
    debugMsgs[2] = debugOperateur2;
    debugMsgs[3] = operande2.debugMsg;
    debugMsgs[4] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(operande2.debugMsg);
    free(operande1.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rOperandeDollarAoIdAf(char* id){

    debugBasique("operande -> $ { ID }");
    sym_t* symId = pilePorteeRecherche(id);
    if (symId == NULL)
        arretSymboleNonDefini(id);
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.resultat = QUADOP_ID(symId->nom);

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "operande( ${ ";
    debugMsgs[1] = id;
    debugMsgs[2] = " } )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    free(id);

    return valExpr;
}

valExpr_t rOperandeDollarAoIdCoOperandeEntierCfAf(char* id, valExpr_t operandeEntier){

    debugBasique("operande -> $ { ID [ operande_entier ]}");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.resultat = quadopElementTableau(id, operandeEntier.resultat);

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 5;
    char* debugMsgs[taille];
    debugMsgs[0] = "operande( ${ ";
    debugMsgs[1] = id;
    debugMsgs[2] = " [ ";
    debugMsgs[3] = operandeEntier.debugMsg;
    debugMsgs[4] = " ]} ";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(operandeEntier.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    free(id);

    return valExpr;
}

valExpr_t rOperandeMot(char* mot){

    debugBasique("operande -> mot");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    size_t index = generationChaine(mot);
    valExpr.resultat = QUADOP_CHAINE(index);

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "operande( ";
    debugMsgs[1] = mot;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rOperandeArg(int arg){

    debugBasique("operande -> ARG");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.resultat = QUADOP_ARG(arg);

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "operande( ";
    debugMsgs[1] = malloc(sizeof(char)*(nombreDeChiffre(arg)+1));
    if (debugMsgs[1] == NULL){
        perror("Erreur de malloc");
        exit(EXIT_FAILURE);
    }
    sprintf(debugMsgs[1], "%d", arg);
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(debugMsgs[1]);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rOperandeAllargs(){

    debugBasique("operande -> $*");
    char* nom = nouveauTemporaire();
    struct quadop resultat = QUADOP_ID(nom);
    sym_t *sym = symNouveau(nom);
    sym->type = S_CHAINE;
    pilePorteeAjout(sym);
    generationCode(QUAD_ALLARGS(resultat));
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.resultat = resultat;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 1;
    char* debugMsgs[taille];
    debugMsgs[0] = "operande( $* )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rOperandeStatut(){

    arretRegleNonImplementee("operande -> $?");
    debugBasique("operande -> $?");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 1;
    char* debugMsgs[taille];
    debugMsgs[0] = "operande( $? )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rOperandeGchaine(char* gchaine){

    debugBasique("operande -> GCHAINE");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    size_t index = generationChaine(gchaine);
    valExpr.resultat = QUADOP_CHAINE(index);

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "operande( ";
    debugMsgs[1] = gchaine;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rOperandeApchaine(char* apchaine){

    debugBasique("operande -> APCHAINE");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    size_t index = generationChaine(apchaine);
    valExpr.resultat = QUADOP_CHAINE(index);

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "operande( ";
    debugMsgs[1] = apchaine;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rOperandeDollarPoExprSommeEntierePf(valExpr_t sommeEntiere){

    debugBasique("operande -> $ ( EXPR somme_entiere )");
    char* nom = nouveauTemporaire();
    struct quadop resultat = QUADOP_ID(nom);
    sym_t *sym = symNouveau(nom);
    sym->type = S_CHAINE;
    pilePorteeAjout(sym);
    struct quad quad;
    quad = QUAD_CONV_E_V_C(resultat, sommeEntiere.resultat);
    generationCode(quad);
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.resultat = resultat;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "operande( $( expr ";
    debugMsgs[1] = sommeEntiere.debugMsg;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(sommeEntiere.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rOperandeDollarPoAppelDeFonctionPf(valExpr_t appelDeFonction){

    arretRegleNonImplementee("operande -> $ ( appel_de_fonction )");
    debugBasique("operande -> $ ( appel_de_fonction )");
    (void)appelDeFonction;
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "operande( $( ";
    debugMsgs[1] = appelDeFonction.debugMsg;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(appelDeFonction.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

char rOperateur1N(){

    debugBasique("operateur1 -> -n");

    return 'n';
}

char rOperateur1Z(){

    debugBasique("operateur1 -> -z");

    return 'z';
}

enum relopType rOperateur2Eq(){

    debugBasique("operateur2 -> -eq");

    return RO_EGAL;
}

enum relopType rOperateur2Ne(){

    debugBasique("operateur2 -> -ne");

    return RO_NEGAL;
}

enum relopType rOperateur2Gt(){

    debugBasique("operateur2 -> -gt");

    return RO_SUP;
}

enum relopType rOperateur2Ge(){

    debugBasique("operateur2 -> -ge");

    return RO_SUPEGAL;
}

enum relopType rOperateur2Lt(){

    debugBasique("operateur2 -> -lt");

    return RO_INF;
}

enum relopType rOperateur2Le(){

    debugBasique("operateur2 -> -le");

    return RO_INFEGAL;
}

valExpr_t rSommeEntiereSommeEntierePlusOuMoinProduitEntier(valExpr_t sommeEntiere,
        char plusOuMoin, valExpr_t produitEntier){

    debugBasique("somme_entiere -> somme_entiere plus_ou_moin produit_entier");
    char* nom = nouveauTemporaire();
    struct quadop resultat = QUADOP_ID(nom);
    sym_t *sym = symNouveau(nom);
    sym->type = S_ENTIER;
    pilePorteeAjout(sym);
    struct quad quad0;
    if (plusOuMoin=='+'){
        quad0 = QUAD_ADD(resultat, sommeEntiere.resultat, produitEntier.resultat);
    }
    else{
        quad0 = QUAD_SOUS(resultat, sommeEntiere.resultat, produitEntier.resultat);
    }
    generationCode(quad0);
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.resultat = resultat;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 7;
    char* debugMsgs[taille];
    debugMsgs[0] = "somme_entiere( ";
    debugMsgs[1] = sommeEntiere.debugMsg;
    debugMsgs[2] = " ";
    if(plusOuMoin == '-')
        debugMsgs[3] = "-";
    else
        debugMsgs[3] = "+";
    debugMsgs[4] = " ";
    debugMsgs[5] = produitEntier.debugMsg;
    debugMsgs[6] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(sommeEntiere.debugMsg);
    free(produitEntier.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rSommeEntiereProduitEntier(valExpr_t produitEntier){

    debugBasique("somme_entiere -> produit_entier");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.resultat = produitEntier.resultat;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "somme_entiere( ";
    debugMsgs[1] = produitEntier.debugMsg;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(produitEntier.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rProduitEntierProduitEntierFoisDivModOperandeEntier(valExpr_t produitEntier,
        char foisDivMod, valExpr_t operandeEntier){

    debugBasique("produit_entier -> produit_entier fois_div_mod operande_entier");
    char* nom = nouveauTemporaire();
    struct quadop resultat = QUADOP_ID(nom);
    sym_t *sym = symNouveau(nom);
    sym->type = S_ENTIER;
    pilePorteeAjout(sym);
    struct quad quad0;
    if (foisDivMod=='*'){
        quad0 = QUAD_MUL(resultat,produitEntier.resultat, operandeEntier.resultat);
    }
    else if (foisDivMod=='/'){
        quad0 = QUAD_DIV(resultat,produitEntier.resultat,operandeEntier.resultat);
    }
    else{
        quad0 = QUAD_MOD(resultat,produitEntier.resultat,operandeEntier.resultat);
    }
    generationCode(quad0);
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.resultat = resultat;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 7;
    char* debugMsgs[taille];
    debugMsgs[0] = "produit_entier( ";
    debugMsgs[1] = produitEntier.debugMsg;
    debugMsgs[2] = " ";
    if(foisDivMod == '*')
        debugMsgs[3] = "*";
    else if(foisDivMod == '/')
        debugMsgs[3] = "/";
    else
        debugMsgs[3] = "%";
    debugMsgs[4] = " ";
    debugMsgs[5] = operandeEntier.debugMsg;
    debugMsgs[6] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(produitEntier.debugMsg);
    free(operandeEntier.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rProduitEntierOperandeEntier(valExpr_t operandeEntier){

    debugBasique("produit_entier -> operande_entier");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.resultat = operandeEntier.resultat;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "produit_entier( ";
    debugMsgs[1] = operandeEntier.debugMsg;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(operandeEntier.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rOperandeEntierDollarAoIdAf(char* id){

    debugBasique("operande_entier -> ${ ID }");
    sym_t* symId = pilePorteeRecherche(id);
    if (symId == NULL)
        arretSymboleNonDefini(id);
    char* nom = nouveauTemporaire();
    struct quadop resultat = QUADOP_ID(nom);
    sym_t *sym = symNouveau(nom);
    sym->type = S_ENTIER;
    pilePorteeAjout(sym);
    struct quad quad;
    quad = QUAD_CONV_C_V_E(resultat, QUADOP_ID(symId->nom));
    generationCode(quad);
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.resultat = resultat;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "operande_entier( ${ ";
    debugMsgs[1] = id;
    debugMsgs[2] = " } )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    free(id);

    return valExpr;
}

valExpr_t rOperandeEntierDollarAoIdCoOperandeEntierCfAf(char* id, valExpr_t operandeEntier){

    debugBasique("operande_entier -> ${ ID [ operande_entier ]}");
    struct quadop val = quadopElementTableau(id, operandeEntier.resultat);
    char* nom = nouveauTemporaire();
    struct quadop resultat = QUADOP_ID(nom);
    sym_t *sym = symNouveau(nom);
    sym->type = S_ENTIER;
    pilePorteeAjout(sym);
    generationCode(QUAD_CONV_C_V_E(resultat, val));
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.resultat = resultat;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 5;
    char* debugMsgs[taille];
    debugMsgs[0] = "operande_entier( ${ ";
    debugMsgs[1] = id;
    debugMsgs[2] = " [ ";
    debugMsgs[3] = operandeEntier.debugMsg;
    debugMsgs[4] = " ]} )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(operandeEntier.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    free(id);

    return valExpr;
}

valExpr_t rOperandeEntierArg(int arg){

    debugBasique("operande_entier -> ARG");
    char* nom = nouveauTemporaire();
    struct quadop resultat = QUADOP_ID(nom);
    sym_t *sym = symNouveau(nom);
    sym->type = S_ENTIER;
    pilePorteeAjout(sym);
    struct quad quad;
    quad = QUAD_CONV_C_V_E(resultat, QUADOP_ARG(arg));
    generationCode(quad);
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.resultat = resultat;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "operande_entier( ";
    debugMsgs[1] = malloc(sizeof(char)*(nombreDeChiffre(arg)+1));
    if (debugMsgs[1] == NULL){
        perror("Erreur de malloc");
        exit(EXIT_FAILURE);
    }
    sprintf(debugMsgs[1], "%d", arg);
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(debugMsgs[1]);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rOperandeEntierPlusOuMoinDollarAoIdAf(char plusOuMoin, char* id){

    struct quadop resultat;
    debugBasique("operande_entier -> plus_ou_moin ${ ID }");
    sym_t* symId = pilePorteeRecherche(id);
    if (symId == NULL)
        arretSymboleNonDefini(id);
    // Conversion
    char* nomC = nouveauTemporaire();
    struct quadop conv = QUADOP_ID(nomC);
    sym_t *sym = symNouveau(nomC);
    sym->type = S_ENTIER;
    pilePorteeAjout(sym);
    generationCode(QUAD_CONV_C_V_E(conv, QUADOP_ID(symId->nom)));
    // Calcul
    if (plusOuMoin == '-'){
        char* nom = nouveauTemporaire();
        resultat = QUADOP_ID(nom);
        sym_t *sym = symNouveau(nom);
        sym->type = S_ENTIER;
        pilePorteeAjout(sym);
        generationCode(QUAD_SOUS(resultat, QUADOP_ENTIER(0), conv));
    } else
        resultat = conv;
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.resultat = resultat;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 5;
    char* debugMsgs[taille];
    debugMsgs[0] = "operande_entier( ";
    if(plusOuMoin == '-')
        debugMsgs[1] = "-";
    else
        debugMsgs[1] = "+";
    debugMsgs[2] = " ${ ";
    debugMsgs[3] = id;
    debugMsgs[4] = " } )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    free(id);

    return valExpr;
}

valExpr_t rOperandeEntierPlusOuMoinDollarAoIdCoOperandeEntierCfAf(char plusOuMoin,
        char* id, valExpr_t operandeEntier){

    struct quadop resultat;
    debugBasique("operande_entier -> plus_ou_moin ${ ID [ operande_entier ]}");
    struct quadop val = quadopElementTableau(id, operandeEntier.resultat);
    // Conversion
    char* nomC = nouveauTemporaire();
    struct quadop conv = QUADOP_ID(nomC);
    sym_t *sym = symNouveau(nomC);
    sym->type = S_ENTIER;
    pilePorteeAjout(sym);
    generationCode(QUAD_CONV_C_V_E(conv, val));
    // Calcul
    if (plusOuMoin == '-'){
        char* nom = nouveauTemporaire();
        resultat = QUADOP_ID(nom);
        sym_t *sym = symNouveau(nom);
        sym->type = S_ENTIER;
        pilePorteeAjout(sym);
        generationCode(QUAD_SOUS(resultat, QUADOP_ENTIER(0), conv));
    } else
        resultat = conv;
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.resultat = resultat;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 7;
    char* debugMsgs[taille];
    debugMsgs[0] = "operande_entier( ";
    if(plusOuMoin == '-')
        debugMsgs[1] = "-";
    else
        debugMsgs[1] = "+";
    debugMsgs[2] = " ${ ";
    debugMsgs[3] = id;
    debugMsgs[4] = " [ ";
    debugMsgs[5] = operandeEntier.debugMsg;
    debugMsgs[6] = " ]} )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(operandeEntier.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    free(id);

    return valExpr;
}

valExpr_t rOperandeEntierPlusOuMoinArg(char plusOuMoin, int arg){

    arretRegleNonImplementee("operande_entier -> plus_ou_moin ARG");
    debugBasique("operande_entier -> plus_ou_moin ARG");
    (void)plusOuMoin;
    (void)arg;
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 5;
    char* debugMsgs[taille];
    debugMsgs[0] = "operande_entier( ";
    if(plusOuMoin == '-')
        debugMsgs[1] = "-";
    else
        debugMsgs[1] = "+";
    debugMsgs[2] = " ";
    debugMsgs[3] = malloc(sizeof(char)*(nombreDeChiffre(arg)+1));
    if (debugMsgs[3] == NULL){
        perror("Erreur de malloc");
        exit(EXIT_FAILURE);
    }
    sprintf(debugMsgs[3], "%d", arg);
    debugMsgs[4] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(debugMsgs[3]);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rOperandeEntierEntier(char* entier){

    debugBasique("operande_entier -> ENTIER");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    int entierValeur = conversionChaineVersEntier(entier, 0);
    struct quadop resultat = QUADOP_ENTIER(entierValeur);
    valExpr.resultat = resultat;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "operande_entier( ";
    debugMsgs[1] = entier;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    free(entier);

    return valExpr;
}

valExpr_t rOperandeEntierPlusOuMoinEntier(char plusOuMoin, char* entier){

    debugBasique("operande_entier -> plus_ou_moin ENTIER");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    int entierValeur = conversionChaineVersEntier(entier, plusOuMoin == '-');
    struct quadop resultat = QUADOP_ENTIER(entierValeur);
    valExpr.resultat = resultat;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 5;
    char* debugMsgs[taille];
    debugMsgs[0] = "operande_entier( ";
    if(plusOuMoin == '-')
        debugMsgs[1] = "-";
    else
        debugMsgs[1] = "+";
    debugMsgs[2] = " ";
    debugMsgs[3] = entier;
    debugMsgs[4] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    free(entier);

    return valExpr;
}

valExpr_t rOperandeEntierPoSommeEntierePf(valExpr_t sommeEntiere){

    debugBasique("operande_entier -> ( somme_entiere )");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;
    valExpr.resultat = sommeEntiere.resultat;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "operande_entier( ( ";
    debugMsgs[1] = sommeEntiere.debugMsg;
    debugMsgs[2] = " ) )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(sommeEntiere.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

char rPlusOuMoinPlus(){

    debugBasique("plus_ou_moin -> +");
    return '+';
}

char rPlusOuMoinMoin(){

    debugBasique("plus_ou_moin -> -");
    return '-';
}

char rFoisDivModFois(){

    debugBasique("fois_div_mod -> *");
    return '*';
}

char rFoisDivModDiv(){

    debugBasique("fois_div_mod -> /");
    return '/';
}

char rFoisDivModMod(){

    debugBasique("fois_div_mod -> %");
    return '%';
}

valExpr_t rDeclarationDeFonctionIdPoPfAoDeclLocListeInstructionsAf(char* id, valExpr_t declLoc,
        valExpr_t listeInstructions){

    arretRegleNonImplementee("declaration_de_fonction -> ID (){ decl_loc liste_instructions }");
    debugBasique("declaration_de_fonction -> ID (){ decl_loc liste_instructions }");
    (void)id;
    (void)declLoc;
    (void)listeInstructions;
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 7;
    char* debugMsgs[taille];
    debugMsgs[0] = "declaration_de_fonction( ";
    debugMsgs[1] = id;
    debugMsgs[2] = " (){ ";
    debugMsgs[3] = declLoc.debugMsg;
    debugMsgs[4] = " ";
    debugMsgs[5] = listeInstructions.debugMsg;
    debugMsgs[6] = " } )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(declLoc.debugMsg);
    free(listeInstructions.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    free(id);

    return valExpr;
}

valExpr_t rDeclLocDeclLocLocalIdEgConcatenationPv(valExpr_t declLoc, char* id,
        valExpr_t concatenation){

    arretRegleNonImplementee("decl_loc -> decl_loc LOCAL ID = concatenation ;");
    debugBasique("decl_loc -> decl_loc LOCAL ID = concatenation ;");
    (void)declLoc;
    (void)id;
    (void)concatenation;
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 7;
    char* debugMsgs[taille];
    debugMsgs[0] = "decl_loc( ";
    debugMsgs[1] = declLoc.debugMsg;
    debugMsgs[2] = " local ";
    debugMsgs[3] = id;
    debugMsgs[4] = " = ";
    debugMsgs[5] = concatenation.debugMsg;
    debugMsgs[6] = " ; )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(declLoc.debugMsg);
    free(concatenation.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    free(id);

    return valExpr;
}

valExpr_t rDeclLocEmpty(){

    arretRegleNonImplementee("decl_loc -> %empty");
    debugBasique("decl_loc -> %empty");
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 1;
    char* debugMsgs[taille];
    debugMsgs[0] = "decl_loc( %empty )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    return valExpr;
}

valExpr_t rAppelDeFonctionIdListeOperandes(char* id, valExpr_t listeOperandes){

    arretRegleNonImplementee("appel_de_fonction -> ID liste_operandes");
    debugBasique("appel_de_fonction -> ID liste_operandes");
    (void)id;
    (void)listeOperandes;
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 5;
    char* debugMsgs[taille];
    debugMsgs[0] = "appel_de_fonction( ";
    debugMsgs[1] = id;
    debugMsgs[2] = " ";
    debugMsgs[3] = listeOperandes.debugMsg;
    debugMsgs[4] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);
    free(listeOperandes.debugMsg);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    free(id);

    return valExpr;
}

valExpr_t rAppelDeFonctionId(char* id){

    arretRegleNonImplementee("appel_de_fonction -> ID");
    debugBasique("appel_de_fonction -> ID");
    (void)id;
    valExpr_t valExpr;
    valExpr.debugMsg = NULL;

    #if SYNTAX_DEBUG_AVANCE != 0
    size_t taille = 3;
    char* debugMsgs[taille];
    debugMsgs[0] = "appel_de_fonction( ";
    debugMsgs[1] = id;
    debugMsgs[2] = " )";
    char* debugMsg = debugAvanceMsgConcatenation(debugMsgs, taille);

    valExpr.debugMsg = debugAvanceMsgCreation();
    printf("%s : %s\n", valExpr.debugMsg, debugMsg);
    free(debugMsg);
    #endif

    free(id);

    return valExpr;
}

adresseQuad_t rMQuad(){
    return quads.taille;
}

adresseQuad_t rGQuad(){
    generationCode(QUAD_GOTO());
    return quads.taille - 1;
}
