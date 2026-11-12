---
plan:
  id: '1'
  name: 1-mvp-solo-arene.md
  link: https://github.com/Exe64/mtg-spark-dice/blob/main/doc/roadmap/1-mvp-solo-arene.md
status: active
date: 2026-11-12
description: >
  MVP SparkDice : firmware jouable sur un seul cadran (Solo + Arène 1–4 joueurs),
  logique testée sur PC, écrans vérifiés en captures, build ESP32-S3 vert.
priority: high
complexity: L
intent: null
scope:
  modules:
    - core/
    - ui/
    - main/
    - profiles/
    - host/
issue:
  id: 1
  url: https://github.com/Exe64/mtg-spark-dice/issues/1
---

# [🎲] Plan #1 - MVP Solo + Arène locale

## Objectif

Livrer un premier firmware SparkDice (compteur de vie MTG inspiré du Soul Dial, identité originale)
sur Waveshare ESP32-S3-Knob-Touch-LCD-1.8B, jouable sur un seul cadran. Le matériel n'étant pas
encore reçu, la logique est testée sur PC et l'UI vérifiée par captures headless.

## Périmètre

### Inclus
- Accueil, Solo (PV géant + delta), Arène radiale 1–4 joueurs
- Molette : delta accumulé, validé après 800 ms d'inactivité
- Tactile : tap = sélection joueur, glisser A→B = dégâts de commandant, appui long = menus
- Haptique DRV2605L (crans, validation, défaite)
- Undo (1 entrée = 1 delta validé), journal d'actions, fin de partie
- Reprise de partie : sauvegarde NVS à chaque delta validé
- Dégâts de commandant (létal 21), poison (létal 10)
- Profils JSON `commander.json` (40 PV) et `standard.json` (20 PV) en LittleFS
- Chaînes UI FR centralisées dans un seul fichier
- Table de partitions 16 Mo (ota_0/ota_1/storage) de la spec

### Hors scope
- Radio (Wi-Fi, ESP-NOW), OTA, API REST / page web → lots 2 et 3
- Mode D&D, 5–6 joueurs → lot 4
- App iOS/Android → lot 5
- Veille écran, télémétrie batterie, assistant de configuration complet → lot 2
- Validation sur matériel réel (fera l'objet d'un plan dédié à réception)

## Étapes

