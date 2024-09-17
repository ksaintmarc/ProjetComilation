/**
 * @file analyse.h
 * @brief Analyseur syntaxique
 * 
 * Permet de faire de l'analyse syntaxique et de la génération de code
 * à partir des règles de grammaire définies dans le bison (cf. sos.y).
 * 
 */

#ifndef ANALYSE_H
#define ANALYSE_H

#include "quad.h"

/**
 * @enum relopType
 * @brief Type d'un opérateur relationnel
 * 
 * @var RO_EGAL
 * @brief Opérateur de comparaison d'égalité d'entiers
 * 
 * @var RO_NEGAL
 * @brief Opérateur de comparaison d'inégalité d'entiers
 * 
 * @var RO_INF
 * @brief Opérateur de comparaison strictement inférieur
 * 
 * @var RO_INFEGAL
 * @brief Opérateur de comparaison inférieur ou égal
 * 
 * @var RO_SUP
 * @brief Opérateur de comparaison strictement supérieur
 * 
 * @var RO_SUPEGAL
 * @brief Opérateur de comparaison supérieur ou égal
 * 
 */
enum relopType {
    RO_EGAL,
    RO_NEGAL,
    RO_INF,
    RO_INFEGAL,
    RO_SUP,
    RO_SUPEGAL
};

/**
 * @brief Conversion d'un entier sous forme de chaine en entier
 * 
 * @param chaine : chaine à convertir en entier
 * @param chaine : si le nombre est négatif
 * @return Le nombre représentant la chaine
 */
int conversionChaineVersEntier(char* chaine, int estNegatif);

/**
 * @brief Ajoute une chaine à la liste des chaines littérales
 * 
 * @param chaine : chaine à ajouter
 * @return L'index de la chaine dans la liste
 */
size_t generationChaine(char* chaine);

/**
 * @brief Crée une nouvelle variable temporaire
 * 
 * @return Le nom de la nouvelle variable temporaire 
 */
char* nouveauTemporaire();

/**
 * @brief Ajoute un quad à la liste des quads
 * @note Cette fonction est utilisée pour indiquer les quads à compléter
 * 
 * @param q : Quad à ajouter
 */
void generationCode(struct quad q);

/**
 * @brief Complète les quads à l'adresse donnée et libère la liste d'adresses de quads
 * 
 * @param l : Liste d'adresses de quads
 * @param adresse : Adresse du quad à compléter
 */
inline static void completeFree(struct listeAdresseQuad* l,
        adresseQuad_t adresse);

/**
 * @struct valExpr_t
 * @brief Structure de retour des réductions
 * 
 * @var valExpr_t::debugMsg
 * @brief Message de debug
 * 
 * @var valExpr_t::resultat
 * @brief Valeur de retour
 * 
 * @var valExpr_t::suivant
 * @brief Liste des adresses des quads à compléter
 * 
 * @var valExpr_t::vrai
 * @brief Liste des adresses des quads à compléter si la valeur est vraie
 * 
 * @var valExpr_t::faux
 * @brief Liste des adresses des quads à compléter si la valeur est fausse
 * 
 */
typedef struct {
    char* debugMsg;
    struct quadop resultat;
    struct listeAdresseQuad suivant;
    struct listeAdresseQuad vrai; 
    struct listeAdresseQuad faux;
} valExpr_t;

/**
 * @brief Duplique une chaine de caractères
 * 
 * @param source : Chaine de caractères à dupliquer
 * @return La chaine de caractères dupliquée
 * 
 */
char* copieChaineDeChar(char* source);

/**
 * @brief Créer un quad QUAD_STACK à 0 par défaut.
 * 
 * @return L'adresse du QUAD_STACK
 * 
 */
adresseQuad_t miseEnPlaceListeOperandes();

/**
 * @brief Crée l'entête d'un for et de sa boucle
 * 
 * @param id : L'identifiant de la variable de la boucle
 * @param estForIn : 1 pour for ... in ... do, 0 pour for ... do
 * 
 * @return L'adresse du premier quadruplet de la boucle
 * 
 */
