---
name: SparkDice
description: A dedicated MTG life-counter dial, drawn as a sealed booster wrapper on a 360×360 round LCD.
colors:
  wrapper-black: "#121014"
  wrapper-ink: "#FFF6E8"
  seat-vermilion: "#E4572E"
  seat-teal: "#179C8E"
  seat-gold: "#F2B134"
  seat-violet: "#8A6CD0"
  foil-silver: "#B9BEC6"
  foil-shine: "#F3F4F6"
typography:
  display-solo:
    fontFamily: "Barlow Condensed ExtraBold"
    fontSize: "200px"
    fontWeight: 800
    fontFeature: "'kern' 0"
  display-arena-2:
    fontFamily: "Barlow Condensed ExtraBold"
    fontSize: "112px"
    fontWeight: 800
    fontFeature: "'kern' 0"
  display-arena-3:
    fontFamily: "Barlow Condensed ExtraBold"
    fontSize: "92px"
    fontWeight: 800
    fontFeature: "'kern' 0"
  display-arena-4:
    fontFamily: "Barlow Condensed ExtraBold"
    fontSize: "80px"
    fontWeight: 800
    fontFeature: "'kern' 0"
  title:
    fontFamily: "Barlow Condensed ExtraBold"
    fontSize: "64px"
    fontWeight: 800
    fontFeature: "'kern' 0"
  headline-hub:
    fontFamily: "Barlow Condensed ExtraBold"
    fontSize: "48px"
    fontWeight: 800
    fontFeature: "'kern' 0"
  row:
    fontFamily: "Barlow Condensed SemiBold"
    fontSize: "30px"
    fontWeight: 600
    fontFeature: "'kern' 0"
  label:
    fontFamily: "Barlow Condensed SemiBold"
    fontSize: "24px"
    fontWeight: 600
    fontFeature: "'kern' 0"
  label-small:
    fontFamily: "Barlow Condensed SemiBold"
    fontSize: "20px"
    fontWeight: 600
    fontFeature: "'kern' 0"
rounded:
  field: "0px"
  hub: "48px"
spacing:
  rule: "2px"
  action-height: "46px"
  seat-chip: "36px"
  chip-pitch: "44px"
  log-row: "44px"
  menu-row: "52px"
components:
  button-primary:
    backgroundColor: "{colors.wrapper-ink}"
    textColor: "{colors.wrapper-black}"
    typography: "{typography.row}"
    rounded: "{rounded.field}"
    height: "{spacing.action-height}"
  button-secondary:
    backgroundColor: "{colors.wrapper-black}"
    textColor: "{colors.wrapper-ink}"
    typography: "{typography.label}"
    rounded: "{rounded.field}"
    height: "{spacing.action-height}"
  menu-row:
    backgroundColor: "{colors.wrapper-black}"
    textColor: "{colors.wrapper-ink}"
    typography: "{typography.row}"
    height: "{spacing.menu-row}"
  menu-row-focus:
    backgroundColor: "{colors.wrapper-ink}"
    textColor: "{colors.wrapper-black}"
    typography: "{typography.row}"
    height: "{spacing.menu-row}"
  seat-chip-on:
    backgroundColor: "{colors.seat-vermilion}"
    textColor: "{colors.wrapper-black}"
    typography: "{typography.label-small}"
    rounded: "{rounded.field}"
    size: "{spacing.seat-chip}"
  pending-band:
    backgroundColor: "{colors.foil-silver}"
    textColor: "{colors.wrapper-black}"
    typography: "{typography.row}"
    rounded: "{rounded.field}"
    height: "46px"
  hub-seal:
    backgroundColor: "{colors.wrapper-black}"
    textColor: "{colors.wrapper-ink}"
    rounded: "{rounded.hub}"
    size: "96px"
  hub-pending:
    backgroundColor: "{colors.foil-silver}"
    textColor: "{colors.wrapper-black}"
    typography: "{typography.headline-hub}"
    rounded: "{rounded.hub}"
    size: "96px"
  hub-undo:
    backgroundColor: "{colors.wrapper-ink}"
    textColor: "{colors.wrapper-black}"
    typography: "{typography.row}"
    rounded: "{rounded.hub}"
    size: "96px"
  undo-bar-solo:
    backgroundColor: "{colors.wrapper-ink}"
    textColor: "{colors.wrapper-black}"
    typography: "{typography.label}"
    rounded: "{rounded.field}"
    width: "160px"
    height: "36px"
