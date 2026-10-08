/*============================================================================
 * main.c — Orchestration for Connect Four (Puissance 4)
 *
 * Contains ONLY main(): menu loop, name reading, game creation/destruction,
 * game play, replay loop, and clean exit.
 * Must NOT contain: game rules, grid access, or alignment logic.
 *============================================================================*/

#include "puissance4.h"

#ifdef _WIN32
#include <windows.h>
static void activerCouleursWindows(void)
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= 0x0004; /* ENABLE_VIRTUAL_TERMINAL_PROCESSING */
            SetConsoleMode(hOut, dwMode);
        }
    }
}
#endif

int main(void)
{
#ifdef _WIN32
    activerCouleursWindows();
#endif
    int choixMenu;

    /* Main menu loop */
    while ((choixMenu = afficherMenu()) != 3 && choixMenu != -1) {
        if (choixMenu == 2) {
            afficherAide();
            continue;
        }

        /* choixMenu == 1: New game */
        int rejouer = 1;

        while (rejouer) {
            /* Read player names */
            char nom1[TAILLE_NOM];
            char nom2[TAILLE_NOM];

            printf("\n");
            if (!lireNomJoueur(nom1, "Joueur 1", 1)) {
                break;  /* EOF during name entry */
            }
            if (!lireNomJoueur(nom2, "Joueur 2", 2)) {
                break;  /* EOF during name entry */
            }

            /* Create game */
            Partie *partie = creerPartie(nom1, nom2);
            if (partie == NULL) {
                printf(COULEUR_ERREUR
                       "  Memory allocation failed. Returning to menu."
                       COULEUR_REINITIALISATION "\n");
                break;
            }

            /* Play the game */
            jouerPartie(partie);

            /* Clean up */
            detruirePartie(partie);
            partie = NULL;

            /* Ask to replay */
            rejouer = demanderRejouer();
        }
    }

    printf("\n" COULEUR_TITRE
           "  Thank you for playing! Goodbye."
           COULEUR_REINITIALISATION "\n\n");

    return 0;
}
