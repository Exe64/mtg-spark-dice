import math, sys, random

CX = 256
TOP = 236              # top surface plane (ellipse centre y)
E0 = (214, 70)         # outer rim
E1 = (168, 52)         # opening
H = 128                # wall height
GROOVE = 84            # groove below the rim
R = 128                # d20 radius
DY = 232               # die centre: lower third sinks behind the front lip
SRC = (CX, TOP + 22)


def ell_pt(rx, ry, cy, a):
    return CX + rx * math.cos(a), cy + ry * math.sin(a)


def front_arc(rx, ry, cy):  # left -> right through the front (bottom) side
    return f"M{CX - rx},{cy} A{rx},{ry} 0 0 0 {CX + rx},{cy}"


hexv = {"T": (0, -1), "UR": (.866, -.5), "LR": (.866, .5), "B": (0, 1), "LL": (-.866, .5), "UL": (-.866, -.5)}
inn = {"a": (0, -.6), "b": (.56, .34), "c": (-.56, .34)}
V = {k: (CX + x * R, DY + y * R) for k, (x, y) in {**hexv, **inn}.items()}
faces = [("T", "UL", "a"), ("T", "UR", "a"), ("UL", "a", "c"), ("UR", "a", "b"), ("UL", "LL", "c"),
         ("UR", "LR", "b"), ("a", "b", "c"), ("LL", "B", "c"), ("LR", "B", "b"), ("B", "b", "c")]
lit = [0, 0, .05, .08, .25, .3, .1, .55, .6, .8]


def pts(n):
    return " ".join("%.1f,%.1f" % V[k] for k in n)


def knurl():
    out, n = [], 15
    for i in range(n + 1):
        a = .12 + (math.pi - .24) * i / n
        x1, y1 = ell_pt(E0[0], E0[1], TOP + 8, a)
        x2, y2 = ell_pt(E0[0], E0[1], TOP + GROOVE - 6, a - .14)
        out.append(f'<line x1="{x1:.1f}" y1="{y1:.1f}" x2="{x2:.1f}" y2="{y2:.1f}"/>')
    return "".join(out)


def rays():
    random.seed(7)
    out = []
    sx, sy = SRC
    for i in range(17):
        sx = CX + (i - 8) * 15
        ang = math.radians(-90 + (i - 8) * 7.5 + random.uniform(-2, 2))
        w = math.radians(random.choice([.8, 1.4, 2.2]))
        L = random.uniform(170, 250)
        p1 = (sx + L * math.cos(ang - w), sy + L * math.sin(ang - w))
        p2 = (sx + L * math.cos(ang + w), sy + L * math.sin(ang + w))
        col = "#ffffff"
        out.append(f'<polygon points="{sx},{sy} {p1[0]:.1f},{p1[1]:.1f} {p2[0]:.1f},{p2[1]:.1f}" fill="{col}" opacity="{random.uniform(.5, .95):.2f}"/>')
    for i in range(0):  # shards (removed)
        ang = math.radians(-90 + random.uniform(-80, 80))
        d = random.uniform(80, 200)
        x, y = sx + d * math.cos(ang), sy + d * math.sin(ang) * .8
        s = random.uniform(3, 7)
        rot = math.degrees(ang) + 90
        col = "#ffffff"
        out.append(f'<polygon points="0,{-s * 2.2} {s * .6},0 0,{s * 2.2} {-s * .6},0" fill="{col}" '
                   f'transform="translate({x:.1f},{y:.1f}) rotate({rot:.0f})" opacity=".9"/>')
    return "\n    ".join(out)


HEX = ("T", "UR", "LR", "B", "LL", "UL")


