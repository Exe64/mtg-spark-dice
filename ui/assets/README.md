# Fonts

Barlow Condensed ExtraBold and SemiBold, from google/fonts (`ofl/barlowcondensed`), SIL Open Font License 1.1 (`OFL.txt`).

Modified with `strip_gpos.py`: the GPOS table tag is renamed so LVGL 9.2.2's tiny_ttf / stb_truetype does not read
past the font buffer (see the script header). Outlines and metrics are unchanged.
