#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "mips.h"
#include "mips_lib.h"
#include "symbol_table.h"
#include "check.h"


extern FILE *yyout;


static void fputcChk(char c){
    CHECK(fputc(c, yyout == NULL ? stdout : yyout) != EOF);
}


static void fputsChk(const char* s){
    CHECK(fputs(s, yyout == NULL ? stdout : yyout) != EOF);
}


static void fprintfChk(const char* format, ...){

    va_list ap;
    va_start(ap, format);

    CHECK(vfprintf(yyout == NULL ? stdout : yyout, format, ap) >= 0);

    va_end(ap);
}


sym_t* pilePorteeRechercheChk(char* id){

    sym_t* res = pilePorteeRecherche(id);

    if (res == NULL){
        fprintf(stderr, "Erreur : Identifiant %s non défini\n", id);
        exit(EXIT_FAILURE);
    }

    return res;
}


static void ecrireLabelQuad(size_t id){

    fprintfChk("q_%lu", id);
}


static int charValide(char c){
    return c >= '0' && c <= '9' || c >= 'A' && c < 'Z' || c >= 'a' && c <= 'z'
            || c == '_';
}


static void ecrireLabelChaineLitterale(size_t id){
    fprintfChk("l_s_%lu", id);
}


static void ecrireLabelVariableGlobale(sym_t* symbole){

    char nomPref = 'e'; // explicite
    const char* nom = symbole->nom;

    if (!charValide(nom[0])){
        nomPref = 'i'; // implicite
        ++nom;
    }

    // On souhaite avoir un nom unique pour éviter les doublons
    // donc on utilise l'adresse de l'entrée dans la table des symboles
    // Autrement :
    // - v : variable (contre f fonction ou q quadruplet ou a tableau)
    // - g : globale (à fin d'exhaustivité ; les locales n'ont pas de label)
    fprintfChk("v_g_%c_%s_%p", nomPref, nom, symbole);
}


static void ecrireLabelTableau(sym_t* symbole){

    // On souhaite avoir un nom unique pour éviter les doublons
    // donc on utilise l'adresse de l'entrée dans la table des symboles
    // Autrement :
    // - a : tableau (array ; contre f fonction ou q quadruplet ou v variable)
    // - g : global (à fin d'exhaustivité ; les tableaux locaux n'existent pas)
    fprintfChk("a_g_%s_%p", symbole->nom, symbole);
}


static void ecrireChaineLitterale(size_t id, const char* str){

    ecrireLabelChaineLitterale(id);
    fprintfChk(": .asciiz \"");

    for (; *str != '\0'; ++str){
        if (str[0] == '\\' && str[1] == '\'')
            ++str; // sauter le \ ne devant pas être écrit devant '
        fputcChk(*str);
    }

    fprintfChk("\"\n");
}


static void ecrireSymboleData(sym_t* symbole){

    if (symbole == NULL)
        return; // pas d'entrée dans la table de hachage

    switch (symbole->type){

    case S_ENTIER:
    case S_CHAINE:
        ecrireLabelVariableGlobale(symbole);
        fputsChk(": .word 0\n");
        break;

    case S_TABLEAU:
        ecrireLabelTableau(symbole);
        fprintfChk(": .space %ld\n", 4 * (long)symbole->tailleTableau);
        break;
    }
}


static void ecrireData(char** strLit, size_t nStrLit){

    for (size_t i = 0; i < nStrLit; ++i)
        ecrireChaineLitterale(i, strLit[i]);

    fprintfChk("\n"
        "\n"
        "# Alignment & CLI arguments\n"
        "argc: .word 0\n"
        "argv: .word 0\n"
        "\n");

    tableSym_t* globSymT = pilePorteeRacine();

    for (int i = 0; i < globSymT->capacite; ++i)
        ecrireSymboleData(globSymT->symboles[i]);
}


static void ecrireChargementChaineLitterale(const char* reg, size_t id){

    fprintfChk("la %s ", reg);
    ecrireLabelChaineLitterale(id);
    fputsChk("\n");
}


