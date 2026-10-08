/*============================================================================
 * project.c — All implementations for Connect Four (Puissance 4)
 *
 * Organized in six sections:
 *   1. Private helpers
 *   2. Memory management
 *   3. Grid operations
 *   4. Alignment detection
 *   5. Turn flow
 *   6. Interface (menu, help, names, replay)
 *============================================================================*/

#include "puissance4.h"
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

/* ========================================================================== *
 *  Forward declarations of private helpers                                    *
 * ========================================================================== */

static int  lireLigne(char *tampon, int taille);
static int  lireEntier(const char *chaine, int *resultat);
static char symboleJoueur(int joueur);
static const char *couleurJoueur(int joueur);
static int  compterDirection(const Partie *partie, int ligne, int colonne,
                             int deltaLigne, int deltaColonne, int joueur);
static int  aGagneDernierCoup(const Partie *partie, int joueur);
static int  alignementDirection(const Partie *partie, int joueur,
                                int deltaLigne, int deltaColonne);

/* ========================================================================== *
 *  SECTION 1 — Private helpers                                                *
 * ========================================================================== */

/*
 * lireLigne — Read one line from stdin into tampon.
 * Strips the trailing newline. If the line was longer than taille-1,
 * the remainder is discarded so that the next read starts fresh.
 * Returns 1 on success, 0 on EOF.
 */
static int lireLigne(char *tampon, int taille)
{
    if (fgets(tampon, taille, stdin) == NULL) {
        return 0;
    }

    size_t longueur = strlen(tampon);

    if (longueur > 0 && tampon[longueur - 1] == '\n') {
        tampon[longueur - 1] = '\0';
    } else {
        /* Line was too long: discard the rest */
        int caractere;
        while ((caractere = getchar()) != '\n' && caractere != EOF) {
            /* consume leftover characters */
        }
    }

    return 1;
}

/*
 * lireEntier — Parse an integer from chaine using strtol.
 * Rejects trailing non-whitespace characters (e.g. "3abc").
 * Rejects overflow/underflow and empty strings.
 * Returns 1 on success (result stored in *resultat), 0 on failure.
 */
static int lireEntier(const char *chaine, int *resultat)
{
    if (chaine == NULL || chaine[0] == '\0') {
        return 0;
    }

    char *finConversion;
    errno = 0;
    long valeur = strtol(chaine, &finConversion, 10);

    /* Reject if nothing was parsed */
    if (finConversion == chaine) {
        return 0;
    }

    /* Reject trailing non-whitespace characters like "3abc" */
    while (*finConversion != '\0') {
        if (!isspace((unsigned char)*finConversion)) {
            return 0;
        }
        finConversion++;
    }

    /* Reject overflow / underflow */
    if (errno == ERANGE || valeur < INT_MIN || valeur > INT_MAX) {
        return 0;
    }

    *resultat = (int)valeur;
    return 1;
}

/*
 * symboleJoueur — Return 'X' or 'O' for the given player value.
 */
static char symboleJoueur(int joueur)
{
    return (joueur == JOUEUR_X) ? 'X' : 'O';
}

/*
 * couleurJoueur — Return the ANSI color escape sequence for a player.
 */
static const char *couleurJoueur(int joueur)
{
    return (joueur == JOUEUR_X) ? COULEUR_JOUEUR_X : COULEUR_JOUEUR_O;
}

/*
 * compterDirection — Count consecutive tokens of 'joueur' starting from
 * (ligne, colonne) and moving by (deltaLigne, deltaColonne).
 * Does NOT count the starting cell itself.
 * Used by aGagneDernierCoup for the fast last-move check.
 */
static int compterDirection(const Partie *partie, int ligne, int colonne,
                            int deltaLigne, int deltaColonne, int joueur)
{
    int nbConsecutifs = 0;
    int ligneActuelle = ligne + deltaLigne;
    int colonneActuelle = colonne + deltaColonne;

    while (ligneActuelle >= 0 && ligneActuelle < NB_LIGNES &&
           colonneActuelle >= 0 && colonneActuelle < NB_COLONNES &&
           partie->grille[ligneActuelle][colonneActuelle] == (char)joueur) {
        nbConsecutifs++;
        ligneActuelle += deltaLigne;
        colonneActuelle += deltaColonne;
    }

    return nbConsecutifs;
}

