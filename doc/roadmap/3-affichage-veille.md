---
plan:
  id: '3'
  name: 3-affichage-veille.md
  link: https://github.com/Exe64/mtg-spark-dice/blob/main/doc/roadmap/3-affichage-veille.md
status: active
date: 2026-11-12
description: >
  Section Affichage des réglages : luminosité, rotation, veille, options d'interaction de l'Arène, calibrage tactile.
priority: medium
complexity: L
intent: null
scope:
  modules:
    - ui/
    - main/
    - core/
    - host/
issue:
  id: 3
  url: https://github.com/Exe64/mtg-spark-dice/issues/3
---

# [🖥️] Plan #3 - Affichage et veille

## Objectif

Remplir la section Affichage : régler l'écran (luminosité, rotation, thème), économiser la batterie et l'écran (assombrir, éteindre, réveil), et ajuster l'interaction de l'Arène.

## Dépendances

Dépend de #2 (cadre Réglages et stockage `sd_settings`) et de #5 (thème lumineux + booster).

## Périmètre

### Inclus
- Luminosité (PWM LEDC GPIO 47)
- Rotation de l'écran par quarts de tour (affichage + tactile)
- Thème : sélecteur Lumineux / Booster (le booster reste disponible comme 2ᵉ thème, plan #5)
- Couleur de fond (thème Lumineux) : 6 teintes sombres prédéfinies avec aperçu en direct ; moteur fourni par le plan #5
- Assombrir après (2 min par défaut), luminosité réduite, écran éteint (5 min)
- Premier toucher pour réveiller (sans modifier de PV)
- Animations : normales / rapides / désactivées
- Afficher +/−, délai de sélection Arène, sélection requise pour les PV, pastille d'annulation on/off
- Calibrage tactile + test de tracé

### Hors scope
- Thèmes supplémentaires au-delà de Lumineux et Booster (backlog)
- Batterie / télémétrie (lot 2)
- Validation sur matériel

## Étapes

- [ ] Étape 1 — Veille : minuteries d'inactivité (assombrir, éteindre), réveil au premier toucher, testées sur PC avec horloge injectée (M · standard → Sonnet)
- [ ] Étape 2 — Luminosité et luminosité réduite (LEDC) côté HAL (S · standard → Sonnet)
- [ ] Étape 3 — Rotation de l'écran par quarts de tour, affichage et tactile (M · reasoning → Opus)
- [ ] Étape 4 — Options d'interaction : +/−, délai de sélection, sélection requise, pastille on/off, animations (M · standard → Sonnet)
- [ ] Étape 5 — Calibrage tactile + test de tracé (M · standard → Sonnet)
- [ ] Étape 6 — Design : captures, revue, DESIGN.md (S · standard → Sonnet)
- [ ] 🧪 Tests — Rédiger et exécuter la procédure de test (avant-dernière étape, obligatoire)
- [ ] ✅ Validation — Vérifier les résultats des tests et clôturer (dernière étape, obligatoire)

## Décisions techniques

| Décision | Choix retenu | Justification |
|----------|-------------|---------------|
| Origine des réglages | Liste fonctionnelle inspirée de la doc de réglages fournie par l'opérateur (app tierce) | Mécaniques reprises, apparence SparkDice (thème booster lisse), pas de copie visuelle |
| Veille | Remontée du lot 2 dans ce plan | Choix opérateur 2026-11-12 |

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

### Session 2026-11-12
- ✅ Fait : cadrage (arborescence des réglages fournie par l'opérateur, découpage en 3 plans #2/#3/#4 validé), création de l'issue #3 et du plan
- 🔄 En cours : —
- 📋 Prochain : étape 1
- 🚧 Blocages : matériel non reçu (parties ressenties validées dans le plan « validation matériel »)
