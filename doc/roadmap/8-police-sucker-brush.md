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

- [ ] Étape 1 — Intégrer Sucker Brush comme police display : vérification des glyphes (0-9, signe moins, lettres des titres), `strip_gpos.py` si nécessaire, embarquement firmware + host, README (S · standard → Sonnet)
- [ ] 🧪 Tests — Rédiger et exécuter la procédure de test (avant-dernière étape, obligatoire)
- [ ] ✅ Validation — Vérifier les résultats des tests et clôturer (dernière étape, obligatoire)

## Décisions techniques

| Décision | Choix retenu | Justification |
|----------|-------------|---------------|
| Portée | PV + centre + titres (police display) ; libellés inchangés | Choix opérateur 2026-11-13 : lisibilité des petits libellés |
| Fichier .ttf | Non committé, dans `ui/assets/licensed/` (git-ignoré) ; le build échoue s'il manque | Licence usage personnel, dépôt public : pas de redistribution |
| Logo | Inchangé (Original Motion) | Hors demande |

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
# à compléter
```

**Résultats attendus :** à compléter.

### Résultats joués et vérifiés

| Date | Test | Attendu | Observé | Verdict |
|------|------|---------|---------|---------|

## Journal de session

### Session 2026-11-13
- ✅ Fait : cadrage (portée, licence), création de l'issue #8 et du plan
- 🔄 En cours : —
- 📋 Prochain : étape 1
- 🚧 Blocages : —
