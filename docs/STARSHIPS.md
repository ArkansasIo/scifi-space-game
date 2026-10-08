# Starships

**Module:** `include/data/starships.h` · `src/data/starships.c++`
**Status:** implemented

---

## 1. Purpose

The catalogue of known hulls, the weapons they mount, and the components that
fit into their slots. Every ship in the game is a **hull class** plus a set of
**fitted components**:

```
Hull class → slots (weapon / defence / system) → components → final stats
```

This is what makes "known ships" data rather than code: a new variant is a
table row, not a new class.

---

## 2. Hull Classes

13 hulls, tier 1 to 8.

| Hull | Type | Tier | Cost | Hull HP | Shield | Armor | Evasion | Speed |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Wasp Interceptor | Shielded | 1 | 120 | 80 | 120 | 4 | 85 | 180 |
| Raptor Interceptor | Shielded | 2 | 260 | 140 | 220 | 6 | 80 | 200 |
| Hammer Bomber | Armored | 2 | 320 | 220 | 80 | 14 | 45 | 120 |
| Sabre Corvette | Hybrid | 2 | 420 | 320 | 260 | 12 | 65 | 150 |
| Bulwark Frigate | Armored | 3 | 780 | 620 | 380 | 22 | 50 | 120 |
| Lancer Destroyer | Hybrid | 3 | 1250 | 980 | 720 | 30 | 40 | 105 |
| Vigil Cruiser | Shielded | 4 | 2400 | 1800 | 1700 | 40 | 32 | 92 |
| Sovereign Battleship | Hybrid | 5 | 5200 | 4200 | 3600 | 62 | 22 | 78 |
| Aegis Carrier | Shielded | 5 | 5800 | 3600 | 4200 | 48 | 18 | 70 |
| Obliterator Dreadnought | Armored | 6 | 11000 | 8800 | 6400 | 90 | 12 | 62 |
| Colossus Titan | Armored | 7 | 24000 | 18000 | 12000 | 120 | 8 | 50 |
| Divine Throne Ship | Hybrid | 8 | 48000 | 34000 | 26000 | 140 | 10 | 55 |
| Orbital Fortress | Armored | 4 | 3600 | 7200 | 2400 | 70 | 0 | 0 |

Note the design intent: **evasion and HP trade against each other.** A Wasp
has 85 evasion and 80 HP; a Colossus has 8 evasion and 18000 HP. There is no
hull that is both fast and tough.

### 2.1 Hardpoints

| Hull | Weapon | Defence | System | Fighters |
| --- | --- | --- | --- | --- |
| Fighter | 1 | 1 | 0 | 0 |
| Interceptor | 2 | 1 | 1 | 0 |
| Bomber | 2 | 1 | 1 | 0 |
| Corvette | 3 | 2 | 1 | 0 |
| Frigate | 4 | 3 | 2 | 0 |
| Destroyer | 6 | 4 | 3 | 0 |
| Cruiser | 8 | 6 | 4 | 1 |
| Battleship | 12 | 9 | 6 | 2 |
| Carrier | 8 | 10 | 8 | 24 |
| Dreadnought | 18 | 14 | 10 | 4 |
| Titan | 24 | 20 | 14 | 8 |
| Flagship | 30 | 24 | 18 | 12 |
| Station | 14 | 10 | 8 | 6 |

### 2.2 Ship types and resistances

| Type | Strong against | Weak to |
| --- | --- | --- |
| Armored | Kinetic (railgun) | Energy (laser, plasma, ion, beam) |
| Shielded | Energy (laser, plasma) | Ion, railgun |
| Hybrid | — | — |
| Stealth | — | (harder to hit, not harder to hurt) |

`weaponDamageVs()` applies these modifiers:

```
vs Armored:   energy ×1.20   kinetic ×0.80
vs Shielded:  ion ×2.00      energy ×0.85   kinetic ×1.30
vs Hybrid:    ×1.00
vs Stealth:   ×1.10
```

The ion cannon's ×2.00 against shields is the single biggest counter in the
table, which is what makes Bring-An-Ion a real decision.

---

## 3. Weapons

18 weapons across 11 types.

| Weapon | Type | Damage | Acc | Range | CD | Shield | Armor | Power |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Pulse Laser | Laser | 40 | 82 | 3 | 2 | 90 | 110 | 6 |
| Heavy Laser | Laser | 90 | 76 | 4 | 3 | 95 | 120 | 14 |
| Plasma Lance | Plasma | 160 | 68 | 5 | 4 | 120 | 90 | 26 |
| Fusion Cannon | Plasma | 300 | 60 | 6 | 6 | 130 | 85 | 48 |
| Light Railgun | Railgun | 120 | 74 | 7 | 4 | 70 | 150 | 22 |
| Spinal Railgun | Railgun | 420 | 62 | 9 | 8 | 60 | 180 | 60 |
| Hornet Missile | Missile | 70 | 88 | 6 | 3 | 100 | 120 | 12 |
| Swarm Missiles | Missile | 55 | 92 | 5 | 2 | 105 | 115 | 10 |
| Heavy Torpedo | Torpedo | 480 | 66 | 8 | 9 | 80 | 200 | 70 |
| Ion Cannon | Ion | 110 | 78 | 5 | 3 | 200 | 40 | 24 |
| Ion Disruptor | Ion | 220 | 70 | 6 | 5 | 240 | 30 | 44 |
| Particle Beam | Beam | 260 | 80 | 7 | 5 | 140 | 140 | 52 |
| Beam Lance | Beam | 520 | 70 | 10 | 9 | 150 | 130 | 95 |
| Flak Battery | Flak | 45 | 86 | 4 | 2 | 70 | 80 | 8 |
| Point Defence Grid | PD | 30 | 94 | 2 | 1 | 60 | 60 | 6 |
| Tesla Arc | Tesla | 95 | 84 | 5 | 3 | 160 | 70 | 20 |
| Graviton Driver | Graviton | 380 | 58 | 9 | 8 | 110 | 170 | 88 |
| Singularity Projector | Graviton | 700 | 50 | 11 | 12 | 120 | 190 | 140 |

