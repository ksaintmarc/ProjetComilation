/**
 * @file check.h
 * @brief Macros de vérification des valeurs de retour des fonctions systèmes
 * 
 */

#ifndef __CHECK_H__
#define __CHECK_H__

#include <stdlib.h>
#include <stdio.h>

/**
 * @def CHECK(x)
 * @brief Vérifie les valeurs de retour des fonctions systèmes.
 * 
 * @param x La valeur de retour de la fonction système.
 * 
 * La macro CHECK vérifie la valeur de retour de la fonction système x. 
 * Si la valeur de retour est négative (c'est à dire qu'elle a échoué), 
 * elle affiche le nom de la fonction et le numéro de ligne où l'erreur 
 * s'est produite, puis elle affiche le message d'erreur correspondant 
 * à la valeur de errno. Enfin, elle termine le programme avec un code d'erreur.
 * 
 */
#define CHECK(x) \
    do { \
        if (!(x)) \
        { \
            fprintf(stderr, "%s:%d: ", __func__, __LINE__); \
            perror(#x); \
            exit(EXIT_FAILURE); \
        } \
    } while (0) 

#endif
