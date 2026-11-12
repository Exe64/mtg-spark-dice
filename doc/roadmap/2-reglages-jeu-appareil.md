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

Dépend de #5 (direction visuelle lumineuse : l'accueil radial, les réglages et la mise en route sont construits dans ce style). Bloque #3 et #4 (cadre Réglages). Passe après #6 et #7 (ordre décidé le 2026-11-13) ; l'entrée Réglages du menu de partie est posée par #7.

## Périmètre

### Inclus
- Accueil refondu : Nouvelle partie (action principale) → écran « Nouvelle partie » (nombre de joueurs + écran de jeu Solo/Arène) ; Reprendre avec « T{tour} | J{joueurs} » ; pastille Type de jeu « MAGIC · 40 PV » → choix du profil et des règles ; bouton Réglages ; la molette déplace le focus entre ces actions, tap/appui = lancer ; places réservées (cachées) pour Héberger/Rejoindre (lot 3) et le badge Mise à jour (lot 2)
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

- [ ] Étape 1 — Cœur : `sd_settings` (défauts, bornes, sérialisation NVS versionnée) + règles paramétrables dans `core/` (KO auto, victoire auto, PV du commandant, ordre du tour, attaque = cmdt, clics/point, délai de validation) + tests host (M · standard → Sonnet)
- [ ] Étape 2 — Traductions : remplacer les macros de `strings_fr.h` par des tables par langue (`strings_fr.c`, `strings_en.c`), sélection à chaud, test host qui vérifie que chaque chaîne existe dans chaque langue (M · standard → Sonnet)
- [ ] Étape 3 — Accueil : Nouvelle partie (+ écran nombre de joueurs et Solo/Arène), Reprendre « T n | J n », pastille Type de jeu → profils, bouton Réglages, focus à la molette ; remplace l'accueil du plan #1 (M · reasoning → Opus)
- [ ] Étape 4 — Cadre Réglages : grille 6 sections, listes éditables, protection PIN, accès accueil/menu (M · standard → Sonnet)
- [ ] Étape 5 — Section Jeu : tous les réglages listés + vue Solo « ma seule vie » à plusieurs joueurs (M · standard → Sonnet)
- [ ] Étape 6 — Section Appareil : version, langue, nom (aléatoire par langue + Régénérer + saisie molette), PIN + écran de verrouillage avec message, sens encodeur, haptique, réinitialisation d'usine (M · standard → Sonnet)
- [ ] Étape 7 — Mise en route : langue, bienvenue, type de jeu/profil, nom, calibrage molette, test tactile + mire, tutoriel des gestes ; relance depuis Appareil (L · reasoning → Opus)
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
| Accueil | Fonctions de l'accueil de référence (Réglages, Nouvelle partie, Reprendre T/J, Type de jeu), apparence SparkDice ; Héberger/Rejoindre cachés jusqu'au lot 3, badge Mise à jour au lot 2 | Choix opérateur (itération 2) |
| Molette sur l'accueil | Déplace le focus entre les actions ; tap/appui lance | Choix opérateur (itération 2) : utilisable sans toucher l'écran |
| Nouvelle partie | Nombre de joueurs + écran de jeu Solo/Arène au lancement | Choix opérateur (itération 2) |
| Découpage | 3 plans : #2 (ce plan), #3 Affichage et veille, #4 Joueurs, tours et minuteur | Choix opérateur 2026-11-12 |
| Ordre | #5 → #6 → #7 → #2 → #4 → #3 | Choix opérateur 2026-11-13 : la parité en partie (marqueurs, outils) passe avant les réglages d'appareil |

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
