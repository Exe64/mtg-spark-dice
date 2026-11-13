---
name: SparkDice
description: A dedicated MTG life-counter dial on a 360×360 round LCD, drawn as a well of light (Lumineux, default) or as a sealed booster wrapper (Booster, theme 2).
colors:
  well-centre: "#18214A"
  well-rim: "#060914"
  light-ink: "#F4F7FF"
  light-ember: "#FF7A45"
  light-aqua: "#3FD8C0"
  light-gold: "#FFC94D"
  light-iris: "#A98BFF"
  ground-noir: "#14151A"
  ground-amethyste: "#2B1A4A"
  ground-foret: "#12382B"
  ground-grenat: "#4A1826"
  ground-ardoise: "#28323F"
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
  pill: "9999px"
  hub: "48px"
  field: "0px"
spacing:
  hairline: "1px"
  rule: "2px"
  knob-ring-inset: "10px"
  action-height: "46px"
  seat-chip: "36px"
  chip-pitch: "44px"
  log-row: "44px"
  menu-row: "52px"
components:
  glass-pill:
    backgroundColor: "{colors.well-rim}"
    textColor: "{colors.light-ink}"
    typography: "{typography.row}"
    rounded: "{rounded.pill}"
    height: "{spacing.action-height}"
  glass-pill-lit:
    backgroundColor: "{colors.well-rim}"
    textColor: "{colors.light-ink}"
    typography: "{typography.row}"
    rounded: "{rounded.pill}"
    height: "{spacing.action-height}"
  menu-row:
    textColor: "{colors.light-ink}"
    typography: "{typography.row}"
    height: "{spacing.menu-row}"
  menu-row-focus:
    backgroundColor: "{colors.well-rim}"
    textColor: "{colors.light-ink}"
    typography: "{typography.row}"
    rounded: "{rounded.pill}"
    height: "47px"
  seat-chip-lit:
    backgroundColor: "{colors.light-ember}"
    textColor: "{colors.light-ink}"
    typography: "{typography.label-small}"
    rounded: "{rounded.pill}"
    size: "{spacing.seat-chip}"
  seat-glass:
    backgroundColor: "{colors.light-ember}"
    textColor: "{colors.light-ink}"
    typography: "{typography.display-arena-4}"
  pending-pill:
    backgroundColor: "{colors.well-rim}"
    textColor: "{colors.light-ink}"
    typography: "{typography.row}"
    rounded: "{rounded.pill}"
    width: "168px"
    height: "44px"
  hub-glass:
    backgroundColor: "{colors.well-rim}"
    textColor: "{colors.light-ink}"
    typography: "{typography.headline-hub}"
    rounded: "{rounded.hub}"
    size: "96px"
  hub-undo:
    backgroundColor: "{colors.well-rim}"
    textColor: "{colors.light-ink}"
    typography: "{typography.row}"
    rounded: "{rounded.hub}"
    size: "96px"
  booster-button-primary:
    backgroundColor: "{colors.wrapper-ink}"
    textColor: "{colors.wrapper-black}"
    typography: "{typography.row}"
    rounded: "{rounded.field}"
    height: "{spacing.action-height}"
  booster-button-secondary:
    backgroundColor: "{colors.wrapper-black}"
    textColor: "{colors.wrapper-ink}"
    typography: "{typography.label}"
    rounded: "{rounded.field}"
    height: "{spacing.action-height}"
  booster-menu-row-focus:
    backgroundColor: "{colors.wrapper-ink}"
    textColor: "{colors.wrapper-black}"
    typography: "{typography.row}"
    height: "{spacing.menu-row}"
  booster-seat-chip-on:
    backgroundColor: "{colors.seat-vermilion}"
    textColor: "{colors.wrapper-black}"
    typography: "{typography.label-small}"
    rounded: "{rounded.field}"
    size: "{spacing.seat-chip}"
  booster-pending-band:
    backgroundColor: "{colors.foil-silver}"
    textColor: "{colors.wrapper-black}"
    typography: "{typography.row}"
    rounded: "{rounded.field}"
    height: "46px"
  booster-hub-seal:
    backgroundColor: "{colors.wrapper-black}"
    textColor: "{colors.wrapper-ink}"
    rounded: "{rounded.hub}"
    size: "96px"
  booster-hub-pending:
    backgroundColor: "{colors.foil-silver}"
    textColor: "{colors.wrapper-black}"
    typography: "{typography.headline-hub}"
    rounded: "{rounded.hub}"
    size: "96px"
  booster-hub-undo:
    backgroundColor: "{colors.wrapper-ink}"
    textColor: "{colors.wrapper-black}"
    typography: "{typography.row}"
    rounded: "{rounded.hub}"
    size: "96px"
  booster-undo-bar-solo:
    backgroundColor: "{colors.wrapper-ink}"
    textColor: "{colors.wrapper-black}"
    typography: "{typography.label}"
    rounded: "{rounded.field}"
    width: "160px"
    height: "36px"