---

# Design System: SparkDice

## Overview

**Creative North Star: "The Sealed Booster"**

The dial is a booster wrapper. Each seat is a saturated wrapper field on a wrapper-interior black ground. A pending change rides a foil band, and a crimp closes over it in exactly 800 ms; when the crimp is closed, the change is committed. This one metaphor governs every surface: colour fields, a single foil, and the crimped seal as the only ornament and the only progress indicator.

Density is low and the type is loud. Every screen has one huge condensed numeral or title, a single line of uppercase labels, and nothing else. Hierarchy comes from size and field fill, not from tints, shadows or opacity. The build is drawn in code (LVGL 9 draw calls into one full-screen object), so the system consists of flat fills, 2 px lines, filled triangles, one horizontal gradient and two TTF faces; in the arena each seat is drawn into its own layer and turned to face its player. The dial is round (360×360) and every layout is centred on (180, 180).

Rejected (from the contract): the default life-counter look of a black dial, a white number, a thin progress ring and rainbow quadrants. Also rejected by the product record: any copy of Soul Dial branding, and any Wizards of the Coast logos, mana symbols or card art.

**Key Characteristics:**
- A strict tonal set: ground, ink, four seat hues, foil. No tints, no alpha fades.
- The crimp (a zigzag or toothed seal) marks every seam, commit, focus, defeat and the active turn.
- One rule weight (2 px) for every line, outline, crimp and arc.
- Condensed ExtraBold numerals and SemiBold uppercase labels, kerning off.
- The foil gradient appears only while something is pending.
- A seat is never identified by hue alone: every seat carries its J1–J4 label.
- Every arena seat faces its player; the hub at the centre is the turn marker, the undo pill and the pending seal.

## Colors

The palette is wrapper black plus four saturated seat hues, a warm ink and one silver foil. Each colour has exactly one job.

### Primary
- **Seat Vermilion** (J1), **Seat Teal** (J2), **Seat Gold** (J3), **Seat Violet** (J4): the seat fields, fixed in that order. When a seat is selected (or in Solo), its hue fills the whole field and the text on it is Wrapper Black. When a seat is not selected, or has been defeated, its hue becomes the colour of its numeral and label on Wrapper Black. Against the ground every seat hue is ≥4.5:1 (vermilion 5.1, teal 5.6, gold 10.0, violet 4.6), so the hues work in both roles.

### Secondary
- **Foil Silver** / **Foil Shine**: the pending state, and nothing else. The foil is a 3-stop horizontal gradient, silver to shine to silver, with the shine stop at 150/255 so the highlight sits slightly right of centre. Text on foil is Wrapper Black.