/*
 * aGagneDernierCoup — Fast O(1) win check centered on the last move.
 * For each of the 4 directions, counts tokens in both senses and checks
 * if total >= NB_ALIGNES. Only valid when derniereLigne >= 0.
 */
static int aGagneDernierCoup(const Partie *partie, int joueur)
{
    if (partie == NULL || partie->derniereLigne < 0) {
        return 0;
    }

    int ligne = partie->derniereLigne;
    int colonne = partie->derniereColonne;

    /* Direction vectors: horizontal, vertical, diag-down-right, diag-down-left */
    static const int directions[NB_DIRECTIONS][2] = {
        {0, 1}, {1, 0}, {1, 1}, {1, -1}
    };

    for (int i = 0; i < NB_DIRECTIONS; i++) {
        int dl = directions[i][0];
        int dc = directions[i][1];
        int total = 1 + compterDirection(partie, ligne, colonne, dl, dc, joueur)
                      + compterDirection(partie, ligne, colonne, -dl, -dc, joueur);
        if (total >= NB_ALIGNES) {
            return 1;
        }
    }

    return 0;
}

/*
 * alignementDirection — Full-grid scan for NB_ALIGNES consecutive tokens
 * of 'joueur' in the direction (deltaLigne, deltaColonne).
 * Used by the three public alignment functions to avoid code duplication.
 */
static int alignementDirection(const Partie *partie, int joueur,
                                int deltaLigne, int deltaColonne)
{
    for (int ligne = 0; ligne < NB_LIGNES; ligne++) {
        for (int colonne = 0; colonne < NB_COLONNES; colonne++) {
            if (partie->grille[ligne][colonne] != (char)joueur) {
                continue;
            }

            /* Check if NB_ALIGNES tokens fit within bounds */
            int finLigne = ligne + (NB_ALIGNES - 1) * deltaLigne;
            int finColonne = colonne + (NB_ALIGNES - 1) * deltaColonne;

            if (finLigne < 0 || finLigne >= NB_LIGNES ||
                finColonne < 0 || finColonne >= NB_COLONNES) {
                continue;
            }

            int estAligne = 1;
            for (int k = 1; k < NB_ALIGNES; k++) {
                int ligneTest = ligne + k * deltaLigne;
                int colonneTest = colonne + k * deltaColonne;
                if (partie->grille[ligneTest][colonneTest] != (char)joueur) {
                    estAligne = 0;
                    break;
                }
            }

            if (estAligne) {
                return 1;
            }
        }
    }

    return 0;
}

/* ========================================================================== *
 *  SECTION 2 — Memory management                                             *
 * ========================================================================== */

Partie *creerPartie(const char *nom1, const char *nom2)
{
    if (nom1 == NULL || nom2 == NULL) {
        return NULL;
    }

    Partie *partie = malloc(sizeof(Partie));
    if (partie == NULL) {
        return NULL;
    }

    /* Initialize grid to empty */
    initialiserGrille(partie);

    /* Copy player names with bounded copy and forced terminator */
    snprintf(partie->noms[0], TAILLE_NOM, "%s", nom1);
    snprintf(partie->noms[1], TAILLE_NOM, "%s", nom2);

    /* Player X (index 0) always starts */
    partie->joueurCourant = JOUEUR_X;

    return partie;
}

void detruirePartie(Partie *partie)
{
    free(partie);  /* free(NULL) is safe per the C standard */
}

/* ========================================================================== *
 *  SECTION 3 — Grid operations                                                *
 * ========================================================================== */

void initialiserGrille(Partie *partie)
{
    if (partie == NULL) {
        return;
    }

    for (int ligne = 0; ligne < NB_LIGNES; ligne++) {
        for (int colonne = 0; colonne < NB_COLONNES; colonne++) {
            partie->grille[ligne][colonne] = CASE_VIDE;
        }
    }

    partie->nbJetons = 0;
    partie->derniereLigne = -1;
    partie->derniereColonne = -1;
}

