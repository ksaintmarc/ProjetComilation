
%{
#include <stdlib.h>
#include <stdio.h>
#include "analyse.h"

extern int yylex();
extern void yyerror(const char* msg);
%}

%union{
    char* chaine;
    char caractere;
    int arg;
    enum relopType operateurRel;
    valExpr_t valExpr;
    adresseQuad_t premierQuad;
}

%token <chaine> ID
%token <chaine> ENTIER
%token <chaine> AUTREMOT
//Guillemet CHAINE
%token <chaine> GCHAINE
//APostrophe CHAINE
%token <chaine> APCHAINE
//ARGument de fonction ('$1', '$2', etc.),
%token <arg> ARG

%type <chaine> mot

%type <valExpr> programme
%type <valExpr> liste_instructions
%type <valExpr> instruction
%type <valExpr> else_part
%type <valExpr> liste_cas
%type <valExpr> filtre
%type <valExpr> filtre_term
%type <valExpr> liste_operandes
%type <valExpr> concatenation
%type <valExpr> test_bloc
%type <valExpr> test_expr
%type <valExpr> test_expr2
%type <valExpr> test_expr3
%type <valExpr> test_instruction
%type <valExpr> operande
%type <caractere> operateur1
%type <operateurRel> operateur2
%type <valExpr> somme_entiere
%type <valExpr> produit_entier
%type <valExpr> operande_entier
%type <caractere> plus_ou_moin
%type <caractere> fois_div_mod
%type <valExpr> declaration_de_fonction
%type <valExpr> decl_loc
%type <valExpr> appel_de_fonction
%type <premierQuad> m_quad
%type <premierQuad> g_quad

/*
    Point Virgule, EGal, Crochet Ouvrant, Crochet Fermant, DECLARE, IF, Fin If
    THEN, FOR, IN, DO, DONE, WHILE, UNTIL, CASE, ESAC, ELIF, ELSE, ECHO, READ, RETURN, EXIT
    Parenthèse Ouvrante, Parenthèse Fermante, Double Point Virgule, OU ('|'), FOIS ('*')
    DOLLAR ('$'), Accolade Ouvrante, Accolade Fermante, TEST, O ('-o'), A ('-a')
    EXClamation ('!'), STATUT ('$?'), EXPR, N ('-n'), Z ('-z'), EQual ('-eq')
    Not Equal ('-ne'), Greater Than ('-gt'), Greater or Equal ('-ge'), Less Than ('-lt')
    Less or Equal ('-le'), PLUS ('+'), MOIN ('-'), DIV ('/'), MOD ('%'), LOCAL, ALLARGumentS ('$*')
*/
%token PV EG CO CF DECLARE IF FI THEN FOR IN DO DONE WHILE UNTIL CASE ESAC ELIF ELSE
%token ECHO_ READ RETURN EXIT PO PF DPV OU FOIS DOLLAR AO AF TEST O A EXC
%token STATUT EXPR N Z EQ NE GT GE LT LE PLUS MOINS DIV MOD LOCAL ALLARGS

%start programme

%%

programme
: liste_instructions {
    $$ = rProgrammeListeInstructions($1);
    }
;

liste_instructions
: liste_instructions PV m_quad instruction {
    $$ = rListeInstructionsListeInstructionsPvInstruction($1, $3, $4);
    }
| instruction {
    $$ = rListeInstructionsInstruction($1);
    }
;

instruction
: ID EG concatenation {
    $$ = rInstructionIdEgConcatenation($1, $3);
    }
| ID CO operande_entier CF EG concatenation {
    $$ = rInstructionIdCoOperandeEntierCfEgConcatenation($1, $3, $6);
    }
| DECLARE ID CO ENTIER CF {
    $$ = rInstructionDeclareIdCoEntierCf($2, $4);
    }
| IF test_bloc THEN m_quad liste_instructions g_quad m_quad else_part FI {
    $$ = rInstructionIfTestBlocThenListeInstructionsElsePartFi($2, $4, $5, $6, $7, $8);
    }
| FOR ID DO liste_instructions DONE {
    $$ = rInstructionForIdDoListeInstructionsDone($2, $4);
    }
| FOR ID IN {$<premierQuad>$ = miseEnPlaceListeOperandes();}
    liste_operandes DO {
    $<premierQuad>$ = miseEnPlaceFor($2, 1);
    } liste_instructions DONE {
    $$ = rInstructionForIdInListeOperandesDoListeInstructionsDone(
            $2, $<premierQuad>4, $5, $<premierQuad>7, $8);
    }
| WHILE m_quad test_bloc DO m_quad liste_instructions DONE {
    $$ = rInstructionWhileTestBlocDoListeInstructionsDone($2, $3, $5, $6);
    }