adresseQuad_t miseEnPlaceFor(char* id, int estForIn);

/**
 * @brief Règle "ListeInstructions" dans "Programme"
 * 
 * @param listeInstructions : Liste d'instructions
 * @return un valExpr_t
 *
 */
valExpr_t rProgrammeListeInstructions(valExpr_t listeInstructions);

/**
 * @brief Règle "liste_instructions PV instruction" dans "liste_instructions"
 * 
 * @param listeInstructions : Liste d'instructions
 * @param marqueur : Marqueur
 * @param instruction : Instruction
 * @return un valExpr_t
 * 
 */
valExpr_t rListeInstructionsListeInstructionsPvInstruction(valExpr_t listeInstructions, adresseQuad_t marqueur,
        valExpr_t instruction);

/**
 * @brief Règle "instruction" dans "liste_instructions"
 * 
 * @param instruction : Instruction
 * @return un valExpr_t
 * 
 */
valExpr_t rListeInstructionsInstruction(valExpr_t instruction);

/**
 * @brief Règle "Id EG concatenation" dans "instruction"
 * 
 * @param id : Identifiant
 * @param concatenation : Concaténation
 * @return un valExpr_t
 * 
 */
valExpr_t rInstructionIdEgConcatenation(char* id, valExpr_t concatenation);

/**
 * @brief Règle "ID CO operande_entier CF EG concatenation" dans "instruction"
 * 
 * @param id : Identifiant
 * @param operandeEntier : Opérande entière
 * @param concatenation : Concaténation
 * @return un valExpr_t
 * 
 */
valExpr_t rInstructionIdCoOperandeEntierCfEgConcatenation(char* id, valExpr_t operandeEntier, 
        valExpr_t concatenation);

/**
 * @brief Règle "declare ID CO ENTIER CF" dans "instruction"
 * 
 * @param id : Identifiant
 * @param entier : Entier
 * @return un valExpr_t
 * 
 */
valExpr_t rInstructionDeclareIdCoEntierCf(char* id, char* entier);

/**
 * @brief Règle "IF test_bloc THEN m_quad liste_instructions g_quad ELSE liste_instructions FI" dans "instruction"
 * 
 * @param testBloc 
 * @param marqueurM
 * @param listeInstructions 
 * @param marqueurG
 * @param elsePart 
 * @return valExpr_t 
 * 
 */
valExpr_t rInstructionIfTestBlocThenListeInstructionsElsePartFi(valExpr_t testBloc, 
        adresseQuad_t marqueurM1 ,valExpr_t listeInstructions, adresseQuad_t marqueurG,
        adresseQuad_t marqueurM2 ,valExpr_t elsePart);

/**
 * @brief Règle "FOR ID DO liste_instructions DONE" dans "instruction"
 * 
 * @param id : Identifiant
 * @param instruction : Instruction
 * @return un valExpr_t
 * 
 */
valExpr_t rInstructionForIdDoListeInstructionsDone(char* id, valExpr_t instruction);

/**
 * @brief Règle "FOR ID IN liste_operandes DO liste_instructions DONE" dans "instruction"
 * 
 * @param id : Identifiant
 * @param listeOperandes : Liste d'opérandes
 * @param premierQuad : L'adresse du premier quadruplet de la boucle
 * @param listeInstructions : Liste d'instructions
 * @return un valExpr_t
 * 
 */
valExpr_t rInstructionForIdInListeOperandesDoListeInstructionsDone(
        char* id, adresseQuad_t addrQuadStack,
        valExpr_t listeOperandes, adresseQuad_t premierQuad,
        valExpr_t listeInstructions);

/**
 * @brief Règle "WHILE test_bloc DO liste_instructions DONE" dans "instruction"
 * 
 * @param testBloc : Bloc de test
 * @param listeInstructions : Liste d'instructions
 * @return un valExpr_t
 * 
 */
