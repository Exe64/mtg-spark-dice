---
plan:
  id: '8'
  name: 8-police-sucker-brush.md
  link: https://github.com/Exe64/mtg-spark-dice/blob/main/doc/roadmap/8-police-sucker-brush.md
status: active
date: 2026-11-13
description: >
  Police Sucker Brush pour les PV, le centre et les titres, à la place de Barlow Condensed ExtraBold ; libellés et logo inchangés.
priority: high
complexity: S
intent: null
scope:
  modules:
    - ui/
    - host/
issue:
  id: 8
  url: https://github.com/Exe64/mtg-spark-dice/issues/8
---

# [🖌️] Plan #8 - Police Sucker Brush

## Objectif

Donner aux PV et aux titres de Dice Spark le style pinceau de Sucker Brush (https://www.dafont.com/fr/sucker-brush.font), choisi par l'opérateur le 2026-11-13.

## Dépendances

Aucune. Indépendant des plans en cours ; touche les mêmes fichiers de thème que #6 et #7, sans conflit fonctionnel.

## Périmètre

### Inclus
- Police display (PV solo et arène, centre, titres) : Sucker Brush, dans les deux thèmes (Lumineux, Booster)
- Embarquement du .ttf (firmware et build PC) depuis `ui/assets/licensed/`, avec message clair si le fichier manque
- Ajustement des tailles en px si les chiffres débordent des sièges
- README des assets : source, licence, emplacement

### Hors scope
- Libellés (Barlow Condensed SemiBold conservée)
- Logo « Dice Spark » (Original Motion, pré-rendu)
- Repli automatique sur Barlow si la police manque
- Validation sur matériel

## Étapes

- [x] Étape 1 — Intégrer Sucker Brush comme police display : vérification des glyphes (0-9, signe moins, lettres des titres), `strip_gpos.py` si nécessaire, embarquement firmware + host, README (S · standard → Sonnet)
- [x] 🧪 Tests — Rédiger et exécuter la procédure de test (avant-dernière étape, obligatoire)
- [ ] ✅ Validation — Vérifier les résultats des tests et clôturer (dernière étape, obligatoire)

## Décisions techniques

| Décision | Choix retenu | Justification |
|----------|-------------|---------------|
| Portée | PV + centre + titres (police display) ; libellés inchangés | Choix opérateur 2026-11-13 : lisibilité des petits libellés |
| Fichier .ttf | Non committé, dans `ui/assets/licensed/` (git-ignoré) ; le build échoue s'il manque | Licence usage personnel, dépôt public : pas de redistribution |
| Logo | Inchangé (Original Motion) | Hors demande |
| Tailles | Solo 200→170, arène 3 92→84, arène 4 80→72, titre 64→56 ; titres longs : 56/48/32 px selon la largeur mesurée (≤ 300 px) ; marques −/+ en arène 3 à 74 px | Le pinceau est ~20 % plus large que Barlow condensé : « COMMANDER » passait sur deux lignes, « 199 » touchait les marques |
| Barlow ExtraBold | Supprimée du dépôt et du firmware | Plus utilisée |
| Garde-fous build | `ui/display_font.cmake` (firmware + host) : arrêt si la police manque ou garde sa table GPOS | Un .ttf retéléchargé sans `strip_gpos.py` lirait hors buffer |

## Tests

> Section obligatoire renseignée aux étapes de tests (mode `autonomous`).

### Prérequis pour les tests

```bash
# environnement du plan #1 (voir doc/roadmap/done/1-mvp-solo-arene.md § Prérequis) ;
# ui/assets/licensed/Sucker Brush.ttf présent
cd /srv/dev/mtg-dial
```

### Procédure de test

```bash
cd /srv/dev/mtg-dial
F="ui/assets/licensed/Sucker Brush.ttf"
# T1 — host : build sans avertissement, 6/6 tests
cmake --build host/build 2>&1 | grep -ci warning; ctest --test-dir host/build | tail -3
# T2 — idem sous ASan/UBSan (la lecture hors police GPOS ferait échouer)
cmake --build host/build-asan 2>&1 | grep -ci warning; ctest --test-dir host/build-asan | tail -3
# T3 — parcours et captures
cmake --build host/build --target screenshots 2>&1 | grep "shots:"; ls shots/*.png | wc -l
# T4 — firmware : build, avertissements, taille, symbole embarqué
bash -c '. ~/esp/esp-idf/export.sh >/dev/null 2>&1 && idf.py build > /tmp/fw.log 2>&1; echo build_exit=$?'
grep -ci "warning:" /tmp/fw.log; stat -c %s build/sparkdice.bin
nm build/sparkdice.elf | grep -c "_binary_Sucker_Brush_ttf_start\|_binary_BarlowCondensed_ExtraBold"
# T5 — garde-fou : police absente
mv "$F" /tmp/sb.ttf; cmake -S host -B host/build 2>&1 | grep -o "Missing .*Sucker Brush.ttf"; mv /tmp/sb.ttf "$F"
# T6 — garde-fou : GPOS non masqué (on remet le tag GPOS sur une copie de travail)
cp "$F" /tmp/sb.ttf
python3 -c "import sys;p=sys.argv[1];d=bytearray(open(p,'rb').read());i=d.find(b'XPOS',0,512);d[i:i+4]=b'GPOS';open(p,'wb').write(d)" "$F"
cmake -S host -B host/build 2>&1 | tr -s ' \n' ' ' | grep -o "still has a GPOS table"
mv /tmp/sb.ttf "$F"; cmake -S host -B host/build >/dev/null && echo reconfigure_ok
# T7 — licence : la police n'est pas suivie par git, Barlow ExtraBold retirée
git check-ignore -q "$F" && echo ignored; git ls-files ui/assets | grep -c "Sucker\|ExtraBold"
# T8 — visuel : shots/03-solo.png, 08-arene-4.png, 15a-format-commander-1v1.png, 02b-nouvelle-partie-joueurs.png,
#      14-fin-de-partie.png ; et totaux 100/199 en arène 3 et 4 (rendu de contrôle hors dépôt)
```

**Résultats attendus :**

| Test | Attendu |
|---|---|
| T1 | `0` avertissement ; `100% tests passed … out of 6` |
| T2 | `0` avertissement ; `100% tests passed … out of 6` |
| T3 | `shots: all flows passed` ; `38` captures |
| T4 | `build_exit=0` ; `0` avertissement ; binaire < 4 194 304 o ; `1` (Sucker Brush présent, ExtraBold absent) |
| T5 | `Missing …/Sucker Brush.ttf` |
| T6 | `still has a GPOS table` puis `reconfigure_ok` |
| T7 | `ignored` ; `0` |
| T8 | chiffres et titres au pinceau ; aucun titre coupé ni sur deux lignes ; total à 3 chiffres sans chevauchement des marques − / + |

### Résultats joués et vérifiés

| Date | Test | Attendu | Observé | Verdict |
|------|------|---------|---------|---------|
| 2026-11-13 | T1 host | 0 avert., 6/6 | 0, 6/6 | ✅ |
| 2026-11-13 | T2 ASan/UBSan | 0 avert., 6/6 | 0, 6/6 | ✅ |
| 2026-11-13 | T3 parcours + captures | all flows passed, 38 | all flows passed, 38 | ✅ |
| 2026-11-13 | T4 firmware | exit 0, 0 avert., < 4 Mio, 1 symbole | exit 0, 0, 2 350 880 o, 1 | ✅ |
| 2026-11-13 | T5 police absente | `Missing …` | `Missing …/Sucker Brush.ttf` | ✅ |
| 2026-11-13 | T6 GPOS non masqué | message GPOS puis reconfigure_ok | idem (1er passage : grep coupé par le retour à la ligne de CMake, rejoué) | ✅ |
| 2026-11-13 | T7 licence | ignored, 0 | ignored, 0 | ✅ |
| 2026-11-13 | T8 visuel (vérificateur indépendant) | pinceau, aucun titre coupé, − / + dégagés | conforme sur 7 images ; constat : le « 1 » de la police ressemble à un « I » (« J1 » → « JI », « 1V1 » → « IVI ») | ✅ |

## Journal de session

### Session 2026-11-13
- ✅ Fait : cadrage (portée, licence), création de l'issue #8 et du plan
- 🔄 En cours : —
- 📋 Prochain : étape 1
- 🚧 Blocages : —
- ✅ Fait (suite) : étape 1 (police display, tailles ajustées, garde-fous build, doc), 🧪 Tests T1–T8 PASS (vérificateur indépendant)
- 📋 Prochain : ✅ Validation : avis de l'opérateur sur le « 1 » qui ressemble à un « I »
