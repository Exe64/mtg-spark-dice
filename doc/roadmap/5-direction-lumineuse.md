---
plan:
  id: '5'
  name: 5-direction-lumineuse.md
  link: https://github.com/Exe64/mtg-spark-dice/blob/main/doc/roadmap/5-direction-lumineuse.md
status: active
date: 2026-11-12
description: >
  Nouvelle direction visuelle « lumineuse » pour toute l'interface (radiale, lueurs, fond bleu nuit, icônes, logo) ; le booster devient 2ᵉ thème.
priority: high
complexity: L
enriched: 2026-11-13
intent: null
scope:
  modules:
    - ui/
    - host/
    - .impeccable/
    - DESIGN.md
    - PRODUCT.md
issue:
  id: 5
  url: https://github.com/Exe64/mtg-spark-dice/issues/5
---

# [✨] Plan #5 - Direction visuelle lumineuse

## Objectif

Donner à SparkDice une direction visuelle lumineuse pour toute l'interface, dans l'esprit de la référence fournie par l'opérateur (disposition radiale, lueur et profondeur, fond sombre bleuté, icônes), avec une identité propre et le logo de l'opérateur. Le thème booster devient le 2ᵉ thème.

## Dépendances

Aucune (plan #1 livré). Bloque #6, #7 et #2 (leurs écrans sont construits dans la nouvelle direction) ; le sélecteur de thème (lumineux / booster) est dans #3.

## Périmètre

### Inclus
- Contrat de direction imposé par l'opérateur (pas de tirage) : disposition radiale, lueur et profondeur, fond sombre bleuté, icônes dessinées par nous, place du logo ; palette propre, couleurs de siège toujours accompagnées du numéro
- Système de rendu « lueur » compatible ESP32-S3 : halos et anneaux pré-calculés en images avec transparence (pas de flou temps réel), dégradés de fond, boutons ronds « verre »
- Jeu d'icônes (réglages, joueurs, reprendre, type de jeu, annuler, tour ; wifi / salon préparés pour plus tard)
- Restyle des écrans existants : Solo, Arène 1–4, moyeu, bande d'attente, pastille d'annulation, marque de défaite, menu de partie, journal, fiche joueur, fin de partie (règles et gestes inchangés)
- Logo de l'opérateur : accueil et écran de démarrage (nom du produit en une seule constante)
- Booster conservé intact comme 2ᵉ thème (structure de thème `ui_theme_t` étendue)
- Mesure du temps de rendu (PC) et estimation ESP32-S3

### Hors scope
- Mise en page de l'accueil radial, Réglages, mise en route (plan #2, construits dans ce style)
- Sélecteur de thème dans les réglages (plan #3)
- Copie d'éléments de la référence : couleurs exactes, icônes, logo, composition au pixel
- Validation sur matériel (coût réel des lueurs)

## Étapes

- [x] Étape 1 — Contrat de direction (impeccable, direction imposée) + PRODUCT.md (esprit de la référence autorisé, pas de copie) (S · reasoning → Opus)
- [x] Étape 2 — Système de thèmes : `ui_theme_t` étendu (fond dégradé, lueurs, icônes, rendu des sièges), couleur de fond paramétrable (dégradé dérivé d'une seule teinte, choix stocké hors partie), booster conservé tel quel (M · reasoning → Opus)
- [ ] Étape 3 — Rendu « lueur » : générateur de halos/anneaux pré-calculés (script, images A8/ARGB), dégradés, boutons « verre », jeu d'icônes dessinées (L · reasoning → Opus)
- [ ] Étape 4 — Écrans de partie dans le thème lumineux : Solo, Arène 1–4, moyeu, bande d'attente, pastille, marque de défaite (L · reasoning → Opus)
- [ ] Étape 5 — Menus dans le thème lumineux : menu de partie, journal, fiche joueur, fin de partie (M · standard → Sonnet)
- [ ] Étape 6 — Logo : intégration (accueil provisoire + écran de démarrage), nom du produit centralisé (S · standard → Sonnet)
- [ ] Étape 7 — Design : captures des 2 thèmes, revue de finition, DESIGN.md (thème lumineux principal + booster), mesure du temps de rendu (M · standard → Sonnet)
- [ ] 🧪 Tests — Rédiger et exécuter la procédure de test (avant-dernière étape, obligatoire)
- [ ] ✅ Validation — Vérifier les résultats des tests et clôturer (dernière étape, obligatoire)

## Décisions techniques

| Décision | Choix retenu | Justification |
|----------|-------------|---------------|
| Portée | Toute l'interface passe au style lumineux | Choix opérateur 2026-11-12 |
| Esprit retenu | Disposition radiale, lueur et profondeur, fond sombre bleuté, icônes | Choix opérateur 2026-11-12 (référence : accueil d'une app tierce) ; esprit repris, pas de copie |
| Booster | Conservé comme 2ᵉ thème, sélecteur dans #3 | Choix opérateur 2026-11-12 |
| Couleur de fond | Au choix de l'utilisateur dans les réglages : 6 teintes sombres prédéfinies (Nuit #18214A par défaut, Noir, Améthyste, Forêt, Grenat, Ardoise) ; le dégradé centre→bord est calculé depuis la teinte ; teinte sombre imposée pour garder le blanc ≥ 7:1 | Demande opérateur 2026-11-12 ; le sélecteur est dans #3 (Affichage), le moteur ici |
| Lueurs | Pré-calculées en images, pas de flou temps réel | Rendu logiciel LVGL sur ESP32-S3 ; coût réel à mesurer à réception |
| Nom du produit | SparkDice provisoire, « DiceSpark » à l'étude ; une seule constante pour renommer | Opérateur indécis (2026-11-12) ; deux compteurs MTG « Spark » existent déjà |
| Logo | `ui/assets/logo.svg` : trait blanc, fond transparent, molette 3/4 crantée, d20 enfoncé, rayons ; généré par `ui/assets/logo_gen.py` | Validé par l'opérateur 2026-11-12, dessiné ensemble (Gemini n'y arrivait pas) |
| Contrat lumineux | « Puits de lumière » : moyeu = source, fond #18214A→#060914, encre blanche au trait, sièges = lumière (#FF7A45, #3FD8C0, #FFC94D, #A98BFF) sur verre, 3 rayons vers le joueur actif, siège vaincu éteint | Étape 1 ; contrat dans `.impeccable/surfaces/ui-ui-c.md` |
| Fond radial | Dégradé radial natif de LVGL 9.2 (`LV_USE_DRAW_SW_COMPLEX_GRADIENTS`), pas de masque pré-calculé | Déjà dans LVGL ; à vérifier sur matériel : coût d'un plein écran et bandes visibles en RGB565 (repli : masque A8 pré-calculé) |
| Teinte du bord | 27 % de chaque composante de la teinte du centre | Redonne exactement le couple du contrat (#18214A → #060914) |
| Thème par défaut | Booster jusqu'à l'étape 4, puis Lumineux (`UI_THEME_DEFAULT`) | Les écrans lumineux n'existent pas avant l'étape 4 |
| Stockage de l'apparence | Clés NVS `theme` et `ground`, hors sauvegarde de partie ; lues au démarrage, écrites par le sélecteur du plan #3 | Étape 2 |
| Jetons de lueur, icônes, sièges | Ajoutés à `ui_theme_t` avec leur premier usage (étapes 3 et 4) | Pas de champ sans code qui le lit |

## Tests

> Section obligatoire renseignée aux étapes de tests (mode `autonomous`).

### Prérequis pour les tests

```bash
# environnement du plan #1 (voir doc/roadmap/done/1-mvp-solo-arene.md § Prérequis) :
# gcc, cmake, ninja-build, libasan/libubsan ; host/third_party/{cJSON,lvgl} ; ESP-IDF v5.4.2 dans ~/esp/esp-idf
cd /srv/dev/mtg-dial
# un sdkconfig généré avant l'étape 2 ignore la nouvelle valeur par défaut : l'activer une fois
sed -i 's/^# CONFIG_LV_USE_DRAW_SW_COMPLEX_GRADIENTS is not set$/CONFIG_LV_USE_DRAW_SW_COMPLEX_GRADIENTS=y/' /srv/dev/mtg-dial/sdkconfig
```

### Procédure de test

```bash
# --- T2 Système de thèmes (étape 2)
# T2.1 tests PC
cmake -S /srv/dev/mtg-dial/host -B /srv/dev/mtg-dial/host/build -G Ninja >/dev/null && cmake --build /srv/dev/mtg-dial/host/build | tail -1 && ctest --test-dir /srv/dev/mtg-dial/host/build | tail -3
# T2.2 mêmes tests sous ASan/UBSan
cmake -S /srv/dev/mtg-dial/host -B /srv/dev/mtg-dial/host/build-asan -G Ninja -DCMAKE_C_FLAGS="-fsanitize=address,undefined -fno-sanitize-recover=all" >/dev/null && cmake --build /srv/dev/mtg-dial/host/build-asan | tail -1 && ctest --test-dir /srv/dev/mtg-dial/host/build-asan | tail -3
# T2.3 booster inchangé : les 20 captures du plan #1 sont identiques à l'octet
bash -c 'n=0; for f in /srv/dev/mtg-dial/.impeccable/review/[0-9]*.png; do cmp -s $f /srv/dev/mtg-dial/host/build/shots-out/$(basename $f) && n=$((n+1)); done; echo identiques=$n/20'
# T2.4 captures du thème lumineux (6 fonds + Arène)
ls /srv/dev/mtg-dial/host/build/shots-out | grep -c lumineux
# T2.5 firmware
bash -c '. ~/esp/esp-idf/export.sh >/dev/null 2>&1 && cd /srv/dev/mtg-dial && idf.py build >/tmp/claude-1000/t2-5.log 2>&1; echo build_exit=$?'
grep -c "warning:" /tmp/claude-1000/t2-5.log
grep -c '^CONFIG_LV_USE_DRAW_SW_COMPLEX_GRADIENTS=y' /srv/dev/mtg-dial/sdkconfig
```

**Résultats attendus :**
- T2.1 et T2.2 : `100% tests passed, 0 tests failed out of 4`. Le test `ui_flows` vérifie pour chacun des 6 fonds : contraste de l'encre ≥ 7:1, pixel du centre = teinte choisie, pixels du bord (haut et gauche) = teinte dérivée ; `#18214A` donne le bord `#060914` ; une valeur de thème hors bornes retombe sur le thème par défaut.
- T2.3 : `identiques=20/20`.
- T2.4 : `7`.
- T2.5 : `build_exit=0`, `0` avertissement, `1` ligne de configuration.

### Résultats joués et vérifiés

| Date | Test | Attendu | Observé | Verdict |
|------|------|---------|---------|---------|
| 2026-11-13 | T2.1 tests PC | 4/4 tests passés | `100% tests passed, 0 tests failed out of 4` | ✅ PASS (vérificateur indépendant) |
| 2026-11-13 | T2.2 ASan/UBSan | 4/4 tests passés | `100% tests passed, 0 tests failed out of 4` | ✅ PASS |
| 2026-11-13 | T2.3 booster inchangé | `identiques=20/20` | `identiques=20/20` | ✅ PASS |
| 2026-11-13 | T2.4 captures lumineuses | `7` | `7` | ✅ PASS |
| 2026-11-13 | T2.5 firmware | `build_exit=0`, `0`, `1` | `build_exit=0`, `0`, `1` | ✅ PASS |

## Journal de session

### Session 2026-11-12
- ✅ Fait : cadrage (style lumineux pour toute l'interface, booster en 2ᵉ thème), création de l'issue #5 et du plan
- 📋 Prochain : étape 1 ; plan #2 attend ce plan
- 🚧 Blocages : nom du produit à trancher
- ✅ Fait : logo dessiné et validé (`ui/assets/logo.svg`) ; étape 1 : contrat de direction lumineuse (brief impeccable, booster conservé en thème 2), PRODUCT.md (nom provisoire, logo, direction)
- 🔄 Ajustement opérateur : couleur de fond au choix dans les réglages (moteur à l'étape 2, sélecteur dans #3)
- 📋 Prochain : validation du contrat par l'opérateur, puis étape 2 (système de thèmes)

### Session 2026-11-13
- ✅ Fait : étape 2. Deux thèmes enregistrés (`UI_THEME_BOOSTER`, `UI_THEME_LUMINEUX`), `ui_set_theme(thème, fond)`, fond radial centre→bord, 6 teintes de fond avec bord dérivé, apparence lue en NVS au démarrage. Le booster rend les mêmes 20 captures qu'au plan #1.
- 🔄 État du thème lumineux : mêmes écrans que le booster avec les couleurs et le fond du contrat ; les sièges en verre, lueurs et icônes viennent aux étapes 3 et 4.
- 📋 Prochain : étape 3 (rendu « lueur », icônes)
- 🚧 À vérifier sur matériel : coût du dégradé radial plein écran et bandes en RGB565