### Neutral
- **Wrapper Black**: the ground of every screen. It is also the text colour on every seat field and on foil, and the sealed hub.
- **Wrapper Ink**: text and rules on the ground, every seam crimp, the hub seal, the active-seat crest (except on the selected field, where it is the field's Wrapper Black), and all chrome fills (the primary button, menu focus, the hub undo pill and the Solo undo bar).

### Named Rules
**The Seat Owns Its Hue Rule.** Seat hues mark seats only: fields, numerals, the J-labels, the active seat's J# in the resting hub, home seat chips and the 14 px seat swatch in the log. Chrome (buttons, menu focus, focus marks, the undo pill, the active-seat crest) is never drawn in a seat hue.

**The One Foil Rule.** The foil is the only gradient in the system, and it shows only while a delta is pending: the Solo and Fiche bands and the pending hub. When nothing is pending, no foil is on screen.

**The No Fade Rule.** Everything is drawn at full opacity. Defeat, inactivity and secondary state are shown by dropping the field fill, or with a crimp, never by lowering alpha.

## Typography

**Display Font:** Barlow Condensed ExtraBold (embedded TTF, OFL)
**Label Font:** Barlow Condensed SemiBold (embedded TTF, OFL)

**Character:** Condensed grotesque used like the print on a wrapper: tall, tight and loud. Narrow glyphs keep a three-digit total inside a 90° sector. Both faces are rendered by LVGL tiny_ttf with kerning off: the GPOS table is hidden from stb_truetype (ui/assets/strip_gpos.py), and Barlow has no kern table.

### Hierarchy
- **Display Solo** (800, 200 px): the single life total in Solo, centred at y 158.
- **Display Arena** (800, 112 / 92 / 80 px for 2 / 3 / 4 players): seat totals, centred on the seat axis at radius 96 / 104 / 108.
- **Title** (800, 64 px): the home profile name, the Fiche seat number, and "J1 GAGNE" / "DÉFAITE".
- **Headline Hub** (800, 48 px): the pending delta in the hub, the active seat's J# in the resting hub, and the poison tally on the Fiche.
- **Row** (600, 30 px, uppercase): menu rows, primary button text, foil-band text, the undo pill's delta or hand-off ("−3", "J1 › J2"), and commander tallies.
- **Label** (600, 24 px, uppercase): the seat/profile line ("J1 · COMMANDER"), seat labels with counters, section labels (POISON, CMDT, JOURNAL), journal rows, the Solo undo bar, and secondary button text.
- **Label Small** (600, 20 px, uppercase): home sub-lines, seat-chip numbers, hub tags (TOUR n, CMDT, ANNULER), hub sub-lines ("= 34", "J3 ▸ J1") and the "TOUCHER POUR REVENIR" hint.

### Named Rules
**The Real Minus Rule.** Negative deltas use U+2212 (−), not a hyphen. Positive deltas always show "+".

**The All Caps Rule.** Every label and row is uppercase French, taken from ui/strings_fr.h. Beyond letters and numerals, text carries only signed deltas and the separators "·", "=", "/" and "›".

**The Drawn Pointer Rule.** In text rows (journal, menu, undo pill) a hand-off or attack is written with U+203A as the separator ("J3 › J1"). In the pending hub the pointer is a drawn filled triangle between the two J#, never a glyph. The +/− beside a selected total are drawn 2 px marks (18 px), never glyphs.

## Layout

The layout is radial, on a 360×360 round canvas centred at (180, 180), radius 180. In the arena, the circle is split into equal sectors (360/n degrees). J1 sits at the bottom (90°) and the other seats follow clockwise. Each sector holds its total on the seat axis. A hub of radius 48 sits at the centre, and seat hit-testing excludes it. Solo, Fiche and Fin de partie are vertical stacks on the centre line, upright.

Every arena seat faces its player. Each seat is drawn upright in its own frame, centred on its numeral point (nx, ny) at the numeral radius, then turned by (seat angle − 90°): J1 at the bottom is drawn directly, every other seat goes through an ARGB8888 layer (300 px wide for 2 players, 190 px for 3–4, line height + 44 px tall). In that frame the numeral sits at ny − 8 and the label line on the rim side at ny + lh/2 − 8, just inside the crest. The +/− marks flank the selected numeral at ny − 6, ±110 px (2 players) or ±64 px (3–4), and the −/+ tap zones are |local x| > 26 px, measured in the same seat frame. Solo has no turn: its marks sit at ±136 px on y 162, and its −/+ tap zones are x < 110 / x ≥ 250.

Rhythm is set by fixed pitches, not by a spacing scale. Action bars are 46 px tall (y 258–304 on Home and Fin de partie). Menu rows have a 52 px pitch, starting at y 76. Log rows have a 44 px pitch, starting at y 104. Home seat chips are 36 px squares at a 44 px pitch. Inset content keeps clear of the round bezel: menu focus spans x 36–323, log rows x 70–310, Fin de partie buttons x 70–290, the Solo undo bar x 100–260 (y 282–318).

**The Seats Face Their Players Rule.** An arena seat's numeral, label, +/− marks and tap zones all live in that seat's frame, turned by (seat angle − 90°), with the label on the rim side. Never lay out a seat element in screen coordinates.

## Elevation & Depth

The system is completely flat, with no shadows, glows or blur. Depth comes from field versus ground: a filled seat hue sits forward, and a seat that is only a numeral on black sits back. The foil gradient is the only implied material sheen. Edges between planes are crimps, not shadows.

### Named Rules
**The Crimp Not Shadow Rule.** Separate planes with a crimped seam in the rule weight. Never use a drop shadow or an offset.

## Shapes

Fields are square-cornered (0 px) and full-bleed, and the round bezel does the clipping. The hub is the only circle (radius 48). Every line, outline, crimp and arc is 2 px. The seal has three forms, all taken from the crimp:
- **Line crimp**: a 2 px zigzag with a 8 px tooth and 4 px amplitude, the last point snapped back to the line. Used for horizontal seals, focus underlines and defeat strikes.
- **Toothed seam**: a 2 px straight line plus filled triangles, 10 px tooth, 5 px depth. Used for the radial arena seams, where a 2 px zigzag aliases into a staircase at 45°.
- **Arc crimp**: a 2 px zigzag around a circle, one point every 7°, alternating outward by 4 px. Used for the hub seal.
- **Crest**: filled teeth along the rim pointing inward, one every 6°, base on radius R − 4 (176 px) and tip at R − 10 (170 px), inset 6° from each seam. Used only to mark the active seat.

**The One Rule Weight Rule.** 2 px everywhere. No hairlines and no heavier strokes.

## Components

### Buttons
- **Shape:** square rectangles (0 px), 46 px tall.
- **Primary:** an ink-inverse fill (Wrapper Ink fill, Wrapper Black Row text). With a saved game, REPRENDRE is primary and JOUER becomes secondary, because JOUER would replace the saved game.
- **Secondary:** a 2 px outline in ink with Label text in ink, on the ground.
- **On a seat field (Fin de partie):** the "ink" is the field's own text colour, Wrapper Black. ANNULER is a 2 px black outline. NOUVELLE PARTIE is a solid black bar, crimped top and bottom in the field hue, with its text in the field hue.
- **Focus:** there is no hover. The knob-driven focus on Home is a 140 px line crimp in ink under the active row.

### Seat chips (Home)
- **Style:** 36 px squares, each labelled J1–J4. In-game seats are filled with their hue and carry black text. Unused seats are a 2 px outline in their hue, with their label in that hue.

### Menu (Menu de partie)
- **Style:** five centred Row-text rows at a 52 px pitch. The focused row is an ink-inverse bar (ink fill, black text, 47 px tall, x 36–323).
- **Undo row:** previews the action it will undo ("ANNULER  J1  PV −3"; for an automatic pass, the hit that caused it), or reads "RIEN À ANNULER" when there is nothing to undo.

### Seat field and numeral (signature)
- **Selected:** the seat hue fills the sector (a full-radius arc) or the whole screen (Solo), with black numeral and label.
- **Unselected:** the numeral and label are in the seat hue on the ground.
- **Defeated:** the seat is never filled. The numeral stays in full seat hue, and an 88 px ink line crimp is drawn across it at mid-height. The label reads "J2  VAINCU". The total is crimped shut, never faded.
- **Counters:** these appear on the label line only when non-zero: "POISON 3", "CMDT 5", or "P3  C12" when both apply.

### Pending band (signature)
- **Style:** a full-width foil band (46 px in Solo at y 250–296; 32 px on the Fiche at y 212–244), with black Row text "−3  = 37".
- **Behaviour:** the top and bottom edges start as straight 2 px lines. Line crimps close over them from left to right, in step with the remaining time, and finish at exactly 800 ms. That is the commit.

### Active-seat crest (signature)
- **Style:** the crest form along the active seat's rim. Black (the field's Wrapper Black) when the active seat is also the selected, filled seat; Wrapper Ink otherwise.
- **Behaviour:** moves clockwise with Next, skipping defeated seats. Solo has no crest and no turn.

