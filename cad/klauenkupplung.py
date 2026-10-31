# -*- coding: utf-8 -*-
"""Parametrische Klauenkupplung (Hirth-artige Stirnverzahnung) fuer paffee.

Erzeugt zwei STEP-Dateien neben diesem Skript:

  gear_cutter.step  WERKZEUGKOERPER. In die Zahnrad-Datei importieren, auf die
                    Stirnflaeche setzen, per Boolean-SCHNITT abziehen.
                    Laesst eine flache Ringtasche mit Saegezahnboden stehen.
  dog_ring.step      Nackter Zahnkranz. In das selbst gebaute Traegerteil
                    importieren und per Boolean-VEREINIGUNG anfuegen.

Koordinaten: z = 0 ist die Zahnradstirnflaeche, Achse = Z.
Das Zahnrad liegt darunter (-Z), der Zahnkranz darueber (+Z).
Beide Koerper stehen bereits in Einbaulage zueinander - dieselbe Placement-
Transformation auf BEIDE anwenden, sonst passt die Teilung nicht mehr.

Aufruf:  freecadcmd klauenkupplung.py
"""
import math, os
import FreeCAD as App
import Part

# ------------------------------------------------------------------ Parameter
N_TEETH     =  20      # Klauenzahl
#N_TEETH     = 20      # Klauenzahl
#FLANK_DEG   = 30.0    # Flankenwinkel gegen die Stirnflaeche
FLANK_DEG   = 45.0    # Flankenwinkel gegen die Stirnflaeche

# HARTE Grenzen der Tasche im Zahnrad. Nicht ueberschreiten, sonst laeuft die
# Tasche beim kleinen Rad des 2. Gangs in den Zahnfuss.
#R_INNER     =  9.5    # Kranz innen [mm]   (d = 19)
R_INNER     = 8.5     # Kranz innen [mm]   (d = 19)
#R_OUTER     = 14.5     # Kranz aussen [mm]  (d = 29)
R_OUTER     = 13.5   # Kranz aussen [mm]  (d = 29)

CLEARANCE   = 0.25    # Spiel [mm]. Geht zulasten des Kranzes, nie der Tasche.
ROOT_RELIEF = 0.40    # Spitzenfreigang axial [mm]
RING_BACK   = 1.0     # Ruecken ueber z=0 [mm]. Ohne das haengen die Zaehne nur
                      # an Messerkanten zusammen und ergeben keinen Volumenkoerper.

# ------------------------------------------------------------------ Geometrie
DELTA  = 2.0 * math.pi / N_TEETH
K      = math.tan(math.radians(FLANK_DEG)) * DELTA / 2.0   # Tiefe = K * Radius
R_MEAN = 0.5 * (R_INNER + R_OUTER)
DPHI   = CLEARANCE / (math.sin(math.radians(FLANK_DEG)) * R_MEAN)
DEPTH_MAX = K * R_OUTER
R_W_IN, R_W_OUT = 0.5, R_OUTER + 8.0


def _v(r, phi, z):
    return App.Vector(r * math.cos(phi), r * math.sin(phi), z)


def _wedge(phi, hw, z_ridge, h_far):
    """Keil ueber (h_far>0) bzw. unter (h_far<0) der V-Flaeche."""
    ri, ro = R_W_IN, R_W_OUT
    A = _v(ri, phi - hw, z_ridge);     D = _v(ro, phi - hw, z_ridge)
    B = _v(ri, phi, z_ridge - K * ri); E = _v(ro, phi, z_ridge - K * ro)
    C = _v(ri, phi + hw, z_ridge);     F = _v(ro, phi + hw, z_ridge)
    At, Bt, Ct, Dt, Et, Ft = (App.Vector(p.x, p.y, h_far) for p in (A, B, C, D, E, F))
    tris = [(A,B,E),(A,E,D),(B,C,F),(B,F,E),
            (At,Et,Bt),(At,Dt,Et),(Bt,Ft,Ct),(Bt,Et,Ft),
            (A,Bt,B),(A,At,Bt),(B,Ct,C),(B,Bt,Ct),
            (D,E,Et),(D,Et,Dt),(E,F,Ft),(E,Ft,Et),
            (A,D,Dt),(A,Dt,At),(C,Ft,F),(C,Ct,Ft)]
    s = Part.makeSolid(Part.makeShell(
        [Part.Face(Part.makePolygon([a, b, c, a])) for a, b, c in tris]))
    if s.Volume < 0:
        s.reverse()
    return s


def _union(hw, z_ridge, h_far):
    u = _wedge(0.0, hw, z_ridge, h_far)
    for i in range(1, N_TEETH):
        u = u.fuse(_wedge(i * DELTA, hw, z_ridge, h_far))
    return u


def _tube(r_in, r_out, z_lo, z_hi):
    outer = Part.makeCylinder(r_out, z_hi - z_lo, App.Vector(0, 0, z_lo))
    if r_in <= 0:
        return outer
    return outer.cut(Part.makeCylinder(r_in, 1000, App.Vector(0, 0, z_lo - 500)))


# Werkzeug fuers Zahnrad: Volumen zwischen Stirnflaeche (z=0) und Saegezahnboden.
# Taschen um DPHI breiter und um ROOT_RELIEF tiefer als nominal.
gear_cutter = (_union(DELTA / 2.0 + DPHI, -ROOT_RELIEF, 500.0)
               .common(_tube(R_INNER, R_OUTER,
                             -(DEPTH_MAX + ROOT_RELIEF + 1.0), 0.0))
               .removeSplitter())

# Zahnkranz: Material oberhalb der nominalen Saegezahnflaeche, radial um
# CLEARANCE eingezogen, mit duennem Ruecken ueber z=0.
dog_ring = (_tube(R_INNER + CLEARANCE, R_OUTER - CLEARANCE,
                  -(DEPTH_MAX + 0.1), RING_BACK)
            .cut(_union(DELTA / 2.0, 0.0, -500.0))
            .removeSplitter())


def build_and_export():
    doc = App.newDocument("Klauenkupplung")
    Part.show(gear_cutter, "gear_cutter")
    Part.show(dog_ring, "dog_ring")
    doc.recompute()
    out = os.path.dirname(os.path.abspath(__file__))
    print("\n=== Klauenkupplung ===")
    print("Zaehne %d, Flanke %.0f Grad" % (N_TEETH, FLANK_DEG))
    print("Tasche im Zahnrad : d %.2f .. %.2f mm, %.2f mm tief (aussen, inkl. Freigang)"
          % (2*R_INNER, 2*R_OUTER, DEPTH_MAX + ROOT_RELIEF))
    print("Zahnkranz         : d %.2f .. %.2f mm, Zaehne %.2f mm hoch (aussen)"
          % (2*(R_INNER+CLEARANCE), 2*(R_OUTER-CLEARANCE), DEPTH_MAX))
    print("Ruecken des Kranzes ueber z=0: %.2f mm" % RING_BACK)
    print("Noetiger Ausrueckweg pro Seite: %.2f mm" % (DEPTH_MAX - ROOT_RELIEF))
    for n, sh in (("gear_cutter", gear_cutter), ("dog_ring", dog_ring)):
        bb = sh.BoundBox
        print("%-12s valid=%s solids=%d  V=%7.1f mm3  Huellmass %.2f x %.2f x %.2f"
              % (n, sh.isValid(), len(sh.Solids), sh.Volume,
                 bb.XLength, bb.YLength, bb.ZLength))
        sh.exportStep(os.path.join(out, n + ".step"))


if not globals().get("SUPPRESS_EXPORT"):
    build_and_export()
