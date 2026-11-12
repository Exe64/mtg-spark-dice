# Fonts

Barlow Condensed ExtraBold and SemiBold, from google/fonts (`ofl/barlowcondensed`), SIL Open Font License 1.1 (`OFL.txt`).

Modified with `strip_gpos.py`: the GPOS table tag is renamed so LVGL 9.2.2's tiny_ttf / stb_truetype does not read
past the font buffer (see the script header). Outlines and metrics are unchanged.

# Defeat mark

`dead.svg`, supplied by the project owner (2026-11-12); resembles the Magic black mana symbol; Wizards materials are allowed by the owner (see PRODUCT.md, Brand Commitments). Converted with `python3 svg2a8.py dead.svg ../skull_a8.c ui_skull_a8 160 2` into a 160×160 A8 alpha mask that the UI tints with the seat hue.