valExpr_t rInstructionWhileTestBlocDoListeInstructionsDone(adresseQuad_t marqueurM1,
        valExpr_t testBloc, adresseQuad_t marqueurM2, valExpr_t listeInstructions);

/**
 * @brief Règle "UNTIL test_bloc DO liste_instructions DONE" dans "instruction"
 * 
 * @param testBloc : Bloc de test
 * @param listeInstructions : Liste d'instructions
 * @return un valExpr_t
 * 
 */
valExpr_t rInstructionUntilTestBlocDoListeInstructionsDone(adresseQuad_t marqueurM1,
        valExpr_t testBloc, adresseQuad_t marqueurM2, valExpr_t listeInstructions);

/**
 * @brief Règle "CASE operande IN liste_cas ESAC" dans "instruction"
 * 
 * @param operande : Opérande
 * @param listeCas : Liste de cas
 * @return un valExpr_t
 * 
 */
valExpr_t rInstructionCaseOperandeInListeCasEsac(valExpr_t operande, valExpr_t listeCas);

/**
 * @brief Règle "ECHO_ liste_operandes" dans "instruction"
 * 
 * @param listeOperande : Liste d'opérandes 
 * @return un valExpr_t
 * 
 */
valExpr_t rInstructionEchoListeOperandes(adresseQuad_t addrQuadStack, valExpr_t listeOperande);

/**
 * @brief Règle "READ ID" dans "instruction"
 * 
 * @param id : Identifiant
 * @return un valExpr_t
 * 
 */
valExpr_t rInstructionReadId(char* id);

/**
 * @brief Règle "READ ID CO operande_entier CF" dans "instruction"
 * 
 * @param id : Identifiant
 * @param operandeEntier : Opérande entière
 * @return un valExpr_t
 * 
 */
valExpr_t rInstructionReadIdCoOperandeEntierCf(char* id, valExpr_t operandeEntier);

/**
 * @brief Règle "declaration_de_fonction" dans "instruction"
 * 
 * @param declarationDeFonction : Déclaration de fonction
 * @return un valExpr_t
 * 
 */
valExpr_t rInstructionDeclarationDeFonction(valExpr_t declarationDeFonction);

/**
 * @brief Règle "appel_de_fonction" dans "instruction"
 * 
 * @param appelDeFonction : Appel de fonction
 * @return un valExpr_t
 * 
 */
valExpr_t rInstructionAppelDeFonction(valExpr_t appelDeFonction);

/**
 * @brief Règle "RETURN" dans "instruction"
 * 
 * @return un valExpr_t
 * 
 */
valExpr_t rInstructionReturn();

/**
 * @brief Règle "RETURN operande_entier" dans "instruction"
 * 
 * @param operandeEntier : Opérande entière
 * @return un valExpr_t
 * 
 */
valExpr_t rInstructionReturnOperandeEntier(valExpr_t operandeEntier);

/**
 * @brief Règle "EXIT" dans "instruction"
 * 
 * @return un valExpr_t
 * 
 */
valExpr_t rInstructionExit();

/**
 * @brief Règle "EXIT operande_entier" dans "instruction"
 * 
 * @param operandeEntier : Opérande entière
 * @return un valExpr_t
 * 
 */
valExpr_t rInstructionExitOperandeEntier(valExpr_t operandeEntier);

/**
 * @brief Règle "ELIF test_bloc THEN liste_instructions else_part" dans "else_part"
 * 
 * @param testBloc : Bloc de test
 * @param listeInstructions : Liste d'instructions
 * @param elsePart : Partie else
 * @return un valExpr_t
 * 
 */
valExpr_t rElsePartElifTestBlocThenListeInstructionsElsePart(valExpr_t testBloc, 
        adresseQuad_t marqueurM1 ,valExpr_t listeInstructions, adresseQuad_t marqueurG,
        adresseQuad_t marqueurM2 ,valExpr_t elsePart);

