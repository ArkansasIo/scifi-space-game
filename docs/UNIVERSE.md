# Universe Generation

**Module:** `include/world/universe_gen.h` · `src/world/universe_gen.c++`
**Status:** implemented, not yet wired into the boot sequence

---

## 1. Purpose

Builds the galaxy the game is played in. Everything is derived from **one
32-bit seed**, so the same seed always produces the same stars, worlds, moons,
gates and anomalies — on any machine, in any session. That makes bug reports
reproducible and lets players share a galaxy.

The design goal is *plausibility first*: a star's class determines its
temperature and luminosity, which determine where the habitable zone falls,
which determines what the worlds in each orbit look like.

---

## 2. Structure

```
Galaxy
└── Sector          8 of them, named, with a development/danger profile
    └── Star System 32 total, distributed evenly across sectors
        ├── Star            1, with a spectral class
        ├── Worlds          1..8, positioned by orbit index
        │   └── Moons       0..4 per world
        ├── Asteroid Belt   optional
        ├── Station         optional, one of 4 kinds
        ├── Jump Gates      2..6, forming a traversable graph
        └── Anomalies       0..6 interstellar objects
```

Capacity per run:

| Record | Max |
| --- | --- |
| Sectors | 8 |
| Systems | 32 |
| Worlds | 128 |
| Objects | 64 |

---

## 3. Determinism

`universe_gen.c++` uses a private linear-congruential generator rather than
`rand()`:

```c
static unsigned int g_rngState = 1u;
g_rngState = (g_rngState * 1664525u) + 1013904223u;
```

It's seeded before each roll from `(galaxySeed + index * prime)`, so each
system, world and moon draws from an independent stream. Reordering the
generation loop does not change any existing record — a property `rand()`
cannot offer.

---

## 4. Star Generation

### 4.1 Spectral distribution

Weighted to match reality: M dwarfs are by far the most common.

| Roll (0..999) | Class | Share |
| --- | --- | --- |
| 0–2 | Black Hole | 0.3% |
| 3–7 | Neutron Star | 0.5% |
| 8–24 | O (blue) | 1.7% |
| 25–54 | B (blue-white) | 3.0% |
| 55–99 | A (white) | 4.5% |
| 100–179 | F (yellow-white) | 8.0% |
| 180–279 | G (yellow) | 10.0% |
| 280–399 | K (orange) | 12.0% |
| 400–649 | Giant | 25.0% |
| 650–999 | M (red dwarf) | 35.0% |

### 4.2 Derived properties

Each class rolls mass, radius, temperature, luminosity and age within a band.
A G-class star, for example:

```
mass        90..110  (x100 solar masses)
radius      90..120  (x100 solar radii)
temperature 5200..6000 K
luminosity  80..300  (x100 solar)
age         4000..10000 Myr
```

### 4.3 Habitability ceiling

A star class sets the *maximum* habitability its worlds can reach. This is the
single most important number in world generation.

| Class | Ceiling | Rationale |
| --- | --- | --- |
| G | 60 | Sol-like: the sweet spot |
| K | 50 | Long-lived and stable |
| F | 45 | |
| M | 30 | Habitable zone is close; worlds tidally locked |
| A | 25 | Short-lived: life has little time |
| Giant | 15 | The habitable zone has moved outward |
| B | 10 | |
| O | 5 | |
| Neutron | 2 | Irradiated |
| Black Hole | 0 | Nothing survives |

World count also depends on the star: black holes support at most 3 worlds,
neutron stars 2, giants 5, everything else 8.

---

## 5. World Generation

### 5.1 Orbital position

Orbits grow geometrically outward:

```
orbitDistance = 30 + (orbitIndex² × 12) + rand(20)     // light-seconds
```

This produces the familiar layout: rocky worlds close in, gas giants far out.

### 5.2 Temperature

```
temperature = 1000 - (orbitDistance × 2) + (starTemp / 100) + rand(-80..80)
```

Floored at 3 K. The star term means a hot star shifts the entire system's
temperature bands outward.

### 5.3 Classification

`classifyWorld(temperature, starClass, waterCoverage)` resolves in order:

| Condition | Class |
| --- | --- |
| temp < 120 and water > 60 | Ice Giant |
| temp > 900 | Gas Giant |
| temp > 700 | Toxic |
| temp > 500 | Lava |
| temp < 180 | Tundra |
| temp < 260 | Barren |
| water > 70 | Ocean |
| temp > 340 | Desert |
| water > 35 (M-class, water > 55) | Jungle, else Terrestrial |
| otherwise | Rocky |

### 5.4 Habitability

Starts from the star's ceiling, then:

