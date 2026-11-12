---
version: 1
slug: "ui-ui-c"
primary_target: "ui/ui.c"
related_targets: []
---

# Surface brief: SparkDice dial UI (firmware, LVGL)

Scope: every screen of the 360×360 round dial (Accueil, Solo, Arène 1–4, Menu de partie, Fiche joueur, Fin de partie). Mode: Operate. Build path: code-led (no image generation). Audience and constraints: see PRODUCT.md (store/tournament, read by the whole table at 60–70 cm, never colour-only).

## Direction contract

THESIS: The dial is a sealed booster. Each seat is a saturated wrapper field; the pending delta rides a foil band and the crimp seals it shut in exactly 800 ms, which is the commit. Refuses the category default: black dial, white number, thin progress ring, rainbow quadrants.

OWN-WORLD: Wrapper-interior black ground #121014. Four seat fields, vermilion #E4572E, teal #179C8E, gold #F2B134, violet #8A6CD0, each labelled J1–J4. Ink #FFF6E8 on ground, black #121014 on every seat field (all ≥4.5:1). Foil is the only gradient, silver #B9BEC6→#F3F4F6, used only for the pending band. Every edge between fields is a 2 px zigzag crimp. Barlow Condensed ExtraBold numerals, SemiBold uppercase labels.

STORY: The player turns the knob, sees "−3" in the foil band, watches the crimp close, feels the seal tick; the table reads every total at a glance.

FIRST VIEWPORT: Solo: full-bleed seat field, life numeral about 150 px tall at centre, "J1 · COMMANDER" above it. During a pending delta, the foil band crosses below the numeral and its crimp teeth close from left to right over 800 ms. Arena: radial sectors with crimped seams (filled teeth). The selected seat is filled; the others are black with their number in seat colour. At rest the hub is a sealed crimp ring. The dial faces its owner: numerals stay upright for J1 and opposite seats read it from the side, because one dial per player is the positioning and LVGL 9.2 cannot rotate label draws without per-frame transformed layers. A defeated total stays in full hue, crimped shut. Chrome (primary buttons, menu focus) is ink-inverse, never a seat hue.

FORM: booster wrapper, candidate 7 of round 1 (pinned by the user after a re-roll), seed: 31f96a16bbb7f208. Raises: crimp length = measured remaining time (Labanotation); the selected seat owns the field, the rest are numbers on black (Saville sleeve); strict tonal set, 4 seat hues + ink + ground + foil (darkroom zones); one rule weight, 2 px, everywhere (reference setting).

FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict, DESIGN.md, and every shipping raster carrying its provenance

## Open decisions
- Theme selector and additional themes: backlog (roadmap.md, "Thèmes d'interface").
