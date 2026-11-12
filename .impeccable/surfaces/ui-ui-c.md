---
version: 1
slug: "ui-ui-c"
primary_target: "ui/ui.c"
related_targets: []
---

# Surface brief: SparkDice dial UI (firmware, LVGL)

Scope: every screen of the 360×360 round dial (Accueil, Solo, Arène 1–4, Menu de partie, Fiche joueur, Fin de partie). Mode: Operate. Build path: code-led (no image generation). Audience and constraints: see PRODUCT.md (store/tournament, read by the whole table at 60–70 cm, never colour-only).

## Direction contract

THESIS: The dial is a sealed booster. Each seat is a saturated wrapper field; the pending delta rides a silver band that the foil sheen fills from left to right in exactly 800 ms, which is the commit. Every edge is smooth: the user rejected zigzag crimps (2026-11-12). Refuses the category default: black dial, white number, thin progress ring, rainbow quadrants.

OWN-WORLD: Wrapper-interior black ground #121014. Four seat fields, vermilion #E4572E, teal #179C8E, gold #F2B134, violet #8A6CD0, each labelled J1–J4. Ink #FFF6E8 on ground, black #121014 on every seat field (all ≥4.5:1). Foil is the only gradient, silver #B9BEC6→#F3F4F6, used only for the pending band. Every edge between fields is a straight 2 px rule; no zigzag anywhere. Barlow Condensed ExtraBold numerals, SemiBold uppercase labels.

STORY: The player turns the knob, sees "−3" in the foil band, watches the sheen fill the band, feels the seal tick; the table reads every total at a glance.

FIRST VIEWPORT: Solo: full-bleed seat field, life numeral about 150 px tall at centre, "J1 · COMMANDER" above it. During a pending delta, the foil band crosses below the numeral and the foil sheen fills it from left to right over 800 ms (bright leading edge on dull silver; edges always complete). Arena: radial sectors with straight 2 px seams. The selected seat is filled; the others are black with their number in seat colour. At rest the hub is a closed 2 px ring; while a delta is pending the ring closes clockwise over 800 ms. Every seat faces its player: each seat's numeral and label are drawn in a layer turned by (seat angle − 90°), label on the rim side; the +/− marks flank the selected numeral in the seat's own frame. The active seat carries a solid 6 px arc along its rim; at rest the hub shows TOUR n and the active seat (tap = next turn); for 4 s after a commit the hub is an ink-inverse ANNULER pill. A drag A→B is an attack drawn in the hub as J3 ▸ J1 (drawn triangle); CMDT tops the hub when it is commander damage. A defeated seat shows the owner's skull mark (ui/assets/dead.svg, A8 mask tinted with the seat hue) instead of its total (the total stays in the log; undo restores it), labelled J# VAINCU. Chrome (primary buttons, menu focus) is ink-inverse, never a seat hue.

FORM: booster wrapper, candidate 7 of round 1 (pinned by the user after a re-roll), seed: 31f96a16bbb7f208. Raises: crimp length = measured remaining time (Labanotation); the selected seat owns the field, the rest are numbers on black (Saville sleeve); strict tonal set, 4 seat hues + ink + ground + foil (darkroom zones); one rule weight, 2 px, everywhere (reference setting).

FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict, DESIGN.md, and every shipping raster carrying its provenance

## Open decisions
- Theme selector and additional themes: backlog (roadmap.md, "Thèmes d'interface").