void afficherGrille(const Partie *partie)
{
    if (partie == NULL) {
        return;
    }

    /* Column numbers header */
    printf("\n  ");
    for (int colonne = 0; colonne < NB_COLONNES; colonne++) {
#ifdef ASCII_ONLY
        printf("  %d ", colonne + 1);
#else
        printf("  %d  ", colonne + 1);
#endif
    }
    printf("\n");

    /* Top border */
    printf("  " BORD_HAUT_GAUCHE);
    for (int colonne = 0; colonne < NB_COLONNES; colonne++) {
        printf(BORD_HORIZONTAL);
        if (colonne < NB_COLONNES - 1) {
            printf(BORD_T_HAUT);
        }
    }
    printf(BORD_HAUT_DROIT "\n");

    /* Grid rows */
    for (int ligne = 0; ligne < NB_LIGNES; ligne++) {
        printf("  " BORD_VERTICAL);
        for (int colonne = 0; colonne < NB_COLONNES; colonne++) {
            char cellule = partie->grille[ligne][colonne];

            if (cellule == (char)JOUEUR_X) {
                printf("%s" JETON_X COULEUR_REINITIALISATION,
                       COULEUR_JOUEUR_X);
            } else if (cellule == (char)JOUEUR_O) {
                printf("%s" JETON_O COULEUR_REINITIALISATION,
                       COULEUR_JOUEUR_O);
            } else {
                printf(COULEUR_DIM JETON_VIDE COULEUR_REINITIALISATION);
            }

            printf(BORD_VERTICAL);
        }
        printf("\n");

        /* Row separator or bottom border */
        if (ligne < NB_LIGNES - 1) {
            printf("  " BORD_T_GAUCHE);
            for (int colonne = 0; colonne < NB_COLONNES; colonne++) {
                printf(BORD_HORIZONTAL);
                if (colonne < NB_COLONNES - 1) {
                    printf(BORD_CROIX);
                }
            }
            printf(BORD_T_DROIT "\n");
        }
    }

    /* Bottom border */
    printf("  " BORD_BAS_GAUCHE);
    for (int colonne = 0; colonne < NB_COLONNES; colonne++) {
        printf(BORD_HORIZONTAL);
        if (colonne < NB_COLONNES - 1) {
            printf(BORD_T_BAS);
        }
    }
    printf(BORD_BAS_DROIT "\n\n");
}

int colonneValide(int colonne)
{
    return (colonne >= 0 && colonne < NB_COLONNES) ? 1 : 0;
}

int colonneLibre(const Partie *partie, int colonne)
{
    if (partie == NULL || !colonneValide(colonne)) {
        return 0;
    }

    /* Row 0 is the top: if it's empty, the column has room */
    return (partie->grille[0][colonne] == (char)CASE_VIDE) ? 1 : 0;
}

int placerJeton(Partie *partie, int colonne)
{
    if (partie == NULL || !colonneValide(colonne) || !colonneLibre(partie, colonne)) {
        return 0;
    }

    /* Walk from bottom to top to find the lowest empty cell */
    for (int ligne = NB_LIGNES - 1; ligne >= 0; ligne--) {
        if (partie->grille[ligne][colonne] == (char)CASE_VIDE) {
            partie->grille[ligne][colonne] = (char)partie->joueurCourant;
            partie->nbJetons++;
            partie->derniereLigne = ligne;
            partie->derniereColonne = colonne;
            return 1;
        }
    }

    return 0;  /* Should not reach here given the colonneLibre check */
}

/* ========================================================================== *
 *  SECTION 4 — Alignment detection                                            *
 * ========================================================================== */

int alignementHorizontal(const Partie *partie, int joueur)
{
    if (partie == NULL) {
        return 0;
    }
    return alignementDirection(partie, joueur, 0, 1);
}

int alignementVertical(const Partie *partie, int joueur)
{
    if (partie == NULL) {
        return 0;
    }
    return alignementDirection(partie, joueur, 1, 0);
}

int alignementDiagonal(const Partie *partie, int joueur)
{
    if (partie == NULL) {
        return 0;
    }
    /* Descending diagonal (↘) and ascending diagonal (↗) */
    return alignementDirection(partie, joueur, 1, 1) ||
           alignementDirection(partie, joueur, 1, -1);
}

