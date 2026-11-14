---
plan:
  id: '2'
  name: 2-reglages-jeu-appareil.md
  link: https://github.com/Exe64/mtg-spark-dice/blob/main/doc/roadmap/2-reglages-jeu-appareil.md
status: active
date: 2026-11-12
description: >
  Nouvel accueil, écran Réglages (6 sections), sections Jeu et Appareil complètes, langues FR/EN, PIN, mise en route au premier démarrage.
priority: high
complexity: L
enriched: 2026-11-13
intent: null
scope:
  modules:
    - core/
    - ui/
    - main/
    - host/
issue:
  id: 2
  url: https://github.com/Exe64/mtg-spark-dice/issues/2
---

# [⚙️] Plan #2 - Réglages, Jeu, Appareil et mise en route

## Objectif

Donner à SparkDice un écran Réglages structuré en 6 sections (Jeu, Joueurs, Tours/Minuteur, Affichage, Connectivité, Appareil), livrer complètement Jeu et Appareil, et guider le premier démarrage. Les sections Joueurs, Tours/Minuteur et Affichage sont remplies par les plans #4 et #3, Connectivité par les lots 2/3.

## Dépendances

Dépend de #5 (direction visuelle lumineuse : l'accueil radial, les réglages et la mise en route sont construits dans ce style). Bloque #3 et #4 (cadre Réglages). Passe avant #6 et #7 (ordre revu le 2026-11-14 : #6 attend 8 icônes) ; l'entrée Réglages est ajoutée au menu de partie actuel (celui du plan #1), #7 la reprendra dans son menu à 6 entrées.

## Périmètre

### Inclus
- Accueil refondu : Nouvelle partie (action principale) → écran « Nouvelle partie » (nombre de joueurs + écran de jeu Solo/Arène) ; Reprendre avec « T{tour} | J{joueurs} » ; pas de pastille Type de jeu : le jeu puis le format se choisissent après « Nouvelle partie » (décision opérateur 2026-11-13, livré dans #5 étape 8) ; bouton Réglages ; la molette déplace le focus entre ces actions, tap/appui = lancer ; Héberger/Rejoindre affichés atténués et inactifs jusqu'au lot 3 et le badge Mise à jour (lot 2)
- Cadre : grille de 6 sections défilante à la molette, listes éditables (molette = valeur, tap = entrer/revenir), accès depuis l'accueil et le menu de partie, réglages en NVS versionnés
- Jeu : type de jeu (Magic), écran de jeu Solo/Arène (Solo = ma seule vie en plein écran, même à plusieurs), ordre du tour (horaire/inversé), PV de départ, clics/point (1–12), démarrer sur (accueil/partie), inverser les chiffres (sièges tournés on/off), KO auto, victoire auto, PV du commandant, attaque glissée = cmdt, seuils KO cmdt et poison, affichage cmdt (cible/source), réinitialiser la partie
- Appareil : version + date de build, langue (FR/EN, changement immédiat), nom de l'appareil (généré au hasard selon la langue, régénérable, modifiable à la molette), assistant de configuration, PIN 4 chiffres (démarrage + protection des réglages ; stocké pour l'appairage futur), message du propriétaire (écran de verrouillage), sens de l'encodeur, intensité haptique, réinitialisation d'usine (double tap)
- Mise en route (à la façon de l'assistant de référence) : 1) langue, 2) bienvenue + Démarrer, 3) type de jeu = profil Magic par défaut (Commander/Standard), 4) nom de l'appareil (aléatoire fantasy dans la langue choisie, Régénérer, Modifier), 5) calibrage molette (sens + impulsions/cran), 6) test tactile 4 cibles + mire, 7) tutoriel des gestes ; étapes 5–7 passables ; relance par Réglages → Appareil → Assistant
- Traductions : chaînes de l'UI en tables par langue (FR, EN), choix à chaud
- Sections Joueurs, Tours/Minuteur, Affichage, Connectivité visibles avec « bientôt » jusqu'aux plans suivants