static void ecrireChargementArgument(const char* reg, int index){

    fprintfChk("lw %s argv\n"
        "lw %s %ld(%s)\n",
        reg, reg, 4 * (long)(index - 1), reg);
}


static void ecrireChargementVariable(const char* reg, char* id){

    fprintfChk("lw %s ", reg);
    ecrireLabelVariableGlobale(pilePorteeRechercheChk(id));
    fputsChk("\n");
}


static void ecrireEnregistrementVariable(const char* reg, char* id){

    fprintfChk("sw %s ", reg);
    ecrireLabelVariableGlobale(pilePorteeRechercheChk(id));
    fputsChk("\n");
}


static void ecrireChargementEntier(const char* reg, struct quadop op);

static void ecrireManipulationElementTableau(const char* reg,
        char* id, struct quadop op, const char* instruction){

    // op vient forcément d'une opérande entière, donc est de type entier
    ecrireChargementEntier("$t0", op);

    // Il est plus rapide de multiplier par quatre comme ceci
    fputsChk("sll $t0 $t0 2\n"
        "la $t1 ");
    ecrireLabelTableau(pilePorteeRechercheChk(id));
    fprintfChk("\n"
            "addu $t0 $t0 $t1\n"
            "%s %s 0($t0)\n",
            instruction, reg);
}


static void ecrireChargementElementTableau(const char* reg,
        char* id, struct quadop op){
    ecrireManipulationElementTableau(reg, id, op, "lw");
}


static void ecrireEnregistrementElementTableau(const char* reg,
        char* id, struct quadop op){
    ecrireManipulationElementTableau(reg, id, op, "sw");
}


static void ecrireManipulationOperande(const char* reg,
        struct quadop op, const char* instruction){

    switch (op.type){

    case QO_ENTIER:
        fprintfChk("%s %s %d($sp)\n", instruction, reg, 4 * op.u.entier);
        break;

    case QO_ID:

        ecrireChargementEntier("$t0", op);

        // Il est plus rapide de multiplier par quatre comme ceci
        fprintfChk("sll $t0 $t0 2\n"
                "addu $t0 $t0 $sp\n"
                "%s %s 0($t0)\n",
                instruction, reg);

        break;
    }
}


static void ecrireChargementOperande(const char* reg, struct quadop op){
    ecrireManipulationOperande(reg, op, "lw");
}


static void ecrireEnregistrementOperande(const char* reg, struct quadop op){
    ecrireManipulationOperande(reg, op, "sw");
}


static void ecrireChargementEntier(const char* reg, struct quadop op){

    switch (op.type){

    case QO_ENTIER:
        fprintfChk("li %s %d\n", reg, op.u.entier);
        break;

    case QO_ID:
        ecrireChargementVariable(reg, op.u.id);
        break;
    }
}


static void ecrireEnregistrementEntier(const char* reg, struct quadop op){

    // op est forcément l'identifiant d'une variable implicite
    ecrireEnregistrementVariable(reg, op.u.id);
}


static void ecrireChargementChaine(const char* reg, struct quadop op){

    switch (op.type){

    case QO_ID:
        ecrireChargementVariable(reg, op.u.id);
        break;

    case QO_ELEMENT_TABLEAU:
        ecrireChargementElementTableau(reg, op.u.id, *op.i);
        free(op.i);
        break;

    case QO_ARG:
        ecrireChargementArgument(reg, op.u.entier);
        break;

    case QO_CHAINE:
        ecrireChargementChaineLitterale(reg, op.u.indexChaine);
        break;
    }
}


static void ecrireEnregistrementChaine(const char* reg, struct quadop op){

    switch (op.type){

    case QO_ID:
        ecrireEnregistrementVariable(reg, op.u.id);
        break;

    case QO_ELEMENT_TABLEAU:
        ecrireEnregistrementElementTableau(reg, op.u.id, *op.i);
        free(op.i);
        break;
    }
}