/**
 * @brief Règle "ELSE liste_instructions" dans "else_part"
 * 
 * @param listeInstructions : Liste d'instructions
 * @return un valExpr_t
 * 
 */
valExpr_t rElsePartElseListeInstructions(valExpr_t listeInstructions);

/**
 * @brief Règle "%empty" dans "else_part"
 * 
 * @return un valExpr_t
 */
valExpr_t rElsePartEmpty();

/**
 * @brief Règle "liste_cas filtre PF liste_instructions DPV" dans "liste_cas"
 * 
 * @param listeCas : Liste de cas
 * @param filtre : Filtre
 * @param listeInstructions : Liste d'instructions
 * @return un valExpr_t
 * 
 */
valExpr_t rListeCasListeCasFiltrePfListeInstructionsDpv(valExpr_t listeCas, 
        valExpr_t filtre, valExpr_t listeInstructions);

/**
 * @brief Règle "filtre PF liste_instructions DPV" dans "liste_cas"
 * 
 * @param filtre : Filtre
 * @param listeInstructions : Liste d'instructions 
 * @return un valExpr_t
 * 
 */
valExpr_t rListeCasFiltrePfListeInstructionsDpv(valExpr_t filtre, valExpr_t listeInstructions);

/**
 * @brief Règle "filtre_term" dans "filtre"
 * 
 * @param filtreTerm : Terme de filtre
 * @return un valExpr_t
 * 
 */
valExpr_t rFiltreFiltreTerm(valExpr_t filtreTerm);

/**
 * @brief Règle "filtre OU filtre_term" dans "filtre"
 * 
 * @param filtre : Filtre
 * @param filtreTerm : Terme de filtre
 * @return un valExpr_t
 * 
 */
valExpr_t rFiltreFiltreOuFiltreTerm(valExpr_t filtre, valExpr_t filtreTerm);

/**
 * @brief Règle "FOIS" dans "filtre"
 * 
 * @return un valExpr_t
 * 
 */
valExpr_t rFiltreFois();

/**
 * @brief Règle "mot" dans "filtre_term"
 * 
 * @param mot : Mot
 * @return un valExpr_t
 *  
 */
valExpr_t rFiltreTermMot(char* mot);

/**
 * @brief Règle "GCHAINE" dans "filtre_term"
 * 
 * @param gchaine : Chaine de caractères avec double guillemets
 * @return un valExpr_t
 * 
 */
valExpr_t rFiltreTermGchaine(char* gchaine);

/**
 * @brief Règle "ACHAINE" dans "filtre_term"
 * 
 * @param apchaine : Chaine de caractères avec simple guillemets
 * @return un valExpr_t
 * 
 */
valExpr_t rFiltreTermApchaine(char* apchaine);

/**
 * @brief Règle "autremot" dans "mot"
 * 
 * @param autremot : Chaine de caractères quelconque
 * @return un valExpr_t
 * 
 */
char* rMotAutremot(char* autremot);

/**
 * @brief Règle "id" dans "mot"
 * 
 * @param id : Identifiant
 * @return un valExpr_t
 * 
 */
char* rMotId(char* id);

/**
 * @brief Règle "entier" dans "mot"
 * 
 * @param entier : Entier
 * @return un valExpr_t
 * 
 */
char* rMotEntier(char* entier);

/**
 * @brief Règle "liste_operandes operande" dans "liste_operandes"
 * 
 * @param listeOperandes : Liste d'opérandes
 * @param operande : Opérande
 * @return un valExpr_t
 * 
 */
valExpr_t rListeOperandesListeOperandesOperande(valExpr_t listeOperandes, valExpr_t operande);

/**
 * @brief Règle "operande" dans "liste_operandes"
 * 
 * @param operande : Opérande
 * @return un valExpr_t
 * 
 */
valExpr_t rListeOperandesOperande(valExpr_t operande);

