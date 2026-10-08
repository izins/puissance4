/*============================================================================
 * puissance4.h — Public interface for Connect Four (Puissance 4)
 *
 * Contains: header guard, includes, constants, enum, Partie type,
 *           ANSI color macros, and all public function prototypes.
 * Must NOT contain: function bodies, global variables, static functions.
 *============================================================================*/

#ifndef PUISSANCE4_H
#define PUISSANCE4_H

#include <stdio.h>
#include <stdlib.h>

/* ── Grid dimensions ─────────────────────────────────────────────────────── */
#define NB_LIGNES              6
#define NB_COLONNES            7
#define NB_JOUEURS             2
#define TAILLE_NOM             21    /* 20 visible chars + '\0' */
#define NB_ALIGNES             4     /* tokens needed to win */
#define TAILLE_TAMPON_SAISIE   256   /* fgets input buffer */
#define NB_DIRECTIONS          4     /* H, V, Diag↘, Diag↗ */

/* ── Cell / player values ────────────────────────────────────────────────── */
enum {
    CASE_VIDE = 0,
    JOUEUR_X  = 1,
    JOUEUR_O  = 2
};

/* ── ANSI color macros ───────────────────────────────────────────────────── */
#define COULEUR_TITRE           "\033[1;36m"   /* bold cyan */
#define COULEUR_ERREUR          "\033[1;31m"    /* bold red */
#define COULEUR_SUCCES          "\033[1;32m"    /* bold green */
#define COULEUR_IMPORTANT       "\033[1;33m"    /* bold yellow */
#define COULEUR_DIM             "\033[2m"       /* dim / faint */
#define COULEUR_JOUEUR_X        "\033[1;31m"    /* bold red for X */
#define COULEUR_JOUEUR_O        "\033[1;34m"    /* bold blue for O */
#define COULEUR_REINITIALISATION "\033[0m"      /* reset */

/* ── Unicode / ASCII border characters ───────────────────────────────────── */
#ifdef ASCII_ONLY
    #define BORD_HAUT_GAUCHE    "+"
    #define BORD_HAUT_DROIT     "+"
    #define BORD_BAS_GAUCHE     "+"
    #define BORD_BAS_DROIT      "+"
    #define BORD_HORIZONTAL     "---"
    #define BORD_VERTICAL       "|"
    #define BORD_T_HAUT         "+"
    #define BORD_T_BAS          "+"
    #define BORD_T_GAUCHE       "+"
    #define BORD_T_DROIT        "+"
    #define BORD_CROIX          "+"
    #define JETON_X             " X "
    #define JETON_O             " O "
    #define JETON_VIDE          " . "
#else
    #define BORD_HAUT_GAUCHE    "┌"
    #define BORD_HAUT_DROIT     "┐"
    #define BORD_BAS_GAUCHE     "└"
    #define BORD_BAS_DROIT      "┘"
    #define BORD_HORIZONTAL     "───"
    #define BORD_VERTICAL       "│"
    #define BORD_T_HAUT         "┬"
    #define BORD_T_BAS          "┴"
    #define BORD_T_GAUCHE       "├"
    #define BORD_T_DROIT        "┤"
    #define BORD_CROIX          "┼"
    #define JETON_X             " X "
    #define JETON_O             " O "
    #define JETON_VIDE          " . "
#endif

/* ── Game state structure ────────────────────────────────────────────────── */
typedef struct {
    char grille[NB_LIGNES][NB_COLONNES]; /* row 0 = top, last row = bottom */
    char noms[NB_JOUEURS][TAILLE_NOM];   /* fixed buffers: no overflow */
    int  joueurCourant;                  /* JOUEUR_X or JOUEUR_O */
    int  nbJetons;                       /* grid full when == NB_LIGNES * NB_COLONNES */
    int  derniereLigne;                  /* last move row,    -1 if none */
    int  derniereColonne;                /* last move column, -1 if none */
} Partie;

/* ── Memory management ───────────────────────────────────────────────────── */

/*
 * creerPartie — Allocate and initialize a new game.
 * Parameters: nom1, nom2 — player names (empty string → default name).
 * Returns:    pointer to a new Partie, or NULL on allocation failure.
 * Side effects: allocates memory on the heap.
 */
Partie *creerPartie(const char *nom1, const char *nom2);

/*
 * detruirePartie — Free the game structure.
 * Parameters: partie — pointer to the Partie to free (NULL is tolerated).
 * Returns:    nothing.
 * Side effects: frees heap memory. Caller must set pointer to NULL after.
 */
void detruirePartie(Partie *partie);

/* ── Grid operations ─────────────────────────────────────────────────────── */

/*
 * initialiserGrille — Clear all cells and reset counters.
 * Parameters: partie — pointer to the Partie to reset.
 * Returns:    nothing.
 * Side effects: sets every cell to CASE_VIDE, resets nbJetons and last move.
 */
void initialiserGrille(Partie *partie);