### 3.1 The damage/accuracy curve

Notice it's a **clean trade**: damage rises as accuracy falls.

```
PD Grid    30 dmg / 94 acc
Pulse      40 dmg / 82 acc
Railgun   120 dmg / 74 acc
Beam      260 dmg / 80 acc   (beam is the exception: high both ways)
Torpedo   480 dmg / 66 acc
Singular  700 dmg / 50 acc
```

Heavy weapons miss. Light weapons chip. A battleship firing railguns at a
Wasp (85 evasion) will miss constantly, which is why escorts matter.

`bestWeaponForHull()` encodes the intended pairing: fighters get pulse lasers,
bombers get torpedoes, dreadnoughts get spinal railguns, titans get beam lances.

---

## 4. Components

34 components across 12 kinds, tier 1 to 5.

| Kind | Count | Examples |
| --- | --- | --- |
| Reactor | 4 | Fission Core I → Quantum Core IV |
| Shield | 4 | Deflector I → Aegis Shield IV |
| Armor | 4 | Ablative Plate I → Neutronium Plate IV |
| Engine | 3 | Ion Drive I → Warp Drive III |
| Sensor | 3 | Sensor Array I → Quantum Sensor III |
| Targeting | 3 | Targeting Computer I → Fire Control III |
| Point Defence | 2 | PD Turret I, PD Grid II |
| Fighter Bay | 2 | Fighter Bay I, II |
| Repair | 2 | Repair Drone Bay I, II |
| Cloak | 1 | Stealth Field II |
| AI Core | 2 | AI Core I, II |
| Weapon | 10 | Pulse Laser Turret → Graviton Driver |

Reactors are the power spine — everything else draws against them:

| Reactor | Output | Cost |
| --- | --- | --- |
| Fission Core I | 200 | 120 |
| Fusion Core II | 520 | 320 |
| Antimatter Core III | 1200 | 780 |
| Quantum Core IV | 2800 | 1800 |

---

## 5. Fit Validation

`validateFit()` returns an error code, so a bad loadout fails predictably
rather than producing nonsense stats:

| Code | Meaning |
| --- | --- |
| 0 | Legal |
| 1 | Invalid hull index |
| 2 | Invalid component index |
| 3 | Too many weapons |
| 4 | Too many defences |
| 5 | Too many systems |
| 6 | Reactor overloaded |

Slot mapping: weapons and point defence consume **weapon** slots; shields,
armor and engines consume **defence** slots; everything else consumes
**system** slots.

### 5.1 Power balance

```c
balance = hull.reactorOutput;
for each component: balance += c.powerOutput - c.powerDraw;
// negative → overloaded, validateFit returns 6
```

This is what stops a Wasp mounting a Quantum Core and a Singularity
Projector: the reactor is fine but the slots aren't, and a cheap hull with
expensive draws goes negative.

### 5.2 Ship value

`fittedShipValue()` produces the number the fleet-power and encounter-budget
maths consume:

```
value  = hullHP/4 + shieldHP/5 + armorRating×3 + weaponSlots×40
       + Σ(component: hp/2 + shield/3 + armor/3 + sensor/4
                    + accuracy/4 + repair/3 + powerOutput/6)
```

---

## 6. Fleet Composition

```c
int fleetHullSupply(const int hulls[], int count);   // total supply use
int recommendedEscorts(hullclass hull);              // escort ratio
const char *hullRoleName(hullclass hull);            // "screen", "line", ...
```

Role names group hulls for formation logic:

| Role | Hulls |
| --- | --- |
| screen | Fighter, Interceptor, Bomber |
| escort | Corvette, Frigate |
| line | Destroyer, Cruiser |
| capital | Battleship, Carrier, Dreadnought |
| super-capital | Titan, Flagship |
| installation | Station |

Escort recommendations scale with hull size: a Dreadnought wants 12 escorts, a
Titan 20 — because a Titan alone is a torpedo magnet.

---

## 7. Presentation

`showShipClassList()`, `showShipClassDetail()`, `showComponentList()`,
`showWeaponList()`, `showShipComparison()`.

---

## 8. Open Questions

- **Weapon mounts.** Weapons have no firing arc. A spinal railgun currently
  hits targets behind the ship.
- **Crew quality.** `crewRequired` and `crewCapacity` are declared but
  unused; veteran crews should modify accuracy and cooldown.
- **Component stacking.** Nothing stops mounting four of the same shield
  beyond the slot count. Diminishing returns per duplicate kind would be
  truer to the genre.