/**
 * @brief Règle "DOLLAR AO ID CO FOIS CF AF" dans "liste_operandes"
 * 
 * @param id : Identifiant
 * @return un valExpr_t
 * 
 */
valExpr_t rListeOperandesDollarAoIdCoFoisCfAf(char* id);

/**
 * @brief Règle "concatenation_operande" dans "operande"
 * 
 * @param concatenation : Concaténation
 * @param operande : Opérande
 * @return un valExpr_t
 * 
 */
valExpr_t rConcatenationConcatenationOperande(valExpr_t concatenation, valExpr_t operande);

/**
 * @brief Règle "operande" dans "concatenation"
 * 
 * @param operande : Opérande
 * @return un valExpr_t
 * 
 */
valExpr_t rConcatenationOperande(valExpr_t operande);

/**
 * @brief Règle "TEST test_expr" dans "test_bloc"
 * 
 * @param testExpr : Expression de test
 * @return un valExpr_t
 * 
 */
valExpr_t rTestBlocTestTestExpr(valExpr_t testExpr);

/**
 * @brief Règle "test_expr O test_expr2" dans "test_bloc"
 * 
 * @param testExpr : Expression de test
 * @param marqueur : Marqueur
 * @param testExpr2 : Expression de test
 * @return un valExpr_t
 * 
 */
valExpr_t rTestExprTestExprOTestExpr2(valExpr_t testExpr, adresseQuad_t marqueur, valExpr_t testExpr2);

/**
 * @brief Règle "test_expr2" dans "test_expr"
 * 
 * @param testExpr2 : Expression de test
 * @return un valExpr_t
 * 
 */
valExpr_t rTestExprTestExpr2(valExpr_t testExpr2);

/**
 * @brief Règle "test_expr2 A test_expr3" dans "test_expr2"
 * 
 * @param testExpr2 : Expression de test
 * @param marqueur : Marqueur
 * @param testExpr3 : Expression de test
 * @return un valExpr_t
 * 
 */
valExpr_t rTestExpr2TestExpr2ATestExpr3(valExpr_t testExpr2, adresseQuad_t marqueur, valExpr_t testExpr3);

/**
 * @brief Règle "test_expr3" dans "test_expr2"
 * 
 * @param testExpr3 : Expression de test
 * @return un valExpr_t
 * 
 */
valExpr_t rTestExpr2TestExpr3(valExpr_t testExpr3);

/**
 * @brief Règle "PO test_expr PF" dans "test_expr3"
 * 
 * @param testExpr : Expression de test
 * @return un valExpr_t
 * 
 */
valExpr_t rTestExpr3PoTestExprPf(valExpr_t testExpr);

/**
 * @brief Règle "EXC PO test_expr PF" dans "test_expr3"
 * 
 * @param testExpr : Expression de test
 * @return un valExpr_t
 * 
 */
valExpr_t rTestExpr3ExcPoTestExprPf(valExpr_t testExpr);

/**
 * @brief Règle "test_instruction" dans "test_expr3"
 * 
 * @param testInstruction : Instruction de test
 * @return un valExpr_t
 * 
 */
valExpr_t rTestExpr3TestInstruction(valExpr_t testInstruction);

/**
 * @brief Règle "EXC test_instruction" dans "test_expr3"
 * 
 * @param testInstruction : Instruction de test
 * @return un valExpr_t
 * 
 */
valExpr_t rTestExpr3ExcTestInstruction(valExpr_t testInstruction);

/**
 * @brief Règle "concatenation EG concatenation" dans "test_instruction"
 * 
 * @param concatenation1 : Concaténation
 * @param concatenation2 : Concaténation
 * @return un valExpr_t
 * 
 */
valExpr_t rTestInstructionConcatenationEgConcatenation(valExpr_t concatenation1,
        valExpr_t concatenation2);