/*
 * afficherGrille — Print the grid with colored tokens and borders.
 * Parameters: partie — pointer to the Partie to display.
 * Returns:    nothing.
 * Side effects: writes to stdout.
 */
void afficherGrille(const Partie *partie);

/*
 * colonneValide — Check whether a column index is within [0, NB_COLONNES).
 * Parameters: colonne — the column index to check (0-based).
 * Returns:    1 if valid, 0 otherwise.
 * Side effects: none.
 */
int colonneValide(int colonne);

/*
 * colonneLibre — Check whether a column has at least one empty cell.
 * Parameters: partie — pointer to the Partie; colonne — 0-based column.
 * Returns:    1 if the column is valid and not full, 0 otherwise.
 * Side effects: none.
 */
int colonneLibre(const Partie *partie, int colonne);

/*
 * placerJeton — Drop a token in the given column for the current player.
 * Parameters: partie — pointer to the Partie; colonne — 0-based column.
 * Returns:    1 on success, 0 if column is invalid or full.
 * Side effects: modifies grille, nbJetons, derniereLigne, derniereColonne.
 */
int placerJeton(Partie *partie, int colonne);

/* ── Alignment detection ─────────────────────────────────────────────────── */

/*
 * alignementHorizontal — Full-grid scan for 4 horizontal tokens.
 * Parameters: partie — pointer to the Partie; joueur — JOUEUR_X or JOUEUR_O.
 * Returns:    1 if an alignment is found, 0 otherwise.
 * Side effects: none.
 */
int alignementHorizontal(const Partie *partie, int joueur);

/*
 * alignementVertical — Full-grid scan for 4 vertical tokens.
 * Parameters: partie — pointer to the Partie; joueur — JOUEUR_X or JOUEUR_O.
 * Returns:    1 if an alignment is found, 0 otherwise.
 * Side effects: none.
 */
int alignementVertical(const Partie *partie, int joueur);

/*
 * alignementDiagonal — Full-grid scan for 4 diagonal tokens (both directions).
 * Parameters: partie — pointer to the Partie; joueur — JOUEUR_X or JOUEUR_O.
 * Returns:    1 if an alignment is found, 0 otherwise.
 * Side effects: none.
 */
int alignementDiagonal(const Partie *partie, int joueur);

/*
 * joueurAGagne — Check whether a player has won (OR of 3 alignment checks).
 * Parameters: partie — pointer to the Partie; joueur — JOUEUR_X or JOUEUR_O.
 * Returns:    1 if the player has won, 0 otherwise.
 * Side effects: none.
 */
int joueurAGagne(const Partie *partie, int joueur);

/* ── Turn flow ───────────────────────────────────────────────────────────── */

/*
 * grillePleine — Check whether the grid is completely filled.
 * Parameters: partie — pointer to the Partie.
 * Returns:    1 if full (nbJetons == NB_LIGNES * NB_COLONNES), 0 otherwise.
 * Side effects: none.
 */
int grillePleine(const Partie *partie);

/*
 * demanderColonne — Prompt the current player for a column (1-7 user input).
 * Parameters: partie — pointer to the Partie.
 * Returns:    valid 0-based column index, or -1 on EOF.
 * Side effects: reads from stdin, writes prompts/errors to stdout.
 */
int demanderColonne(const Partie *partie);

/*
 * changerJoueur — Switch the current player (X ↔ O).
 * Parameters: partie — pointer to the Partie.
 * Returns:    nothing.
 * Side effects: modifies joueurCourant.
 */
void changerJoueur(Partie *partie);

/*
 * jouerPartie — Run the complete game loop until win, draw, or EOF.
 * Parameters: partie — pointer to the Partie.
 * Returns:    nothing.
 * Side effects: modifies the entire Partie state, reads/writes stdio.
 */
void jouerPartie(Partie *partie);

/* ── UI helpers (public, implemented in project.c) ───────────────────────── */

/*
 * afficherMenu — Display the main menu and return the user's choice.
 * Returns:    1 (new game), 2 (how to play), 3 (quit), or -1 on EOF.
 * Side effects: reads from stdin, writes to stdout.
 */
int afficherMenu(void);

/*
 * afficherAide — Display the "How to Play" screen.
 * Returns:    nothing.
 * Side effects: writes to stdout.
 */
void afficherAide(void);

/*
 * lireNomJoueur — Read a player name from stdin with a default fallback.
 * Parameters: destination — buffer of TAILLE_NOM bytes;
 *             nomParDefaut — name to use if the input is empty;
 *             numero — player number (1 or 2, for the prompt).
 * Returns:    1 on success, 0 on EOF.
 * Side effects: reads from stdin, writes prompt to stdout, fills destination.
 */
int lireNomJoueur(char *destination, const char *nomParDefaut, int numero);

/*
 * demanderRejouer — Ask the user whether they want to play again.
 * Returns:    1 for yes, 0 for no or EOF.
 * Side effects: reads from stdin, writes to stdout.
 */
int demanderRejouer(void);

#endif /* PUISSANCE4_H */
