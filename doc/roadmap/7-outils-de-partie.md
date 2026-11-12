---
plan:
  id: '7'
  name: 7-outils-de-partie.md
  link: https://github.com/Exe64/mtg-spark-dice/blob/main/doc/roadmap/7-outils-de-partie.md
status: active
date: 2026-11-13
description: >
  Dés, tirage du premier joueur, menu de partie à 6 entrées, fin de partie (vainqueur, égalité, revanche, quitter) et historique des parties avec courbe de PV.
priority: high
complexity: L
intent: null
scope:
  modules:
    - core/
    - ui/
    - main/
    - host/
issue:
  id: 7
  url: https://github.com/Exe64/mtg-spark-dice/issues/7
---

# [🎲] Plan #7 - Outils de partie

## Objectif

Apporter les outils de table que la référence (Soul Dial) propose autour du compteur : lancer des dés, tirer le premier joueur, terminer proprement une partie et revoir les parties passées.

## Dépendances

Dépend de #6 (les parties archivées utilisent son format) et de #5 (thème lumineux). Passe avant #2 (ordre décidé le 2026-11-13). Les entrées Minuteur et Réglages du menu sont remplies par #4 et #2.

## Périmètre

### Inclus
- Dés : d4, d6, d8, d10, d12, d20, d100, pile ou face ; la molette choisit le dé, un tap lance
- Tirage du premier joueur, proposé au lancement d'une partie et depuis le menu ; il fixe le joueur actif
- Menu de partie à 6 entrées : Dés, Premier joueur, Parties passées, Réinitialiser, Minuteur, Réglages (ces deux dernières marquées « bientôt » jusqu'à #4 et #2)
- Le journal de la partie en cours se consulte depuis Parties passées ; l'annulation reste sur la pastille
- Fin de partie : désigner un vainqueur, égalité, revanche (mêmes joueurs et règles), quitter
- Historique : les parties terminées sont archivées sur l'appareil (nombre borné, les plus anciennes sont effacées) ; pour chacune : résultat, durée en tours, journal, courbe de PV calculée depuis le journal

### Hors scope
- Plateau de dés 3D, mini-jeux et Pomodoro de la référence
- Minuteur (plan #4) et Réglages (plan #2)
- Statistiques par joueur nommé ou par deck (demandent les bibliothèques de #4)
- Envoi de l'historique vers un service en ligne (lot 2)
- Validation sur matériel

## Étapes

- [ ] Étape 1 — Cœur : tirages (dés, pile ou face, premier joueur) avec source d'aléa injectée, issue de partie (vainqueur, égalité), revanche + tests host (M · standard → Sonnet)
- [ ] Étape 2 — Menu de partie à 6 entrées, navigation à la molette ; remplace le menu du plan #1 (M · standard → Sonnet)
- [ ] Étape 3 — Écrans Dés et Premier joueur ; aléa matériel côté HAL (M · reasoning → Opus)
- [ ] Étape 4 — Fin de partie : vainqueur, égalité, revanche, quitter ; remplace l'écran de fin du plan #1 (S · standard → Sonnet)
- [ ] Étape 5 — Historique : archivage borné des parties terminées, liste, détail (résultat, journal), courbe de PV (L · reasoning → Opus)
- [ ] Étape 6 — Design : captures, revue de finition, DESIGN.md (S · standard → Sonnet)
- [ ] 🧪 Tests — Rédiger et exécuter la procédure de test (avant-dernière étape, obligatoire)
- [ ] ✅ Validation — Vérifier les résultats des tests et clôturer (dernière étape, obligatoire)

## Décisions techniques

| Décision | Choix retenu | Justification |
|----------|-------------|---------------|
| Référence | Menu de partie et fin de partie repris du Soul Dial (page d'aide lue le 2026-11-12), Magic uniquement | Demande opérateur : se rapprocher au maximum de la référence |
| Dés | Affichage 2D du résultat, pas de plateau 3D | Rendu logiciel LVGL sur ESP32-S3 |
| Courbe de PV | Calculée depuis le journal archivé, pas de série stockée à part | Le journal contient déjà chaque changement de PV |
| Ordre | #5 → #6 → #7 → #2 → #4 → #3 | Choix opérateur 2026-11-13 |

## Tests

> Section obligatoire renseignée aux étapes de tests (mode `autonomous`).

### Prérequis pour les tests

```bash
# environnement du plan #1 (voir doc/roadmap/done/1-mvp-solo-arene.md § Prérequis) :
# gcc, cmake, ninja-build, libasan/libubsan ; host/third_party/{cJSON,lvgl} ; ESP-IDF v5.4.2 dans ~/esp/esp-idf
cd /srv/dev/mtg-dial
```

### Procédure de test

```bash
# à compléter
```

**Résultats attendus :** à compléter.

### Résultats joués et vérifiés

| Date | Test | Attendu | Observé | Verdict |
|------|------|---------|---------|---------|

## Journal de session

### Session 2026-11-13
- ✅ Fait : comparaison avec le Soul Dial, création de l'issue #7 et du plan
- 🔄 En cours : —
- 📋 Prochain : étape 1 (après #6)
- 🚧 Blocages : matériel non reçu