/**
 * @brief Règle "concatenation EXC EG concatenation" dans "test_instruction"
 * 
 * @param concatenation1 : Concaténation
 * @param concatenation2 : Concaténation
 * @return un valExpr_t
 * 
 */
valExpr_t rTestInstructionConcatenationExcEgConcatenation(valExpr_t concatenation1,
        valExpr_t concatenation2);

/**
 * @brief Règle "operateur1 concatenation" dans "test_instruction"
 * 
 * @param operateur1 : Opérateur
 * @param concatenation : Concaténation
 * @return un valExpr_t
 * 
 */
valExpr_t rTestInstructionOperateur1Concatenation(char operateur1, valExpr_t concatenation);

/**
 * @brief Règle "operande operateur2 operande " dans "test_instruction"
 * 
 * @param operande1 : Opérande
 * @param operateur2 : Opérateur
 * @param operande2 : Opérande
 * @return un valExpr_t
 * 
 */
valExpr_t rTestInstructionOperandeOperateur2Operande(valExpr_t operande1, enum relopType operateur2,
        valExpr_t operande2);

/**
 * @brief Règle "DOLLAR AO ID AF" dans "operande"
 * 
 * @param id : Identifiant
 * @return un valExpr_t
 * 
 */
valExpr_t rOperandeDollarAoIdAf(char* id);

/**
 * @brief Règle "DOLLAR AO ID CO operande_entier CF AF" dans "operande"
 * 
 * @param id : Identifiant
 * @param operandeEntier : Opérande entière
 * @return un valExpr_t
 * 
 */
valExpr_t rOperandeDollarAoIdCoOperandeEntierCfAf(char* id, valExpr_t operandeEntier);

/**
 * @brief Règle "mot" dans "operande"
 * 
 * @param mot : Mot
 * @return un valExpr_t
 *  
 */
valExpr_t rOperandeMot(char* mot);

/**
 * @brief Règle "ARG" dans "operande"
 * 
 * @param arg : Numéro de l'argument
 * @return un valExpr_t
 */
valExpr_t rOperandeArg(int arg);

/**
 * @brief Règle "ALLARGS" dans "operande"
 * 
 * @return un valExpr_t
 */
valExpr_t rOperandeAllargs();

/**
 * @brief Règle "STATUT" dans "operande"
 * 
 * @return un valExpr_t
 */
valExpr_t rOperandeStatut();

/**
 * @brief Règle "GCHAINE" dans "operande"
 * 
 * @param gchaine : Chaîne de caractères avec double guillemets
 * @return un valExpr_t
 */
valExpr_t rOperandeGchaine(char* gchaine);

/**
 * @brief Règle "APCHAINE" dans "operande"
 * 
 * @param apchaine : Chaîne de caractères avec simple guillemets
 * @return un valExpr_t
 */
valExpr_t rOperandeApchaine(char* apchaine);

/**
 * @brief Règle "DOLLAR PO EXPR somme_entiere PF" dans "operande"
 * 
 * @param sommeEntiere : Somme entière
 * @return un valExpr_t
 * 
 */
valExpr_t rOperandeDollarPoExprSommeEntierePf(valExpr_t sommeEntiere);

/**
 * @brief Règle "DOLLAR PO appel_de_fonction PF" dans "operande"
 * 
 * @param appelDeFonction : Appel de fonction
 * @return un valExpr_t
 * 
 */
valExpr_t rOperandeDollarPoAppelDeFonctionPf(valExpr_t appelDeFonction);

/**
 * @brief Règle "N" dans "operateur1"
 * 
 * @return 'n'
 * 
 */
char rOperateur1N();

/**
 * @brief Règle "Z" dans "operateur1"
 * 
 * @return 'z'
 * 
 */
char rOperateur1Z();

/**
 * @brief Règle "R" dans "operateur1"
 * 
 * @return RO_EGAL
 * 
 */
enum relopType rOperateur2Eq();

/**
 * @brief Règle "NE" dans "operateur2"
 * 
 * @return RO_NEGAL
 * 
 */
