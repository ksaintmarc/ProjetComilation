char data[] = ".data\n\
\n\
\n\
EMPTY: .asciiz \"\"\n\
\n\
ATOI_FAIL_PREFIX: .asciiz \"Erreur: La chaîne \\\"\"\n\
ATOI_FAIL_SUFFIX: .asciiz \"\\\" ne représente pas un entier relatif.\\n\"\n\
\n\
\n\
SCANFSTR_BUFFER: .space 2048\n\
\n\
ATOI_FAIL_BUFFER: .space 16\n\
\n\
\n\
";

char text[] = "\n\
\n\
\n\
\n\
.text\n\
\n\
\n\
# Déclaration précoce de symboles\n\
\n\
.globl main\n\
\n\
\n\
\n\
\n\
# Saut à la routine main\n\
\n\
j main\n\
\n\
\n\
\n\
\n\
### Fonction arrcpy ###\n\
\n\
# Copie un tableau de mots vers un espace déjà alloué\n\
\n\
# Entrées :\n\
# $a0 adresse de destination, accessible sur une longueur de $a2 mots ou plus\n\
# $a1 adresse du début du tableau d'origine,\n\
#     accessible sur une longeur de $a2 mots ou plus\n\
# $a2 nombre maximum de mots à copier\n\
\n\
# Sorties :\n\
# $v0 adresse de destination (émule le standard C)\n\
\n\
\n\
arrcpy:\n\
\n\
  move $v0 $a0 # $v0 = $a0\n\
\n\
  arrcpy_loop:\n\
\n\
    ble $a2 $0 arrcpy_loop_end # if ($a2 <= 0) break;\n\
\n\
    lw $t0 0($a1) # $t0 = *$a1\n\
    sw $t0 0($a0) # *$a0 = $t0\n\
\n\
    addiu $a0 $a0 4 # $a0 += sizeof(word)\n\
    addiu $a1 $a1 4 # $a1 += sizeof(word)\n\
    addiu $a2 $a2 -1 # $a2--\n\
\n\
    j arrcpy_loop\n\
\n\
  arrcpy_loop_end:\n\
\n\
  jr $ra\n\
\n\
\n\
\n\
\n\
### Fonction strlen ###\n\
\n\
# Calcule la longueur d'une chaîne de caractères\n\
\n\
# Entrées :\n\
# $a0 adresse du début de la chaîne, accessible et terminée par \\0\n\
\n\
# Sorties :\n\
# $v0 longueur de la chaîne de caractères (sans le \\0)\n\
# $v1 adresse du \\0\n\
\n\
\n\
strlen:\n\
\n\
  move $t0 $a0 # $t0 = $a0\n\
\n\
  strlen_loop:\n\
\n\
    lb $t1 0($t0) # $t1 = *$t0\n\
    beq $t1 $0 strlen_loop_end # if ($t1 == '\\0') break\n\
\n\
    addiu $t0 $t0 1 # $t0++\n\
\n\
    j strlen_loop\n\
\n\
  strlen_loop_end: # maintenant $a0 pointe sur le \\0, donc on en déduit $v0\n\
\n\
  subu $v0 $t0 $a0 # $v0 = $t0 - $a0\n\
  move $v1 $t0 # $v1 = $t0\n\
\n\
  jr $ra\n\
\n\
\n\
\n\
\n\
### Fonction streql ###\n\
\n\
# Teste si deux chaînes de caractères sont égales\n\
\n\
# Entrées :\n\
# $a0 adresse du début de la première chaîne de caractères,\n\
#     accessible et terminée par \\0\n\
# $a1 adresse du début de la seconde chaîne de caractères,\n\
#     accessible et terminée par \\0\n\
\n\
# Sorties :\n\
# $v0 1 si les chaînes sont égales, 0 si elles diffèrent\n\
\n\
\n\
streql:\n\
\n\
  streql_loop:\n\
\n\
    lb $t0 0($a0) # $t0 = *$a0\n\
    lb $t1 0($a1) # $t1 = *$a1\n\
\n\
    bne $t0 $t1 streql_0 # if ($t0 != $t1) return 0\n\
    beq $t0 $0 streql_1 # if ($t0 == '\\0') return 1\n\
\n\
    addiu $a0 $a0 1 # $a0++\n\
    addiu $a1 $a1 1 # $a1++\n\
\n\
    j streql_loop\n\
\n\
  streql_0:\n\
\n\
  li $v0 0\n\
  jr $ra\n\
\n\
  streql_1:\n\
\n\
  li $v0 1\n\
  jr $ra\n\
\n\
\n\
\n\
\n\
### Fonction strncpy ###\n\
\n\
# Copie une chaîne de caractères vers un espace déjà alloué\n\
# Si un caractère \\0 apparaît avant une longueur de $a2,\n\
# il est copié et est le dernier à l'être,\n\
# sinon $a2 caractères sont copiés et aucun \\0 n'est ajouté à la destination\n\
\n\
# Entrées :\n\
# $a0 adresse de destination, accessible sur une longueur de $a2 ou plus\n\
# $a1 adresse du début de la chaîne d'origine,\n\
#     accessible sur une longeur de $a2 ou plus ou terminée par un \\0 avant\n\
# $a2 nombre maximum de caractères à copier\n\
\n\
# Sorties :\n\
# $v0 adresse de destination (émule le standard C)\n\
# $v1 longueur de la chaîne copiée (sans le \\0)\n\
\n\
\n\
strncpy:\n\
\n\
  move $t0 $a0 # $t0 = $a0\n\
  move $t1 $a1 # $t1 = $a1\n\
\n\
  strncpy_loop:\n\
\n\
    subu $t2 $t0 $a0 # $t2 = $t0 - $a0 // (nombre de caractères déjà copiés)\n\
    bge $t2 $a2 strncpy_loop_end # if ($t2 >= $a2) break\n\
\n\
    lb $t2 0($t1) # $t2 = *$t1\n\
    sb $t2 0($t0) # *$t0 = $t2\n\
    beq $t2 $0 strncpy_loop_end # if ($t2 == 0) break\n\
\n\
    addiu $t0 $t0 1 # $t0++\n\
    addiu $t1 $t1 1 # $t1++\n\
\n\
    j strncpy_loop\n\
\n\
  strncpy_loop_end: # maintenant $a0 pointe sur le \\0 s'il a été copié,\n\
                    # ou après le dernier caractère, donc on en déduit $v1\n\
\n\
  move $v0 $a0 # $v0 = $a0\n\
  subu $v1 $t0 $a0 # $v1 = $t0 - $a0\n\
\n\
  jr $ra\n\
\n\
\n\
\n\
\n\
### Fonction concat ###\n\
\n\
# Concatène deux chaînes de caractères en une nouvelle\n\
\n\
# Entrées :\n\
# $a0 adresse du début de la première chaîne de caractères,\n\
#     accessible et terminée par \\0\n\
# $a1 adresse du début de la seconde chaîne de caractères,\n\
#     accessible et terminée par \\0\n\
\n\
# Sorties :\n\
# $v0 adresse du début d'une chaîne de caractères nouvellement allouée\n\
#     résultant de la concaténation de $a0 et $a1\n\
# $v1 longueur de la nouvelle chaîne de caractères (sans le \\0)\n\
\n\
\n\
concat:\n\
\n\
  addiu $sp $sp -20\n\
  sw $ra 0($sp)\n\
  sw $s0 4($sp)\n\
  sw $s1 8($sp)\n\
  sw $s2 12($sp)\n\
  sw $s3 16($sp)\n\
\n\
  move $s0 $a0 # $s0 = $a0\n\
  move $s1 $a1 # $s1 = $a1\n\
\n\
  jal strlen\n\
  move $s2 $v0 # $s2 = strlen($s0)\n\
\n\
  move $a0 $s1\n\
  jal strlen\n\
  addiu $s3 $v0 1 # $s3 = strlen($s1) + 1\n\
\n\
  addu $a0 $s2 $s3\n\
  li $v0 9 # sbrk\n\
  syscall # $v0 = sbrk($s2 + $s3)\n\
\n\
  move $a0 $v0 # $a0 = $v0 // destination\n\
  move $a1 $s0 # $a1 = $s0 // source\n\
  move $a2 $s2 # $a2 = $s2 // longueur\n\
  jal strncpy\n\
  move $s0 $v0 # $s0 = strncpy($v0, $s0, $s2)\n\
\n\
  addu $a0 $v0 $s2 # $a0 = $s0 + s2 // destination\n\
  move $a1 $s1 # $a1 = $s1 // source\n\
  move $a2 $s3 # $a2 = $s3 // longueur\n\
  jal strncpy\n\
\n\
  move $v0 $s0 # $v0 = $s0\n\
  addu $v1 $s2 $s3 # $v1 = $s2 + $s3\n\
  addiu $v1 $v1 -1 # $v1--\n\
\n\
  lw $s3 16($sp)\n\
  lw $s2 12($sp)\n\
  lw $s1 8($sp)\n\
  lw $s0 4($sp)\n\
  lw $ra 0($sp)\n\
  addiu $sp $sp 20\n\
\n\
  jr $ra\n\
\n\
\n\
\n\
\n\
### Fonction join_space ###\n\
\n\
# Concatène toutes les chaînes de caractères d'un tableau\n\
# en ajoutant un espace entre chacune\n\
\n\
# Entrées :\n\
# $a0 adresse du début de la liste de chaînes de caractères,\n\
#     chacune accessible et terminée par \\0\n\
# $a1 nombre de chaînes de caractères dans le tableau\n\
\n\
# Sorties :\n\
# $v0 adresse du début d'une chaîne de caractères nouvellement allouée\n\
#     résultant de la concaténation des chaînes de caractères du tableau\n\
# $v1 longueur de la nouvelle chaîne de caractères (sans le \\0)\n\
\n\
\n\
join_space:\n\
\n\
  addiu $sp $sp -24\n\
  sw $ra 0($sp)\n\
  sw $s0 4($sp)\n\
  sw $s1 8($sp)\n\
  sw $s2 12($sp)\n\
  sw $s3 16($sp)\n\
  sw $s4 20($sp)\n\
\n\
  beq $a1 $0 join_space_empty # if ($a1 == 0) goto join_space_empty\n\
\n\
  move $s0 $a0 # $s0 = $a0\n\
  move $s1 $a1 # $s1 = $a1\n\
\n\
  # Calcul de la taille totale nécessaire\n\
  # strlen(str) + 1 pour chaque str, comme pour n chaînes, (n-1) ' ' et 1 '\\0'\n\
\n\
  move $s2 $a0 # $s2 = $a0 // pointeur\n\
  move $s3 $a1 # $s3 = $a1 // itérateur\n\
  li $s4 0 # $s4 = 0 // longueur\n\
\n\
  join_space_len_loop:\n\
\n\
    beq $s3 $0 join_space_len_loop_end # if ($s3 == 0) break\n\
\n\
    lw $a0 0($s2)\n\
    jal strlen\n\
    addu $s4 $s4 $v0 # $s4 += strlen(*$s2)\n\
    addiu $s4 $s4 1 # $s4++\n\
\n\
    addiu $s2 $s2 4 # $s2 += sizeof(word)\n\
    addiu $s3 $s3 -1 # $s3--\n\
\n\
    j join_space_len_loop\n\
\n\
  join_space_len_loop_end:\n\
\n\
  addiu $t0 $s4 -1\n\
  beq $t0 $0 join_space_empty # if ($s4 == 1) goto join_space_empty\n\
\n\
  # Copie des chaînes\n\
\n\
  move $a0 $s4\n\
  li $v0 9 # sbrk\n\
  syscall # $v0 = sbrk($s4)\n\
\n\
  # $s0 pointeur\n\
  # $s1 itérateur\n\
  move $s2 $v0 # $s2 = $v0 // adresse de destination à parcourir\n\
  move $s3 $v0 # $s3 = $v0 // adresse de destination à retourner\n\
\n\
  join_space_cat_loop:\n\
\n\
    move $a0 $s2 # $a0 = $s2 // destination\n\
    lw $a1 0($s0) # $a1 = *$s0 // source\n\
    move $a2 $s4 # $a2 = $s4 // longueur ($s4 pour être sûr de tout copier)\n\
    jal strncpy # $v0, $v1 = strncpy($s2, *$s0, $s4)\n\
\n\
    addiu $s0 $s0 4 # $s0 += sizeof(word)\n\
    addiu $s1 $s1 -1 # $s1--\n\
    addu $s2 $s2 $v1 # $s2 += $v1\n\
\n\
    li $t0 32 # ' '\n\
    sb $t0 0($s2) # *$s2 = ' '\n\
    addiu $s2 $s2 1 # $s2++\n\
\n\
    bne $s1 $0 join_space_cat_loop # if ($s1 == 0) break\n\
\n\
  addiu $s2 $s2 -1 # $s2--\n\
  sb $0 0($s2) # *$s2 = '\\0'\n\
\n\
  move $v0 $s3 # $v0 = $s3\n\
  addiu $v1 $s4 -1 # $v1 = $s4 - 1\n\
\n\
  join_space_epilogue:\n\
\n\
  lw $s4 20($sp)\n\
  lw $s3 16($sp)\n\
  lw $s2 12($sp)\n\
  lw $s1 8($sp)\n\
  lw $s0 4($sp)\n\
  lw $ra 0($sp)\n\
  addiu $sp $sp 24\n\
\n\
  jr $ra\n\
\n\
  join_space_empty:\n\
\n\
  la $v0 EMPTY\n\
  li $v1 0\n\
  j join_space_epilogue\n\
\n\
\n\
\n\
\n\
### Fonction puts_stderr ###\n\
\n\
# Affiche une chaîne de caractères sur stderr\n\
\n\
# Entrées :\n\
# $a0 adresse du début de la chaîne de caractères\n\
\n\
# Sorties :\n\
# aucune\n\
\n\
\n\
puts_stderr:\n\
\n\
  addiu $sp $sp -8\n\
  sw $ra 0($sp)\n\
  sw $a0 4($sp) # save = $a0=str\n\
\n\
  jal strlen\n\
\n\
  li $a0 2 # $a0 = stderr\n\
\n\
  lw $a1 4($sp) # $a1 = save\n\
\n\
  move $a2 $v0 # $a2 = strlen(str)\n\
\n\
  li $v0 15\n\
  syscall\n\
\n\
  lw $ra 0($sp)\n\
  addiu $sp $sp 8\n\
\n\
  jr $ra\n\
\n\
\n\
\n\
\n\
### Fonction parseInt ###\n\
\n\
# Tente de lire un entier relatif à partir d'une chaîne de caractères\n\
# Format : [+-]?[0-9]+\n\
\n\
# Entrées :\n\
# $a0 adresse du début de la chaîne, accessible et terminée par \\0\n\
\n\
# Sorties :\n\
# $v0 nombre lu, si succès\n\
# $v1 statut : 0 en cas de succès, 1 si le nombre ne suivait pas le format\n\
\n\
\n\
parseInt:\n\
\n\
  li $v0 0 # $v0 = 0\n\
  li $v1 0 # $v1 = 0\n\
\n\
  li $t2 10 # $t2 = 10\n\
  li $t3 0 # 0 si positif, 1 si négatif\n\
\n\
  lb $t0 0($a0) # $t0 = *$a0\n\
\n\
  li $t1 43 # $t1 = '+'\n\
  beq $t0 $t1 parseInt_plus\n\
\n\
  li $t1 45 # $t1 = '-'\n\
  bne $t0 $t1 parseInt_loop\n\
\n\
  li $t3 1 # préfixe - donc négatif\n\
\n\
  parseInt_plus: # partie partagée entre préfixe + et préfixe -\n\
\n\
  addiu $a0 $a0 1\n\
\n\
  parseInt_loop:\n\
\n\
    lb $t0 0($a0) # $t0 = *$a0\n\
    beq $t0 $0 parseInt_loop_end # if ($t0 == '\\0') break\n\
\n\
    blt $t0 48 parseInt_nondigit # if ($t0 < '0') goto fail\n\
    bge $t0 58 parseInt_nondigit # if ($t0 >= '9' + 1) goto fail\n\
\n\
    multu $v0 $t2\n\
    mflo $v0 # $v0 *= 10\n\
\n\
    addiu $t0 $t0 -48 # $t0 -= '0'\n\
    addu $v0 $v0 $t0 # $v0 += $t0\n\
\n\
    addiu $a0 $a0 1 # $a0++\n\
\n\
    j parseInt_loop\n\
\n\
  parseInt_nondigit:\n\
\n\
  li $v1 1 # $v1 = 1 // format non-suivi (passer au retour juste en-dessous)\n\
\n\
  parseInt_loop_end:\n\
\n\
  beq $t3 $t0 parseInt_end\n\
  subu $v0 $0 $v0 # if ($t3) $v0 = -$v0\n\
\n\
  parseInt_end:\n\
\n\
  jr $ra\n\
\n\
\n\
\n\
\n\
### Fonction intlen ###\n\
\n\
# Calcule la longueur de la chaîne de caractères la plus courte\n\
# représentant un entier relatif\n\
\n\
# Entrées :\n\
# $a0 entier relatif\n\
\n\
# Sorties :\n\
# $v0 longueur de la chaîne de caractère la plus courte représentant cet entier\n\
#     (sans le \\0)\n\
\n\
\n\
intlen:\n\
\n\
  beq $a0 $0 intlen_zero\n\
\n\
  li $t0 10 # $t0 = 10\n\
  slti $v0 $a0 0 # $v0 = $a0 < 0\n\
  sub $a0 $0 $a0 # $a0 = -$a0\n\
\n\
  intlen_loop:\n\
\n\
    beq $a0 $0 intlen_end\n\
\n\
    div $a0 $t0\n\
    mflo $a0 # $a0 /= 10\n\
\n\
    addiu $v0 $v0 1\n\
\n\
    j intlen_loop\n\
\n\
  intlen_zero:\n\
\n\
  li $v0 1\n\
\n\
  intlen_end:\n\
\n\
  jr $ra\n\
\n\
\n\
\n\
\n\
### Fonction sprintint ###\n\
\n\
# Écrit un entier relatif comme chaîne de caractères -?(0|[1-9][0-9]*)\n\
# (sans le \\0)\n\
\n\
# Entrées :\n\
# $a0 adresse de destination, accessible sur une longueur de $a2 ou plus\n\
# $a1 entier relatif\n\
# $a2 longueur de la chaîne la plus courte représentant $a1\n\
\n\
# Sorties :\n\
# $v0 longueur de la chaîne écrite (sans le \\0)\n\
\n\
\n\
sprintint:\n\
\n\
  bge $a1 0 sprintint_positive\n\
\n\
  li $t0 45 # $t0 = '-'\n\
  sb $t0 0($a0) # *$a0 = '-'\n\
  sub $a1 $0 $a1 # $a1 = -$a1\n\
  addiu $a0 $a0 1 # $a0++\n\
  addiu $a2 $a2 -1 # $a2--\n\
\n\
  sprintint_positive:\n\
\n\
  addu $a0 $a0 $a2 # $a0 += $a2\n\
  addiu $a0 $a0 -1 # $a0--\n\
\n\
  li $v0 0 # $v0 = 0\n\
  li $t0 10 # $t0 = 10\n\
\n\
  sprintint_loop:\n\
\n\
    beq $a2 $0 intlen_end\n\
\n\
    div $a1 $t0\n\
    mflo $a1 # $a1 /= 10\n\
    mfhi $t1 # $t1 = $a1_prev % 10\n\
\n\
    addiu $t1 $t1 48 # $t1 += '0'\n\
    sb $t1 0($a0) # *$a0 = $t1\n\
\n\
    addiu $v0 $v0 1\n\
    addiu $a0 $a0 -1 # $a0--\n\
    addiu $a2 $a2 -1 # $a2--\n\
\n\
    j sprintint_loop\n\
\n\
  sprintint_end:\n\
\n\
  jr $ra\n\
\n\
\n\
\n\
\n\
### Fonction scanfstr ###\n\
\n\
# Lit une chaîne de caractères de longueur maximale de 2047 (sans le \\0)\n\
# depuis l'entrée standard, bloquant l'exécution entre-temps, et la copie\n\
# dans une chaîne de caractères nouvellement allouée avec un \\0 à sa suite\n\
\n\
# Entrées :\n\
# aucune\n\
\n\
# Sorties :\n\
# $v0 adresse du début de la chaîne de caractères allouée\n\
\n\
\n\
scanfstr:\n\
\n\
\n\
  addiu $sp $sp -8\n\
  sw $ra 0($sp)\n\
  sw $s0 4($sp)\n\
\n\
  la $a0 SCANFSTR_BUFFER\n\
  li $a1 2048\n\
  li $v0 8\n\
  syscall # lecture de la chaîne vers le tampon\n\
\n\
  la $a0 SCANFSTR_BUFFER # pas nécessaire en principe, mais dépend peut-être\n\
                         # du simulateur/compilateur, on joue la sécurité\n\
\n\
  jal strlen\n\
  move $s0 $v0 # $s0 = strlen($a0)\n\
\n\
  beq $s0 $0 scanfstr_nolf # if ($s0 == 0) goto scanfstr_nolf\n\
  li $t0 2047\n\
  beq $s0 $t0 scanfstr_nolf # if ($s0 == 2047) goto scanfstr_nolf\n\
\n\
  la $t0 SCANFSTR_BUFFER\n\
  addu $t0 $t0 $s0\n\
  addiu $t0 $t0 -1\n\
  lb $t1 0($t0) # $t1 = buf[$s0 - 1]\n\
  li $t2 10 # $t2 = '\\n'\n\
  bne $t1 $t2 scanfstr_nolf # if ($t1 != $t2) goto scanfstr_nolf\n\
\n\
  sb $0 0($t0) # buf[$s0 - 1] = '\\0'\n\
  addiu $s0 $s0 -1 # $s0--\n\
\n\
  scanfstr_nolf:\n\
\n\
  addiu $a0 $s0 1\n\
  li $v0 9 # sbrk\n\
  syscall # $v0 = sbrk($s0 + 1)\n\
\n\
  move $a0 $v0 # $a0 = $v0 // destination\n\
  la $a1 SCANFSTR_BUFFER # // source\n\
  addiu $a2 $s0 1 # $a2 = $s0 + 1 // longueur\n\
  jal strncpy # $v0 = strncpy($a0, $a1, $a2)\n\
\n\
  lw $s0 4($sp)\n\
  lw $ra 0($sp)\n\
  addiu $sp $sp 8\n\
\n\
  jr $ra\n\
\n\
\n\
\n\
\n\
### Fonction atoi ###\n\
\n\
# Récupère la valeur numérique en entier relatif d'une chaîne de caractères\n\
# Termine le programme avec un message d'erreur si le format est incorrect\n\
\n\
# Entrées :\n\
# $a0 adresse du début de la chaîne de caractères\n\
\n\
# Sorties :\n\
# $v0 valeur en entier relatif de la chaîne de caractères\n\
\n\
\n\
atoi:\n\
\n\
  addiu $sp $sp -8\n\
  sw $ra 0($sp)\n\
  sw $s0 4($sp)\n\
\n\
  move $s0 $a0 # $s0 = $a0\n\
  jal parseInt # $v0, $v1 = parseInt($a0)\n\
\n\
  beq $v1 $0 atoi_end # format correct\n\
\n\
  # Format incorrect\n\
\n\
  la $a0 ATOI_FAIL_PREFIX\n\
  jal puts_stderr\n\
\n\
  la $a0 ATOI_FAIL_BUFFER\n\
  move $a1 $s0\n\
  li $a2 16\n\
  jal strncpy\n\
\n\
  la $a0 ATOI_FAIL_BUFFER\n\
\n\
  li $t0 16\n\
  blt $v1 $t0 atoi_fail_buf_set\n\
\n\
  li $t0 46 # $t0 = '.'\n\
  sb $t0 12($a0) # .\n\
  sb $t0 13($a0) # .\n\
  sb $t0 14($a0) # .\n\
  sb $0 15($a0) # ajout du \\0\n\
\n\
  atoi_fail_buf_set:\n\
\n\
  jal puts_stderr\n\
\n\
  la $a0 ATOI_FAIL_SUFFIX\n\
  jal puts_stderr\n\
\n\
  j exit_failure\n\
\n\
  atoi_end:\n\
\n\
  lw $s0 4($sp)\n\
  lw $ra 0($sp)\n\
  addiu $sp $sp 8\n\
\n\
  jr $ra\n\
\n\
\n\
\n\
\n\
### Fonction itoa ###\n\
\n\
# Représente un entier relatif dans une nouvelle chaîne de caractères\n\
\n\
# Entrées :\n\
# $a0 entier relatif\n\
\n\
# Sorties :\n\
# $v0 adresse du début d'une nouvelle chaîne de caractères représentant $a0\n\
\n\
\n\
itoa:\n\
\n\
  addiu $sp $sp -16\n\
  sw $ra 0($sp)\n\
  sw $s0 4($sp)\n\
  sw $s1 8($sp)\n\
  sw $s2 12($sp)\n\
\n\
  move $s1 $a0 # $s1 = $a0\n\
\n\
  jal intlen # $v0 = intlen($a0)\n\
  move $s2 $v0 # $s2 = $v0\n\
\n\
  addiu $a0 $s2 1\n\
  li $v0 9 # sbrk\n\
  syscall\n\
  move $s0 $v0 # $s0 = sbrk($s2 + 1)\n\
\n\
  move $a0 $s0 # $a0 = $s0 // chaîne\n\
  move $a1 $s1 # $a1 = $s1 // entier\n\
  move $a2 $s2 # $a2 = $s2 // longueur sans \\0\n\
  addu $s2 $s2 $a0 # $s2 += $a0 // position du \\0\n\
  jal sprintint\n\
\n\
  sb $0 0($s2) # *$s2 = '\\0'\n\
\n\
  move $v0 $s0\n\
\n\
  lw $s2 12($sp)\n\
  lw $s1 8($sp)\n\
  lw $s0 4($sp)\n\
  lw $ra 0($sp)\n\
  addiu $sp $sp 16\n\
\n\
  jr $ra\n\
\n\
\n\
\n\
\n\
### Étiquettes de sortie ###\n\
\n\
\n\
exit_success:\n\
  li $v0 10\n\
  syscall\n\
\n\
exit_failure:\n\
  li $a0 1\n\
  li $v0 17\n\
  syscall\n\
\n\
\n\
\n\
\n\
";

const char* getDataStart(){
    return data;
}

const char* getTextStart(){
    return text;
}
