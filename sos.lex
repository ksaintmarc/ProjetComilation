
%{
#include <stdlib.h>
#include <stdio.h>
#include "analyse.h"
#include "sos.tab.h"
%}

%option nounput
%option noyywrap
%option yylineno

CHAR_NOSPACE [a-zA-Z0-9_]
INTEGER 0|[1-9][0-9]*
IDENTIFIER [a-zA-Z_][a-zA-Z0-9_]*
STRING ([^\'\\\"]|\\.)

%%
    /* Affectation */
=      { return EG; }
    /* Tableau */
\[     { return CO; }
\]     { return CF; }
    /* Parenthèses */
\(     { return PO; }
\)     { return PF; }
    /* Mot-clés prédéfinies */
declare { return DECLARE; }
if      { return IF; }
fi      { return FI; }
then    { return THEN; }
for     { return FOR; }
in      { return IN; }
do      { return DO; }
done    { return DONE; }
while   { return WHILE; }
until   { return UNTIL; }
case    { return CASE; }
esac    { return ESAC; }
elif    { return ELIF; }
else    { return ELSE; }
echo    { return ECHO_; }
read    { return READ; }
return  { return RETURN; }
exit    { return EXIT; }
test    { return TEST; }
expr    { return EXPR; }
local   { return LOCAL; }
    /* Arguments */
\$[1-9][0-9]* { yylval.arg = atoi(yytext+1); return ARG; }
\$\*    { return ALLARGS; }
\$\?    { return STATUT; }
    /* Dollar */ 
\$      { return DOLLAR; }
    /* Opérateurs logiques */
\|      { return OU; }
\*      { return FOIS; }
\{      { return AO; }
\}      { return AF; }
    /* Opérateurs de comparaison */
\-o     { return O; }
\-a     { return A; }
\!      { return EXC; }
\-n     { return N; }
\-z     { return Z; }
\-eq    { return EQ; }
\-ne    { return NE; }
\-gt    { return GT; }
\-ge    { return GE; }
\-lt    { return LT; }
\-le    { return LE; }
    /* Opérateurs arithmétiques */
\+      { return PLUS; }
\-      { return MOINS; }
\/      { return DIV; }
\%      { return MOD; }
    /* Confirmation d'une instruction */
;;      { return DPV; }
;       { return PV; }
    /* Entiers, identifiants et toutes types de chaînes de caractères */
{INTEGER}           { yylval.chaine = copieChaineDeChar(yytext); return ENTIER; }
{IDENTIFIER}        { yylval.chaine = copieChaineDeChar(yytext); return ID; }
{CHAR_NOSPACE}+     { yylval.chaine = copieChaineDeChar(yytext); return AUTREMOT; }
\"{STRING}*\"       {
        yylval.chaine = copieChaineDeChar(yytext + 1);
        yylval.chaine[strlen(yylval.chaine) - 1] = '\0';
        return GCHAINE;
    }
\'{STRING}*\'       {
        yylval.chaine = copieChaineDeChar(yytext + 1);
        yylval.chaine[strlen(yylval.chaine) - 1] = '\0';
        return APCHAINE;
    }
    /* Commentaires et espace vide */
\#[^\n]*\n
[[:space:]]
    /* Erreur si le caractère n'est pas reconnue */
. { 
    fprintf(stderr, "Erreur lexicale (%d) (%s) (Ligne %d)\n",
            *yytext, yytext, yylineno);
    exit(EXIT_FAILURE);
}

%%

void lexFree(){
    yy_delete_buffer(YY_CURRENT_BUFFER);
    free(yy_buffer_stack);
}
