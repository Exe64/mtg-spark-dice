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
    - profiles/
    - main/
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
- [x] Étape 3 — Rendu « lueur » : générateur de halos/anneaux pré-calculés (script, images A8/ARGB), dégradés, boutons « verre », jeu d'icônes dessinées (L · reasoning → Opus)
- [x] Étape 4 — Écrans de partie dans le thème lumineux : Solo, Arène 1–4, moyeu, bande d'attente, pastille, marque de défaite (L · reasoning → Opus)
- [x] Étape 5 — Menus dans le thème lumineux : menu de partie, journal, fiche joueur, fin de partie (M · standard → Sonnet)
- [x] Étape 6 — Logo : intégration (accueil provisoire + écran de démarrage), nom du produit centralisé (S · standard → Sonnet)
- [x] Étape 7 — Design : captures des 2 thèmes, revue de finition, DESIGN.md (thème lumineux principal + booster), mesure du temps de rendu (M · standard → Sonnet)
- [x] Étape 8 — Retouches de validation (opérateur, 2026-11-13) : lueur du joueur actif à la place des rayons, accueil radial, écran « Nouvelle partie » (format puis joueurs), 10 formats (4 singleton, 6 construits en duel à 20 PV sans mention de commandant) (M · standard → Sonnet)
- [x] 🧪 Tests — Rédiger et exécuter la procédure de test (avant-dernière étape, obligatoire)
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
| Fond radial | Calculé une fois par teinte dans un tampon au format de l'écran, puis recopié ; tramé sur les écrans 16 bits. Le dégradé radial de LVGL, essayé à l'étape 2, est retiré | Mesure de l'étape 7 : le dégradé recalculé à chaque image coûtait 8 fois un fond uni. Le tramage vise les bandes en RGB565 ; il n'est compilé que pour le firmware et reste à voir sur matériel |
| Teinte du bord | 27 % de chaque composante de la teinte du centre | Redonne exactement le couple du contrat (#18214A → #060914) |
| Thème par défaut | Booster jusqu'à l'étape 4, puis Lumineux (`UI_THEME_DEFAULT`) | Les écrans lumineux n'existent pas avant l'étape 4 |
| Stockage de l'apparence | Clés NVS `theme` et `ground`, hors sauvegarde de partie ; lues au démarrage, écrites par le sélecteur du plan #3 | Étape 2 |
| Jetons de lueur, icônes, sièges | Ajoutés à `ui_theme_t` avec leur premier usage (étapes 3 et 4) | Pas de champ sans code qui le lit |
| Primitives de lumière | `ui/fx.c` : halo, anneau, rayon, icône, bouton verre ; masques A8 générés par `ui/assets/glow_gen.py`, teintés au dessin | Contrat : pas de flou temps réel ; un appel = un tracé de masque |
| Icônes | 8 icônes au trait de 40 px (réglages, joueurs, reprendre, type de jeu, annuler, tour, wifi, salon), dessinées par le script | Aucune source externe ; wifi et salon prêts pour les lots 2 et 3 |
| Deux styles de dessin | Un commutateur `light` dans le thème ; Solo, Arène et moyeu ont une variante lumineuse, le booster garde son code | Le booster reste tel quel ; un 3ᵉ style justifierait des fonctions par thème |
| Thème par défaut | Lumineux depuis l'étape 4 | Ses écrans de partie existent |
| Arc d'attente | Les angles négatifs sont ramenés dans 0–360° avant le tracé, dans les deux thèmes | Défaut du plan #1 : LVGL 9.2 ignore les angles négatifs, l'arc partait de 3 h et restait invisible le premier quart du délai. Seule retouche du booster (captures 09, 10a, 10) |
| Pastille d'annulation de l'Arène | Icône « annuler » + montant, sans le mot ANNULER | Lisible par les joueurs qui voient le moyeu à l'envers |
| Conversion du logo | `ui/assets/logo2a8.py` : rendu par GdkPixbuf + librsvg, deux masques A8 recadrés (237×195 démarrage, 72×60 accueil) | Le logo a des flous, masques et du texte que `svg2a8.py` ne sait pas tracer ; ces bibliothèques sont déjà sur la machine |
| Écran de démarrage | Logo + nom pendant 1,5 s (`UI_BOOT_MS`), dans les deux thèmes ; un tap ou un cran de molette le passe | Étape 6 |
| Accueil lumineux | Provisoire : mêmes lignes et zones de tap que l'accueil du plan #1, logo en haut, pastilles de verre | La refonte de l'accueil est dans le plan #2 |
| Nom du produit | `UI_PRODUCT_NAME` dans `ui/ui.h`, seule occurrence | PRODUCT.md : le nom vit dans une constante |
| Halos | Un masque calculé par rayon à sa taille d'affichage, 6 rayons gardés en mémoire | Mesure de l'étape 7 : redimensionner un masque unique à chaque image coûtait plusieurs fois un simple tracé |
| Captures de référence | `.impeccable/review/` = thème lumineux, `.impeccable/review/booster/` = booster | Étape 7 ; les procédures T2 à T6 ci-dessous citent l'ancien emplacement du booster |
| Revue de finition | Faite le 2026-11-13 contre le contrat : « à livrer après corrections ». Corrigé : moins de lumière sous les chiffres (blanc ≥ 7:1), rayons du joueur actif de part et d'autre du chiffre, libellé du siège sélectionné et « PARTIE TERMINÉE » en blanc, encre du siège vaincu à 60 %, CMDT dégagé de l'anneau, libellés de siège à 30 px, texte secondaire du moyeu à 24 px, jointures symétriques | Agent de revue impeccable ; 6 corrections sur 7 appliquées |
| Jointures de l'Arène | Secteurs rognés de 2° de chaque côté (coins de fond), pas une bande constante de 6 px | LVGL tronque les angles d'arc au degré : 1,5° donnait une jointure inclinée |
| Graine de concept | Non vérifiée : l'outil `impeccable` n'est pas installé ; le contrat indique « seed: none (unavailable) » | 7ᵉ point de la revue, sans effet visible |
| Temps de rendu | Rapport lumineux/booster sur PC : 1,7 à 2,6 (accueil, menu, démarrage, Arène), 5 en Solo. Pas d'estimation en ms pour l'ESP32-S3 | Seul le rapport se transpose ; la valeur absolue demande le boîtier |
| Siège sélectionné | Verre à peine plus clair (46 contre 36 sur 255) et bande de 5 px en pleine teinte sur le bord du secteur | Un verre plus clair faisait passer le chiffre blanc sous 7:1 sur le siège or ; la bande signale la sélection sans toucher au contraste |
| Halos sous les titres | Fiche joueur et fin de partie à 30 %, pastilles de l'accueil à 20 % | Signalé par l'agent de documentation : titres blancs estimés entre 2,8:1 et 5,3:1 avant correction. Les contrastes sur halo sont estimés par calcul, pas mesurés |
| Fin de la pastille d'annulation | Redessin complet en thème lumineux | Son halo dépasse la zone du moyeu ; un redessin partiel l'aurait laissé à l'écran (invisible sur PC, où chaque capture redessine tout) |
| Joueur actif (retouche) | Un liseré fin (2 px) de la couleur du siège sur les trois côtés de sa zone ; le siège sélectionné garde sa bande épaisse sur le bord. Ni rayons ni lueur blanche | Opérateur 2026-11-13 : les rayons puis la lueur blanche écartés, liseré proposé par l'opérateur |
| Accueil (retouche) | Radial, inspiré de la référence : Nouvelle partie au centre, Reprendre « T n \| J n » en bas, Réglages / Héberger / Rejoindre affichés mais atténués et inactifs ; pas de pastille Type de jeu | Opérateur 2026-11-13 ; les trois boutons inactifs reçoivent leur destination dans #2 et les lots 2/3 |
| Nouvelle partie (retouche) | Après « Nouvelle partie » : le format, puis le nombre de joueurs. L'étape « jeu » est sautée tant qu'il n'existe qu'un jeu (Magic) | Opérateur 2026-11-13 (jeu puis format) ; un choix à une seule entrée coûterait un tap par partie |
| Formats (retouche) | 10 profils JSON : Commander (40 PV, 2–4), Commander 1v1 (30 PV), Commander 2HG (60 PV par équipe, poison 15, un siège = une équipe), Duel Commander (20 PV, sans dégâts de commandant), puis Standard, Pioneer, Modern, Legacy, Pauper, Peasant (20 PV, duel, poison 10, aucune mention de commandant) | Tableaux fournis par l'opérateur 2026-11-13 |
| SOLO | Reste proposé à l'étape joueurs de chaque format, y compris en duel (afficher ma seule vie) | Fonction existante du plan #1 ; un duel reste un duel |
| Commander à 2 | Autorisé (2–4) alors que le tableau dit 3 à 5+ | La partie à deux à 40 PV se joue beaucoup ; se change dans `commander.json` (`players_min`) ; 5 et 6 joueurs avec #6 |
| Accueil, 2ᵉ retouche | Nom « DICE SPARK » en haut à la place du logo ; le logo passe dans le bouton central, libellé « START » ; Réglages = icône curseurs à la place de l'engrenage ; la constante du nom du produit devient « DICE SPARK » (écran de démarrage compris) | Opérateur 2026-11-13 ; majuscules gardées comme tout le reste de l'interface |

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

# --- T3 Rendu « lueur » et icônes (étape 3)
# T3.1 le générateur redonne exactement le fichier commité
python3 /srv/dev/mtg-dial/ui/assets/glow_gen.py /tmp/claude-1000/fx_a8.c && cmp /tmp/claude-1000/fx_a8.c /srv/dev/mtg-dial/ui/fx_a8.c && echo generateur_ok
# T3.2 tests PC (5 tests, dont `fx`) et planche d'essai
cmake -S /srv/dev/mtg-dial/host -B /srv/dev/mtg-dial/host/build -G Ninja >/dev/null && cmake --build /srv/dev/mtg-dial/host/build | tail -1 && ctest --test-dir /srv/dev/mtg-dial/host/build | tail -3
ls /srv/dev/mtg-dial/host/build/shots-out/fx-specimen.png
# T3.3 mêmes tests sous ASan/UBSan
cmake -S /srv/dev/mtg-dial/host -B /srv/dev/mtg-dial/host/build-asan -G Ninja -DCMAKE_C_FLAGS="-fsanitize=address,undefined -fno-sanitize-recover=all" >/dev/null && cmake --build /srv/dev/mtg-dial/host/build-asan | tail -1 && ctest --test-dir /srv/dev/mtg-dial/host/build-asan | tail -3
# T3.4 booster inchangé
bash -c 'n=0; for f in /srv/dev/mtg-dial/.impeccable/review/[0-9]*.png; do cmp -s $f /srv/dev/mtg-dial/host/build/shots-out/$(basename $f) && n=$((n+1)); done; echo identiques=$n/20'
# T3.5 firmware
bash -c '. ~/esp/esp-idf/export.sh >/dev/null 2>&1 && cd /srv/dev/mtg-dial && idf.py build >/tmp/claude-1000/t3-5.log 2>&1; echo build_exit=$?'
grep -c "warning:" /tmp/claude-1000/t3-5.log

# --- T4 Écrans de partie lumineux (étape 4)
# T4.1 tests PC : 6 tests, les parcours d'écrans tournent dans les deux thèmes (ui_flows = lumineux, ui_flows_booster)
cmake -S /srv/dev/mtg-dial/host -B /srv/dev/mtg-dial/host/build -G Ninja >/dev/null && cmake --build /srv/dev/mtg-dial/host/build | tail -1 && ctest --test-dir /srv/dev/mtg-dial/host/build | tail -3
# T4.2 mêmes tests sous ASan/UBSan
cmake -S /srv/dev/mtg-dial/host -B /srv/dev/mtg-dial/host/build-asan -G Ninja -DCMAKE_C_FLAGS="-fsanitize=address,undefined -fno-sanitize-recover=all" >/dev/null && cmake --build /srv/dev/mtg-dial/host/build-asan | tail -1 && ctest --test-dir /srv/dev/mtg-dial/host/build-asan | tail -3
# T4.3 booster : ses 20 captures sont identiques à la référence
bash -c 'n=0; for f in /srv/dev/mtg-dial/.impeccable/review/[0-9]*.png; do cmp -s $f /srv/dev/mtg-dial/host/build/shots-out/booster/$(basename $f) && n=$((n+1)); done; echo identiques=$n/20'
# T4.4 le thème par défaut est le lumineux : ses captures de partie diffèrent toutes de celles du booster
bash -c 'n=0; for f in 03-solo 04-solo-delta-en-attente 05-solo-valide 08-arene-4 08b-arene-pastille-annuler 08c-arene-tour-j2 09-arene-delta-en-attente 10a-arene-attaque 10-arene-degats-commandant 13-arene-joueur-vaincu 15-arene-2-standard 16-arene-3; do cmp -s /srv/dev/mtg-dial/host/build/shots-out/$f.png /srv/dev/mtg-dial/host/build/shots-out/booster/$f.png || n=$((n+1)); done; echo differentes=$n/12'
# T4.5 firmware
bash -c '. ~/esp/esp-idf/export.sh >/dev/null 2>&1 && cd /srv/dev/mtg-dial && idf.py build >/tmp/claude-1000/t4-5.log 2>&1; echo build_exit=$?'
grep -c "warning:" /tmp/claude-1000/t4-5.log

# --- T5 Menus lumineux (étape 5)
# T5.1 tests PC et ASan/UBSan (les parcours passent par le menu, le journal, la fiche joueur et la fin de partie)
cmake --build /srv/dev/mtg-dial/host/build | tail -1 && ctest --test-dir /srv/dev/mtg-dial/host/build | tail -3
cmake --build /srv/dev/mtg-dial/host/build-asan | tail -1 && ctest --test-dir /srv/dev/mtg-dial/host/build-asan | tail -3
# T5.2 booster identique à la référence
bash -c 'n=0; for f in /srv/dev/mtg-dial/.impeccable/review/[0-9]*.png; do cmp -s $f /srv/dev/mtg-dial/host/build/shots-out/booster/$(basename $f) && n=$((n+1)); done; echo identiques=$n/20'
# T5.3 les 4 écrans de menu du thème lumineux diffèrent de ceux du booster
bash -c 'n=0; for f in 06-menu 11-fiche-joueur 12-journal 14-fin-de-partie; do cmp -s /srv/dev/mtg-dial/host/build/shots-out/$f.png /srv/dev/mtg-dial/host/build/shots-out/booster/$f.png || n=$((n+1)); done; echo differentes=$n/4'
# T5.4 firmware
bash -c '. ~/esp/esp-idf/export.sh >/dev/null 2>&1 && cd /srv/dev/mtg-dial && idf.py build >/tmp/claude-1000/t5-4.log 2>&1; echo build_exit=$?'
grep -c "warning:" /tmp/claude-1000/t5-4.log

# --- T6 Logo (étape 6)
# T6.1 le convertisseur redonne exactement le fichier commité (demande python3-gobject, gdk-pixbuf2, librsvg2)
python3 /srv/dev/mtg-dial/ui/assets/logo2a8.py /tmp/claude-1000/logo_a8.c && cmp /tmp/claude-1000/logo_a8.c /srv/dev/mtg-dial/ui/logo_a8.c && echo logo_ok
# T6.2 tests PC et ASan/UBSan (écran de démarrage : logo et nom dessinés, passage à l'accueil après UI_BOOT_MS)
cmake -S /srv/dev/mtg-dial/host -B /srv/dev/mtg-dial/host/build -G Ninja >/dev/null && cmake --build /srv/dev/mtg-dial/host/build | tail -1 && ctest --test-dir /srv/dev/mtg-dial/host/build | tail -3
cmake -S /srv/dev/mtg-dial/host -B /srv/dev/mtg-dial/host/build-asan -G Ninja -DCMAKE_C_FLAGS="-fsanitize=address,undefined -fno-sanitize-recover=all" >/dev/null && cmake --build /srv/dev/mtg-dial/host/build-asan | tail -1 && ctest --test-dir /srv/dev/mtg-dial/host/build-asan | tail -3
ls /srv/dev/mtg-dial/host/build/shots-out/00-demarrage.png /srv/dev/mtg-dial/host/build/shots-out/booster/00-demarrage.png | wc -l
# T6.3 booster identique à la référence (son accueil n'a pas changé)
bash -c 'n=0; for f in /srv/dev/mtg-dial/.impeccable/review/[0-9]*.png; do cmp -s $f /srv/dev/mtg-dial/host/build/shots-out/booster/$(basename $f) && n=$((n+1)); done; echo identiques=$n/20'
# T6.4 le nom du produit n'est écrit qu'une fois dans le code compilé
grep -rn '"SPARKDICE"\|"SparkDice"' --include=*.c --include=*.h /srv/dev/mtg-dial/ui /srv/dev/mtg-dial/core /srv/dev/mtg-dial/profiles /srv/dev/mtg-dial/main | wc -l
# T6.5 firmware
bash -c '. ~/esp/esp-idf/export.sh >/dev/null 2>&1 && cd /srv/dev/mtg-dial && idf.py build >/tmp/claude-1000/t6-5.log 2>&1; echo build_exit=$?'
grep -c "warning:" /tmp/claude-1000/t6-5.log

# --- T7 Design (étape 7) — à partir d'ici la référence du booster est .impeccable/review/booster/
# T7.1 tests PC et ASan/UBSan
cmake -S /srv/dev/mtg-dial/host -B /srv/dev/mtg-dial/host/build -G Ninja >/dev/null && cmake --build /srv/dev/mtg-dial/host/build | tail -1 && ctest --test-dir /srv/dev/mtg-dial/host/build | tail -3
cmake -S /srv/dev/mtg-dial/host -B /srv/dev/mtg-dial/host/build-asan -G Ninja -DCMAKE_C_FLAGS="-fsanitize=address,undefined -fno-sanitize-recover=all" >/dev/null && cmake --build /srv/dev/mtg-dial/host/build-asan | tail -1 && ctest --test-dir /srv/dev/mtg-dial/host/build-asan | tail -3
# T7.2 les captures de référence des deux thèmes sont celles que produit le code
bash -c 'n=0; for f in /srv/dev/mtg-dial/.impeccable/review/[0-9]*.png /srv/dev/mtg-dial/.impeccable/review/fx-specimen.png; do cmp -s $f /srv/dev/mtg-dial/host/build/shots-out/$(basename $f) && n=$((n+1)); done; echo lumineux=$n/28'
bash -c 'n=0; for f in /srv/dev/mtg-dial/.impeccable/review/booster/[0-9]*.png; do cmp -s $f /srv/dev/mtg-dial/host/build/shots-out/booster/$(basename $f) && n=$((n+1)); done; echo booster=$n/21'
# T7.3 le booster n'a pas bougé depuis l'étape 4 (20 captures du plan #1, 3 retouchées à l'étape 4)
bash -c 'cd /srv/dev/mtg-dial; n=0; for f in $(git ls-tree --name-only 1859bf7 .impeccable/review/ | grep "/[0-9]"); do git show 1859bf7:$f | cmp -s - .impeccable/review/booster/$(basename $f) && n=$((n+1)); done; echo inchanges=$n/20'
# T7.4 les générateurs redonnent les fichiers commités
python3 /srv/dev/mtg-dial/ui/assets/glow_gen.py /tmp/claude-1000/fx_a8.c && cmp /tmp/claude-1000/fx_a8.c /srv/dev/mtg-dial/ui/fx_a8.c && echo generateur_ok
# T7.5 le dégradé radial de LVGL n'est plus utilisé
grep -c "COMPLEX_GRADIENTS\|lv_grad_radial" /srv/dev/mtg-dial/ui/ui.c /srv/dev/mtg-dial/host/lv_conf.h /srv/dev/mtg-dial/sdkconfig.defaults /srv/dev/mtg-dial/host/CMakeLists.txt
# T7.6 mesure du temps de rendu (indicatif : dépend de la machine)
/srv/dev/mtg-dial/host/build/bench /srv/dev/mtg-dial/profiles/data
# T7.7 DESIGN.md décrit les deux thèmes
grep -c -i "lumineux" /srv/dev/mtg-dial/DESIGN.md; grep -c -i "booster" /srv/dev/mtg-dial/DESIGN.md
# T7.8 firmware (T8.5 réutilise ces deux commandes)
bash -c '. ~/esp/esp-idf/export.sh >/dev/null 2>&1 && cd /srv/dev/mtg-dial && idf.py build >/tmp/claude-1000/t7-8.log 2>&1; echo build_exit=$?'
grep -c "warning:" /tmp/claude-1000/t7-8.log

# --- T8 Retouches de validation (étape 8) — remplace T7.2 (nombres de captures) et T7.3 (l'accueil du booster a changé)
# T8.1 tests PC et ASan/UBSan : les deux commandes de T7.1
# T8.2 les captures de référence des deux thèmes sont celles que produit le code
bash -c 'n=0; for f in /srv/dev/mtg-dial/.impeccable/review/[0-9]*.png /srv/dev/mtg-dial/.impeccable/review/fx-specimen.png; do cmp -s $f /srv/dev/mtg-dial/host/build/shots-out/$(basename $f) && n=$((n+1)); done; echo lumineux=$n/33'
bash -c 'n=0; for f in /srv/dev/mtg-dial/.impeccable/review/booster/[0-9]*.png; do cmp -s $f /srv/dev/mtg-dial/host/build/shots-out/booster/$(basename $f) && n=$((n+1)); done; echo booster=$n/26'
# T8.3 formats : 10 profils ; les 6 construits sont des duels à 20 PV, poison 10, sans dégâts de commandant
python3 -c "
import json,glob
P=[json.load(open(f)) for f in sorted(glob.glob('/srv/dev/mtg-dial/profiles/data/*.json'))]
c=[p for p in P if p['family']=='Construit']
ok=all((p['start_life'],p['players_min'],p['players_max'],p['commander_damage'],p['poison'])==(20,2,2,0,10) for p in c)
d={p['name']:(p['start_life'],p['players_max'],p['commander_damage'],p['poison']) for p in P}
print(len(P),len(c),ok,d['Commander 1v1'],d['Commander 2HG'],d['Duel Commander'],d['Commander'])"
# T8.4 plus de rayons à l'écran, plus de « CMDT — » pour un format sans dégâts de commandant
grep -c "fx_ray" /srv/dev/mtg-dial/ui/ui.c; grep -rc "S_OFF\|S_PROFILE_SUB_FMT" /srv/dev/mtg-dial/ui/ui.c /srv/dev/mtg-dial/ui/strings_fr.h
# T8.5 firmware : les deux commandes de T7.8
# T8.6 contrôle visuel (ouvrir les images de /srv/dev/mtg-dial/.impeccable/review/)
#   01-accueil.png, 07-accueil-reprendre.png, 02-nouvelle-partie-format.png, 02b-nouvelle-partie-joueurs.png,
#   15b-format-standard.png, 15c-standard-solo.png, 08c-arene-tour-j2.png, 15-arene-2-standard.png
```

**Résultats attendus :**
- T2.1 et T2.2 : `100% tests passed, 0 tests failed out of 4`. Le test `ui_flows` vérifie pour chacun des 6 fonds : contraste de l'encre ≥ 7:1, pixel du centre = teinte choisie, pixels du bord (haut et gauche) = teinte dérivée ; `#18214A` donne le bord `#060914` ; une valeur de thème hors bornes retombe sur le thème par défaut.
- T2.3 : `identiques=20/20`.
- T2.4 : `7`.
- T2.5 : `build_exit=0`, `0` avertissement, `1` ligne de configuration.
- T3.1 : `generateur_ok`.
- T3.2 et T3.3 : `100% tests passed, 0 tests failed out of 5`, et le fichier `fx-specimen.png` existe. Le test `fx` vérifie : halo le plus lumineux au centre et nul au-delà de son rayon ; anneau lumineux sur son cercle, sombre au centre ; rayons partant dans la direction demandée, de la longueur demandée ; seul le bouton en focus porte un halo ; chaque icône a de l'encre et une marge libre.
- T3.4 : `identiques=20/20`.
- T3.5 : `build_exit=0`, `0` avertissement.
- T4.1 et T4.2 : `100% tests passed, 0 tests failed out of 6`. `ui_flows` vérifie en plus qu'à mi-délai l'arc de lumière du Solo va de midi à 6 h.
- T4.3 : `identiques=20/20`. La référence `.impeccable/review/` a été mise à jour pour 3 captures (09, 10a, 10) : voir la décision « Arc d'attente ».
- T4.4 : `differentes=12/12`.
- T4.5 : `build_exit=0`, `0` avertissement.
- T5.1 : deux fois `100% tests passed, 0 tests failed out of 6`.
- T5.2 : `identiques=20/20`.
- T5.3 : `differentes=4/4`.
- T5.4 : `build_exit=0`, `0` avertissement.
- T6.1 : `logo_ok`.
- T6.2 : deux fois `100% tests passed, 0 tests failed out of 6`, puis `2`.
- T6.3 : `identiques=20/20`.
- T6.4 : `1`.
- T6.5 : `build_exit=0`, `0` avertissement.
- T7.1 : deux fois `100% tests passed, 0 tests failed out of 6`.
- T7.2 : `lumineux=28/28` (27 écrans et la planche des primitives) et `booster=21/21`.
- T7.3 : `inchanges=20/20` (les 20 captures du booster sont, à l'octet, celles du commit de l'étape 4).
- T7.4 : `generateur_ok`.
- T7.5 : `0` pour chacun des 4 fichiers.
- T7.6 : un tableau de 7 écrans ; rapport lumineux/booster attendu sous 3 hors Solo, sous 6 en Solo (mesuré le 2026-11-13 : 1,7 à 2,6, et 5 en Solo).
- T7.7 : deux nombres supérieurs à 0.
- T7.8 : `build_exit=0`, `0` avertissement.
- T8.1 : deux fois `100% tests passed, 0 tests failed out of 6`. Les parcours (`host/shots.c`) vérifient en plus : les boutons atténués et la molette ne font rien sur l'accueil ; Retour ramène du nombre de joueurs au format puis à l'accueil ; Reprendre rend la partie Solo à 40 PV ; Standard ne propose que SOLO et 2 et lance un duel à 20 PV sans dégâts de commandant.
- T8.2 : `lumineux=33/33` (32 écrans et la planche des primitives) et `booster=26/26`.
- T8.3 : `10 6 True (30, 2, 21, 10) (60, 2, 21, 15) (20, 2, 0, 10) (40, 4, 21, 10)`.
- T8.4 : `0`, puis `0` pour chacun des 2 fichiers.
- T8.5 : `build_exit=0`, `0` avertissement.
- T8.6 : accueil radial (Nouvelle partie au centre, Réglages / Héberger / Rejoindre atténués, Reprendre « T1 | J1 » sur 07) ; « SINGLETON » puis « JOUEURS » au-dessus de COMMANDER ; STANDARD affiche « 20 PV · POISON 10 » sans CMDT et deux pastilles J1 J2 ; plus aucun trait blanc entre le moyeu et le bord sur 08c et 15, une lueur du côté du joueur actif.

### Résultats joués et vérifiés

| Date | Test | Attendu | Observé | Verdict |
|------|------|---------|---------|---------|
| 2026-11-13 | T2.1 tests PC | 4/4 tests passés | `100% tests passed, 0 tests failed out of 4` | ✅ PASS (vérificateur indépendant) |
| 2026-11-13 | T2.2 ASan/UBSan | 4/4 tests passés | `100% tests passed, 0 tests failed out of 4` | ✅ PASS |
| 2026-11-13 | T2.3 booster inchangé | `identiques=20/20` | `identiques=20/20` | ✅ PASS |
| 2026-11-13 | T2.4 captures lumineuses | `7` | `7` | ✅ PASS |
| 2026-11-13 | T2.5 firmware | `build_exit=0`, `0`, `1` | `build_exit=0`, `0`, `1` | ✅ PASS |
| 2026-11-13 | T3.1 générateur | `generateur_ok` | `generateur_ok` | ✅ PASS (vérificateur indépendant) |
| 2026-11-13 | T3.2 tests PC + planche | 5/5, `fx-specimen.png` régénéré | 5/5, fichier écrit 2 s avant le contrôle | ✅ PASS |
| 2026-11-13 | T3.3 ASan/UBSan | 5/5 | 5/5 | ✅ PASS |
| 2026-11-13 | T3.4 booster inchangé | `identiques=20/20` | `identiques=20/20` | ✅ PASS |
| 2026-11-13 | T3.5 firmware | `build_exit=0`, `0` | `build_exit=0`, `0` | ✅ PASS |
| 2026-11-13 | T4.1 tests PC, deux thèmes | 6/6, captures régénérées | 6/6, captures écrites 1 à 4 s avant le contrôle | ✅ PASS (vérificateur indépendant) |
| 2026-11-13 | T4.2 ASan/UBSan | 6/6 | 6/6 | ✅ PASS |
| 2026-11-13 | T4.3 booster = référence | `identiques=20/20` | `identiques=20/20` | ✅ PASS |
| 2026-11-13 | T4.4 lumineux ≠ booster | `differentes=12/12` | `differentes=12/12` | ✅ PASS |
| 2026-11-13 | T4.5 firmware | `build_exit=0`, `0` | `build_exit=0`, `0` | ✅ PASS |
| 2026-11-13 | T5.1 tests PC + ASan/UBSan | 6/6 deux fois, captures régénérées | 6/6 deux fois, captures écrites 1 à 4 s avant le contrôle | ✅ PASS (vérificateur indépendant) |
| 2026-11-13 | T5.2 booster = référence | `identiques=20/20` | `identiques=20/20` | ✅ PASS |
| 2026-11-13 | T5.3 menus lumineux ≠ booster | `differentes=4/4` | `differentes=4/4` | ✅ PASS |
| 2026-11-13 | T5.4 firmware | `build_exit=0`, `0` | `build_exit=0`, `0` | ✅ PASS |
| 2026-11-13 | T6.1 convertisseur du logo | `logo_ok` | `logo_ok` | ✅ PASS (vérificateur indépendant) |
| 2026-11-13 | T6.2 tests PC + ASan/UBSan | 6/6 deux fois, `2`, capture régénérée | 6/6 deux fois, `2`, capture écrite 19 s avant le contrôle | ✅ PASS |
| 2026-11-13 | T6.3 booster = référence | `identiques=20/20` | `identiques=20/20` | ✅ PASS |
| 2026-11-13 | T6.4 nom du produit | `1` | `1` | ✅ PASS |
| 2026-11-13 | T6.5 firmware | `build_exit=0`, `0` | `build_exit=0`, `0` | ✅ PASS |
| 2026-11-13 | T7.1 tests PC + ASan/UBSan | 6/6 deux fois, captures régénérées | 6/6 deux fois, capture écrite 17 s avant le contrôle | ✅ PASS (vérificateur indépendant) |
| 2026-11-13 | T7.2 captures de référence | `lumineux=28/28`, `booster=21/21` | `lumineux=28/28`, `booster=21/21` | ✅ PASS |
| 2026-11-13 | T7.3 booster inchangé depuis l'étape 4 | `inchanges=20/20` | `inchanges=20/20` | ✅ PASS |
| 2026-11-13 | T7.4 générateur | `generateur_ok` | `generateur_ok` | ✅ PASS |
| 2026-11-13 | T7.5 plus de dégradé radial LVGL | `0` ×4 | `0` ×4 | ✅ PASS |
| 2026-11-13 | T7.6 temps de rendu | rapport < 3 hors Solo, < 6,5 en Solo | 1,9 à 2,6 ; Solo 4,4 et 4,5 | ✅ PASS |
| 2026-11-13 | T7.7 DESIGN.md | deux nombres > 0, `json_ok` | `27`, `36`, `json_ok` | ✅ PASS |
| 2026-11-13 | T7.8 firmware | `build_exit=0`, `0` | `build_exit=0`, `0` | ✅ PASS |
| 2026-11-13 | 🧪 Tests finaux : T7.1 à T7.8 sur le code final (ils reprennent tous les contrôles des étapes 2 à 6) + T6.1 et T6.4 rejoués | tout PASS, `logo_ok`, `1` | T7 : PASS (vérificateur indépendant) ; `logo_ok`, `1` (rejoués par l'exécuteur, même contexte) | ✅ PASS |
| 2026-11-13 | T8.1 tests PC + ASan/UBSan | 6/6 deux fois | 6/6 deux fois, captures régénérées avant T8.2 | ✅ PASS (vérificateur indépendant) |
| 2026-11-13 | T8.2 captures de référence | `lumineux=33/33`, `booster=26/26` | `lumineux=33/33`, `booster=26/26` | ✅ PASS |
| 2026-11-13 | T8.3 formats | `10 6 True (30, 2, 21, 10) (60, 2, 21, 15) (20, 2, 0, 10) (40, 4, 21, 10)` | identique | ✅ PASS |
| 2026-11-13 | T8.4 plus de rayons ni de « CMDT — » | `0`, `0` ×2 | `0`, `0` ×2 | ✅ PASS |
| 2026-11-13 | T8.5 firmware | `build_exit=0`, `0` | `build_exit=0`, `0` | ✅ PASS |
| 2026-11-13 | T8.6 contrôle visuel (8 images) | voir T8.6 | les 8 images conformes ; lueur du côté du joueur actif mesurée sur 08c et 15 | ✅ PASS |
| 2026-11-13 | 2ᵉ retouche de l'accueil : T8.1, T8.2, T8.4, T8.5 et T7.4 rejoués | 6/6 deux fois, `33/33`, `26/26`, `0`, `build_exit=0`, `0`, `generateur_ok` | identique | ✅ PASS (rejoués par l'exécuteur, même contexte ; pas de vérificateur indépendant pour cette retouche d'apparence) |
| 2026-11-13 | 3ᵉ retouche (liseré du joueur actif) : T8.1, T8.2, T8.4, T8.5 rejoués | 6/6 deux fois, `33/33`, `26/26`, `0`, `build_exit=0`, `0` | identique | ✅ PASS (rejoués par l'exécuteur, même contexte) |

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
- ✅ Fait : étape 3. Primitives de lumière et icônes (`ui/fx.c`, `ui/fx_a8.c`), planche d'essai `fx-specimen.png`. Pas encore utilisées par les écrans : c'est l'étape 4.
- 📋 Prochain : étape 4 (écrans de partie dans le thème lumineux, qui devient le thème par défaut)
- ✅ Fait : étape 4. Solo, Arène 1–4, moyeu, attente, pastille et marque de défaite dans le thème lumineux, qui devient le thème par défaut. Les parcours de test tournent dans les deux thèmes (captures du booster dans `shots-out/booster/`).
- 🔧 Corrigé en passant : l'arc d'attente partait de 3 h au lieu de midi (les deux thèmes).
- 📋 Prochain : étape 5 (menu de partie, journal, fiche joueur, fin de partie)
- ✅ Fait : étape 5. Menu de partie, journal, fiche joueur et fin de partie dans le thème lumineux (pastilles de verre, halo du siège, arc d'attente sur l'anneau pour le poison). Le texte des lignes de menu et de journal est partagé par les deux thèmes.
- 📋 Prochain : étape 6 (logo : accueil provisoire et écran de démarrage, nom du produit centralisé)
- ✅ Fait : étape 6. Écran de démarrage avec le logo et le nom, accueil lumineux provisoire avec le logo, nom du produit en une constante.
- 📋 Prochain : étape 7 (captures des deux thèmes, revue de finition, DESIGN.md, mesure du temps de rendu)
- ✅ Fait : étape 7. Mesure du rendu (`host/bench.c`) puis fond et halos calculés une fois au lieu de chaque image ; captures de référence rangées par thème ; revue de finition et ses corrections ; DESIGN.md réécrit pour les deux thèmes.
- 📋 Prochain : 🧪 Tests finaux, puis ✅ Validation par l'opérateur (regarder `.impeccable/review/contact-sheet.png`)
- 🚧 À vérifier sur matériel : tramage du fond en RGB565 (code compilé pour le firmware seulement), coût réel du rendu
- ✅ Fait : 🧪 Tests finaux (campagne T7 sur le code final, vérificateur indépendant : PASS).
- 📋 Prochain : ✅ Validation par l'opérateur, sur la planche `.impeccable/review/contact-sheet.png`, puis clôture (plan vers `done/`, roadmap, issue #5)
- 📝 Retouches demandées par l'opérateur à la validation → étape 8 : lueur du joueur actif à la place des rayons, accueil radial, écran « Nouvelle partie », 10 formats. Campagne T8 : PASS (vérificateur indépendant).
- 📋 Prochain : ✅ Validation par l'opérateur sur `.impeccable/review/contact-sheet.png`, puis clôture
- 📝 2ᵉ retouche de l'accueil (opérateur) : nom en haut, logo + START au centre, icône curseurs pour les réglages.
- 📝 3ᵉ retouche (opérateur) : le joueur actif est désigné par un liseré sur les trois côtés de sa zone.