def logo():
    W = 'stroke="#fff" fill="none"'
    wall_l, wall_r = CX - E0[0], CX + E0[0]
    return f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 512 512">
  <defs>
    <radialGradient id="flare" cx="{SRC[0]}" cy="{SRC[1]}" r="120" gradientUnits="userSpaceOnUse">
      <stop offset="0" stop-color="#fff" stop-opacity=".9"/><stop offset="1" stop-color="#fff" stop-opacity="0"/>
    </radialGradient>
    <radialGradient id="fade" cx="{CX}" cy="{TOP}" r="250" gradientUnits="userSpaceOnUse">
      <stop offset=".35" stop-color="#fff"/><stop offset="1" stop-color="#000"/>
    </radialGradient>
    <clipPath id="above"><rect x="0" y="0" width="512" height="{TOP}"/>
      <ellipse cx="{CX}" cy="{TOP}" rx="{E1[0]}" ry="{E1[1]}"/></clipPath>
    <clipPath id="band"><path d="M{wall_l},{TOP} A{E0[0]},{E0[1]} 0 0 0 {wall_r},{TOP} L{wall_r},{TOP + GROOVE} A{E0[0]},{E0[1]} 0 0 1 {wall_l},{TOP + GROOVE} Z"/></clipPath>
    <mask id="behind"><rect width="512" height="512" fill="url(#fade)"/>
      <polygon points="{pts(HEX)}" fill="#000" stroke="#000" stroke-width="10" stroke-linejoin="round"/></mask>
    <mask id="behindsolid"><rect width="512" height="512" fill="#fff"/>
      <polygon points="{pts(HEX)}" fill="#000" stroke="#000" stroke-width="10" stroke-linejoin="round"/></mask>
    <filter id="blur" x="-30%" y="-30%" width="160%" height="160%"><feGaussianBlur stdDeviation="5"/></filter>
    <filter id="blur2" x="-30%" y="-30%" width="160%" height="160%"><feGaussianBlur stdDeviation="1.4"/></filter>
  </defs>
  <g {W} stroke-width="5" stroke-linecap="round">
    <path d="{front_arc(E0[0], E0[1], TOP + H)}" stroke-width="7"/>
    <line x1="{wall_l}" y1="{TOP}" x2="{wall_l}" y2="{TOP + H}" stroke-width="7"/>
    <line x1="{wall_r}" y1="{TOP}" x2="{wall_r}" y2="{TOP + H}" stroke-width="7"/>
    <path d="{front_arc(E0[0], E0[1], TOP + GROOVE)}"/>
    <path d="{front_arc(E0[0], E0[1], TOP + GROOVE + 10)}" stroke-width="3"/>
    <g stroke-width="3.5" clip-path="url(#band)">{knurl()}</g>
    <path d="{front_arc(E0[0], E0[1], TOP)}" stroke-width="7"/>
    <path d="{front_arc(E1[0], E1[1], TOP)}"/>
  </g>
  <g clip-path="url(#above)">
    <g mask="url(#behind)"><g filter="url(#blur2)">
    {rays()}
    </g></g>
    <g mask="url(#behindsolid)">
      <ellipse cx="{CX}" cy="{TOP}" rx="{E0[0]}" ry="{E0[1]}" {W} stroke-width="7"/>
      <ellipse cx="{CX}" cy="{TOP}" rx="{E1[0]}" ry="{E1[1]}" {W} stroke-width="5"/>
      <ellipse cx="{SRC[0]}" cy="{SRC[1]}" rx="130" ry="60" fill="url(#flare)"/>
    </g>
    <g {W} stroke-width="5" stroke-linejoin="round">
      {"".join(f'<polygon points="{pts(f)}"/>' for f in faces)}
      <polygon points="{pts(HEX)}" stroke-width="7"/>
    </g>
    <text x="{CX}" y="{(V['a'][1] + 2 * V['b'][1]) / 3 + 22:.1f}" text-anchor="middle" font-family="Barlow Condensed" font-weight="600" font-size="64" fill="#fff">20</text>
    <path d="M{CX - E1[0] + 20},{TOP + 22} Q{CX},{TOP + E1[1] + 16} {CX + E1[0] - 20},{TOP + 22}" fill="none" stroke="#fff" stroke-width="16" opacity=".5" filter="url(#blur)"/>
  </g>
</svg>'''


open(sys.argv[1] + "/logo.svg", "w").write(logo())