enum relopType rOperateur2Ne();

/**
 * @brief Règle "GT" dans "operateur2"
 * 
 * @return RO_SUP
 * 
 */
enum relopType rOperateur2Gt();

/**
 * @brief Règle "GE" dans "operateur2"
 * 
 * @return RO_SUPEGAL
 * 
 */
enum relopType rOperateur2Ge();

/**
 * @brief Règle "LT" dans "operateur2"
 * 
 * @return RO_INF
 * 
 */
enum relopType rOperateur2Lt();

/**
 * @brief Règle "LE" dans "operateur2"
 * 
 * @return RO_INFEGAL
 * 
 */
enum relopType rOperateur2Le();

/**
 * @brief Règle "somme_entiere plus_ou_moin produit_entier" dans "somme_entiere"
 * 
 * @param sommeEntiere : Somme entière
 * @param plusOuMoin : Plus ou moins
 * @param produitEntier : Produit entier
 * @return un valExpr_t
 * 
 */
valExpr_t rSommeEntiereSommeEntierePlusOuMoinProduitEntier(valExpr_t sommeEntiere,
        char plusOuMoin, valExpr_t produitEntier);

/**
 * @brief Règle "produit_entier" dans "somme_entiere"
 * 
 * @param produitEntier : Produit entier
 * @return un valExpr_t
 * 
 */
valExpr_t rSommeEntiereProduitEntier(valExpr_t produitEntier);

/**
 * @brief Règle "produit_entier fois_div_mod operande_entier" dans "produit_entier"
 * 
 * @param produitEntier : Produit entier
 * @param foisDivMod : Fois, div ou mod
 * @param operandeEntier : Opérande entier
 * @return un valExpr_t
 * 
 */
valExpr_t rProduitEntierProduitEntierFoisDivModOperandeEntier(valExpr_t produitEntier,
        char foisDivMod, valExpr_t operandeEntier);

/**
 * @brief Règle "operande_entier" dans "produit_entier"
 * 
 * @param operandeEntier : Opérande entier
 * @return un valExpr_t
 *  
 */
valExpr_t rProduitEntierOperandeEntier(valExpr_t operandeEntier);

/**
 * @brief Règle "DOLLAR AO ID AF" dans "operande_entier"
 * 
 * @param id : Identifiant
 * @return un valExpr_t
 * 
 */
valExpr_t rOperandeEntierDollarAoIdAf(char* id);

/**
 * @brief Règle "DOLLAR AO ID CO operande_entier CF" dans "operande_entier"
 * 
 * @param id : Identifiant
 * @param operandeEntier : Opérande entier
 * @return un valExpr_t
 * 
 */
valExpr_t rOperandeEntierDollarAoIdCoOperandeEntierCfAf(char* id, valExpr_t operandeEntier);

/**
 * @brief Règle "ARG" dans "operande_entier"
 * 
 * @param arg : Argument
 * @return un valExpr_t
 * 
 */
valExpr_t rOperandeEntierArg(int arg);

/**
 * @brief Règle "plus_ou_moin DOLLAR AO ID AF" dans "operande_entier"
 * 
 * @param plusOuMoin : Plus ou moins
 * @param id : Identifiant
 * @return un valExpr_t
 * 
 */
valExpr_t rOperandeEntierPlusOuMoinDollarAoIdAf(char plusOuMoin, char* id);

/**
 * @brief Règle "plus_ou_moin DOLLAR AO ID CO operande_entier CF" dans "operande_entier"
 * 
 * @param plusOuMoin : Plus ou moins
 * @param id : Identifiant
 * @param operandeEntier : Opérande entier
 * @return un valExpr_t
 * 
 */
valExpr_t rOperandeEntierPlusOuMoinDollarAoIdCoOperandeEntierCfAf(char plusOuMoin,
        char* id, valExpr_t operandeEntier);