static void ecrireOperationArithmetiqueBinaire(struct quad q, const char* op){

    ecrireChargementEntier("$s0", q.op1);
    ecrireChargementEntier("$s1", q.op2);

    fputsChk(op);

    ecrireEnregistrementEntier("$s0", q.op0);
}


static void ecrireConditionArithmetiqueBinaire(struct quad q, const char* op){

    // Les opérandes son des chaînes censées représenter des entiers,
    // et non des entiers elles-mêmes, il faut donc les convertir !
    // On ne passe pas par un quadruplet de conversion,
    // comme on sait que ce sont forcément des chaînes,
    // on évite donc de passer par des variables temporaires.

    // ... sauf pour les conditions de boucle de for :
    if (q.op0.type == QO_ENTIER || q.op1.type == QO_ENTIER){

        ecrireChargementEntier("$s0", q.op0);
        ecrireChargementEntier("$v0", q.op1);

    } else {

        ecrireChargementChaine("$a0", q.op0);

        fputsChk("jal atoi\n"
                "move $s0 $v0\n");

        ecrireChargementChaine("$a0", q.op1);

        fputsChk("jal atoi\n");

    }

    fputsChk(op);
    fputsChk(" $s0 $v0 ");
    ecrireLabelQuad(q.op2.u.adresse);
    fputsChk("\n");
}


