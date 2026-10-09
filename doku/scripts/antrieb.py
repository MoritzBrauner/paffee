#!/usr/bin/env python3
"""Auslegung von Antrieb und Energieversorgung, erzeugt die Zahlen aus
doku/06-antrieb-elektronik.md.

Rechnet: Gewichtsschaetzung -> Zugkraft- und Drehmomentbedarf am Triebrad ->
Fahrleistung je Gang mit dem JGB37-520 am 3S-Akku -> Strombudget und Laufzeit.

Stand 1:10: Triebrad 15 Z (Teilkreis 57,72 mm), Zweiganggetriebe 13:47 / 28:32,
Seitenvorgelege 13:47. Motordaten aus der Herstellertabelle NFP-JGB37-520-EN-12100,
Zeile 6,25:1 (bei 12 V), mit dem linearen Gleichstrommotor-Modell auf die
Akkuspannung umgerechnet. Die Tabellenwerte sind nicht am eigenen Motor gemessen.
"""
import math

G = 9.81

# --- Gewichtsschaetzung (aus der 1:11-Auslegung) -----------------------------
WEIGHTS = [
    ("Wanne + Rahmen gedruckt (PETG/ASA, ~2,5 mm Wand)", 1.30),
    ("Turm + Blende + Rohr gedruckt", 0.55),
    ("10 Laufraeder + Trieb-/Leitraeder + Stuetzrollen", 0.40),
    ("Kette, 158 Glieder (1:11-Wert)", 0.45),
    ("Schuerzen, Panels, Details", 0.40),
    ("2x Fahrmotor mit Getriebe", 0.60),
    ("Turmantrieb + Hoehenrichtung", 0.20),
    ("BB-Einheit", 0.15),
    ("Akku (Reserve Ausbaustufe 3S Li-Ion; LiPo 1500 mAh ~0,12 kg)", 0.55),
    ("Elektronik, Kabel, Lautsprecher", 0.25),
    ("Wellen, Lager, Schrauben, Federstahl", 0.45),
]
DESIGN_MASS = 7.0          # Auslegungsmasse mit Reserve

# --- Fahrzeug / Laufwerk 1:10 ------------------------------------------------
SPROCKET_R = 0.05772 / 2   # wirksamer Radius = Teilkreis/2 [m], 15 Z bei 12 mm Teilung (konservativ)
TRAVEL_PER_REV = 15 * 12 / 1000.0    # [m]
CONTACT_LEN = 0.285       # Aufstandslaenge [m], M24 2851 mm
TRACK_GAUGE = 0.2616      # Abstand der Kettenmitten [m], 300 mm Breite minus eine Kettenbreite
F_ROLL = 0.15             # Rollwiderstandsbeiwert Kettenlaufwerk (Annahme)
MU_LAT = 0.6              # Querreibung beim Wenden auf der Stelle (Annahme)

# --- Motor: JGB37-520, 12 V, 6,25:1, 1600 1/min (Tabellenwerte bei 12 V) ------
MOTOR_U_REF = 12.0
MOTOR_N0_REF = 1600.0     # Leerlaufdrehzahl am Motorabtrieb [1/min]
MOTOR_I0 = 0.15           # Leerlaufstrom [A]
MOTOR_I_STALL_REF = 2.4   # Blockierstrom [A]
MOTOR_M_STALL_REF = 0.0883  # Blockiermoment am Motorabtrieb [Nm] (0,9 kg*cm)
MOTOR_I_PEAK_DESIGN = 3.5   # Auslegungswert fuer Treiber/Sicherung je Motor [A]
DAUER_ANTEIL = 0.5          # Dauerbetrieb bis hoechstens 50 % des Blockiermoments

U_BAT = 11.1              # 3S LiPo nominal [V]

ETA = 0.85                # Wirkungsgrad Zweiganggetriebe + Seitenvorgelege
I_SV = 47 / 13            # Seitenvorgelege
GEARS = [("hoch (2. Gang, 28:32)", 32 / 28 * I_SV), ("niedrig (1. Gang, 13:47)", 47 / 13 * I_SV)]

BASE_A = 0.5              # Grundlast Elektronik, Funk, Servo in Ruhe (Annahme)
PEAK_LOADS = [("Turmantrieb", 1.5), ("BB-Einheit", 3.0), ("Servos", 2.0), ("Elektronik/Sound", 1.0)]
PACK_AH = 1.5             # ein 3S-Airsoft-Pack, 1500 mAh
PACK_USABLE = 0.8


def motor(u):
    """Lineares Motormodell bei Spannung u: Leerlaufdrehzahl, Blockiermoment, Blockierstrom."""
    r = MOTOR_U_REF / MOTOR_I_STALL_REF
    n0 = MOTOR_N0_REF * (u - MOTOR_I0 * r) / (MOTOR_U_REF - MOTOR_I0 * r)
    m_stall = MOTOR_M_STALL_REF * (u / r - MOTOR_I0) / (MOTOR_U_REF / r - MOTOR_I0)
    return n0, m_stall, u / r