/**
 * @brief Règle "plus_ou_moin ARG" dans "operande_entier"
 * 
 * @param plusOuMoin : Plus ou moins
 * @param arg : Argument
 * @return un valExpr_t
 *  
 */
valExpr_t rOperandeEntierPlusOuMoinArg(char plusOuMoin, int arg);

/**
 * @brief Règle "ENTIER" dans "operande_entier"
 * 
 * @param entier : Entier
 * @return un valExpr_t
 * 
 */
valExpr_t rOperandeEntierEntier(char* entier);

/**
 * @brief Règle "plus_ou_moin ENTIER" dans "operande_entier"
 * 
 * @param plusOuMoin : Plus ou moins
 * @param entier : Entier
 * @return un valExpr_t
 * 
 */
valExpr_t rOperandeEntierPlusOuMoinEntier(char plusOuMoin, char* entier);

/**
 * @brief Règle "PO somme_entiere PF"
 * 
 * @param sommeEntiere : Somme entière
 * @return un valExpr_t 
 * 
 */
valExpr_t rOperandeEntierPoSommeEntierePf(valExpr_t sommeEntiere);

/**
 * @brief Règle "PLUS" dans "plus_ou_moin"
 * 
 * @return un valExpr_t
 * 
 */
char rPlusOuMoinPlus();

/**
 * @brief Règle "MOINS" dans "plus_ou_moin"
 * 
 * @return un valExpr_t
 * 
 */
char rPlusOuMoinMoin();

/**
 * @brief Règle "FOIS" dans "fois_div_mod"
 * 
 * @return un valExpr_t
 * 
 */
char rFoisDivModFois();

/**
 * @brief Règle "DIV" dans "fois_div_mod"
 * 
 * @return un valExpr_t
 * 
 */
char rFoisDivModDiv();

/**
 * @brief Règle "MOD" dans "fois_div_mod"
 * 
 * @return un valExpr_t
 * 
 */
char rFoisDivModMod();

/**
 * @brief Règle "ID PO PF AO decl_loc liste_instructions AF" dans "declaration_de_fonction"
 * 
 * @param id : Identifiant
 * @param declLoc : Déclaration locale
 * @param listeInstructions : Liste d'instructions
 * @return un valExpr_t
 * 
 */
valExpr_t rDeclarationDeFonctionIdPoPfAoDeclLocListeInstructionsAf(char* id, valExpr_t declLoc,
        valExpr_t listeInstructions);

/**
 * @brief Règle "decl_loc LOCAL ID EG concatenation PV" dans "decl_loc"
 * 
 * @param declLoc : Déclaration locale
 * @param id : Identifiant
 * @param concatenation : Concaténation
 * @return un valExpr_t
 * 
 */
valExpr_t rDeclLocDeclLocLocalIdEgConcatenationPv(valExpr_t declLoc, char* id,
        valExpr_t concatenation);

/**
 * @brief Règle "%empty" dans "decl_loc"
 * 
 * @return un valExpr_t
 * 
 */
valExpr_t rDeclLocEmpty();

/**
 * @brief Règle "ID liste_operandes" dans "appel_de_fonction"
 * 
 * @param id : Identifiant
 * @param listeOperandes : Liste d'opérandes
 * @return un valExpr_t
 * 
 */
valExpr_t rAppelDeFonctionIdListeOperandes(char* id, valExpr_t listeOperandes);

/**
 * @brief Règle "ID" dans "appel_de_fonction"
 * 
 * @param id : Identifiant
 * @return un valExpr_t
 * 
 */
valExpr_t rAppelDeFonctionId(char* id);

/**
 * @brief Règle marqueur pour récupérer l'adresse du prochain quadruplet
 * 
 * @return l'adresse du prochain quadruplet
 * 
 */
adresseQuad_t rMQuad();

/**
 * @brief Règle marqueur pour récupérer l'adresse du prochain goto
 * 
 * @return l'adresse du prochain goto
 * 
 */
adresseQuad_t rGQuad();

#endif
