# Roadmap SparkDice

> Backlog actif. L'historique détaillé est archivé dans `_archives/`.
> Dernière mise à jour : 2026-11-14.

## Contexte

Backlog consolidé : plans nommés d'après leur issue GitHub (voir `doc/roadmap/`).
- « À faire » : travaux restants, triés par priorité.
- « Fait » : ce qui a été livré et validé.
- Ordre d'exécution (revu le 2026-11-14) : #5 → #2 → #6 → #7 → #4 → #3. #2 passe devant : #6 attend 8 icônes de l'opérateur.

---

## À faire

### [Issue: #2] Réglages, Jeu, Appareil et mise en route [PRIORITÉ: HAUTE] [Complexité: L] 🟢

Plan : [`2-reglages-jeu-appareil.md`](2-reglages-jeu-appareil.md)

Nouvel accueil, Réglages en 6 sections, Jeu et Appareil, langues FR/EN, PIN, mise en route. Dépend de #5 ; passe avant #6 et #7 (ordre revu le 2026-11-14) ; bloque #3 et #4.

---

### [Issue: #6] Marqueurs et cœur 6 joueurs [PRIORITÉ: HAUTE] [Complexité: L] 🟢

Plan : [`6-marqueurs-coeur-6-joueurs.md`](6-marqueurs-coeur-6-joueurs.md)

Marqueurs Magic (énergie, expérience, Trésor, Nourriture, tempête, radiation, Ascension, monarque, initiative, Jour/Nuit) avec accès direct ; cœur à 6 joueurs et commandant partenaire, un seul nouveau format de sauvegarde. Dépend de #5 ; bloque #7 et #4.

---

### [Issue: #7] Outils de partie [PRIORITÉ: HAUTE] [Complexité: L] 🟢

Plan : [`7-outils-de-partie.md`](7-outils-de-partie.md)

Dés, tirage du premier joueur, menu de partie à 6 entrées, fin de partie (vainqueur, égalité, revanche), historique avec courbe de PV. Dépend de #6 et #5.

---

### [Issue: #3] Affichage et veille [PRIORITÉ: MOYENNE] [Complexité: L] 🟢

Plan : [`3-affichage-veille.md`](3-affichage-veille.md)

Luminosité, rotation de l'écran, veille (assombrir, éteindre, réveil), options d'interaction de l'Arène, calibrage tactile. Dépend de #2.

---

### [Issue: #4] Joueurs, tours et minuteur [PRIORITÉ: MOYENNE] [Complexité: XL] 🟢

Plan : [`4-joueurs-tours-minuteur.md`](4-joueurs-tours-minuteur.md)

1 à 6 joueurs, identité et commandant partenaire, bibliothèques joueurs/decks, suivi des tours, minuteur avec pause et alertes. Dépend de #2 et de #6 (cœur à 6 joueurs).

---

### Backlog (sans plan ni issue)

- **Simulateur navigateur** — UI + cœur compilés en WebAssembly (LVGL), jouable à la souris avant réception des boîtiers
- **Validation matériel** — à réception des 2 boîtiers (2026-11-24 → 2026-12-10) : brochage `main/board.h`, init ST77916, crans de molette, haptique DRV2605, coût des calques tournés sur ESP32-S3
- **Lot 2** — Wi-Fi + API REST/JSON + page web de gestion (profils, réglages) + OTA/rollback + batterie (la veille est passée dans #3)
- **Lot 3** — Multijoueur ESP-NOW (hôte/invité, code de salle, sync d'état) — nécessite ≥ 2 boîtiers
- **Lot 4** — Mode D&D (l'Arène 5–6 joueurs est passée dans #4)
- **Thèmes d'interface** — (sélecteur Lumineux / Booster désormais dans #3) thèmes supplémentaires (Table de pod, Notation de loyauté, Signalétique aéroport, Grille Crouwel) sur la structure de thème du lot 1
- **Lot 5** — Application native iOS/Android sur l'API du lot 2

---

## Fait

### [Issue: #1] Lot 1 — MVP Solo + Arène locale [PRIORITÉ: HAUTE] [Complexité: L] 🔵

Plan : [`done/1-mvp-solo-arene.md`](done/1-mvp-solo-arene.md)

Firmware SparkDice : Solo + Arène 1–4 (sièges orientés), tours, attaque par glisser / dégâts de commandant, poison, annulation, journal, reprise NVS, profils JSON. Logique testée sur PC (ASan/UBSan), 20 captures, build ESP32-S3 sans avertissement. Validé le 2026-11-12 (sans matériel).

---

### [Issue: #5] Direction visuelle lumineuse [PRIORITÉ: HAUTE] [Complexité: L] 🔵

Plan : [`done/5-direction-lumineuse.md`](done/5-direction-lumineuse.md)

Thème Lumineux par défaut (fond radial, sièges en verre, lueurs, icônes, logo, nom « Dice Spark »), Booster conservé en 2ᵉ thème ; accueil radial, écran « Nouvelle partie » (format puis joueurs), 10 formats Magic. Tests PC (ASan/UBSan), 32 + 26 captures, build ESP32-S3 sans avertissement. Validé le 2026-11-13 (sans matériel).

---

### [Issue: #8] Noms de format en Original Motion (Sucker Brush abandonnée) [PRIORITÉ: HAUTE] [Complexité: S] 🔵

Plan : [`done/8-police-sucker-brush.md`](done/8-police-sucker-brush.md)

Noms de format en Original Motion, comme le nom « Dice Spark » ; chiffres et autres titres en Barlow. Sucker Brush essayée puis retirée (« 1 » lu « I »). Polices sous licence hors dépôt, garde-fou de build. Tests PC (ASan/UBSan), 38 captures, build ESP32-S3 sans avertissement. Validé le 2026-11-14 (sans matériel).
