# Product

<!-- impeccable:product-schema 1 -->

## Platform

embedded

(Not one of web/ios/android/adaptive: firmware UI rendered by LVGL 9 on a 360×360 round 1.8" LCD, ESP32-S3. No browser, no OS design language.)

## Users

Magic: The Gathering players in a game store or tournament setting (FNM, 1v1 rounds, also multiplayer pods). Fast pace, noisy room, strong overhead light. Each player tracks life totals during a match.

## Product Purpose

SparkDice is a dedicated life counter: a rotary knob + touch round screen that replaces the phone app or the dice on the table. Success = life changes are entered in one gesture, never mis-committed, and every player at the table can read the totals at a glance.

## Positioning

- A dedicated object: no phone on the table, the physical gesture of the knob, haptic detents.
- One dial per player (later lot: several boxes synced over ESP-NOW).
- A table accessory people want to bring out, like sleeves or a playmat.

## Operating Context

- Dial placed on the table in front of a player; totals must be legible by the whole table at ~60–70 cm.
- Knob: accumulated delta, committed after 800 ms of inactivity. Touch: tap selects a player (tap either side of the selected number = −/+), drag A→B = an attack (combat damage; a hub tap turns it into commander damage), tap on the hub at rest = next turn, long press = menus.
- Turn tracking: active player + Next (clockwise, defeated players skipped, undoable); the journal notes the turn.
- Formats: Standard profile (20 life, 2 players) and Commander profile (40 life, 4 players, commander damage lethal at 21). Poison lethal at 10.
- Hardware: Waveshare ESP32-S3-Knob-Touch-LCD-1.8 (2 units ordered, expected 2026-11-24 → 2026-12-10).

## Capabilities and Constraints

- MVP: Home (profile, player count), Solo (giant life + pending delta), Arena 1–4 players, game menu (undo, log, new game), player sheet (poison, commander damage), game over. Undo, NVS game resume.
- UI strings in French, centralized in one file.
- Out of scope for now: radio, OTA, web/API, D&D mode, 5–6 players, sleep/battery.
- Embedded constraints: LVGL bitmap/TTF fonts, RGB565 on target, limited animation budget.

## Brand Commitments

- Name: SparkDice. Original identity: must not copy Soul Dial branding or visuals.
- Wizards of the Coast materials (mana and set symbols, iconography, game terms) may be used, owner decision 2026-11-12; the defeat mark `ui/assets/dead.svg` resembles the black mana symbol. Fact to keep in mind, not a restriction: Wizards' Fan Content Policy covers free, non-commercial fan content; selling the device or firmware with these assets would need a licence from Wizards.

## Product Principles

1. Legible from across the table beats decorative.
2. One gesture per change; the pending value is always visibly distinct from the committed one.
3. Nothing irreversible without Undo.
4. A player is never identified by color alone.

## Accessibility & Inclusion

- Life totals readable at 60–70 cm by every player under bright store light; each seat faces its player.
- Players distinguished by position + label/number, not by hue only (color-blind safe).
