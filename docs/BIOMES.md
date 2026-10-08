# Biomes, Worlds and Classes

**Module:** `include/world/biome.h` · `src/world/biome.c++`
**Status:** implemented

---

## 1. Purpose

Two taxonomies that share a file because they're both "what kind of thing is
this":

- **Biomes** — what a world looks like, what it yields, and what it costs to
  settle.
- **Classes** — what a character is, what it's good at, and what it becomes.

---

## 2. Biome Taxonomy

28 biomes in seven families. Every biome has a defined temperature and water
band, so a world lands in the right one automatically.

### 2.1 Families

| Family | Biomes | Character |
| --- | --- | --- |
| **Rocky** | Barren Rock, Cratered, Dust Flat | Dead worlds, ore-rich |
| **Hot** | Volcanic, Lava Sea, Magma Cavern | High ore, extreme hazard |
| **Cold** | Ice Sheet, Tundra, Frozen Ocean | Low yield, survivable |
| **Temperate** | Grassland, Forest, Jungle, Savanna, Wetland | Food-rich, colonisable |
| **Dry** | Desert, Dunes, Badlands | Energy-rich, water-poor |
| **Aquatic** | Ocean, Reef, Archipelago | Food + research |
| **Exotic** | Crystalline, Fungal, Gas Envelope, Crystal Cloud, Hive World | Research-rich, hazardous |
| **Artificial** | Ecumenopolis, Machine World, Ruined World | Built, not natural |

### 2.2 Biome record

```c
struct biominfo
{
    char name[32];
    biome parent;          // BIOME_NONE for a top-level biome
    int minTemperature;    // kelvin band it appears in
    int maxTemperature;
    int minWater;          // water coverage band
    int maxWater;
    int habitabilityBonus;
    int foodYield;         // 0..100
    int oreYield;
    int energyYield;
    int researchYield;
    int coloniseCost;
    int hazard;            // 0..100
    int unitType;          // what it can raise
};
```

### 2.3 Selected yields

| Biome | Food | Ore | Energy | Research | Cost | Hazard |
| --- | --- | --- | --- | --- | --- | --- |
| Grassland | 70 | 25 | 20 | 50 | 90 | 5 |
| Forest | 55 | 40 | 15 | 65 | 100 | 10 |
| Jungle | 60 | 45 | 10 | 80 | 130 | 30 |
| Ocean | 65 | 20 | 25 | 55 | 140 | 15 |
| Desert | 15 | 35 | 70 | 35 | 105 | 25 |
| Volcanic | 5 | 60 | 70 | 30 | 200 | 75 |
| Crystalline | 0 | 75 | 85 | 90 | 300 | 45 |
| Machine World | 10 | 80 | 95 | 110 | 800 | 70 |
| Hive World | 30 | 50 | 40 | 100 | 500 | 95 |

The pattern: **you can have yield or safety, not both.** Grassland is cheap
and safe but yields little; Crystalline is expensive and dangerous but is the
best research world in the game.

---

## 3. Biome Selection

`selectBiome(temperature, waterCoverage, habitability)` resolves in strict
order — temperature first, because it's the dominant factor:

```
temp >= 900                      → Lava Sea
temp >= 700                      → Volcanic
temp >= 450  (water > 50)        → Gas Envelope
temp >= 450                      → Desert
temp <= 120  (water > 70)        → Frozen Ocean
temp <= 120                      → Ice Sheet
temp <= 200                      → Tundra
habitability >= 70               → Archipelago / Forest / Grassland
habitability >= 40               → Ocean / Wetland / Savanna
habitability >= 15               → Ocean / Grassland / Dunes
otherwise                        → Frozen Ocean / Badlands / Barren Rock
```

`selectSecondaryBiome()` returns the primary's child if one exists, otherwise
a neighbouring temperature band — so a Terrestrial world is usually paired
with Forest, a Volcanic world with Lava Sea.

---

## 4. Terrain Features

12 features: River, Mountain Range, Canyon, Geothermal Vents, Crystal Field,
Ruins, Crater, Ice Cavern, Underwater Vent, Floating Isles, Subterranean Sea.

A world rolls 0–3, deterministically from its seed.

---

## 5. World Detail

`buildWorldDetail()` combines the orbital record with the biome table:

```c
worlddetail d;
buildWorldDetail(d, w, seed);

d.primaryBiome      // from temperature + water + habitability
d.secondaryBiome    // child or neighbour band
d.foodYield         // biome food
d.oreYield          // biome ore + (w.resources / 2)
d.energyYield       // biome energy
d.researchYield     // biome research
d.terrainDifficulty // (gravity / 3) + (hazard / 2), capped 100
d.hazardRating      // biome hazard
```