int joueurAGagne(const Partie *partie, int joueur)
{
    if (partie == NULL) {
        return 0;
    }
    return alignementHorizontal(partie, joueur) ||
           alignementVertical(partie, joueur) ||
           alignementDiagonal(partie, joueur);
}

/* ========================================================================== *
 *  SECTION 5 — Turn flow                                                      *
 * ========================================================================== */

int grillePleine(const Partie *partie)
{
    if (partie == NULL) {
        return 0;
    }
    return (partie->nbJetons == NB_LIGNES * NB_COLONNES) ? 1 : 0;
}

static void renommerJoueurEnCours(Partie *partie)
{
    if (partie == NULL) {
        return;
    }

    char tampon[TAILLE_TAMPON_SAISIE];
    printf("\n" COULEUR_TITRE
           "  Changer le nom de quel joueur ? (1: %s, 2: %s) : "
           COULEUR_REINITIALISATION,
           partie->noms[0], partie->noms[1]);
    fflush(stdout);

    if (!lireLigne(tampon, TAILLE_TAMPON_SAISIE)) {
        return;
    }

    int num;
    if (!lireEntier(tampon, &num) || (num != 1 && num != 2)) {
        printf(COULEUR_ERREUR
               "  Choix invalide. Entrez 1 ou 2."
               COULEUR_REINITIALISATION "\n\n");
        return;
    }

    char nomParDefaut[TAILLE_NOM];
    snprintf(nomParDefaut, sizeof(nomParDefaut), "Joueur %d", num);

    char nouveauNom[TAILLE_NOM];
    printf("\n");
    if (lireNomJoueur(nouveauNom, nomParDefaut, num)) {
        size_t idx = (size_t)(num - 1);
        snprintf(partie->noms[idx], TAILLE_NOM, "%.*s", TAILLE_NOM - 1, nouveauNom);
        printf(COULEUR_SUCCES
               "  -> Le Joueur %d (%c) s'appelle desormais %s !"
               COULEUR_REINITIALISATION "\n\n",
               num, (num == 1) ? 'X' : 'O', partie->noms[idx]);
    }
}

int demanderColonne(const Partie *partie)
{
    if (partie == NULL) {
        return -1;
    }

    char tampon[TAILLE_TAMPON_SAISIE];
    int joueur = partie->joueurCourant;

    for (;;) {
        printf("%s%s (%c)%s, entrez la colonne (1-%d) [0:passer, n:nom, r:reset] : ",
               couleurJoueur(joueur),
               partie->noms[(size_t)(joueur - 1)],
               symboleJoueur(joueur),
               COULEUR_REINITIALISATION,
               NB_COLONNES);
        fflush(stdout);

        if (!lireLigne(tampon, TAILLE_TAMPON_SAISIE)) {
            return -1;  /* EOF */
        }

        /* Support 'c', 'C', 's', 'S' for changing player turn */
        if (tampon[0] == 'c' || tampon[0] == 'C' || tampon[0] == 's' || tampon[0] == 'S') {
            return -2;  /* Change player turn */
        }

        /* Support 'n', 'N' for renaming a player mid-game */
        if (tampon[0] == 'n' || tampon[0] == 'N') {
            return -3;  /* Rename player */
        }

        /* Support 'r', 'R' for resetting/restarting the current game */
        if (tampon[0] == 'r' || tampon[0] == 'R') {
            return -4;  /* Reset game */
        }

        int choix;
        if (!lireEntier(tampon, &choix)) {
            printf(COULEUR_ERREUR
                   "  Saisie invalide. Entrez 1-%d, 0 (passer), n (nom) ou r (reset)."
                   COULEUR_REINITIALISATION "\n", NB_COLONNES);
            continue;
        }

        if (choix == 0) {
            return -2;  /* Change player turn */
        }

        int colonne = choix - 1;  /* Convert from 1-based to 0-based */

        if (!colonneValide(colonne)) {
            printf(COULEUR_ERREUR
                   "  Colonne %d hors limites. Choisissez entre 1 et %d."
                   COULEUR_REINITIALISATION "\n", choix, NB_COLONNES);
            continue;
        }

        if (!colonneLibre(partie, colonne)) {
            printf(COULEUR_ERREUR
                   "  La colonne %d est pleine. Choisissez une autre colonne."
                   COULEUR_REINITIALISATION "\n", choix);
            continue;
        }

        return colonne;
    }
}