### Hub seal (signature)
- **At rest:** a black disc with a closed arc crimp in ink. Everything is committed. It shows "TOUR n" (Label Small, ink) over the active seat's J# (Headline Hub, in that seat's hue). Tap = next turn.
- **Undo pill (4 s after any commit):** the disc turns ink-inverse (Wrapper Ink fill, no crimp) with "ANNULER" (Label Small) over the change it reverses in Row: the delta as its effect on life, or "J1 › J2" after a pass. When a hit defeated the active player and passed the turn automatically, the pill shows that hit; one tap reverts both. Tap = undo.
- **Pending:** the disc turns foil and shows the delta in Headline Hub as its effect on life (commander damage reads −5), with "= total" below it. Every drag A→B is an attack and shows "J3 ▸ J1" (source, drawn triangle, target) instead; commander damage adds a CMDT tag above the delta. The arc crimp sweeps clockwise from 12 o'clock over 800 ms, and a plain 2 px arc covers the rest of the circle. A hub tap toggles attack ↔ commander damage (restarting the seal), or commits a non-attack delta immediately.

**The Hub-Only Repaint Rule.** Arena seats show committed values. While only the pending value, the seal or the pill changes, invalidate the hub box (radius 48 + 6 px) and never the turned seat layers.

### Solo undo bar
- **Style:** an ink-inverse bar (x 100–260, y 282–318, 0 px) reading "ANNULER  −3" in Label, shown for 4 s after a commit in place of the arena's pill. It never coexists with the foil band.