---

# Design System: SparkDice

## Overview

**Creative North Star: "The Well of Light"**

The dial is the knob's well of light seen from above, as in the logo. The hub at the centre is the light source and everything is lit from the centre outward: a dark ground that falls from a bluish centre to a near-black rim, white ink, and seat hues that exist only as light (a tint on the seat's glass, a soft halo behind its numeral, the colour of its label). A pending change is light gathering: an arc of white light runs once around the ring in exactly 800 ms, and when it closes the change is committed. The active player is the one the light leans toward: a soft glow on the hub's edge that leans toward that seat. This is the Lumineux theme, the default (`UI_THEME_DEFAULT`).

Two themes share one layout and one set of gestures. Booster, the wrapper theme shipped first, is kept as theme 2: saturated seat fields on wrapper black, a silver foil that seals a pending change, straight 2 px rules, no light. Each canonical section below describes Lumineux first, then what Booster does differently under "Theme 2: Booster". A rule stated for one theme binds that theme only; rules marked shared bind both. `ui_set_theme(theme, ground)` switches at run time and the firmware reads both values at start; the settings screen that lets the user choose is not built yet.

Density is low and the type is loud. Every screen has one huge condensed numeral or title, one line of uppercase labels, and nothing else. Everything is drawn in code (LVGL 9.2 draw calls into one full-screen object on a 360×360 round LCD), and every layout is centred on (180, 180). The light is baked: the ground, the halos, the ring, the ray and the icons are alpha masks tinted at draw time, and no blur is computed per frame.

Rejected by the direction contract: flat colour quadrants, a thin progress ring on black, and the neon gamer HUD (scanlines, chrome bevels, hexagon grids). Rejected by the user (2026-11-12): any zigzag or crimped edge. The product record takes the spirit of a third-party dial app without copying its colours, icons, logo or composition. Wizards of the Coast materials are allowed by the owner (PRODUCT.md, Brand Commitments).

**Key Characteristics:**
- Two themes, one layout, one set of gestures. Lumineux is the default; Booster is theme 2.
- Lumineux: a radial dark ground, white ink, and seat hues used as light on glass, never as paint.
- The hub is the light source: turn marker, pending value and undo all live there, and its glow leans toward the active seat.
- Pending is always an arc or band whose drawn extent is the elapsed share of the 800 ms commit delay.
- Condensed ExtraBold numerals and SemiBold uppercase labels, kerning off, in both themes.
- A seat is never identified by hue alone: every seat carries its J1–J4 label and its position.
- Every arena seat faces its player.
- All light is pre-computed alpha masks; nothing is blurred at run time.

## Colors

Lumineux is a dark radial ground, one white ink and four seat lights. Booster is wrapper black, a warm ink, four saturated seat paints and one silver foil.

### Primary
- **Ember** (J1), **Aqua** (J2), **Gold Light** (J3), **Iris** (J4) (`light-ember`, `light-aqua`, `light-gold`, `light-iris`): the seat lights, fixed in that order. A seat hue tints its arena sector (36/255 opacity at rest, 46/255 when selected, with a full-hue band on the rim), tints its halo, and colours its label when the seat is not selected. It also colours the active J# in the resting hub, the two J# of a pending attack, the section labels and column labels of the player sheet, the lit home chips, and the point of light at the start of a journal row. Numerals are never drawn in a seat hue. On the bare ground every seat hue is at least 4.8:1 on every ground preset (computed: 6.0 / 8.7 / 10.1 / 5.8 on Nuit).

