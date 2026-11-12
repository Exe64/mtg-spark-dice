---
plan:
  id: '6'
  name: 6-marqueurs-coeur-6-joueurs.md
  link: https://github.com/Exe64/mtg-spark-dice/blob/main/doc/roadmap/6-marqueurs-coeur-6-joueurs.md
status: active
date: 2026-11-13
description: >
  Marqueurs Magic (énergie, expérience, Trésor, Nourriture, tempête, radiation, Ascension, monarque, initiative, Jour/Nuit) et cœur à 6 joueurs avec commandant partenaire, en un seul changement de format de sauvegarde.
priority: high
complexity: L
intent: null
scope:
  modules:
    - core/
    - ui/
    - host/
issue:
  id: 6
  url: https://github.com/Exe64/mtg-spark-dice/issues/6
---

# [🔮] Plan #6 - Marqueurs et cœur 6 joueurs

## Objectif

Suivre en partie les marqueurs Magic que la référence (Soul Dial) propose au-delà du poison, avec un accès direct depuis le Solo et l'Arène. Le modèle de joueur du cœur change une seule fois : marqueurs, 6 joueurs et commandant partenaire arrivent ensemble, avec un seul nouveau format de sauvegarde.

## Dépendances

Dépend de #5 (les écrans sont construits dans le thème lumineux). Bloque #7 (format de partie archivé) et #4 (qui s'appuie sur le cœur à 6 joueurs et le partenaire). Passe avant #2 (ordre décidé le 2026-11-13).

## Périmètre

### Inclus
- Cœur : jusqu'à 6 joueurs ; 2 compteurs de blessures de commandant par joueur source (commandant partenaire) ; étape reprise du plan #4
- Cœur : marqueurs par joueur en tableau générique : poison (existant), énergie, expérience, Trésor, Nourriture, tempête, radiation
- Cœur : Ascension (acquise ou non, par joueur)
- Cœur : états de table : monarque et initiative (un seul détenteur, ou personne), Jour/Nuit (aucun, jour, nuit)
- Journal et annulation pour tous ces changements
- Sauvegarde : un seul nouveau format ; une partie enregistrée dans l'ancien format est ignorée (nouvelle partie)
- Accès direct aux marqueurs : « + » latéral en Solo, flèche au bord du siège en Arène ; la molette règle le marqueur choisi
- Affichage en partie : marqueurs non nuls sur le siège et en Solo ; détenteur du monarque et de l'initiative ; Jour/Nuit
- Icônes des marqueurs (matériel Wizards autorisé, voir PRODUCT.md)

### Hors scope
- Arène 5–6 sièges, noms et couleurs des joueurs, bibliothèques (plan #4)
- Marqueurs personnalisés : demandent la saisie de texte à la molette, livrée par #2
- Règles automatiques liées aux marqueurs (pioche du monarque, remise à zéro de la tempête en fin de tour)
- Synchronisation entre cadrans (lot 3)
- Validation sur matériel

## Étapes

- [ ] Étape 1 — Cœur : modèle joueur (6 joueurs, 2 compteurs de commandant par source, tableau de marqueurs, Ascension), états de table (monarque, initiative, Jour/Nuit), journal et annulation, nouveau format de sauvegarde unique + tests host (L · reasoning → Opus)
- [ ] Étape 2 — Panneau Marqueurs : accès direct en Solo (« + » latéral) et en Arène (flèche au bord du siège), réglage à la molette, attribution du monarque et de l'initiative, bascule Jour/Nuit (M · reasoning → Opus)
- [ ] Étape 3 — Affichage en partie : marqueurs non nuls sur les sièges et en Solo, détenteurs du monarque et de l'initiative, Jour/Nuit ; fiche joueur adaptée au partenaire (M · standard → Sonnet)
- [ ] Étape 4 — Icônes des marqueurs dans la chaîne d'assets existante (S · standard → Sonnet)
- [ ] Étape 5 — Design : captures, revue de finition, DESIGN.md (S · standard → Sonnet)
- [ ] 🧪 Tests — Rédiger et exécuter la procédure de test (avant-dernière étape, obligatoire)
- [ ] ✅ Validation — Vérifier les résultats des tests et clôturer (dernière étape, obligatoire)

## Décisions techniques

| Décision | Choix retenu | Justification |
|----------|-------------|---------------|
| Référence | Liste de marqueurs reprise du Soul Dial (page d'aide lue le 2026-11-12), Magic uniquement | Demande opérateur : se rapprocher au maximum de la référence |
| Format de sauvegarde | Un seul changement pour marqueurs, 6 joueurs et partenaire | Choix opérateur 2026-11-13 ; l'étape 1 de #4 est déplacée ici |
| Anciennes sauvegardes | Ignorées, pas de migration | Aucun boîtier en service avant ce plan |
| Ordre | #5 → #6 → #7 → #2 → #4 → #3 | Choix opérateur 2026-11-13 : la parité en partie passe avant les réglages d'appareil |
| Marqueurs personnalisés | Reportés après #2 | Dépendent de la saisie de texte à la molette |

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
- ✅ Fait : comparaison avec le Soul Dial, création de l'issue #6 et du plan ; étape cœur « 6 joueurs + partenaire » reprise de #4
- 🔄 En cours : —
- 📋 Prochain : étape 1 (après #5)
- 🚧 Blocages : matériel non reçu
