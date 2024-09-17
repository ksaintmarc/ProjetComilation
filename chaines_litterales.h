/**
 * @file chaines_litterales.h
 * @brief Liste de chaines de caractères littérales
 * 
 * Structure de donnée permet d'indexer et garder en mémoire
 * une liste des chaines littérales écrite dans le code SoS.
 * 
 */

#ifndef __CHAINES_LITTERALES_H__
#define __CHAINES_LITTERALES_H__

#include <stddef.h>

/**
 * @struct listeChaineLitterale
 * @brief Structure d'une liste de chaines
 * 
 * @var listeChaineLitterale::taille
 * @brief Nombre d'éléments dans la liste
 * 
 * @var listeChaineLitterale::tailleMax
 * @brief Taille maximale de la liste
 * 
 * @var listeChaineLitterale::chaines
 * @brief Liste de chaines littérales
 * 
 */
struct listeChaineLitterale {
    size_t taille;
    size_t tailleMax;
    char** chaines;
};

/**
 * @brief Ajoute une chaine à la liste des chaines littérales
 * 
 * @param l : Liste de chaines littérales
 * @param chaine : chaine à ajouter
 * @return L'index de la chaine dans la liste
 */
size_t ajoutListeChaineLitterale(struct listeChaineLitterale* l, char* chaine);

/**
 * @brief Libère en mémoire la liste des chaines littérales
 * 
 * @param l : Liste de chaines littérales
 * 
 */
void listeChaineLitteraleFree(struct listeChaineLitterale* l);
#endif