### Neutral
- **Well Centre** and **Well Rim** (`well-centre`, `well-rim`): the default ground, Nuit, a radial gradient from the centre colour at (180, 180) to the rim colour at radius 180. The rim colour is also the fill of every glass surface (pills, hub) and of the screen outside the dial.
- **Ground presets** (`ground-noir`, `ground-amethyste`, `ground-foret`, `ground-grenat`, `ground-ardoise`; Nuit is `well-centre`): the six centre hues the user may choose from, index 0 (Nuit) being the default. Only the centre is stored; the rim is derived.
- **Light Ink** (`light-ink`): every numeral, every piece of chrome text, the +/− marks, the knob ring, the rays, the pending arcs, the icons and the logo. Opacity steps in use: 100 % (content), 80 % (lit pill border, home sub-line), 70 % (rays, unfocused menu rows, hints, resting hub glow), 60 % (a defeated seat's skull and label), 40 % (knob ring, journal rule), 30 % (unlit glass border, hub glow while pending), 20 % (the halo behind the boot logo).

### Named Rules
**The Hue Is Light Rule.** In Lumineux a seat hue is never an opaque field. It is a translucent tint on glass, a halo, or the colour of a label. Numerals and chrome stay in Light Ink.

**The Seat Owns Its Hue Rule.** Shared. Seat hues mark seats only. Chrome (buttons, menu focus, focus marks, the undo pill, the pending arc, the rays) is never drawn in a seat hue; in Lumineux focus is white light.

**The Dark Ground Rule.** Every ground preset keeps Light Ink at 7:1 or more on the bare ground; `host/shots.c` asserts it for all six (computed: 12.0 to 17.0 at the centre, 18 or more at the rim). The rim is 27 % of each channel of the centre, rounded, which maps the Nuit centre exactly onto Well Rim. A new preset must pass the same assertion.

**The Low Light Under Numerals Rule.** Light behind a white numeral is kept low so the numeral stays readable: Solo halo at 40 %, the selected seat's glass at 46/255 (36/255 unselected), its halo at 20 %, the sheet and game-over halos at 30 %. The target is 7:1 for numerals and 4.5:1 for text on glass. Do not raise these values to make a seat look brighter.

### Theme 2: Booster
- **Seat Vermilion** (J1), **Seat Teal** (J2), **Seat Gold** (J3), **Seat Violet** (J4): the seat paints. A selected seat (or Solo) is filled with its hue and its text is Wrapper Black. An unselected or defeated seat shows its numeral and label in its hue on Wrapper Black. Against the ground every hue is at least 4.5:1 (5.1 / 5.6 / 10.0 / 4.6).
- **Foil Silver** / **Foil Shine**: the pending state and nothing else. On the hub, a 3-stop horizontal gradient (silver, shine at 150/255, silver). On the band, solid silver that a 2-stop gradient (silver to shine) fills from the left; the bright leading edge is the timer (user decision, 2026-11-12). Text on foil is Wrapper Black.
- **Wrapper Black**: the flat ground of every screen, the text colour on seat fields and on foil, and the sealed hub.
- **Wrapper Ink**: text and rules on the ground, every seam, the hub ring, the active-seat crest (Wrapper Black when the crest sits on the selected field), and all chrome fills.

**The One Foil Rule.** Booster only. The foil is Booster's only gradient and it shows only while a delta is pending.

**The No Fade Rule.** Booster only. Everything is drawn at full opacity. Defeat is shown by dropping the field fill and replacing the total with the skull mark in full seat hue, never by lowering alpha. (Lumineux does the opposite on purpose: a defeated seat goes dark.)

## Typography

**Display Font:** Barlow Condensed ExtraBold (embedded TTF, OFL)
**Label Font:** Barlow Condensed SemiBold (embedded TTF, OFL)

**Character:** A condensed grotesque, tall, tight and loud. Narrow glyphs keep a three-digit total inside a 90° sector. Both themes use the same two faces at the same nine sizes, rendered by LVGL tiny_ttf with kerning off: the GPOS table is hidden from stb_truetype (`ui/assets/strip_gpos.py`), and Barlow has no kern table.

### Hierarchy
- **Display Solo** (800, 200 px): the single life total in Solo, centred at y 158.
- **Display Arena** (800, 112 / 92 / 80 px for 2 / 3 / 4 players): seat totals, centred on the seat axis at radius 96 / 104 / 108.
- **Title** (800, 64 px): the home profile name, the seat number on the player sheet, and "J1 GAGNE" / "DÉFAITE".
- **Headline Hub** (800, 48 px): the pending delta in the hub, the active seat's J# in the resting hub, the poison tally on the player sheet, and the product name on the boot screen.
- **Row** (600, 30 px, uppercase): menu rows, the main button, the pending pill or band, the hub undo amount or hand-off ("−3", "J1 › J2"), commander tallies. In Lumineux also the arena seat labels, sized to be read across the table.
- **Label** (600, 24 px, uppercase): the seat/profile line ("J1 · COMMANDER"), section labels (POISON, CMDT, JOURNAL), journal rows, the Solo undo, the two-button home row. In Lumineux also the hub's "TOUR n" and "= total"; in Booster also the arena seat labels.
- **Label Small** (600, 20 px, uppercase): home sub-lines, seat-chip numbers, the CMDT tag and the two J# of an attack in the hub, player-sheet column labels, the "TOUCHER POUR REVENIR" hint. In Booster also "TOUR n", "ANNULER" and the "= total" line in the hub.

### Named Rules
**The Real Minus Rule.** Shared. Negative deltas use U+2212 (−), not a hyphen. Positive deltas always show "+".

**The All Caps Rule.** Shared. Every label and row is uppercase French, taken from `ui/strings_fr.h`. Beyond letters and numerals, text carries only signed deltas and the separators "·", "=", "/", "›" and "—" (a rule that is off).

**The Drawn Pointer Rule.** Shared. In text rows (journal, menu, undo) a hand-off or attack is written with U+203A as the separator ("J3 › J1"). In the pending hub the pointer is a drawn filled triangle between the two J#, never a glyph. The +/− beside a selected total are drawn 2 px marks (18 px), never glyphs.

## Layout

The layout is radial and shared by both themes: a 360×360 round canvas centred at (180, 180), radius 180. In the arena the circle is split into equal sectors (360/n degrees). J1 sits at the bottom (90°) and the other seats follow clockwise. A hub of radius 48 sits at the centre, and seat hit-testing excludes it. Solo, the player sheet, the journal, the menu, the home and the game-over screen are upright vertical stacks on the centre line.

Every arena seat faces its player. Each seat is drawn upright in its own frame, centred on its numeral point (nx, ny) at the numeral radius, then turned by (seat angle − 90°): J1 at the bottom is drawn directly, every other seat goes through an ARGB8888 layer (300 px wide for 2 players, 190 px for 3–4, line height + 44 px tall). In that frame the numeral sits at ny − 8 and the label on the rim side (ny + lh/2 − 6 in Lumineux, − 8 in Booster). The +/− marks flank the selected numeral at ny − 6, ±110 px (2 players) or ±64 px (3–4), and the −/+ tap zones are |local x| > 26 px in the same frame. Solo has no turn: its marks sit at ±136 px on y 162, and its −/+ tap zones are x < 110 / x ≥ 250.

In Lumineux the arena sectors are glass: arcs from radius 56 to radius 178, each trimmed by 2° on both sides so that a 4° wedge of ground parts two seats. The contract asked for 6 px gaps; LVGL truncates arc angles to whole degrees, so the gap is a whole-degree wedge that widens toward the rim. No rule is drawn between seats. The knob ring sits at radius 170 on Solo, the player sheet and the game-over screen.

Rhythm is set by fixed pitches, not by a spacing scale. Action pills and bars are 46 px tall (y 258–304 on the home and game-over screens). Menu rows have a 52 px pitch, starting at y 76. Journal rows have a 44 px pitch, starting at y 104, with the seat mark centred at x 77 and the text left-aligned at x 96. Home seat chips are 36 px at a 44 px pitch. The boot screen shows the logo centred at (180, 142) and the product name at y 290 for 1.5 s; any tap or knob step skips it.

**The Seats Face Their Players Rule.** Shared. An arena seat's numeral, label, +/− marks and tap zones all live in that seat's frame, turned by (seat angle − 90°), with the label on the rim side. Never lay out a seat element in screen coordinates.

**The Parted By Ground Rule.** Lumineux only. Seats are separated by a wedge of ground (2° trimmed from each side of each sector, in whole degrees), never by a drawn line.

### Theme 2: Booster
Booster uses the same geometry with painted sectors that touch: the selected seat is a full-radius wedge, and straight 2 px seams run from the hub edge to the rim between seats. Inset content keeps clear of the round bezel with fixed boxes: menu focus x 36–323, journal rows x 70–310, game-over buttons x 70–290, the Solo undo bar x 100–260 (y 282–318). The Booster home and new-game screens use the Lumineux geometry with square blocks: the product name replaces the logo, the primary action is an ink-filled block, the others are 2 px outlines.

## Elevation & Depth

Lumineux has depth, and all of it is light. There are no drop shadows and no offsets. Three layers read from back to front: the well (the radial ground), glass (translucent dark or tinted surfaces that let the well show through), and light (halos, rays, the glowing ring, the pending arc, white ink). Brighter means nearer and more important.

### Light Vocabulary
- **Well**: the radial ground, computed once per ground colour into a screen-sized buffer in the display's pixel format and then copied. On 16-bit displays it is ordered-dithered (4×4 Bayer) against banding. If the buffer cannot be allocated the ground falls back to the flat centre colour.
- **Halo** (`fx_halo`): a soft disc of light, a Gaussian that reaches zero at its radius. One A8 mask is computed per radius at its drawn size and cached (6 radii, least recently used evicted). Used behind the Solo numeral (radius 150, 40 %), behind the selected arena numeral (radius 120 for 2 players, 84 for 3–4, 20 %), behind the sheet title (radius 96, 30 %), for the winner (radius 170, 30 %), behind lit home chips (radius 30, 20 %), as the journal's seat point (radius 14, 100 %), and in ink behind the boot logo (radius 150, 20 %).
- **Glowing ring** (`fx_ring`): a pre-rendered 128×128 A8 ring, scaled to the hub radius. The hub's glow: ink at 70 % at rest, 30 % while a delta is pending.
- **Ray** (`fx_ray`): a pre-rendered 32×128 A8 beam that widens and fades as it travels, stretched to its length and rotated. No screen draws it since 2026-11-13; it remains in the primitives and on the specimen sheet.
- **Glass**: the rim colour at 215/255 with a rounded-full outline (pills), or at 190/255 with a 1 px border and a 1 px highlight arc on the upper edge (the round `fx_glass`, used for the hub undo). Lit or focused glass gets a brighter white border, and the round button a white halo of twice its radius at 80 %.

### Named Rules
**The Baked Light Rule.** No blur, glow or radial gradient is computed per frame. Light is an alpha mask blit: pre-rendered by `ui/assets/glow_gen.py`, or computed once and cached (halos per radius, the well per ground). A new light effect must fit that model.

**The Never Light Alone Rule.** A halo, a tint or a ray never carries information alone. The seat is named by its J# and its position, the active seat by the hub's J#, defeat by the skull and the word VAINCU. If a halo cannot be allocated it is simply not drawn and nothing is lost.

### Theme 2: Booster
Booster is completely flat: no shadows, glows or blur. Depth is field versus ground. The foil gradient is its only implied sheen.

**The Rule Not Shadow Rule.** Booster only. Separate planes with a straight seam in the rule weight, never with a shadow, a glow or an offset.

## Shapes

Lumineux is round. Every button is a rounded-full pill, home chips are circles, the hub is a disc of radius 48, and the round bezel clips everything else. Strokes are thin white lines: 1 px for the knob ring circle and unlit glass borders, 2 px for the detent ticks, the +/− marks, the focus rule, the journal rule and a lit pill's border. The only heavier strokes are the two pending arcs (4 px on the hub, 5 px on the knob ring), which have to be seen from across the table.

- **Knob ring**: a 1 px circle at radius 170 with 16 ticks (every 22.5°, 2 px, from radius 170 to 177), all in ink at 40 %. It is the logo's knurl. Present on Solo, the player sheet and the game-over screen.
- **Active glow**: a white halo of radius 44 at 50 %, centred 6 px outside the hub edge on the active seat's axis. It replaced three rays across the seat, which the owner rejected on 2026-11-13 (two white strokes flanking the numeral).
- **Pending arcs**: drawn clockwise from 12 o'clock (−90°) to −90° + 360° × progress. On the hub: 4 px at radius 51, just outside the glass edge. On the knob ring: 5 px at radius 172.
- **Line icons**: 40×40 A8 masks drawn from plain strokes (2.5 px, round caps), no fill, tinted in ink. Eight ship (settings, players, resume, game type, undo, turn, wifi, room); the current screens use one, undo, in the hub.
- **Logo**: the owner-approved line-art mark, as two ink-tinted A8 masks (237×195 on the boot screen, 72×60 at the top of the home).

**The No Zigzag Rule.** Shared. The user rejected the jagged look (2026-11-12). No zigzag, toothed, serrated or crimped edge on any line, seam, ring, band or arc. Every pending indicator is a smooth form whose drawn extent is the progress.

### Theme 2: Booster
Booster is square: fields are 0 px, full-bleed, and the hub is its only circle.
- **Rule**: a straight 2 px line (arena seams, home focus underline, journal title rule, band edges).
- **Band seal**: a 2-stop foil fill across the solid silver band, from x 0 to 360 × progress. The top and bottom 2 px edges are always complete.
- **Ring seal**: a 2 px arc at radius 48, closed at rest, drawn clockwise from −90° while pending.
- **Crest**: a solid 6 px arc at radius 176, inset 6° from each end of the seat span, marking the active seat.

**The One Rule Weight Rule.** Booster only. 2 px everywhere, with one exception, the 6 px active-seat crest.

## Components

Lumineux components first; Booster's equivalents are listed at the end.

### Glass pills (buttons)
- **Shape:** rounded-full, 46 px tall, filled with the rim colour at 215/255.
- **Lit:** the thing to tap right now. A 2 px ink border at 80 %. NOUVELLE PARTIE on the home; REPRENDRE when a saved game exists (a new game would replace it, so NOUVELLE PARTIE becomes unlit); SUIVANT and JOUER on the new-game screen; NOUVELLE PARTIE on the game-over screen; the Solo undo.
- **Unlit:** a 1 px ink border at 30 %. Text is ink at full opacity in both states.
- **Focus:** there is no hover and, since the 2026-11-13 home, no knob focus mark: the lit pill is the only emphasis.

### Seat chips (home)
- **Style:** 36 px circles at a 44 px pitch, each labelled J1–J4 in ink. A seat in play is lit: its hue at 50/255 inside a 2 px border in its hue, over a halo at 20 %. A seat not in play is empty glass: a 2 px ink border at 30 %.
- Seat chips now live on the new-game screen's table-size step: as many chips as the format allows, lit up to the chosen size; SOLO lights J1 only.

### Menu
- **Style:** five centred Row-text rows at a 52 px pitch. The focused row sits in a lit glass pill, 47 px tall, whose half-width is the widest that stays 8 px inside the round bezel, capped at 144 px. Unfocused rows are ink at 70 %.
- **Undo row:** previews the action it will undo ("ANNULER  J1  PV −3"; for an automatic pass, the hit that caused it), or reads "RIEN À ANNULER".

### Seat glass and numeral (signature)
- **At rest:** the sector is glass tinted by the seat hue at 36/255. The numeral is ink; the label (Row, 30 px) is in the seat hue.
- **Selected:** the tint rises only to 46/255 and a halo in the seat hue sits behind the numeral (20 %); the selection is carried by a 5 px band in the full seat hue along the sector's rim, away from any ink. The numeral, the label and the drawn +/− marks are all ink.
- **Defeated:** the seat goes dark. No tint, no halo. The skull mark replaces the total in ink at 60 %, and the label reads "J2  VAINCU" in ink at 60 %. The total stays in the journal and returns on undo.
- **Counters:** on the label line only when non-zero: "POISON 3", "CMDT 5", or "P3  C12" when both apply.
- **Skull mark:** the owner-supplied `ui/assets/dead.svg` as a 160×160 A8 mask, drawn with `lv_draw_image`, tinted and scaled about its centre. Arena: line height × 84 %, 14 px above the seat point. Solo: 150 px at (180, 158). It is an image mask, never a font glyph.

### Solo
- **Style:** the well, the knob ring, a halo in the seat hue (radius 150, 40 %) centred at (180, 160), "J1 · COMMANDER" in the seat hue at y 62, the numeral in ink, the +/− marks in ink, counters in ink at y 326.
- **Pending:** the delta waits in an unlit glass pill under the numeral (x 96–264, y 252–296, Row text "−3  = 37") while the 5 px light arc runs once around the knob ring.
- **Undo:** for 4 s after a commit, a lit glass pill (x 100–260, y 282–318) reads "ANNULER  −3" in Label.
- **Defeated:** no halo; the skull, "VAINCU" at y 262 and the seat line, all in ink at 60 %.

### Hub (signature)
- **At rest:** a disc of the rim colour inside the glowing ring (ink, 70 %). "TOUR n" in Label, ink, over the active seat's J# in Headline Hub in that seat's hue. Tap = next turn.
- **Pending:** the glow drops to 30 % and the 4 px light arc closes clockwise at radius 51 over exactly the commit delay. The delta is shown in Headline Hub, ink, as its effect on life (commander damage reads −5), with "= total" in Label below. Every drag A→B is an attack and shows the two J#, each in its own seat hue, around a drawn ink triangle. Commander damage adds a CMDT tag (Label Small) above the delta, placed at y − 31 with the delta lowered to y − 3 so the tag stays clear of the ring. A hub tap toggles attack ↔ commander damage (restarting the arc), or commits a non-attack delta at once.
- **Undo (4 s after any commit):** the hub becomes a focused round glass button with a white halo. It shows the undo line icon over the amount it reverses in Row, or "J1 › J2" after a pass. It carries no word, so players who see the hub upside down can still read it. When a hit defeated the active player and passed the turn automatically, it shows that hit; one tap reverts both.
- **Active seat:** the hub's glow leans toward it (see Shapes) and the hub names it. The light moves clockwise with Next and skips defeated seats. Solo has no turn.

**The Hub-Only Repaint Rule.** Shared. Arena seats show committed values. While only the pending value, the arc or the undo state changes, invalidate the hub box and never the turned seat layers. The box is the hub radius plus 18 px in Lumineux (the glow passes the edge) and plus 6 px in Booster.

### Player sheet
- **Header:** the knob ring and a halo in the seat hue (radius 96, 30 %) behind the seat number in Title, ink.
- **Poison:** a POISON label in the seat hue, then "n / 10" in Headline Hub, ink. A pending poison delta waits in an unlit glass pill (x 106–254, y 208–246, Label text) while the light arc runs around the knob ring.
- **CMDT:** a CMDT label in the seat hue, then one column per opposing seat at a 78 px pitch: the seat label in its own hue (Label Small) over the tally in ink (Row). The tally shows "n / 21" once it is within 5 of lethal.

### Journal
- **Style:** the JOURNAL label over a 120 px rule in ink at 40 %. Each row starts with a point of the seat's light (a halo of radius 14), then the action in Label, ink, left-aligned, newest first. Action rows carry the turn prefix ("T1  J4  POISON +3", "T1  J3 › J1  PV −5"); turn rows read "TOUR 1  J1 › J2". The point always comes with the "J#" text.

### Game over
- **Style:** the knob ring; the winner's light fills the well (a halo of radius 170 at 30 %, centred at (180, 120)); "J1 GAGNE" in Title and "PARTIE TERMINÉE" in Label, both ink at full opacity. ANNULER is an unlit pill, NOUVELLE PARTIE the lit one (x 70–290). With no winner (Solo), "DÉFAITE" is ink at 60 % and there is no halo.

### Boot screen (both themes)
- **Style:** the ground, the logo mask in the theme's ink, and the product name in Headline Hub. Lumineux adds a soft ink halo behind the logo. The name comes from one constant, `UI_PRODUCT_NAME`.

### Theme 2: Booster
- **Buttons:** square, 46 px tall. Primary is ink-inverse (Wrapper Ink fill, Wrapper Black Row text); secondary is a 2 px ink outline with Label text. With a saved game, REPRENDRE is primary and NOUVELLE PARTIE secondary. On a seat field (game over) the "ink" is the field's own text colour: ANNULER is a 2 px black outline, NOUVELLE PARTIE a solid black bar with text in the field hue.
- **Seat chips:** 36 px squares. In-game seats are filled with their hue and carry black text; unused seats are a 2 px outline in their hue.
- **Menu:** the focused row is an ink-inverse bar (47 px tall, x 36–323).
- **Seat field:** selected, the hue fills the sector (or the whole screen in Solo) with black numeral and label; unselected, numeral and label in the hue on the ground; defeated, never filled, the skull mark in full seat hue replaces the total.
- **Pending band:** a full-width silver band (46 px in Solo at y 250–296, 32 px on the player sheet at y 212–244) with black Row text; the foil sheen fills it from the left over the commit delay.
- **Active-seat crest:** the 6 px rim arc, black on the selected field, Wrapper Ink otherwise.
- **Hub seal:** at rest a black disc with a closed 2 px ink ring, "TOUR n" over the active J# in its hue. For 4 s after a commit, an ink-inverse disc reading "ANNULER" over the amount. Pending, a foil disc with the delta, the sub-line, and the 2 px ink ring closing clockwise from 12 o'clock.
- **Solo undo bar:** an ink-inverse bar (x 100–260, y 282–318) reading "ANNULER  −3".
- **Player sheet:** the seat hue fills the top band to y 132 with the seat number in black; CMDT columns at an 84 px pitch.
- **Journal:** a 14 px square swatch in the seat hue before each row; the title rule and the hint at full opacity.

## Do's and Don'ts

### Do:
- **Do** treat Lumineux as the default. Design a new screen in Lumineux first, then give Booster the same layout and tap zones.
- **Do** keep numerals in ink in Lumineux, and show the seat hue as glass tint, halo and label.
- **Do** show pending state as an arc (Lumineux) or a foil seal (Booster) whose drawn extent equals the elapsed share of the commit delay (800 ms, linear), with nothing drawn ahead of it.
- **Do** pair every seat hue with its J1–J4 label and its position.
- **Do** mark the thing to tap with white light: a lit pill border, a white halo, the hub's glow.
- **Do** keep light behind numerals at the shipped levels (Solo halo 40 %, selected glass 46/255, seat halo 20 %, sheet and game-over halos 30 %).
- **Do** build every light effect from an A8 mask that is pre-rendered or cached, and give every new raster a provenance line in `ui/assets/README.md`.
- **Do** take every user-facing string from `ui/strings_fr.h`, in uppercase.
- **Do** draw each arena seat in its own frame, turned by (seat angle − 90°), with the label on the rim side.
- **Do** keep the hub the single place for turn, undo and pending state, and repaint only the hub while only it changes.
- **Do** check a new ground preset with the 7:1 assertion in `host/shots.c`.

### Don't:
- **Don't** paint an opaque seat field in Lumineux, and don't add halos, glass or rays to Booster.
- **Don't** use a seat hue for chrome, focus, the pending arc, the undo or the rays.
- **Don't** compute a blur, a glow or a radial gradient per frame.
- **Don't** let a halo, a tint or a ray be the only signal of anything.
- **Don't** draw rules between Lumineux seats; they are parted by ground.
- **Don't** draw a zigzag, toothed or crimped edge anywhere; the user rejected it.
- **Don't** use a hyphen for minus, or a glyph for the hub pointer, the +/− marks, the skull or an icon.
- **Don't** fade a defeated seat in Booster, and don't leave a defeated seat lit in Lumineux.
- **Don't** fall back to the category default (flat colour quadrants, a thin progress ring on black) or to a neon gamer HUD (scanlines, chrome bevels, hexagon grids).
- **Don't** copy the third-party dial app's colours, icons, logo or composition.

## Asset Provenance

Every raster the firmware ships is recorded in `ui/assets/README.md`. Nothing is taken from an icon set, a stock library or an image generator.

- **Fonts:** Barlow Condensed ExtraBold and SemiBold from google/fonts (`ofl/barlowcondensed`), SIL Open Font License 1.1 (`ui/assets/OFL.txt`). Modified by `strip_gpos.py`: the GPOS table tag is renamed so LVGL 9.2.2's tiny_ttf does not read past the font buffer. Outlines and metrics are unchanged.
- **Defeat mark:** `ui/assets/dead.svg`, supplied by the project owner (2026-11-12), converted by `svg2a8.py` into a 160×160 A8 mask (`ui/skull_a8.c`). It resembles the Magic black mana symbol; Wizards materials are allowed by the owner. PRODUCT.md records that Wizards' Fan Content Policy covers free, non-commercial fan content only.
- **Light masks and line icons:** `ui/fx_a8.c`, generated by `glow_gen.py` (Python standard library only): the glowing ring (128×128), the ray (32×128) and eight 40×40 line icons. Every mask is computed by the script; the icons are drawn for this project from plain strokes, with no external source.
- **Halos and the well:** not assets. `ui/fx.c` and `ui/ui.c` compute them at run time, once per radius and once per ground.
- **Logo masks:** `ui/logo_a8.c`, generated by `logo2a8.py` from `ui/assets/logo.svg` (owner-approved 2026-11-12, drawn by `logo_gen.py`): two A8 masks cropped to the drawing, 237×195 and 72×60, rendered through GdkPixbuf and librsvg.

## Open Questions

- **Finish review.** A finish review ran on 2026-11-13 against the direction contract (`.impeccable/surfaces/ui-ui-c.md`), with the verdict "ship after fixes", as reported by the build session; the repository holds no review file. The fixes are in the code: less light under numerals (Solo halo 40 %, selected glass 46/255, seat halo 20 %, sheet and game-over halos 30 %); rays that flank the numeral (side rays at ±24–40°, centre ray stopping at 72 px) at 70 %; the selected seat's label and the game-over subline in ink; a defeated seat's ink at 60 %; the CMDT tag kept clear of the hub ring and the pending arc drawn at radius 51; arena seat labels at 30 px and the hub's secondary text at 24 px; whole-degree gaps between seats.
- **Contrast over light is estimated, not measured.** The only automated check is ink on the bare ground. A linear-blend estimate at the halo centre on Nuit, after the levels were lowered on 2026-11-13, gives about 7.3:1 under the Solo numeral, about 7:1 or more under a selected arena numeral for every seat (Gold is the tightest), about 8:1 or more under the sheet and winner titles, and about 6.6:1 or more for a lit home chip's J#. An unlit chip's J# and the home's three inactive buttons (ink at 40 %) are about 3.5:1: they are not available actions. These are estimates on a PC render, to be re-checked on the display.
- **Departures from the contract.** The contract's seat glass is a 12–18 % lift; the build uses 14 % at rest and 25 % when selected. The contract's strokes are round-capped, 2 or 3 px; the build draws 1, 2, 4 and 5 px strokes with LVGL's default caps, and only the icons are round-capped. The contract's chrome is glass buttons with line icons; the build's buttons carry text, and one icon (undo) is used.
- **Nothing has been seen on real hardware.** The dithering of the ground in RGB565 is compiled only for the firmware and has not been looked at. The render cost on the ESP32-S3 has not been measured.
- **Render cost, PC only.** `host/bench.c` (unoptimised build, full 360×360 redraw) puts Lumineux at about 2–2.6 times Booster on the boot, home and menu screens, about 5 times on Solo, and about 1.6–1.8 times in the 4-player arena. The ratio is expected to carry over; the absolute times are the PC's.
- **Undo halo and partial repaint.** The hub undo's white halo has a radius of 96 px, but the hub-only repaint box is 66 px. Whether a trace of the halo stays on screen when the undo expires has not been checked on a partial-refresh display.
- **Home and new game (2026-11-13).** The home is radial: logo y 40; NOUVELLE PARTIE disc r 64 at (180, 160) with a 70 % ink ring glow when lit; dimmed settings disc r 22 at (92, 92), host and join discs r 34 at (76, 206) and (284, 206) with their label below; REPRENDRE pill x 116–244, y 276–326 with "T n | J n". Settings, host and join are drawn at 40 % and do nothing until plan #2 and the connectivity lots; the knob does nothing on the home. New game (both themes): family or JOUEURS y 60, format name y 112 (64 px, 48 px beyond 9 letters), rules line y 154 (commander damage and poison only when the format has them), list dots or seat chips y 204–210 between two chevrons at x 56 / 304, back disc x 66–110 and SUIVANT / JOUER x 124–264 at y 262–310. The knob, or a tap left or right of centre above y 240, changes the value. Formats come from `profiles/data` (Singleton: Commander, Commander 1v1, Commander 2HG, Duel Commander; Construit: Standard, Pioneer, Modern, Legacy, Pauper, Peasant); a constructed format is a 20-life duel with no commander damage, and its only table sizes are SOLO and 2.
- **Concept seed.** The contract says "seed: none (unavailable)" and that there was no direction roll, so the seed could not be checked. `.impeccable/questions/roll.json` holds a roll with a seed; its relation to this build is not established.
- **Unused tokens.** `ui_theme_lumineux` still defines `seat_ink` and the three foil tokens, which no Lumineux screen reads. `ui/fx.c` draws glass borders and the focus halo in pure white rather than Light Ink.
- **Theme selector.** `ui_set_theme` and the six grounds exist; the settings screen that exposes them is plan #3.
