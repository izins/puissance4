# Puissance 4 (Connect Four) en C11

Implémentation complète, propre et défendable du jeu **Puissance 4** à deux joueurs en C11, conçue selon le rapport de conception et le cahier des charges de l'université.

---

## 📁 Structure du Projet

```text
PUISSANCE4/
├── puissance4.h       # Interface publique (constantes, structure Partie, prototypes)
├── project.c          # Implémentation de la logique du jeu et helpers privés
├── main.c             # Orchestration principale (menu, boucle de jeu, rejouer)
├── Makefile           # Script de compilation (all, run, debug, ascii, clean)
└── README.md          # Documentation du dépôt
```

---

## 🛠️ Compilation et Exécution

### Compilation Standard (Recommandée)
```bash
make
```
*Génère l'exécutable `puissance4` (ou `puissance4.exe` sous Windows) avec les drapeaux stricts `-std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -O2` sans aucun avertissement.*

### Lancement
```bash
./puissance4
```
ou via Make :
```bash
make run
```

---

## 🎨 Options de Compilation Supplémentaires

### Mode ASCII (si le terminal ne supporte pas Unicode)
```bash
make ascii
```

### Mode Débogage (avec AddressSanitizer et UndefinedBehaviorSanitizer)
```bash
make debug
```

---

## 🧹 Nettoyage des Fichiers Compilés
```bash
make clean
```

---

## 📋 Caractéristiques Clés
- **Gestion Mémoire** : Allocation dynamique sécurisée via `creerPartie()` (`malloc`) et `detruirePartie()` (`free`).
- **Saisie Sécurisée** : Protection contre les débordements de tampon (`fgets`), rejet des chaînes invalides type `"3abc"` (`strtol` avec vérification de `endptr`), et gestion propre de l'EOF (Ctrl+D / Ctrl+Z).
- **Détection de Victoire** :
  - Détection ultra-rapide en $O(1)$ basée sur le dernier coup joué (`aGagneDernierCoup`).
  - Fonctions d'alignement complet (`alignementHorizontal`, `alignementVertical`, `alignementDiagonal`) partagées via un helper générique.
  - Vérification de la victoire **avant** le match nul (victoire au 42ème coup gérée correctement).