void changerJoueur(Partie *partie)
{
    if (partie == NULL) {
        return;
    }
    /* Toggle between JOUEUR_X (1) and JOUEUR_O (2): 3 - 1 = 2, 3 - 2 = 1 */
    partie->joueurCourant = 3 - partie->joueurCourant;
}

void jouerPartie(Partie *partie)
{
    if (partie == NULL) {
        return;
    }

    printf("\n" COULEUR_TITRE
           "  La partie commence ! %s (X) vs %s (O)"
           COULEUR_REINITIALISATION "\n",
           partie->noms[0], partie->noms[1]);

    for (;;) {
        afficherGrille(partie);

        int colonne = demanderColonne(partie);
        if (colonne == -1) {
            /* EOF: end the game cleanly */
            printf("\n" COULEUR_IMPORTANT
                   "  Fin de saisie detectee. Partie interrompue."
                   COULEUR_REINITIALISATION "\n");
            return;
        }

        if (colonne == -2) {
            /* Change player / pass turn */
            changerJoueur(partie);
            printf("\n" COULEUR_IMPORTANT
                   "  -> Tour passe ! C'est maintenant a %s (%c) de jouer."
                   COULEUR_REINITIALISATION "\n",
                   partie->noms[(size_t)(partie->joueurCourant - 1)],
                   symboleJoueur(partie->joueurCourant));
            continue;
        }

        if (colonne == -3) {
            /* Rename a player mid-game */
            renommerJoueurEnCours(partie);
            continue;
        }

        if (colonne == -4) {
            /* Reset/Restart game mid-game */
            initialiserGrille(partie);
            partie->joueurCourant = JOUEUR_X;
            printf("\n" COULEUR_IMPORTANT
                   "  -> La grille a ete reinitialisee ! C'est a %s (X) de recommencer."
                   COULEUR_REINITIALISATION "\n\n",
                   partie->noms[0]);
            continue;
        }

        placerJeton(partie, colonne);

        /* Check victory BEFORE draw (a win on the 42nd token is a win) */
        if (aGagneDernierCoup(partie, partie->joueurCourant)) {
            afficherGrille(partie);
            printf(COULEUR_SUCCES
                   "  Felicitations, %s (%c) remporte la partie !"
                   COULEUR_REINITIALISATION "\n\n",
                   partie->noms[(size_t)(partie->joueurCourant - 1)],
                   symboleJoueur(partie->joueurCourant));
            return;
        }

        if (grillePleine(partie)) {
            afficherGrille(partie);
            printf(COULEUR_IMPORTANT
                   "  La grille est pleine. Match nul !"
                   COULEUR_REINITIALISATION "\n\n");
            return;
        }

        changerJoueur(partie);
    }
}

/* ========================================================================== *
 *  SECTION 6 — Interface (menu, help, name input, replay)                     *
 * ========================================================================== */

int afficherMenu(void)
{
    char tampon[TAILLE_TAMPON_SAISIE];

    printf("\n");
#ifdef ASCII_ONLY
    printf(COULEUR_TITRE
           "  +---------------------------+\n"
           "  |      PUISSANCE  4         |\n"
           "  +---------------------------+\n"
           "  |  1. Nouvelle Partie       |\n"
           "  |  2. Comment Jouer         |\n"
           "  |  3. Quitter               |\n"
           "  +---------------------------+"
           COULEUR_REINITIALISATION "\n\n");
#else
    printf(COULEUR_TITRE
           "  ╔═══════════════════════════╗\n"
           "  ║      PUISSANCE  4         ║\n"
           "  ╠═══════════════════════════╣\n"
           "  ║  1. Nouvelle Partie       ║\n"
           "  ║  2. Comment Jouer         ║\n"
           "  ║  3. Quitter               ║\n"
           "  ╚═══════════════════════════╝"
           COULEUR_REINITIALISATION "\n\n");
#endif

    for (;;) {
        printf("  Votre choix (1-3) : ");
        fflush(stdout);

        if (!lireLigne(tampon, TAILLE_TAMPON_SAISIE)) {
            return -1;  /* EOF */
        }

        int choix;
        if (!lireEntier(tampon, &choix) || choix < 1 || choix > 3) {
            printf(COULEUR_ERREUR
                   "  Choix invalide. Veuillez entrer 1, 2 ou 3."
                   COULEUR_REINITIALISATION "\n");
            continue;
        }

        return choix;
    }
}