| UNTIL m_quad test_bloc DO m_quad liste_instructions DONE {
    $$ = rInstructionUntilTestBlocDoListeInstructionsDone($2, $3, $5, $6);
    }
| CASE operande IN liste_cas ESAC {
    $$ = rInstructionCaseOperandeInListeCasEsac($2, $4);
    }
| ECHO_ {$<premierQuad>$ = miseEnPlaceListeOperandes();} liste_operandes {
    $$ = rInstructionEchoListeOperandes($<premierQuad>2, $3);
    }
| READ ID {
    $$ = rInstructionReadId($2);
    }
| READ ID CO operande_entier CF {
    $$ = rInstructionReadIdCoOperandeEntierCf($2, $4);
    }
| declaration_de_fonction {
    $$ = rInstructionDeclarationDeFonction($1);
    }
| appel_de_fonction {
    $$ = rInstructionAppelDeFonction($1);
    }
| RETURN {
    $$ = rInstructionReturn();
    }
| RETURN operande_entier {
    $$ = rInstructionReturnOperandeEntier($2);
    }
| EXIT {
    $$ = rInstructionExit();
    }
| EXIT operande_entier {
    $$ = rInstructionExitOperandeEntier($2);
    }
;

else_part
: ELIF test_bloc THEN m_quad liste_instructions g_quad m_quad else_part {
    $$ = rElsePartElifTestBlocThenListeInstructionsElsePart($2, $4, $5, $6, $7, $8);
    }
| ELSE liste_instructions {
    $$ = rElsePartElseListeInstructions($2);
    }
| %empty {
    $$ = rElsePartEmpty();
    }
;

liste_cas
: liste_cas filtre PF liste_instructions DPV {
    $$ = rListeCasListeCasFiltrePfListeInstructionsDpv($1, $2, $4);
    }
| filtre PF liste_instructions DPV {
    $$ = rListeCasFiltrePfListeInstructionsDpv($1, $3);
    }
;

//MODIFICATION DE CETTE REGLE DE GRAMMAIRE

/* Version de base
filtre
: mot
| GCHAINE
| APCHAINE
| filtre OU mot
| filtre OU GCHAINE
| filtre OU APCHAINE
| FOIS
;
*/

filtre
: filtre_term {
    $$ = rFiltreFiltreTerm($1);
    }
| filtre OU filtre_term {
    $$ = rFiltreFiltreOuFiltreTerm($1, $3);
    }
| FOIS {
    $$ = rFiltreFois();
    }
;

//Ajout de "filtre_term" pour simplifier la lecture de "filtre"
//Ajout de "mot" pour que les ID et ENTIER soit compris dedans
//Car ID et ENTIER sont des mots

filtre_term
: mot {
    $$ = rFiltreTermMot($1);
    }
| GCHAINE {
    $$ = rFiltreTermGchaine($1);
    }
| APCHAINE {
    $$ = rFiltreTermApchaine($1);
    }
;

//On crée "mot" pour simplifier la catégorisation d'un mot

mot
: AUTREMOT {
    $$ = rMotAutremot($1);
    }
| ID {
    $$ = rMotId($1);
    }
| ENTIER {
    $$ = rMotEntier($1);
    }
;

liste_operandes
: liste_operandes operande {
    $$ = rListeOperandesListeOperandesOperande($1, $2);
    }
| operande {
    $$ = rListeOperandesOperande($1);
    }
| DOLLAR AO ID CO FOIS CF AF {
    $$ = rListeOperandesDollarAoIdCoFoisCfAf($3);
    }
;

concatenation
: concatenation operande {
    $$ = rConcatenationConcatenationOperande($1, $2);
    }
| operande {
    $$ = rConcatenationOperande($1);
    }
;

test_bloc
: TEST test_expr {
    $$ = rTestBlocTestTestExpr($2);
    }
;

test_expr
: test_expr O m_quad test_expr2 {
    $$ = rTestExprTestExprOTestExpr2($1, $3, $4);
    }
| test_expr2 {
    $$ = rTestExprTestExpr2($1);
    }
;

test_expr2
: test_expr2 A m_quad test_expr3 {
    $$ = rTestExpr2TestExpr2ATestExpr3($1, $3, $4);
    }
| test_expr3 {
    $$ = rTestExpr2TestExpr3($1);
    }
;

test_expr3
: PO test_expr PF {
    $$ = rTestExpr3PoTestExprPf($2);
    }
| EXC PO test_expr PF {
    $$ = rTestExpr3ExcPoTestExprPf($3);
    }
| test_instruction {
    $$ = rTestExpr3TestInstruction($1);
    }
| EXC test_instruction {
    $$ = rTestExpr3ExcTestInstruction($2);
    }
;

test_instruction
: concatenation EG concatenation {
    $$ = rTestInstructionConcatenationEgConcatenation($1, $3);
    }
| concatenation EXC EG concatenation {
    $$ = rTestInstructionConcatenationExcEgConcatenation($1, $4);
    }
