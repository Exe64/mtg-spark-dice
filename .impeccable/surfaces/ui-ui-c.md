---
version: 1
slug: "ui-ui-c"
primary_target: "ui/ui.c"
related_targets: []
---

# Surface brief: SparkDice dial UI (firmware, LVGL)

Scope: every screen of the 360×360 round dial (Accueil, Solo, Arène 1–4, Menu de partie, Journal, Fiche joueur, Fin de partie; Réglages and Mise en route of plan #2 inherit it). Mode: Operate. Build path: code-led (no image generation). Audience and constraints: see PRODUCT.md (store/tournament, read by the whole table at 60–70 cm, never colour-only).

Two themes share one layout and one set of gestures: **Lumineux** (primary, below) and **Booster** (2nd theme, kept as shipped in plan #1). The selector is in plan #3.

## Direction contract

THESIS: The dial is the knob's well of light, seen from above, as in the logo: the hub is the light source and everything is lit from the centre outward. The pending delta is light gathering; the commit is the light reaching the rim. The active player is the one the light points at. Refuses: flat colour quadrants, thin progress ring on black, neon gamer HUD (no scanlines, no chrome bevels, no hexagon grids).

OWN-WORLD: Night-well ground, a radial gradient from #18214A at the centre to #060914 at the rim. The user picks the ground hue in the settings from six dark presets: Nuit (default), Noir, Améthyste, Forêt, Grenat and Ardoise. The rim tone is derived from the chosen hue, and every preset keeps white ink at 7:1 or more. White line ink #F4F7FF, the logo's language: round-capped strokes of 2 or 3 px, and line-art icons with no fill. Seat hues are light, never paint: J1 ember #FF7A45, J2 aqua #3FD8C0, J3 gold #FFC94D, J4 iris #A98BFF. Each tints its seat's glass (a 12–18 % lift) and its halo, and is always paired with J#. The knob ring is a thin white circle near the rim with 16 detent ticks, the logo's knurl. Glows are pre-rendered A8 halos tinted at draw time, with no runtime blur. Glass buttons are round, with a dark fill, a 1 px top highlight and a soft halo when focused. Typography: Barlow Condensed ExtraBold for numerals and SemiBold for labels, kept from plan #1; format names in Original Motion, the product name's face (plan #8). Numerals are white over a halo in the seat hue.

STORY: The player turns the knob and sees "−3" appear in white with the seat's light swelling behind it. The light runs once around the ring in 800 ms, then the total changes and the haptic ticks. The table reads every total at a glance, and sees who is playing because the light points at that seat.

FIRST VIEWPORT:
- **Solo:** night-well ground with the knob ring at R−10. The life numeral is about 150 px tall at the centre, white over a large halo in the seat hue, with "J1 · COMMANDER" above it. While a delta is pending, the delta sits in a glass pill under the numeral and a light arc runs clockwise along the knob ring in exactly 800 ms.
- **Arena:** radial glass sectors separated by 6 px gaps of ground, with no rules.
  - The selected seat's glass is brighter and carries its halo; every seat's numeral and label face its player (rotated layers, as in plan #1).
  - The hub is a glass disc that glows. At rest it shows TOUR n and the active J#; tap = next turn. While a delta is pending, its light ring closes clockwise over 800 ms. For 4 s after a commit it becomes an ANNULER pill.
  - The active seat receives three soft rays from the hub, the logo's rays: the signature.
  - A drag A→B draws J3 ▸ J1 in the hub; CMDT sits on top when it is commander damage.
  - A defeated seat goes dark: its glass and halo turn off, and the skull mark (ui/assets/dead.svg) appears in dimmed ink with J# VAINCU.
- **Chrome:** glass buttons with white line icons. Focus is a white halo, never a seat hue.

FORM: luminous well, pinned by the user from their reference (radial layout, glow and depth, dark bluish ground, icons), plus the owner's logo (ui/assets/logo.svg). There was no direction roll. seed: none (unavailable)

FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict, DESIGN.md, and every shipping raster carrying its provenance

## Theme 2: Booster (kept)

As shipped in plan #1 (DESIGN.md until plan #5 step 7 rewrites it): a sealed-booster wrapper. Seats are saturated fields (#E4572E, #179C8E, #F2B134, #8A6CD0) on #121014, with ink #FFF6E8. A foil band fills in 800 ms. Edges are straight 2 px rules, never zigzag. Chrome is ink-inverse. Same layout, gestures and skull mark.

## Constraints for the light

- Glows and rays are pre-rendered A8 masks; drawing them costs only alpha blits. The cost of a full-screen redraw on the ESP32-S3 is measured at step 7, then checked on hardware.
- Every text on glass reaches at least 4.5:1, and every numeral at least 7:1 (white on the night well is about 15:1).
- Halos never carry information alone: J#, position and rays always back them up.

## Open decisions
- Product name: SparkDice (working name) or DiceSpark, one constant.
- Settings frame, game and device sections, first-start assistant: shipped in plan #2 (DESIGN.md, Components). Rotation, sleep and timer flows: plans #3–#4.
