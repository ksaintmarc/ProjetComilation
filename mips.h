/**
 * @file mips.h
 * @brief Génération de code MIPS
 * 
 * Contient les fonctions permettant de générer le code MIPS
 * correspondant à la liste de quadruplets fournies.
 * 
 */

#ifndef __MIPS_H__
#define __MIPS_H__

#include "quad.h"


/**
 * @brief Écrit dans yyout le code MIPS correspondant aux quadruplets fournis
 * 
 * @param quads La liste de quadruplets à convertir
 * @param nQuads Le nombre de quadruplets de la liste
 * @param strLit La liste de chaînes littérales à utiliser
 * @param nStrLit Le nombre de chaînes de la liste
 */
void quadsAMips(struct quad* quads, size_t nQuads,
        char** strLit, size_t nStrLit);
// Si passage à plusieurs listes de quads (fonctions non-entourées de goto),
// struct quad** quads, size_t* nQuads, size_t nListes
// avec la liste 0 ou bien nListes-1 étant le code global à mettre dans main


#endif
