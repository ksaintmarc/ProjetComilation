# Documentation des tests unitaires
Si il y a des bugs ou erreurs, veuillez ouvrir un ticket sur le dépôt GitHub. 

## 1) Présentation
Dans le fichier unit_test.c, vous pouvez écrire des tests unitaires
pour les fonctions de votre projet. Cela permettera de bien tester
une fonction à implémenter et de s'assurer qu'elle fonctionne correctement.

## 2) Utilisation
Pour compiler en mode test, il faut utiliser la commande `make test`.
Le résulat des tests sont affichés aussi tôt sur la sortie standard.

## 3) Documentation
Pour plus de détails, voir le fichier `unit_test.h` et son implémentation
dans `lib_test.c`.

| Macros :                                         | Description :                                                         | Notes :                                                                                                                                                                                         |
|--------------------------------------------------|-----------------------------------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `OK`                                             | Indique que le test est OK.                                           | OK = 1                                                                                                                                                                                          |
| `FAIL`                                           | Indique que le test a FAIL.                                           | FAIL = 0                                                                                                                                                                                        |
| `EXIT`                                           | Indique que le test a FAIL via un `ASSERT()`.                         | EXIT = -1                                                |
| `ERR`                                            | Indique une erreur interne.                                           | ERR = -2                                                                                                                                        
| `BEGIN_SUITE(<nom_test>)`                        | Déclare une suite de tests.                                           | Elle doit être déclarée en dehors du `main()`, et agit comme une déclaration de fonction.                                                                                                           |
| `CREATE_SUITE(<nom_test>, <description>)`        | Bind à une suite existante.                                           | Elle doit être appelée dans le main(), `nom_test` doit faire référence à un `BEGIN_SUITE()` déja déclarée.                                                                                        |
| `CREATE_TEST(<nom_test>, <description>)`         | Déclare un test.                                                      | On peut créer autant de fois de tests dans un `BEGIN_SUITE()`, Mais un test ne peut que contenir un seul `CHECK()` ou `ASSERT()`.  `<nom_test>` doit faire référence à un `BEGIN_SUITE()` déja déclarée. |
| `CHECK_EQ(<val>, <val_a_comparer>)`              | Vérifie 2 valeurs.                                                    | On privilégie d'évaluer la fonction dans `<val>` et comparer la valeur obtenue avec `<val_a_comparer>`.                                                                                             |
| `CHECK_STR_EQ(<str1>, <str2>, <val_a_comparer>)` | Compare 2 chaînes de caractères.                                      | On compare `<str1>` avec `<str2>`, renvoie OK si ils sont égaux, sinon FAIL.                                                                                                                        |
| `ASSERT_EQ(<val>, <val_a_comparer>)`             | Vérifie 2 valeurs, mais arrête les tests si elle échoue.              |  Même chose que `CHECK_EQ()` mais quitte les tests en cas d'erreur.                                                                                                                                                                              |
| `ASSERT_STR_EQ(<str1>, <str2>,<val_a_comparer>)` | Compare 2 chaîne de caractères, mais arrête les tests si elle échoue. | Même chose que `CHECK_STR_EQ()` mais quitte les tests en cas d'erreur.                                                                                                                                                                                |
| `END()`                                          | Fin de la suite de tests.                                             | Elle est obligatoire à la fin d'un `BEGIN_SUITE()`.                                                                                                                                              |
| `RUN()`                                          | Execute l'ensemble des tests.                                         | Elle est obligatoire lors du return dans le `main()`.                                                                                                                                             |

## 4) Exemple

Soit les fonctions suivantes que on veut tester :
```c
// addition.c
int addition(int x, int y){
 
    int resultat = x + y;
    return resultat;
}

// hello_world.c
void hello_world(int cond, char *buffer){
 
    if (cond == 1)
        strcpy(buffer, "Hello World!");
    else if (cond == 2)
        strcpy(buffer, "Bye!");
    else
        exit(1);
}
```

Il faut inclure ces fichiers .c dans le fichier `unit_test.c`.
Ensuite, on peut écrire les tests unitaires pour ces fonctions :
```c
// unit_test.c
BEGIN_SUITE(test_add){
    // Zone de setup
    int x, y;
    x = 1;
    y = 2;

    CREATE_TEST(test_add, "addition 1+2 = 3"){
        // Zone d'execution
        CHECK_EQ(addition(x, y), 3);
    }
    
    CREATE_TEST(test_add, "addition 3+2 != 4"){
        // Zone d'execution
        int x, y, resultat;
        x = 3;
        y = 2;
        resultat = addition(x, y);
        ASSERT_EQ(resultat == 4, FAIL);
    }
    // Zone de teardown
    x = 0;
    y = 0;

    END();
}

// Exemple : suite de tests pour la fonction hello()
BEGIN_SUITE(test_hello){

    char buffer[20];

    CREATE_TEST(test_hello, "hello avec cond == 1"){
        hello_world(1, buffer);
        CHECK_STR_EQ(buffer, "Hello World!", OK);
    }

    END();
}
```
Pour tester une fonction, il faut utiliser `BEGIN_SUITE()`.
`BEGIN_SUITE()` doit être toujours déclarée en dehors d'une fonction,
elle est toujours suivi par une série de tests (ou pas), et se terminer
par `END()`.

Pour tester les différents cas d'une fonction, il faut utiliser `CREATE_TEST()`.
`CREATE_TEST()` doit être toujours déclarée dans un `BEGIN_SUITE()`,
et elle est toujours suivi d'un seul `CHECK()` ou `ASSERT()`

Dans le main il faudra bind les suites de tests avec `CREATE_SUITE()`,
```c
// unit_test.c
int main(void){

    CREATE_SUITE(test_add, "Test de la fonction addition()");
    CREATE_SUITE(test_hello, "Test de la fonction hello(), qui affiche Hello World!");

    return RUN();
}
```

Après un `make test`, voici le résultat obtenu dans le terminal :
```
|=================================================================|
 [🛠️]  TEST (1/2) : Test de la fonction addition() 
|=================================================================|
   - addition 1+2 = 3        ✅
   - addition 3+2 != 4        ✅

   - [✅] TEST 1 OK
|=================================================================|
 [🛠️]  TEST (2/2) : Test de la fonction hello(), qui affiche Hello World! 
|=================================================================|
   - hello avec cond == 1       ✅
   
   - [✅] TEST 2 OK
|=================================================================|
 [✅] OK C'EST BON
|=================================================================|
```

## 5) Comportement inattendus
Veuillez respecter la syntaxe donnée dans l'exemple ci-dessus 
afin d'avoir à gérèr des comportements inattendus et pour des questions
de lisibilité.

Actuellement, les tests unitaires sont pas réalisés en totale isolation.
Il faut donc faire attention à ne pas pas quitter le programme brutalement
à cause d'un exit(), car il est impossible de vérifier le retour de exit().
