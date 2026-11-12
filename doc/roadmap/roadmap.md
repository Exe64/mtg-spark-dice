# Roadmap SparkDice

> Backlog actif. L'historique détaillé est archivé dans `_archives/`.
> Dernière mise à jour : 2026-11-12.

## Contexte

Backlog consolidé : plans nommés d'après leur issue GitHub (voir `doc/roadmap/`).
- « À faire » : travaux restants, triés par priorité.
- « Fait » : ce qui a été livré et validé.

---

## À faire

### Backlog (sans plan ni issue)

- **Validation matériel** — à réception des 2 boîtiers (2026-11-24 → 2026-12-10) : brochage `main/board.h`, init ST77916, crans de molette, haptique DRV2605, coût des calques tournés sur ESP32-S3
- **Lot 2** — Wi-Fi + API REST/JSON + page web de gestion (profils, réglages) + OTA/rollback + veille/batterie
- **Lot 3** — Multijoueur ESP-NOW (hôte/invité, code de salle, sync d'état) — nécessite ≥ 2 boîtiers
- **Lot 4** — Mode D&D, Arène 5–6 joueurs
- **Thèmes d'interface** — sélecteur de thème dans les réglages (NVS, puis page web du lot 2) et thèmes supplémentaires (Table de pod, Notation de loyauté, Signalétique aéroport, Grille Crouwel) sur la structure de thème du lot 1
- **Lot 5** — Application native iOS/Android sur l'API du lot 2

---

## Fait

### [Issue: #1] Lot 1 — MVP Solo + Arène locale [PRIORITÉ: HAUTE] [Complexité: L] 🔵

Plan : [`done/1-mvp-solo-arene.md`](done/1-mvp-solo-arene.md)

Firmware SparkDice : Solo + Arène 1–4 (sièges orientés), tours, attaque par glisser / dégâts de commandant, poison, annulation, journal, reprise NVS, profils JSON. Logique testée sur PC (ASan/UBSan), 20 captures, build ESP32-S3 sans avertissement. Validé le 2026-11-12 (sans matériel).
