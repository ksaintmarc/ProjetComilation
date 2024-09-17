/**
 * @file mips_lib.h
 * @brief Helper pour la génération de code MIPS
 * 
 * Contient les fonctions aidant à placer les chaînes de caractères
 * dans les segments data et text lors de la génération de code MIPS.
 * 
 */

#ifndef __MIPS_LIB_H__
#define __MIPS_LIB_H__


/**
 * @brief Renvoie la chaîne de caractères du code du segment data de la librairie
 * 
 * @return La chaîne de caractères du segment data de la "librairie standard"
 * 
 */
const char* getDataStart();

/**
 * @brief Renvoie la chaîne de caractères du code du segment text de la librairie
 * 
 * @return La chaîne de caractères du segment text de la "librairie standard"
 * 
 */
const char* getTextStart();


#endif
