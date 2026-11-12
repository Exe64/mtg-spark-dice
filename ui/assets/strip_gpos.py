"""Hide the GPOS table of a TTF from stb_truetype by renaming its directory tag to 'XPOS'.

Why: LVGL 9.2.2 tiny_ttf calls stbtt_GetGlyphKernAdvance even with kerning disabled, and its bundled
stb_truetype computes the GPOS lookup list without the table base (lookupList = lookupListOffset),
reading past the font buffer (ASan global-buffer-overflow). Glyph outlines and metrics are untouched;
only GPOS kerning is lost. Usage: python3 strip_gpos.py <font.ttf>...
"""
import struct
import sys

for path in sys.argv[1:]:
    data = bytearray(open(path, 'rb').read())
    n = struct.unpack('>H', data[4:6])[0]
    for i in range(n):
        off = 12 + 16 * i
        if data[off:off + 4] == b'GPOS':
            data[off:off + 4] = b'XPOS'
            open(path, 'wb').write(data)
            print(path, 'GPOS hidden')
            break
    else:
        print(path, 'no GPOS table')
