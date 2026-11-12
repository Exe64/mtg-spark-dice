---
plan:
  id: '4'
  name: 4-joueurs-tours-minuteur.md
  link: https://github.com/Exe64/mtg-spark-dice/blob/main/doc/roadmap/4-joueurs-tours-minuteur.md
status: active
date: 2026-11-12
description: >
  Sections Joueurs (1–6, identité, partenaires, bibliothèques joueurs/decks) et Tours/Minuteur (suivi, minuteur, pause, alertes).
priority: medium
complexity: XL
enriched: 2026-11-13
intent: null
scope:
  modules:
    - core/
    - ui/
    - main/
    - profiles/
    - host/
issue:
  id: 4
  url: https://github.com/Exe64/mtg-spark-dice/issues/4
---

# [⏱️] Plan #4 - Joueurs, tours et minuteur

## Objectif

Gérer les joueurs (jusqu'à 6, nommés, avec commandant partenaire, bibliothèques de joueurs et de decks) et le temps de jeu (suivi des tours activable, minuteur de tour avec pause et alertes).

## Dépendances

Dépend de #2 (cadre Réglages et stockage `sd_settings`) et de #6 (cœur à 6 joueurs, commandant partenaire, format de sauvegarde).

## Périmètre

### Inclus
- Joueurs 1 à 6 : Arène 5–6 sièges (absorbe la partie « 5–6 joueurs » du lot 4) ; le cœur à 6 joueurs vient du plan #6
- Modifier les joueurs actifs : nom, couleur, commandant partenaire (interface ; les deux compteurs par source viennent du plan #6)
- Bibliothèque de joueurs (LittleFS)
- Configurer les decks (bibliothèque de decks, LittleFS)
- Suivi des tours on/off
- Minuteur de tour : durée et mode
- Pause rapide par l'anneau (au lieu de terminer le tour)
- Premier tour manuel (partie chronométrée en pause jusqu'au tour 1)
- Vibration d'alerte en fin de temps

### Hors scope
- Mode D&D (reste au lot 4)
- Synchronisation des joueurs entre cadrans (lot 3)
- Import de decks en ligne (backlog)
- Validation sur matériel

## Étapes

- [ ] Étape 1 — Arène 5–6 sièges : géométrie, tailles, orientation (M · reasoning → Opus)
- [ ] Étape 2 — Joueurs actifs : nom, couleur, partenaire ; bibliothèques joueurs et decks en LittleFS (L · reasoning → Opus)
- [ ] Étape 3 — Tours/Minuteur : suivi on/off, minuteur (durée, mode), pause par l'anneau, premier tour manuel, vibration d'alerte, testés avec horloge injectée (M · standard → Sonnet)
- [ ] Étape 4 — Design : captures, revue, DESIGN.md (S · standard → Sonnet)
- [ ] 🧪 Tests — Rédiger et exécuter la procédure de test (avant-dernière étape, obligatoire)
- [ ] ✅ Validation — Vérifier les résultats des tests et clôturer (dernière étape, obligatoire)

## Décisions techniques

| Décision | Choix retenu | Justification |
|----------|-------------|---------------|
| Origine des réglages | Liste fonctionnelle inspirée de la doc de réglages fournie par l'opérateur (app tierce) | Mécaniques reprises, apparence SparkDice (thème booster lisse), pas de copie visuelle |
| Lot 4 | La partie « 5–6 joueurs » est absorbée ici ; le mode D&D reste au lot 4 | Découpage validé 2026-11-12 |
| Cœur 6 joueurs + partenaire | Étape déplacée dans le plan #6 | Choix opérateur 2026-11-13 : un seul changement de format de sauvegarde, avec les marqueurs |

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
- ✅ Fait : cadrage (arborescence des réglages fournie par l'opérateur, découpage en 3 plans #2/#3/#4 validé), création de l'issue #4 et du plan
- 🔄 En cours : —
- 📋 Prochain : étape 1
- 🚧 Blocages : matériel non reçu (parties ressenties validées dans le plan « validation matériel »)

### Session 2026-11-13
- ✅ Fait : étape cœur (6 joueurs, partenaire, sauvegarde) déplacée dans #6 ; étapes renumérotées (4 étapes) ; dépendance à #6 ajoutée
- 📋 Prochain : étape 1 (Arène 5–6 sièges), après #2