| Condition | Modifier |
| --- | --- |
| temp 240–320 K | +25 |
| temp 200–360 K | +10 |
| otherwise | −25 |
| gravity 0.60–1.60 g | +10 |
| otherwise | −20 |

Then `rand(-10..10)`, clamped to 0..100.

### 5.5 Resources

```
base            rand(10..70)
Lava or Rocky   +20
Gas/Ice Giant   +15
+ orbitIndex
```

Richer further out — the classic "outer system is worth the trip" incentive.

### 5.6 Moons

| Parent | Moon chance |
| --- | --- |
| Gas Giant | 85% (2–4 moons) |
| Ice Giant | 85% (1–3 moons) |
| radius > 300 | 45% |
| radius > 120 | 25% |
| otherwise | 10% |

Moons roll radius, resources, habitability, tidal locking and whether they
host a station.

---

## 6. Jump Gates

The gate graph decides where a player can actually go.

`linkSystemGates()` builds it in two passes:

1. **A ring** — every system links to its immediate neighbours. This
   *guarantees* the graph is connected, so no system is ever unreachable.
2. **Chords** — each system has a 45% chance (twice) of an extra link 2–5
   systems ahead, giving the map interesting routing without becoming a mesh.

Result: typically 2–4 exits per system.

`jumpDistance(from, to)` is a breadth-first search over this graph, so the
distance is the true minimum number of jumps.

---

## 7. Interstellar Objects

13 kinds, biased by the system's danger rating. Dangerous systems are more
likely to hold pirate strongholds and derelicts; safe ones hold research
anomalies and mining fields.

| Kind | Notes |
| --- | --- |
| Asteroid Belt | Ore-rich, patrol risk |
| Comet | Temporary, low value |
| Nebula | Sensor interference |
| Pulsar | Navigation hazard, research value |
| Quasar | Distant, high research |
| Derelict | Salvage; may be hostile |
| Precursor Relic | Rare, high reward |
| Wormhole | Fast travel, unpredictable exit |
| Jump Gate | Traversable link |
| Mining Field | Repeatable ore |
| Pirate Stronghold | Combat encounter |
| Research Anomaly | Research points |

Anomalies roll magnitude, danger and reward. Reward scales with magnitude:

```
reward = rand(10..500) × (magnitude / 20 + 1)
```

---

## 8. Sectors

Development and danger run in opposite directions from the core outward:

```
development = 100 - (index × 12)      floor 5
danger      = 10  + (index × 12)      cap 95
```

The Galactic Federation owns the first three sectors; the rest are unclaimed.
This creates the classic difficulty gradient: safe, developed core → lawless,
undeveloped frontier.

---

## 9. API

```c
void generateUniverse(unsigned int seed);        // build everything
void generateStarSystem(systemworld &, int idx, int sector, unsigned seed);
void rollStar(star &, unsigned seed);
void rollWorld(world &, int orbit, const star &, unsigned seed);
void rollMoons(world &, unsigned seed);
int  rollAnomalies(systemworld &, unsigned seed);
void linkSystemGates(unsigned seed);
```

Queries:

```c
int findSystemWorld(const char *name);
int countTotalWorlds();
int countHabitableWorlds(int systemIndex);
int richestWorldInSystem(int systemIndex);
int reachableSystems(int systemIndex, int out[], int outMax);
int jumpDistance(int fromSystem, int toSystem);
int sectorForSystem(int systemIndex);
int pickRandomSystem(unsigned int seed);
```

---

## 10. Presentation

`showUniverseOverview()`, `showSector()`, `showSystemWorld()`,
`showSystemList()`, `showAnomalyList()` — all fit the 80×24 target.

A system view looks like:

```
+------------------------------+
|   Altair IV                  |
+------------------------------+
  Star             : Class G (yellow)
  Temperature (K)  : 5780
  Mass (x100)      : 101
  Age (Myr)        : 6200
  Development      : 74
  Danger           : 26

  Worlds:
    1. Altair IV I  (Terrestrial)
       orbit 42 ls  gravity 1.02g  temp 288K  hab 71  res 44  moons 1
       moons: Moon I(res 31)
    2. Altair IV II  (Gas Giant)
       orbit 90 ls  gravity 2.41g  temp 190K  hab 8  res 68  moons 4
```

---

## 11. Open Questions

- **Binary systems.** The star record is singular. A `companion` field would
  let a system hold two suns, which changes habitability maths substantially.
- **Real-time stellar drift.** Stars currently never change. Adding
  stellar evolution (giant → neutron → black hole) would need a per-turn
  hook; `scheduler.h` is the natural home.
- **Faction expansion.** `ownerFaction` is set once and never updated. The
  4X layer needs to claim systems over time.