def load_cases(mass):
    """Zugkraft je Seite [N] fuer die Auslegungsfaelle."""
    w = mass * G
    a30 = math.atan(0.30)
    return [
        ("Ebene rollen", F_ROLL * w / 2),
        ("Steigung 30 % (16,7 Grad)", (w * math.sin(a30) + F_ROLL * w * math.cos(a30)) / 2),
        ("Steigung 100 % (45 Grad)", (w * math.sin(math.pi / 4) + F_ROLL * w * math.cos(math.pi / 4)) / 2),
        ("Wenden auf der Stelle", MU_LAT * w * CONTACT_LEN / 4 / TRACK_GAUGE + F_ROLL * w / 2),
    ]


def operating_point(f, ratio, n0, m_stall, i_stall):
    """Betriebspunkt fuer Zugkraft f je Seite: Anteil am Blockiermoment, v, Strom, Leistung."""
    m_motor = f * SPROCKET_R / (ratio * ETA)
    share = m_motor / m_stall
    if share >= 1:
        return share, None, None, None
    rpm = n0 * (1 - share)
    v = rpm / ratio * TRAVEL_PER_REV / 60
    amps = MOTOR_I0 + (i_stall - MOTOR_I0) * share
    watts = m_motor * 2 * math.pi * rpm / 60
    return share, v, amps, watts


def main():
    print("== Gewichtsschaetzung ==")
    for name, kg in WEIGHTS:
        print("  %-50s %5.2f kg" % (name, kg))
    expected = sum(kg for _, kg in WEIGHTS)
    print("  %-50s %5.2f kg   -> Auslegung auf %.1f kg" % ("Summe", expected, DESIGN_MASS))

    print("\n== Bedarf am Triebrad, je Seite, %.1f kg ==" % DESIGN_MASS)
    cases = load_cases(DESIGN_MASS)
    for name, f in cases:
        print("  %-26s F = %5.1f N   M = %.3f Nm" % (name, f, f * SPROCKET_R))

    n0, m_stall, i_stall = motor(U_BAT)
    print("\n== Motor JGB37-520 bei %.1f V: Leerlauf %.0f 1/min, Blockiermoment %.4f Nm, "
          "Blockierstrom %.2f A ==" % (U_BAT, n0, m_stall, i_stall))
    print("  max. mechanische Leistung je Motor: %.1f W" % (m_stall * n0 * 2 * math.pi / 60 / 4))

    for mass in (DESIGN_MASS, expected):
        print("\n== Fahrleistung je Gang, %.1f kg ==" % mass)
        for name, ratio in GEARS:
            v0 = n0 / ratio * TRAVEL_PER_REV / 60
            print("  Gang %-26s %.2f:1  v_leer = %.2f m/s = %.1f km/h, Blockiermoment am Triebrad %.3f Nm"
                  % (name, ratio, v0, v0 * 3.6, m_stall * ratio * ETA))
            for case, f in load_cases(mass):
                share, v, amps, watts = operating_point(f, ratio, n0, m_stall, i_stall)
                if v is None:
                    print("      %-26s geht nicht (%.0f %% des Blockiermoments)" % (case, share * 100))
                    continue
                tag = "" if share <= DAUER_ANTEIL else "  -> nur kurzzeitig"
                print("      %-26s %3.0f %%  v = %.2f m/s = %.1f km/h, %.2f A je Motor, %.1f W%s"
                      % (case, share * 100, v, v * 3.6, amps, watts, tag))

    print("\n== Strombudget (Spitze) ==")
    peak = [("2x Fahrmotor (Auslegung %.1f A je Motor)" % MOTOR_I_PEAK_DESIGN, 2 * MOTOR_I_PEAK_DESIGN)] + PEAK_LOADS
    for name, amps in peak:
        print("  %-40s %5.1f A" % (name, amps))
    total = sum(a for _, a in peak)
    print("  %-40s %5.1f A  = %.0f W bei %.1f V" % ("Spitze gesamt", total, total * U_BAT, U_BAT))

    print("\n== Laufzeit, ein Pack %.1f Ah, %d %% nutzbar, Grundlast %.1f A ==" % (PACK_AH, PACK_USABLE * 100, BASE_A))
    hi, lo = GEARS[0][1], GEARS[1][1]
    for label, f, ratio in (("Ebene, hoher Gang", cases[0][1], hi),
                            ("Ebene, niedriger Gang", cases[0][1], lo),
                            ("Steigung 30 %, niedriger Gang", cases[1][1], lo)):
        amps = operating_point(f, ratio, n0, m_stall, i_stall)[2]
        total_a = 2 * amps + BASE_A
        print("  %-30s %.2f A gesamt -> %.0f min" % (label, total_a, PACK_AH * PACK_USABLE / total_a * 60))


if __name__ == "__main__":
    main()