static void ecrireQuadText(struct quad q, size_t id){

    ecrireLabelQuad(id);
    fputsChk(":\n");

    switch (q.type){

    case Q_AFF:

        if (q.op1.type == QO_ENTIER){

            ecrireChargementEntier("$s0", q.op1);
            ecrireEnregistrementEntier("$s0", q.op0);

        } else {

            ecrireChargementChaine("$s0", q.op1);
            ecrireEnregistrementChaine("$s0", q.op0);
        }

        break;

    case Q_ADD:
        ecrireOperationArithmetiqueBinaire(q,
                "addu $s0 $s0 $s1\n");
        break;

    case Q_SOUS:
        ecrireOperationArithmetiqueBinaire(q,
                "subu $s0 $s0 $s1\n");
        break;

    case Q_MUL:
        ecrireOperationArithmetiqueBinaire(q,
                "multu $s0 $s1\n"
                "mflo $s0\n");
        break;

    case Q_DIV:
        ecrireOperationArithmetiqueBinaire(q,
                "divu $s0 $s1\n"
                "mflo $s0\n");
        break;

    case Q_MOD:
        ecrireOperationArithmetiqueBinaire(q,
                "divu $s0 $s1\n"
                "mfhi $s0\n");
        break;

    case Q_EGAL:
        ecrireConditionArithmetiqueBinaire(q, "beq");
        break;

    case Q_NEGAL:
        ecrireConditionArithmetiqueBinaire(q, "bne");
        break;

    case Q_INF:
        ecrireConditionArithmetiqueBinaire(q, "blt");
        break;

    case Q_INFEGAL:
        ecrireConditionArithmetiqueBinaire(q, "ble");
        break;

    case Q_SUP:
        ecrireConditionArithmetiqueBinaire(q, "bgt");
        break;

    case Q_SUPEGAL:
        ecrireConditionArithmetiqueBinaire(q, "bge");
        break;

    case Q_CHAINE_EGAL:

        ecrireChargementChaine("$a0", q.op0);
        ecrireChargementChaine("$a1", q.op1);

        fputsChk("jal streql\n"
                "bne $v0 $0 ");

        ecrireLabelQuad(q.op2.u.adresse);

        fputsChk("\n");

        break;

    case Q_CHAINE_NEGAL:

        ecrireChargementChaine("$a0", q.op0);
        ecrireChargementChaine("$a1", q.op1);

        fputsChk("jal streql\n"
                "beq $v0 $0 ");

        ecrireLabelQuad(q.op2.u.adresse);

        fputsChk("\n");

        break;

    case Q_CHAINE_VIDE:

        ecrireChargementChaine("$t0", q.op0);

        fputsChk("lb $t0 0($t0)\n"
                "beq $t0 $0 ");

        ecrireLabelQuad(q.op1.u.adresse);

        fputsChk("\n");

        break;

    case Q_CHAINE_NON_VIDE:

        ecrireChargementChaine("$t0", q.op0);

        fputsChk("lb $t0 0($t0)\n"
                "bne $t0 $0 ");

        ecrireLabelQuad(q.op1.u.adresse);

        fputsChk("\n");

        break;

    case Q_GOTO:

        fputsChk("j ");

        ecrireLabelQuad(q.op0.u.adresse);

        fputsChk("\n");

        break;

    case Q_CONV_ENTIER_VERS_CHAINE:

        ecrireChargementEntier("$a0", q.op1);

        fputsChk("jal itoa\n");

        ecrireEnregistrementChaine("$v0", q.op0);

        break;

    case Q_CONV_CHAINE_VERS_ENTIER:

        ecrireChargementChaine("$a0", q.op1);

        fputsChk("jal atoi\n");

        ecrireEnregistrementEntier("$v0", q.op0);

        break;

    case Q_CONCAT:

        ecrireChargementChaine("$a0", q.op1);
        ecrireChargementChaine("$a1", q.op2);

        fputsChk("jal concat\n");

        ecrireEnregistrementChaine("$v0", q.op0);

        break;

    case Q_STACK:
        fprintfChk("addiu $sp $sp %d\n", -4 * q.op0.u.entier);
        break;

    case Q_CHARGE_ARG:
        ecrireChargementOperande("$t0", q.op1);
        ecrireEnregistrementChaine("$t0", q.op0);
        break;

    case Q_PASSE_ARG:
        ecrireChargementChaine("$t0", q.op0);
        ecrireEnregistrementOperande("$t0", q.op1);
        break;

    case Q_PASSE_TAB:

        fprintfChk("addiu $a0 $sp %d\n"
            "la $a1 ",
            4 * q.op1.u.entier);
        ecrireLabelTableau(pilePorteeRechercheChk(q.op0.u.id));
        fprintfChk("\n"
                "li $a2 %d\n", q.op2.u.entier);

        fputsChk("jal arrcpy\n");

        break;

    case Q_ALLARGS:

        fputsChk("lw $a0 argv\n"
            "lw $a1 argc\n"
            "jal join_space\n");

        ecrireEnregistrementChaine("$v0", q.op0);

        break;

    case Q_ECHO:

        fprintfChk("li $a1 %d\n", q.op0.u.entier);

        fputsChk("move $a0 $sp\n"
                "jal join_space\n"
                "move $a0 $v0\n"
                "li $v0 4\n"
                "syscall\n");

        break;

    case Q_READ:

        fputsChk("jal scanfstr\n");

        ecrireEnregistrementChaine("$v0", q.op0);

        break;

    case Q_EXIT:

        if (q.op0.type == QO_VIDE)
            fputsChk("j exit_success\n");
        else {
            ecrireChargementEntier("$a0", q.op0);
            fputsChk("li $v0 17\n"
                "syscall\n");
        }

        break;
    }

    fputsChk("\n");
}


static void ecrireEnregistrementArguments(){

    // On considère que le premier argument ($1) est "a" lorsque l'on lance :
    // programme a b c d
    // On modifie donc argc et argv en conséquence

    fputsChk("addiu $a0 $a0 -1\n"
        "addiu $a1 $a1 4\n"
        "sw $a0 argc\n"
        "sw $a1 argv\n"
        "\n");
}


static void ecrireText(struct quad* quads, size_t nQuads){

    fputsChk("main:\n\n");

    ecrireEnregistrementArguments();

    for (size_t i = 0; i < nQuads; ++i)
        ecrireQuadText(quads[i], i);

    fputsChk("# Fallback\n");

    ecrireQuadText(QUAD_EXIT_VIDE(), nQuads);
}


void quadsAMips(struct quad* quads, size_t nQuads,
        char** strLit, size_t nStrLit){

    fputsChk(getDataStart());

    ecrireData(strLit, nStrLit);

    fputsChk(getTextStart());

    ecrireText(quads, nQuads);
}