### 5.1 Colonisation cost

```
cost = biome.cost + (terrainDifficulty × 2) + (hazardRating × 3)
```

Impacts stack: a high-gravity Crystalline world costs roughly three times a
low-gravity Grassland world.

### 5.2 Settleability

A world can be settled when:

- habitability ≥ 25, **or** the biome is Crystalline or Fungal (life finds a
  way where the yield justifies the risk)
- hazard rating < 95 (Hive Worlds resist colonisation by design)

---

## 6. Class Taxonomy

20 classes across 5 trees, each 4 deep: base → three specialisations.

```
Warrior   → Guardian, Berserker, Champion
Scout     → Ranger, Assassin, Sniper
Engineer  → Technician, Mechanic, Architect
Psionic   → Mystic, Telepath, Warlock
Officer   → Commander, Admiral, Diplomat
```

### 6.1 Roles

| Role | Function |
| --- | --- |
| Tank | Holds threat, absorbs damage |
| Healer | Sustains the group |
| Melee DPS | Close-range damage |
| Ranged DPS | Long-range damage |
| Magic DPS | Elemental/void damage |
| Support | Buffs, utilities, repairs |
| Controller | Crowd control, zones |

### 6.2 Class record

```c
struct classinfo
{
    char name[32];
    characterclass parent;    // CLS_NONE for a base class
    classrole role;
    int baseStr, baseDex, baseCon, baseInt, baseWis;
    int baseVit, baseSpi, baseLck, baseWil, baseCha;
    int hpPerLevel, powerPerLevel, attackPerLevel, defencePerLevel;
    int resourceType;         // resourcetype enum
    char signatureSkill[32];
};
```

### 6.3 Base classes compared

| Class | Role | STR | DEX | CON | INT | WIS | HP/lvl | Resource |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Warrior | Tank | 12 | 6 | 10 | 3 | 4 | 14 | Rage |
| Scout | Ranged DPS | 6 | 12 | 6 | 5 | 6 | 10 | Energy |
| Engineer | Support | 7 | 6 | 8 | 11 | 8 | 11 | Heat |
| Psionic | Magic DPS | 4 | 7 | 6 | 12 | 12 | 9 | Aether |
| Officer | Support | 8 | 8 | 8 | 8 | 8 | 11 | Focus |

### 6.4 Specialisations

Specs trade breadth for focus. The Warrior tree is the clearest example:

| Class | Role | Signature | Identity |
| --- | --- | --- | --- |
| Warrior | Tank | Shield Wall | Baseline |
| Guardian | Tank | Bulwark | +CON, +armor, +HP — pure mitigation |
| Berserker | Melee DPS | Bloodlust | +STR, −CON — glass cannon |
| Champion | Melee DPS | Executioner | Balanced bruiser |

### 6.5 Advancement

`classCanAdvance(from, to)` is a direct parent/child test, so the tree is
strict: a Warrior can become a Guardian but not a Sniper. Multi-step paths
(e.g. Warrior → Champion → something later) would need a depth field.

`applyClass(stats, class)` adds the class's base attributes onto a stat block
and re-derives sub-attributes and combat stats.

---

## 7. API Summary

**Biomes**

```c
void initializeBiomes();
biome selectBiome(int temperature, int waterCoverage, int habitability);
biome selectSecondaryBiome(biome primary, int seed);
int rollBiomeFeatures(biomefeature out[], int outMax, int seed);
void buildWorldDetail(worlddetail &d, const struct world &w, int seed);
int worldFoodYield(const worlddetail &d);
int worldOreYield(const worlddetail &d);
int worldEnergyYield(const worlddetail &d);
int worldResearchYield(const worlddetail &d);
int worldColoniseCost(const worlddetail &d);
int worldIsSettleable(const worlddetail &d, int habitability);
```

**Classes**

```c
void initializeClasses();
characterclass findClass(const char *name);
int classChildren(characterclass parent, characterclass out[], int outMax);
int classCanAdvance(characterclass from, characterclass to);
void applyClass(struct statblock &stats, characterclass c);
classrole classRoleOf(characterclass c);
```

---

## 8. Open Questions

- **Biome drift.** Terraforming would let a colony change a world's biome over
  turns. The record has no `terraformTarget` field yet.
- **Class cross-speccing.** The tree is strictly hierarchical. Hybrid specs
  (a Warrior who takes Engineer utilities) need a second axis.
- **Unit types.** `biominfo.unitType` is an index with no table behind it;
  the RTS layer needs a real unit taxonomy.
