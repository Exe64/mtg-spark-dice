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
    - profiles/
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

- [x] Étape 1 — Cœur : modèle joueur (6 joueurs, 2 compteurs de commandant par source, tableau de marqueurs, Ascension), états de table (monarque, initiative, Jour/Nuit), journal et annulation, nouveau format de sauvegarde unique + tests host (L · reasoning → Opus)
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
| Modèle des marqueurs | Un genre d'action par marqueur (`SD_POISON` … `SD_RAD`), rangés dans `counter[]` ; même API `sd_game_adjust` que la vie | Aucun paramètre ajouté, l'interface du plan #5 ne change pas |
| Partenaire | `SD_CMD` = premier commandant de la source, `SD_CMD2` = le second ; chacun est létal séparément | Règle Magic : 21 blessures d'un même commandant |
| États (Ascension, monarque, initiative, Jour/Nuit) | `sd_game_set()` : effet immédiat, sans délai de validation, journalisé et annulable ; aucune règle automatique (la couronne ne bouge pas seule si son détenteur est vaincu) | Hors périmètre : règles automatiques |
| Nom du format | Enregistré dans la partie (`format`), posé par l'interface | Une partie reprise d'un format construit s'affichait « STANDARD » (limite notée au plan #5) |
| Sièges affichés | Le cœur joue à 6, les profils restent plafonnés à 4 sièges (`SEATS_MAX` dans `profiles/profile.c`) | L'Arène à 5 et 6 sièges est dans le plan #4 |

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
# --- T1 Cœur (étape 1)
# T1.1 tests PC et ASan/UBSan (test_core couvre : 6 joueurs, partenaire, 7 marqueurs, états, sauvegarde)
cmake -S /srv/dev/mtg-dial/host -B /srv/dev/mtg-dial/host/build -G Ninja >/dev/null && cmake --build /srv/dev/mtg-dial/host/build | tail -1 && ctest --test-dir /srv/dev/mtg-dial/host/build | tail -3
cmake -S /srv/dev/mtg-dial/host -B /srv/dev/mtg-dial/host/build-asan -G Ninja -DCMAKE_C_FLAGS="-fsanitize=address,undefined -fno-sanitize-recover=all" >/dev/null && cmake --build /srv/dev/mtg-dial/host/build-asan | tail -1 && ctest --test-dir /srv/dev/mtg-dial/host/build-asan | tail -3
/srv/dev/mtg-dial/host/build-asan/test_core
# T1.2 les trois nouveaux tests existent et sont appelés
grep -c "test_six_players_partner\|test_counters\|test_states" /srv/dev/mtg-dial/host/test_core.c
# T1.3 l'interface n'a pas bougé : les captures de référence du plan #5 sont toujours celles que produit le code
bash -c 'n=0; for f in /srv/dev/mtg-dial/.impeccable/review/[0-9]*.png /srv/dev/mtg-dial/.impeccable/review/fx-specimen.png; do cmp -s $f /srv/dev/mtg-dial/host/build/shots-out/$(basename $f) && n=$((n+1)); done; echo lumineux=$n/33'
bash -c 'n=0; for f in /srv/dev/mtg-dial/.impeccable/review/booster/[0-9]*.png; do cmp -s $f /srv/dev/mtg-dial/host/build/shots-out/booster/$(basename $f) && n=$((n+1)); done; echo booster=$n/26'
# T1.4 nouveau format de sauvegarde, limites du modèle
grep -c "SDG3" /srv/dev/mtg-dial/core/game.c; grep -c "define SD_MAX_PLAYERS 6\|define SD_CMD_SLOTS 2" /srv/dev/mtg-dial/core/game.h
# T1.5 firmware
bash -c '. ~/esp/esp-idf/export.sh >/dev/null 2>&1 && cd /srv/dev/mtg-dial && idf.py build >/tmp/claude-1000/t6-1.log 2>&1; echo build_exit=$?'
grep -c "warning:" /tmp/claude-1000/t6-1.log
```

**Résultats attendus :**
- T1.1 : deux fois `100% tests passed, 0 tests failed out of 6`, puis `core: all tests passed`.
- T1.2 : `6` (trois définitions, trois appels).
- T1.3 : `lumineux=33/33`, `booster=26/26`.
- T1.4 : `1`, puis `2`.
- T1.5 : `build_exit=0`, `0` avertissement.

### Résultats joués et vérifiés

| Date | Test | Attendu | Observé | Verdict |
|------|------|---------|---------|---------|
| 2026-11-13 | T1.1 tests PC + ASan/UBSan | 6/6 deux fois, `core: all tests passed` | identique | ✅ PASS (vérificateur indépendant) |
| 2026-11-13 | T1.2 nouveaux tests présents | `6` | `6` | ✅ PASS |
| 2026-11-13 | T1.3 interface inchangée | `lumineux=33/33`, `booster=26/26` | identique | ✅ PASS |
| 2026-11-13 | T1.4 format et limites | `1`, `2` | `1`, `2` | ✅ PASS |
| 2026-11-13 | T1.5 firmware | `build_exit=0`, `0` | `build_exit=0`, `0` | ✅ PASS |

## Journal de session

### Session 2026-11-13
- ✅ Fait : comparaison avec le Soul Dial, création de l'issue #6 et du plan ; étape cœur « 6 joueurs + partenaire » reprise de #4
- 🔄 En cours : —
- 📋 Prochain : étape 1 (après #5)
- 🚧 Blocages : matériel non reçu
- ✅ Fait : étape 1. Cœur à 6 joueurs, commandant partenaire, 7 marqueurs, Ascension, monarque, initiative, Jour/Nuit, journal et annulation, format de sauvegarde « SDG3 » (1202 octets) avec le nom du format. L'interface est inchangée (elle n'utilise encore que le premier commandant et le poison).
- 📋 Prochain : étape 2 (panneau Marqueurs)
- 🔎 Relecture du vérificateur : aucun défaut dans le cœur. Cas non couverts par une assertion (code correct à la lecture) : aller-retour de sauvegarde à 6 joueurs, non-létalité d'expérience / Trésor / Nourriture / tempête, annulation d'une Ascension gagnée, initiative remise à « personne ».
- ⚠️ À traiter aux étapes 2 et 3 (interface, inatteignable aujourd'hui) : les lignes du journal indexent la couleur du siège par `a->player`, qui vaut `SD_NOBODY` ou une valeur Jour/Nuit pour un état ; la pastille d'annulation n'inverse le signe que pour `SD_CMD`, pas `SD_CMD2`.