- [x] Étape 1 — Socle (ESP-IDF v5.4.2 installé par l'opérateur) : git init + remote, ESP-IDF v5.x dans `~/esp`, squelette projet (partitions 16 Mo, PSRAM octale), CMake host pour les tests (S · standard → Sonnet)
- [x] Étape 2 — Cœur de jeu C pur (`core/`) : état joueurs, delta 800 ms (horloge injectable), Undo, défaites, sérialisation NVS + tests host (M · standard → Sonnet)
- [x] Étape 3 — Profils JSON (cJSON) : schéma, `commander.json` / `standard.json`, chargement + validation + tests host (S · standard → Sonnet)
- [x] Étape 4 — Rendu LVGL headless sur PC : snapshot → PNG 360×360 (lodepng intégré à LVGL 9), script de captures (M · standard → Sonnet)
- [x] Étape 5 — UI : Accueil, Solo, Arène 1–4, Menu de partie, Fiche joueur, chaînes FR ; direction visuelle via skill impeccable (L · reasoning → Opus)
- [ ] Étape 6 (🔄 code écrit, build bloqué : `idf.py reconfigure` à lancer par l'opérateur) — HAL ESP32-S3 : ST77916 QSPI, CST816, encodeur, DRV2605, NVS, LittleFS ; pinout démo Waveshare ; compilé, non validé matériel (L · reasoning → Opus)
- [ ] 🧪 Tests — Rédiger et exécuter la procédure de test (avant-dernière étape, obligatoire)
- [ ] ✅ Validation — Vérifier les résultats des tests et clôturer (dernière étape, obligatoire)

## Décisions techniques

| Décision | Choix retenu | Justification |
|----------|-------------|---------------|
| Nom / identité | SparkDice, UI originale | Aucune reprise de marque/visuels Soul Dial |
| Socle | ESP-IDF v5.x + LVGL 9 | Drivers esp_lcd ST77916/CST816, OTA/rollback natifs pour lot 2 |
| Stratégie de test sans matériel | Logique C pur testée host + captures PNG headless | Machine headless (Rocky 10), pas de SDL ; même code core sur cible |
| Undo | 1 entrée par delta validé | Correspond à l'action perçue par le joueur |
| Persistance | NVS à chaque delta validé | Quelques centaines d'écritures/partie, wear leveling NVS suffisant |
| Profils | JSON en LittleFS dès le MVP | Prépare la gestion des profils via API/app (lot 2, lot 5) |
| Gestion future via app | API REST/JSON documentée (lot 2), page web d'abord, app native ensuite | Une seule API pour web + iOS/Android ; BLE possible plus tard |
| Ordre des lots | 1 MVP → 2 Wi-Fi/API/OTA → 3 ESP-NOW → 4 D&D/5–6 joueurs → 5 app native | OTA utile à tous les lots ; radio manager avant ESP-NOW ; ESP-NOW exige 2 boîtiers |
| Arène | 1–4 joueurs au MVP | Lisibilité des secteurs sur 1,8" |
| Langue | FR, chaînes centralisées | i18n ultérieure sans refonte |
| Direction visuelle | « Emballage de booster » (impeccable, contrat dans `.impeccable/surfaces/ui-ui-c.md`) | Choisie par l'opérateur ; un seul thème au MVP, structure `ui_theme_t`, sélecteur au backlog |
| Polices | Barlow Condensed ExtraBold/SemiBold (OFL) via tiny_ttf, table GPOS masquée | Bug LVGL 9.2.2 : lecture hors police dans le crénage GPOS (ASan) |
| Orientation des chiffres | Le cadran fait face à son propriétaire (pas de rotation) | `lv_draw_label` 9.2 sans rotation ; à confirmer par l'opérateur (voir journal) |
| Brochage | `main/board.h`, d'après des portages communautaires de la démo Waveshare | Wiki inaccessible (403) ; à vérifier à réception |

## Tests

> Section obligatoire renseignée aux étapes de tests (mode `autonomous`).

### Prérequis pour les tests

```bash
# Rocky Linux 10, depuis la racine du projet
cd /srv/dev/mtg-dial
# outils host (installés le 2026-11-12) : gcc 14, cmake, ninja-build (dépôt CRB), zlib-ng-compat-devel libasan libubsan
sudo dnf install -y --enablerepo=crb cmake ninja-build zlib-ng-compat-devel libasan libubsan
# firmware : ESP-IDF v5.4.2 (installé par l'opérateur le 2026-11-12)
mkdir -p ~/esp && cd ~/esp && git clone -b v5.4.2 --recursive --shallow-submodules --depth 1 https://github.com/espressif/esp-idf.git && ./esp-idf/install.sh esp32s3
# dépendances host vendorées (non commitées, host/third_party/ est dans .gitignore)
git clone --depth 1 -b v1.7.18 https://github.com/DaveGamble/cJSON.git /srv/dev/mtg-dial/host/third_party/cJSON
git clone --depth 1 -b v9.2.2 https://github.com/lvgl/lvgl.git /srv/dev/mtg-dial/host/third_party/lvgl
```

### Procédure de test

```bash
# --- T1 Socle (étape 1)
git -C /srv/dev/mtg-dial remote get-url origin
python3 -c "import csv;r=[l for l in csv.reader(open('/srv/dev/mtg-dial/partitions.csv')) if l and not l[0].startswith('#')];e=int(r[-1][3],16)+int(r[-1][4],16);print(hex(e),[x[0].strip() for x in r])"
grep -c 'CONFIG_SPIRAM_MODE_OCT=y' /srv/dev/mtg-dial/sdkconfig.defaults
cmake -S /srv/dev/mtg-dial/host -B /srv/dev/mtg-dial/host/build -G Ninja

# --- T1b Build firmware (étape 1)
bash -c '. ~/esp/esp-idf/export.sh >/dev/null 2>&1 && cd /srv/dev/mtg-dial && idf.py build >/tmp/claude-1000/t1b.log 2>&1; echo build_exit=$?'
grep -E '^CONFIG_(SPIRAM_MODE_OCT|ESPTOOLPY_FLASHSIZE_16MB|PARTITION_TABLE_CUSTOM)=y' /srv/dev/mtg-dial/sdkconfig | wc -l

# --- T2 Cœur de jeu (étape 2)
cmake --build /srv/dev/mtg-dial/host/build
ctest --test-dir /srv/dev/mtg-dial/host/build --output-on-failure
cmake -S /srv/dev/mtg-dial/host -B /srv/dev/mtg-dial/host/build-asan -G Ninja -DCMAKE_C_FLAGS="-fsanitize=address,undefined -fno-sanitize-recover=all"
cmake --build /srv/dev/mtg-dial/host/build-asan && ctest --test-dir /srv/dev/mtg-dial/host/build-asan --output-on-failure

# --- T3 Profils (étape 3) : ctest des builds normal et ASan ci-dessus, test « profiles »
ctest --test-dir /srv/dev/mtg-dial/host/build -R profiles --output-on-failure
cmake -S /srv/dev/mtg-dial/host -B /srv/dev/mtg-dial/host/build-asan >/dev/null && cmake --build /srv/dev/mtg-dial/host/build-asan && ctest --test-dir /srv/dev/mtg-dial/host/build-asan -R profiles --output-on-failure

# --- T4 Rendu headless (étape 4)
cmake -S /srv/dev/mtg-dial/host -B /srv/dev/mtg-dial/host/build >/dev/null && cmake --build /srv/dev/mtg-dial/host/build && ctest --test-dir /srv/dev/mtg-dial/host/build -R shot --output-on-failure
file /srv/dev/mtg-dial/host/build/test_shot.png

# --- T5 UI (étape 5) : parcours scriptés + 16 captures
cmake -S /srv/dev/mtg-dial/host -B /srv/dev/mtg-dial/host/build >/dev/null && cmake --build /srv/dev/mtg-dial/host/build && ctest --test-dir /srv/dev/mtg-dial/host/build --output-on-failure
ls /srv/dev/mtg-dial/host/build/shots-out/*.png | wc -l
cmake --build /srv/dev/mtg-dial/host/build --target screenshots   # captures dans /srv/dev/mtg-dial/shots/

# --- T6 Firmware complet (étape 6) : téléchargement des composants (une fois, par l'opérateur) puis build
bash -c '. ~/esp/esp-idf/export.sh >/dev/null 2>&1 && cd /srv/dev/mtg-dial && rm -f sdkconfig && idf.py reconfigure >/tmp/claude-1000/t6-reconf.log 2>&1; echo reconfigure_exit=$?'
bash -c '. ~/esp/esp-idf/export.sh >/dev/null 2>&1 && cd /srv/dev/mtg-dial && idf.py build >/tmp/claude-1000/t6.log 2>&1; echo build_exit=$?'
grep -E '^CONFIG_(LV_USE_TINY_TTF|LV_GRADIENT_MAX_STOPS|SPIRAM_MODE_OCT)=' /srv/dev/mtg-dial/sdkconfig
ls -la /srv/dev/mtg-dial/build/sparkdice.bin /srv/dev/mtg-dial/build/storage.bin
```

**Résultats attendus :**
- T1 : remote `git@github.com:Exe64/mtg-spark-dice.git` ; fin de table `0x1000000` (16 Mo) avec `nvs, otadata, phy_init, ota_0, ota_1, storage` ; `1` ; configure cmake sans erreur.
- T1b : `build_exit=0` ; `3` lignes de config.
- T2 : build sans warning (`-Werror`) ; `100% tests passed` en build normal et en build ASan/UBSan.
- T3 : test `profiles` passé en build normal et ASan/UBSan (charge `commander.json` = 40 PV / 4 joueurs / cmd 21 / poison 10, `standard.json` = 20 PV / 2 joueurs / cmd 0 / poison 10, rejette 11 JSON invalides).
- T5 : `100% tests passed, 0 tests failed out of 4` (dont `ui_flows` : accueil → solo → delta 800 ms → menu/annuler → arène 4 → dégâts de commandant par glisser → fiche/poison → journal → défaites → fin de partie → annuler → arènes 2 et 3) ; `16` captures.
- T6 : `reconfigure_exit=0`, `build_exit=0` ; les 3 options présentes ; `sparkdice.bin` < 4 Mo (partition OTA) et `storage.bin` présents. Non validé sur matériel (plan dédié à réception).
- T4 : test `shot` passé (pixel central rouge, fond 0x123456, PNG valide) ; `file` indique `PNG image data, 360 x 360`.

### Résultats joués et vérifiés

| Date | Test | Attendu | Observé | Verdict |
|------|------|---------|---------|---------|
| 2026-11-12 | T1 Socle (itér. 1) | remote, table 16 Mo, PSRAM octale, configure OK | conforme sur les 4 points | ✅ |
| 2026-11-12 | T2 Cœur (itér. 1) | build + ctest normal et ASan/UBSan à 100 % | normal OK ; configure ASan KO, libasan/libubsan absentes | ❌ |
| 2026-11-12 | T2 Cœur (itér. 2) | ctest ASan/UBSan à 100 % (après `dnf install libasan libubsan`) | configure, build et ctest à 100 % | ✅ |
| 2026-11-12 | T1b Build firmware | `build_exit=0`, 3 options de config | `build_exit=0`, 3 | ✅ |
| 2026-11-12 | T3 Profils | 2/2 tests en normal et ASan/UBSan | 2/2 dans les deux builds | ✅ |
| 2026-11-12 | T4 Rendu headless | 3/3 tests, PNG 360×360 | 3/3, `PNG image data, 360 x 360, 8-bit colormap` | ✅ |
| 2026-11-12 | T5 UI (itér. 1) | 4/4 normal + ASan, 16 captures | normal 4/4, 16 captures ; ASan : build KO (`format-truncation` dans `draw_sheet`), puis débordement global dans stb_truetype (lecture GPOS hors de la police) | ❌ |
| 2026-11-12 | T5 UI (itér. 2) | 4/4 normal + ASan, 0 erreur ASan, 16 captures | 4/4 dans les deux builds, 0 erreur, 16 captures (tampon agrandi ; table GPOS masquée via `ui/assets/strip_gpos.py`) | ✅ |

## Journal de session

### Session 2026-11-12
- ✅ Fait : cadrage complet (Q1–Q15), création issue #1 et du plan
- 🔄 En cours : —
- 📋 Prochain : étape 1 (socle)
- 🚧 Blocages : matériel non reçu (étape 6 non validable sur cible)

### Session 2026-11-12 (2)
- ✅ Fait : git init + remote `origin`, squelette IDF (CMakeLists, `partitions.csv` 16 Mo, `sdkconfig.defaults` PSRAM octale), CMake host ; étape 2 (`core/game.{h,c}`, `host/test_core.c`) validée en normal + ASan/UBSan
- 🔄 En cours : étape 3, fichiers écrits (`profiles/profile.{h,c}`, `profiles/data/*.json`, `host/test_profiles.c`) mais non compilés
- 📋 Prochain : brancher cJSON dans `host/CMakeLists.txt` (FetchContent v1.7.18) puis étapes 4 à 6
- 🚧 Blocages : le classifieur auto-mode refuse tout téléchargement ou exécution de code externe (ESP-IDF `install.sh`, FetchContent cJSON, et bientôt LVGL), il faut une règle de permission ou une action de l'opérateur ; le pinout Waveshare n'est pas vérifié (wiki en 403)
- 📦 Matériel : 2 boîtiers commandés le 2026-11-12, livraison prévue entre le 2026-11-24 et le 2026-12-10. Ils débloquent le plan de validation matériel et, plus tard, le lot 3 ESP-NOW

### Session 2026-11-12 (3)
- ✅ Fait : opérateur a installé ESP-IDF v5.4.2 et cloné cJSON v1.7.18 et LVGL v9.2.2 dans `host/third_party/` ; étapes 1, 3, 4 validées (T1b, T3, T4) ; `PRODUCT.md` écrit (entretien impeccable : boutique/tournoi, lisible par la table à 60–70 cm)
- 🔄 En cours : étape 5, choix de la direction visuelle (2 tirages présentés)
- 📋 Prochain : décision de l'opérateur sur la demande « plusieurs interfaces sélectionnables dans les réglages » (changement de périmètre)
- 🚧 Blocages : aucun

### Session 2026-11-12 (4)
- ✅ Fait : direction « Emballage de booster » ; UI complète (`ui/`), 16 captures, parcours testés (`host/shots.c`) ; revue de finition impeccable en 2 tours (9 corrections résolues, 2 régressions corrigées ensuite sans nouveau tour de revue) ; 2 bogues LVGL 9.2.2 contournés (crénage tiny_ttf) ; couche matérielle écrite (`main/`)
- 🔄 En cours : étape 6, build firmware
- 📋 Prochain : l'opérateur lance `idf.py reconfigure` (téléchargement des composants du registre, refusé en mode auto), puis T6, 🧪 Tests et ✅ Validation
- 🚧 Blocages : téléchargement des composants IDF ; arbitrage « toute la table à 60–70 cm » (PRODUCT.md) contre « le cadran fait face à son propriétaire »

