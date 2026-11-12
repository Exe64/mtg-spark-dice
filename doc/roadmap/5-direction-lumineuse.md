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

Aucune (plan #1 livré). Bloque #2 (ses écrans sont construits dans la nouvelle direction) ; le sélecteur de thème (lumineux / booster) est dans #3.

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
- [ ] Étape 2 — Système de thèmes : `ui_theme_t` étendu (fond dégradé, lueurs, icônes, rendu des sièges), couleur de fond paramétrable (dégradé dérivé d'une seule teinte, choix stocké hors partie), booster conservé tel quel (M · reasoning → Opus)
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
- ✅ Fait : cadrage (style lumineux pour toute l'interface, booster en 2ᵉ thème), création de l'issue #5 et du plan
- 📋 Prochain : étape 1 ; plan #2 attend ce plan
- 🚧 Blocages : nom du produit à trancher
- ✅ Fait : logo dessiné et validé (`ui/assets/logo.svg`) ; étape 1 : contrat de direction lumineuse (brief impeccable, booster conservé en thème 2), PRODUCT.md (nom provisoire, logo, direction)
- 🔄 Ajustement opérateur : couleur de fond au choix dans les réglages (moteur à l'étape 2, sélecteur dans #3)
- 📋 Prochain : validation du contrat par l'opérateur, puis étape 2 (système de thèmes)