void afficherAide(void)
{
    printf("\n");
#ifdef ASCII_ONLY
    printf(COULEUR_TITRE
           "  =========== COMMENT JOUER ==========="
           COULEUR_REINITIALISATION "\n\n");
#else
    printf(COULEUR_TITRE
           "  ═══════════ COMMENT JOUER ═══════════"
           COULEUR_REINITIALISATION "\n\n");
#endif

    printf("  Le Puissance 4 se joue a deux joueurs.\n");
    printf("  Chaque joueur depose un jeton (X ou O) a tour de role\n");
    printf("  dans l'une des 7 colonnes. Le jeton tombe jusqu'a la\n");
    printf("  case libre la plus basse disponible.\n\n");
    printf("  " COULEUR_IMPORTANT "Objectif :" COULEUR_REINITIALISATION
           " Aligner 4 jetons identiques (horizontalement,\n");
    printf("  verticalement ou en diagonale) avant son adversaire.\n\n");
    printf("  " COULEUR_IMPORTANT "Saisie :" COULEUR_REINITIALISATION
           " Entrez un numero de colonne entre 1 et 7.\n");
    printf("  Si une colonne est pleine, choisissez-en une autre.\n\n");
    printf("  " COULEUR_IMPORTANT "Match nul :" COULEUR_REINITIALISATION
           " Si les 42 cases sont remplies sans vainqueur,\n");
    printf("  la partie s'arrete sur une egalite.\n\n");
}

int lireNomJoueur(char *destination, const char *nomParDefaut, int numero)
{
    if (destination == NULL || nomParDefaut == NULL) {
        return 0;
    }

    char tampon[TAILLE_TAMPON_SAISIE];

    printf("  Entrez le nom du Joueur %d (par defaut : %s) : ", numero, nomParDefaut);
    fflush(stdout);

    if (!lireLigne(tampon, TAILLE_TAMPON_SAISIE)) {
        /* EOF: use default name */
        snprintf(destination, TAILLE_NOM, "%.*s", TAILLE_NOM - 1, nomParDefaut);
        return 0;
    }

    if (tampon[0] == '\0') {
        /* Empty input: use default name, announce it */
        snprintf(destination, TAILLE_NOM, "%.*s", TAILLE_NOM - 1, nomParDefaut);
        printf(COULEUR_DIM "  (Nom par defaut utilise : %s)"
               COULEUR_REINITIALISATION "\n", nomParDefaut);
    } else {
        snprintf(destination, TAILLE_NOM, "%.*s", TAILLE_NOM - 1, tampon);
    }

    return 1;
}

int demanderRejouer(void)
{
    char tampon[TAILLE_TAMPON_SAISIE];

    printf("\n" COULEUR_TITRE
           "  === FIN DE LA PARTIE ===" COULEUR_REINITIALISATION "\n"
           "  1. Rejouer directement avec les MEMES joueurs\n"
           "  2. Nouvelle partie avec de NOUVEAUX joueurs\n"
           "  3. Retour au menu principal\n\n");

    for (;;) {
        printf("  Votre choix (1-3) [defaut: 1] : ");
        fflush(stdout);

        if (!lireLigne(tampon, TAILLE_TAMPON_SAISIE)) {
            return 0;  /* EOF */
        }

        /* Default to 1 (same players) if user hits Enter */
        if (tampon[0] == '\0' || tampon[0] == '1' || tampon[0] == 'm' || tampon[0] == 'M') {
            return 2;  /* Replay with same players */
        }
        if (tampon[0] == '2' || tampon[0] == 'n' || tampon[0] == 'N') {
            return 1;  /* New players */
        }
        if (tampon[0] == '3' || tampon[0] == 'q' || tampon[0] == 'Q') {
            return 0;  /* Return to menu */
        }

        printf(COULEUR_ERREUR
               "  Choix invalide. Entrez 1 (memes joueurs), 2 (nouveaux), 3 (menu)."
               COULEUR_REINITIALISATION "\n");
    }
}
