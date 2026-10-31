# -*- coding: utf-8 -*-
"""Einbau-Simulation fuer klauenkupplung.py.  Aufruf: freecadcmd verify.py"""
import math, os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
SUPPRESS_EXPORT = True
import FreeCAD as App, Part
import klauenkupplung as kk

GEAR_BORE_D = 12.0   # nur fuer den Dummy-Rohling dieser Pruefung

blank = kk._tube(GEAR_BORE_D/2.0, 16.0, -8.0, 0.0)
gear  = blank.cut(kk.gear_cutter).removeSplitter()
ring  = kk.dog_ring

ov = gear.common(ring)
ovv = sum(s.Volume for s in ov.Solids) if ov.Solids else 0.0

print("\n=== Einbau-Simulation ===")
print("Abgetragen am Zahnrad : %8.1f mm3" % (blank.Volume - gear.Volume))
print("DURCHDRINGUNG         : %8.4f mm3   <- muss 0 sein" % ovv)

turned = ring.copy()
turned.rotate(App.Vector(0,0,0), App.Vector(0,0,1), math.degrees(kk.DELTA)/2.0)
c = gear.common(turned)
print("Halbe Teilung verdreht: %8.1f mm3   <- muss > 0 sein (Formschluss)"
      % (sum(s.Volume for s in c.Solids) if c.Solids else 0.0))

print("Ausruecken:")
for lift in (1.0, 1.5, 1.9, 2.1):
    up = ring.copy(); up.translate(App.Vector(0,0,lift))
    up.rotate(App.Vector(0,0,0), App.Vector(0,0,1), math.degrees(kk.DELTA)/2.0)
    c = gear.common(up)
    v = sum(s.Volume for s in c.Solids) if c.Solids else 0.0
    print("   Hub %.1f mm -> %7.3f mm3 %s" % (lift, v, "FREI" if v < 1e-6 else ""))
