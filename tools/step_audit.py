#!/usr/bin/env python3
"""step_audit.py - extract NOMINAL geometry from a STEP file for spec auditing.

Pure stdlib. Built for analytic parts (cylinders, planes, hole patterns): reports
cylinder radii -> diameters, hole patterns (count, pitch radius, angular positions,
rotational-symmetry order), and the overall bounding box, so an export can be
diffed against the airframe spec (docs/cad/airframe-architecture.md S3).

IMPORTANT - what this is and isn't:
  * It reads NOMINAL design geometry (what you modelled).
  * It therefore validates DESIGN INTENT and FIT-STACK MATH: e.g. did the bore
    come out 150, did the OD update to 162, are there 8 holes at PR 67.5, is the
    clocking pattern actually asymmetric (rot-symmetry order 1), does a hole
    pattern on mating parts match.
  * It does NOT model as-printed FDM tolerance (hole shrink, elephant foot). That
    is empirical - the coupon print measures it. The two are complementary:
    this catches "did you model what you meant"; the coupon catches "did it print
    to size".

Usage: python3 tools/step_audit.py <file.step> [--round N]
"""
import sys, re, math
from collections import defaultdict

FLOAT = r'-?\d+\.?\d*(?:[eE][+-]?\d+)?'


def load_entities(text):
    m = re.search(r'DATA;(.*?)ENDSEC;', text, re.S)
    body = (m.group(1) if m else text).replace('\n', ' ')
    ents = {}
    for stmt in body.split(';'):
        stmt = stmt.strip()
        m = re.match(r'#(\d+)\s*=\s*([A-Z0-9_]+)\s*\((.*)\)\s*$', stmt, re.S)
        if m:
            ents[int(m.group(1))] = (m.group(2), m.group(3))
    return ents


def nums(s):
    return [float(x) for x in re.findall(FLOAT, re.sub(r'#\d+', ' ', s))]


def first_ref(s):
    m = re.search(r'#(\d+)', s)
    return int(m.group(1)) if m else None


def point_of(ents, pid):
    typ, args = ents.get(pid, (None, ''))
    if typ == 'CARTESIAN_POINT':
        f = nums(args)
        return f[:3] if len(f) >= 3 else None
    return None


def placement_location(ents, plid):
    typ, args = ents.get(plid, (None, ''))
    if typ and 'PLACEMENT' in typ:
        return point_of(ents, first_ref(args))
    return None


def rot_symmetry(angles, tol=1.0):
    """Largest k such that rotating the set by 360/k maps it onto itself."""
    n = len(angles)
    if n < 2:
        return 1
    aset = sorted(a % 360 for a in angles)
    for k in range(n, 1, -1):
        if n % k:
            continue
        step = 360.0 / k
        if all(any(min((((a + step) - b) % 360), ((b - (a + step)) % 360)) < tol
                   for b in aset) for a in aset):
            return k
    return 1


def main(path, rnd=2):
    text = open(path, errors='ignore').read()
    ents = load_entities(text)

    xs = ys = zs = None
    pts = [nums(a) for t, a in ents.values() if t == 'CARTESIAN_POINT' and len(nums(a)) >= 3]
    if pts:
        xs = [p[0] for p in pts]; ys = [p[1] for p in pts]; zs = [p[2] for p in pts]

    groups = defaultdict(list)  # radius -> list of (x, y, z) axis locations
    for typ, args in ents.values():
        if typ == 'CYLINDRICAL_SURFACE':
            n = nums(args)
            loc = placement_location(ents, first_ref(args))
            if n and loc:
                groups[round(n[-1], 3)].append(tuple(round(c, rnd) for c in loc))

    sch = re.search(r'FILE_SCHEMA[^;]*', text)
    print(f"== STEP audit: {path} ==")
    print(f"schema : {sch.group(0).split('((')[-1][:48] if sch else '?'}")
    print(f"entities: {len(ents)}")
    if xs:
        print(f"bbox mm : X[{min(xs):.1f},{max(xs):.1f}] "
              f"Y[{min(ys):.1f},{max(ys):.1f}] Z[{min(zs):.1f},{max(zs):.1f}]")
        print(f"extents : {max(xs)-min(xs):.1f} x {max(ys)-min(ys):.1f} x {max(zs)-min(zs):.1f} mm")
    print("\ncylindrical surfaces (radius -> diameter):")
    for r in sorted(groups):
        distinct = sorted(set((x, y) for (x, y) in [(p[0], p[1]) for p in groups[r]]))
        off = [(x, y) for (x, y) in distinct if math.hypot(x, y) > 0.5]
        dia = f"Ø{2*r:.2f}"
        if off:
            pr = sum(math.hypot(x, y) for x, y in off) / len(off)
            angs = sorted(math.degrees(math.atan2(y, x)) % 360 for x, y in off)
            print(f"  r{r:<6g} {dia:>9}: {len(off):2d} holes @ PR {pr:6.2f} mm | "
                  f"rot-symmetry order {rot_symmetry(angs)} | "
                  f"angles {[round(a, 1) for a in angs]}")
        else:
            print(f"  r{r:<6g} {dia:>9}: {len(groups[r])} on-axis surface(s) "
                  f"(OD / bore / opening)")
    print("\n[nominal geometry only - validates design intent, not as-printed "
          "FDM tolerance; print a coupon for that]")


if __name__ == '__main__':
    if len(sys.argv) < 2:
        sys.exit("usage: python3 tools/step_audit.py <file.step> [--round N]")
    rnd = int(sys.argv[sys.argv.index('--round') + 1]) if '--round' in sys.argv else 2
    main(sys.argv[1], rnd)