| operateur1 concatenation {
    $$ = rTestInstructionOperateur1Concatenation($1, $2);
    }
| operande operateur2 operande {
    $$ = rTestInstructionOperandeOperateur2Operande($1, $2, $3);
    }
;

//Ajout de "mot" pour que les ID et ENTIER soit compris dedans
//Car ID et ENTIER sont des mots

operande
: DOLLAR AO ID AF {
    $$ = rOperandeDollarAoIdAf($3);
    }
| DOLLAR AO ID CO operande_entier CF AF {
    $$ = rOperandeDollarAoIdCoOperandeEntierCfAf($3, $5);
    }
| mot {
    $$ = rOperandeMot($1);
    }
| ARG {
    $$ = rOperandeArg($1);
    }
| ALLARGS {
    $$ = rOperandeAllargs();
    }
| STATUT {
    $$ = rOperandeStatut();
    }
| GCHAINE {
    $$ = rOperandeGchaine($1);
    }
| APCHAINE {
    $$ = rOperandeApchaine($1);
    }
| DOLLAR PO EXPR somme_entiere PF {
    $$ = rOperandeDollarPoExprSommeEntierePf($4);
    }
| DOLLAR PO appel_de_fonction PF {
    $$ = rOperandeDollarPoAppelDeFonctionPf($3);
    }
;

operateur1
: N {
    $$ = rOperateur1N();
    }
| Z {
    $$ = rOperateur1Z();
    }
;

operateur2
: EQ {
    $$ = rOperateur2Eq();
    }
| NE {
    $$ = rOperateur2Ne();
    }
| GT {
    $$ = rOperateur2Gt();
    }
| GE {
    $$ = rOperateur2Ge();
    }
| LT {
    $$ = rOperateur2Lt();
    }
| LE {
    $$ = rOperateur2Le();
    }
;

somme_entiere
: somme_entiere plus_ou_moin produit_entier {
    $$ = rSommeEntiereSommeEntierePlusOuMoinProduitEntier($1, $2, $3);
    }
| produit_entier {
    $$ = rSommeEntiereProduitEntier($1);
    }
;

produit_entier
: produit_entier fois_div_mod operande_entier {
    $$ = rProduitEntierProduitEntierFoisDivModOperandeEntier($1, $2, $3);
    }
| operande_entier {
    $$ = rProduitEntierOperandeEntier($1);
    }
;

operande_entier
: DOLLAR AO ID AF {
    $$ = rOperandeEntierDollarAoIdAf($3);
    }
| DOLLAR AO ID CO operande_entier CF AF {
    $$ = rOperandeEntierDollarAoIdCoOperandeEntierCfAf($3, $5);
    }
| ARG {
    $$ = rOperandeEntierArg($1);
    }
| plus_ou_moin DOLLAR AO ID AF {
    $$ = rOperandeEntierPlusOuMoinDollarAoIdAf($1, $4);
    }
| plus_ou_moin DOLLAR AO ID CO operande_entier CF AF {
    $$ = rOperandeEntierPlusOuMoinDollarAoIdCoOperandeEntierCfAf($1, $4, $6);
    }
| plus_ou_moin ARG {
    $$ = rOperandeEntierPlusOuMoinArg($1, $2);
    }
| ENTIER {
    $$ = rOperandeEntierEntier($1);
    }
| plus_ou_moin ENTIER {
    $$ = rOperandeEntierPlusOuMoinEntier($1, $2);
    }
| PO somme_entiere PF {
    $$ = rOperandeEntierPoSommeEntierePf($2);
    }
;

plus_ou_moin
: PLUS {
    $$ = rPlusOuMoinPlus();
    }
| MOINS {
    $$ = rPlusOuMoinMoin();
    }
;

fois_div_mod
: FOIS {
    $$ = rFoisDivModFois();
    }
| DIV {
    $$ = rFoisDivModDiv();
    }
| MOD {
    $$ = rFoisDivModMod();
    }
;

declaration_de_fonction
: ID PO PF AO decl_loc liste_instructions AF {
    $$ = rDeclarationDeFonctionIdPoPfAoDeclLocListeInstructionsAf($1, $5, $6);
    }
;

decl_loc
: decl_loc LOCAL ID EG concatenation PV {
    $$ = rDeclLocDeclLocLocalIdEgConcatenationPv($1, $3, $5);
    }
| %empty {
    $$ = rDeclLocEmpty();
    }
;

appel_de_fonction
: ID liste_operandes {
    $$ = rAppelDeFonctionIdListeOperandes($1, $2);
    }
| ID {
    $$ = rAppelDeFonctionId($1);
    }
;

m_quad
: %empty {
    $$ = rMQuad();
    }
;

g_quad
: %empty {
    $$ = rGQuad();
    }
;

%%