### Hors scope
- Langues autres que FR/EN (ajout = un fichier de table ; le japonais demanderait une autre police)
- Autres jeux que Magic (Lorcana, Yu-Gi-Oh…) : plus tard via de nouveaux profils
- Étapes Thème, Wi-Fi, Enregistrement, Mise à jour de l'assistant de référence (thèmes au backlog, Wi-Fi/OTA au lot 2, pas d'enregistrement cloud)
- Source de l'encodeur (une seule valeur utile)
- Connectivité (lots 2/3), jeu en ligne / cloud ; Héberger/Rejoindre sur l'accueil (lot 3), badge Mise à jour (lot 2)
- Contenu des sections Affichage (#3), Joueurs et Tours/Minuteur (#4)
- Validation sur matériel

## Étapes

- [x] Étape 1 — Cœur : `sd_settings` (défauts, bornes, sérialisation NVS versionnée) + règles paramétrables dans `core/` (KO auto, victoire auto, PV du commandant, ordre du tour, attaque = cmdt, clics/point, délai de validation) + tests host (M · standard → Sonnet)
- [x] Étape 2 — Traductions : remplacer les macros de `strings_fr.h` par des tables par langue (`strings_fr.c`, `strings_en.c`), sélection à chaud, test host qui vérifie que chaque chaîne existe dans chaque langue (M · standard → Sonnet)
- [x] Étape 3 — Accueil : l'accueil radial et l'écran « Nouvelle partie » (format puis joueurs) existent depuis #5 (étape 8) ; reste à brancher le bouton Réglages, le focus à la molette et le choix Solo/Arène ; pas de pastille Type de jeu (S · standard → Sonnet)
- [x] Étape 4 — Cadre Réglages : grille 6 sections, listes éditables, protection PIN, accès accueil/menu (M · standard → Sonnet)
- [x] Étape 5 — Section Jeu : tous les réglages listés + vue Solo « ma seule vie » à plusieurs joueurs (M · standard → Sonnet)
- [x] Étape 6 — Section Appareil : version, langue, nom (aléatoire par langue + Régénérer + saisie molette), PIN + écran de verrouillage avec message, sens encodeur, haptique, réinitialisation d'usine (M · standard → Sonnet)
- [x] Étape 7 — Mise en route : langue, bienvenue, type de jeu/profil, nom, calibrage molette, test tactile + mire, tutoriel des gestes ; relance depuis Appareil (L · reasoning → Opus)
- [ ] Étape 8 — HAL : appliquer sens/impulsions de l'encodeur et intensité haptique, PIN au démarrage, version de build (S · standard → Sonnet)
- [ ] Étape 9 — Design : extension du thème, captures, revue de finition, DESIGN.md (S · standard → Sonnet)
- [ ] 🧪 Tests — Rédiger et exécuter la procédure de test (avant-dernière étape, obligatoire)
- [ ] ✅ Validation — Vérifier les résultats des tests et clôturer (dernière étape, obligatoire)

## Décisions techniques

| Décision | Choix retenu | Justification |
|----------|-------------|---------------|
| Origine des réglages | Liste fonctionnelle inspirée de la doc de réglages fournie par l'opérateur (app tierce) | Mécaniques reprises, apparence SparkDice (thème booster lisse), pas de copie visuelle |
| PIN | 4 chiffres, protège l'entrée des réglages et la réinitialisation, optionnel au démarrage ; stocké pour l'appairage des lots 2/3 | Choix opérateur 2026-11-12 |
| Langues | FR + EN dès ce plan, choisies à la mise en route et dans Appareil | Choix opérateur (itération 1) : « mettre la langue dès le départ » |
| Nom de l'appareil | Généré au hasard (2 mots fantasy dans la langue choisie), Régénérer, Modifier à la molette | Choix opérateur (itération 1) |
| Type de jeu | Magic seul ; l'étape choisit le profil par défaut | Choix opérateur (itération 1) |
| Nom du produit | SparkDice (pas SparkDial) | Confirmé par l'opérateur (itération 1) |
| Style de l'accueil | Esprit de la référence (radial, lueur, fond bleu nuit, icônes) via le thème lumineux du plan #5, logo de l'opérateur | Choix opérateur (itération 3) |
| Accueil | Fonctions de l'accueil de référence (Réglages, Nouvelle partie, Reprendre T/J), apparence SparkDice ; sans Type de jeu (2026-11-13) ; Héberger/Rejoindre atténués jusqu'au lot 3, badge Mise à jour au lot 2 | Choix opérateur (itération 2) |
| Molette sur l'accueil | Déplace le focus entre les actions ; tap/appui lance | Choix opérateur (itération 2) : utilisable sans toucher l'écran |
| Nouvelle partie | Nombre de joueurs + écran de jeu Solo/Arène au lancement | Choix opérateur (itération 2) |
| Réglages numériques | Une table `int16` indexée par `sd_setting_id_t` (défauts et bornes au même endroit), sauvegarde positionnelle : un firmware plus récent relit une sauvegarde plus courte | Une seule source pour les bornes ; ajout d'un réglage sans perdre les anciens |
| Règles d'une partie | Options (`SD_OPT_*`) et délai de validation copiés dans `sd_rules_t` au lancement, donc sauvegardés avec la partie (format `SDG4`) | Une partie reprise garde les règles avec lesquelles elle a commencé |
| PV de départ, seuils cmdt/poison | 0 = « celui du format » ; un seuil ne réactive jamais une règle que le format n'a pas | Les formats restent la référence |
| « PV du commandant » | Interprété comme « les dégâts de commandant retirent aussi des PV » (oui par défaut) | Libellé ambigu dans la liste d'origine ; à corriger si le sens voulu est autre |
| Traductions | Une table par langue (`ui/strings_fr.c`, `strings_en.c`) indexée par un énuméré commun ; les formats `printf` portent le suffixe `_FMT` et le test `strings` vérifie qu'ils prennent les mêmes arguments dans chaque langue | Une langue = un fichier ; une chaîne oubliée ou un format divergent fait échouer le build de test |
| Famille de format en anglais | Champ optionnel `family_en` dans les profils JSON | « Construit » venait des données, pas des tables |
| Cadre Réglages | Grille 2 × 3 de pastilles + bouton retour (7 crans de molette) ; une section = une liste en carrousel, ligne focalisée au centre ; tap = entrer/sortir de l'édition, molette = valeur ; tap hors bouton = action focalisée | Tout se fait à la molette et d'un tap, comme sur l'accueil |
| Lignes de réglage | Tables de données (`row_t`) : libellé, type, réglage, pas ; les bornes viennent du cœur | Ajouter un réglage = une ligne ; #3 et #4 n'ont qu'à fournir leurs tables |
| PIN | Saisi chiffre par chiffre à la molette, sans limite d'essais, stocké en clair | Garde-fou de table de jeu, pas une protection contre un attaquant |
| Vue Solo à plusieurs | Le cadran Solo montre le siège J1 ; les dégâts de commandant reçus se saisissent sur la fiche (tap sur la colonne du joueur, molette) ; pas de moyeu, donc pas de passage de tour | Chaque joueur suit sa vie sur son boîtier ; les autres totaux viendront du multijoueur (lot 3) |
| Portée des réglages de règles | Options (KO auto, victoire auto, cmdt/PV, ordre du tour) et délai : appliqués tout de suite à la partie en cours ; PV de départ et seuils : à la prochaine partie ou par « Réinitialiser la partie » | Un seuil « du format » ne peut pas être retrouvé en cours de partie |
| Clics par point | S'applique aux écrans où la molette compte des points (partie, fiche, marqueurs) ; un tap sur − / + vaut toujours 1 | Les listes et menus restent à un cran par ligne |
| Saisie de texte | Un écran pour le nom, le message et le PIN : la molette choisit le caractère, un tap l'ajoute ; majuscules, chiffres et « - ' ! ? », sans accents | Suffit pour un nom ou un numéro ; les accents s'ajoutent en allongeant l'alphabet |
| Noms au hasard | 12 noms × 12 adjectifs par langue, dans le fichier de la langue ; tirage par un générateur brassé par l'heure des taps | Pas besoin de l'aléa matériel pour un nom (il arrive avec les dés de #7) |
| Réinitialisation d'usine | Efface partie et réglages (crochet HAL `factory_reset`), puis relance la mise en route | Retour à l'état de sortie de boîte |
| Calibrage de la molette | L'assistant demande 10 crans vers la droite et compare aux crans reçus : signe = sens, rapport = impulsions par cran ; rien tourné = étape passée | Pas de mode brut dans le HAL : l'UI ne voit que des crans, le rapport suffit |
| Test tactile | Mire (anneaux, 4 couleurs de siège) et 4 cibles haut/droite/bas/gauche ; le bouton central passe l'étape | Vérifie les bords de la dalle ronde, là où les sièges sont touchés |
| Découpage | 3 plans : #2 (ce plan), #3 Affichage et veille, #4 Joueurs, tours et minuteur | Choix opérateur 2026-11-12 |
| Ordre | #5 → #2 → #6 → #7 → #4 → #3 | Choix opérateur 2026-11-14 : #2 est lancé sans attendre #6 (8 icônes en attente) ni #7 ; remplace l'ordre du 2026-11-13 |

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
| 2026-11-14 | Étape 1 — host + ASan/UBSan (`test_rule_options`, `test_settings`) | 0 avert., 6/6 deux fois | 0, 6/6, 6/6 (auto-check, même contexte) | ✅ |
| 2026-11-14 | Étape 2 — host + ASan/UBSan (`strings`, bascule FR→EN à chaud dans `ui_flows`, capture `18-nouvelle-partie-en`) | 0 avert., 7/7 deux fois, écran en anglais | 0, 7/7, 7/7, « CONSTRUCTED / 20 LIFE · POISON 10 / NEXT » (auto-check) | ✅ |
| 2026-11-14 | Étape 3 — host + ASan/UBSan (`ui_flows` : focus molette START/REPRENDRE/Réglages, tap hors bouton = action focalisée, 3ᵉ pas « écran de jeu ») ; captures `01b`, `15d`, `15e` relues | 0 avert., 7/7 deux fois ; bouton Réglages allumé au focus ; ARÈNE/SOLO lisibles dans les deux thèmes | 0, 7/7, 7/7 ; conforme (auto-check) | ✅ |
| 2026-11-14 | Étape 4 — host + ASan/UBSan (`ui_flows` : grille, section « bientôt », liste, édition molette/chevrons, bornes, PIN refusé puis accepté, abandon, accès depuis le menu) ; captures `19`–`19g` relues dans les deux thèmes | 0 avert., 7/7 deux fois ; 4 enregistrements de réglages ; écrans lisibles | 0, 7/7, 7/7 ; 4 ; conforme. Constat : la ligne ANNULER du menu déborde de sa pastille quand le libellé est long (déjà le cas avant, à reprendre avec le menu de #7) (auto-check) | ✅ |
| 2026-11-14 | Étape 5 — host + ASan/UBSan (`ui_flows` : Solo à 4, dégâts de commandant par la fiche, affichage source, 3 clics/point, sièges droits, attaque = cmdt, ordre inversé et KO manuel appliqués à la partie en cours, réinitialisation en deux taps ; `boot_start-on-*`) ; captures `20`–`20e` relues | 0 avert., 10/10 deux fois | 0, 10/10, 10/10 ; conforme. Constat : sièges non tournés, les marques − / + des sièges de gauche et de droite touchent le bord et le moyeu (les zones de tap restent justes) (auto-check) | ✅ |
| 2026-11-14 | Étape 6 — host + ASan/UBSan (`ui_flows` : version, langue à chaud, nom au hasard, nom saisi « JU », PIN défini puis retiré en deux taps, PIN au démarrage, message, sens/impulsions de molette, vibration, réinitialisation d'usine en deux taps ; `boot_pin`, `boot_pin-not-at-boot`) ; captures `21`–`21f`, `22` relues | 0 avert., 12/12 deux fois ; écran verrouillé sans échappatoire, message du propriétaire affiché | 0, 12/12, 12/12 ; conforme (auto-check) | ✅ |
| 2026-11-14 | Étape 7 — host + ASan/UBSan (`ui_flows` démarre par l'assistant : langue à chaud, bienvenue, format par défaut, nom tiré/régénéré/modifié, calibrage molette « 10 crans reçus −5 » → sens inversé et 2 impulsions/cran, 4 cibles, gestes ; relance et abandon depuis Appareil ; assistant après réinitialisation d'usine) ; captures `00a`–`00h` relues dans les deux thèmes | 0 avert., 12/12 deux fois ; `SETUP_DONE` enregistré à la fin | 0, 12/12, 12/12 ; conforme (auto-check) | ✅ |

## Journal de session

### Session 2026-11-12
- ✅ Fait : cadrage (arborescence des réglages fournie par l'opérateur, découpage en 3 plans #2/#3/#4 validé), création de l'issue #2 et du plan
- 🔄 En cours : —
- 📋 Prochain : étape 1
- 🚧 Blocages : matériel non reçu (parties ressenties validées dans le plan « validation matériel »)

### Session 2026-11-12 (itération 1 : mise en route)
- ✅ Fait : mise en route alignée sur l'assistant de référence (langue, bienvenue, type de jeu, nom) + nos étapes (calibrage, test, tutoriel) ; ajout d'une étape Traductions FR/EN ; étapes renumérotées (8 étapes)
- 📋 Prochain : autres itérations de cadrage, puis étape 1

### Session 2026-11-12 (itération 2 : accueil)
- ✅ Fait : refonte de l'accueil ajoutée (nouvelle étape 3) ; plan à 9 étapes


### Session 2026-11-13
- ✅ Fait : ordre revu, ce plan passe après #6 (marqueurs, cœur 6 joueurs) et #7 (outils de partie)
- 📋 Prochain : étape 1, après #7