### Fiche joueur
- **Header:** the seat hue fills the top band to y 132, with a black line crimp at its edge and the seat number in Title.
- **Poison:** a POISON label, then "n / 10" in Headline Hub.
- **CMDT:** one column per opposing seat, at an 84 px pitch: the seat label in its hue (Label Small) over the tally (Row). The tally shows "n / 21" once it is within 5 of lethal.

### Journal
- **Style:** the JOURNAL label over a 120 px crimp. Rows show a 14 px seat-hue swatch, then the action in Label text, left-aligned at x 96, newest first at a 44 px pitch. Action rows carry the turn prefix ("T1  J4  POISON +3", "T1  J3 › J1  PV −5"); turn rows read "TOUR 1  J1 › J2", with the swatch of the seat that passed. The swatch always comes with the "J#" text.

## Do's and Don'ts

### Do:
- **Do** fill the selected seat, and only the selected seat, with its hue. Draw all other totals in their hue on Wrapper Black.
- **Do** draw every edge, seam, focus mark, seal and defeat strike as a crimp in the 2 px rule weight.
- **Do** show pending state on foil, with the crimp length equal to the remaining time to commit (800 ms, linear).
- **Do** pair every seat hue with its J1–J4 label.
- **Do** use ink-inverse fills (or the field's own text colour on a seat field) for primary actions and focus; the hub undo pill and the Solo undo bar are always Wrapper Ink.
- **Do** take every user-facing string from ui/strings_fr.h, in uppercase.
- **Do** draw each arena seat in its own frame, turned by (seat angle − 90°), with the label on the rim side.
- **Do** keep the hub the single place for turn, undo and pending state; repaint only the hub while only it changes.

### Don't:
- **Don't** use a seat hue for chrome, buttons, menu focus, the undo pill or the crest.
- **Don't** add a second gradient, or show foil when nothing is pending.
- **Don't** fade, dim or lower the opacity of a defeated or inactive seat. Crimp it.
- **Don't** add shadows, glows or offset depth.
- **Don't** use strokes other than 2 px.
- **Don't** use a hyphen for minus, or a glyph for the hub pointer or the +/− marks.
- **Don't** fall back to the category default (black dial, white number, thin progress ring, rainbow quadrants), and don't copy Soul Dial branding or use WotC marks, mana symbols or card art.

## Open Questions

- **Layer cost on the ESP32-S3.** Each turned seat is an ARGB8888 layer redrawn on every full repaint (up to three per frame in a 4-player game). The Hub-Only Repaint Rule keeps pending updates off them, but the cost on the device has not been measured.
- **Themes.** Only the booster theme ships (ui_theme_booster). A theme selector is in the backlog. Per-theme signature drawing (the crimp) is still hard-coded in ui.c.
