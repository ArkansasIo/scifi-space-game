# Space Battle Wars - RPG / MMO Framework

Design reference for the `krypton v1.0` engine
(Space Battle Wars Simulation Program 2.0.0).

This document is the single source of truth for the stat model, combat math,
equipment, buffs/debuffs, enemies, AI, scaling and value formulas. Everything
here is written to map 1:1 onto the C++ types in `include/core/types.h` and the
modules under `include/data/` and `include/game/`. Where a rule is not yet
implemented in code, it is marked **[planned]**.

---

## 0. Layered Model

Combat power is resolved in layers, cheapest first:

```
Core Attributes      (persistent, grown by level / talent / class)
        ↓
Sub-Attributes       (derived from primaries + gear + passives)
        ↓
Combat Stats         (final runtime values used in formulas)
        ↓
Buffs / Debuffs      (temporary modifiers with stacking + DR)
        ↓
Context Scaling      (PvE vs PvP multipliers)
        ↓
Encounter Modifier   (boss tuning, difficulty, party size)
```

`Item Level ≠ Power ≠ Effectiveness`. Every number below is context dependent.

### 0.1 Mapping to this codebase

| Concept | Engine implementation |
| --- | --- |
| Core attributes | `struct player` (`power`, `sif`, `health`, `level`, `sp`) |
| Technology | `struct Technology` (`attackpower`, `defencepower`, `bonus`) |
| Upgrades | `struct upgrads` |
| Artifacts / Glyphs / Enchantments | `struct ship_item` (shared shape) |
| Enemy ships | `struct enemyships` (`sp`, `health`, `power`, `sif`) |
| Enemy classes | `struct enemyshipclass` |
| Locations | `struct space` (`forward` / `backward` / `starboard` / `port`) |
| Story beats | `struct storynode` |
| Difficulty | `struct difficultmode` |
| Run score | `struct targetscore` |

Global instances live in `src/core/universe.c++` and are declared `extern` in
`include/core/globals.h`. All tables are `[17]` sized, matching the existing
data arrays.

---

## 1. Core Attributes

Persistent growth stats. `struct player` currently holds the space-navy subset;
the full RPG table is the target schema.

| Attribute | Function | Engine field |
| --- | --- | --- |
| STR | Physical / hull-breaching damage | `attackpower` contribution |
| DEX | Accuracy, evasion, crit, initiative | `[planned]` |
| CON | Health, resistances | `health` |
| INT | Spell / tech power | `[planned]` |
| WIS | Repair & energy sustain | `[planned]` |
| VIT | Max SIF | `sif` |
| SPI | Resource regeneration | `[planned]` |
| LCK | Proc & drop chance | `[planned]` |
| WIL | Crowd-control resistance, resolve | `[planned]` |
| CHA | Threat control, leadership | `[planned]` |

Rule: these grow by leveling, talent points and base class only.

---

## 2. Sub-Attributes

Derived from primaries + gear + passives.

**Physical** — Physical Power, Accuracy, Armor Penetration, Attack Speed,
Crit Chance, Crit Damage, Evasion, Block / Parry, Stagger / Poise Damage,
Weapon Mastery (per type).

**Magical / Tech** — Spell Power, Cast Speed, Magic Penetration, Elemental
Mastery, Healing Power, Control Power (CC strength), Mana Efficiency,
Overcharge / Overheal.

**Survival** — Max HP, HP Regen, Shield Power (absorb), Armor, Magic
Resistance, Damage Reduction %, Tenacity (anti-CC), Poise (anti-stagger),
Barrier Regen.

---

## 3. Combat Stats (Final Runtime Values)

**Offense** — Base Attack (ATK), Skill Power %, Crit Chance, Crit Damage,
Vulnerability Damage %, Backstab / Weakpoint Multiplier, DoT Power,
Execute Damage %.

**Defense** — Armor, Resistances (per type), Mitigation %, Block Chance +
Block Value, Parry Window / Chance, Dodge Chance, Damage Taken Modifiers
(PvP dampening, boss tuning).

**Tempo** — Move Speed, Attack Speed, Cast Speed, Cooldown Reduction (CDR),
Global Cooldown (GCD), Resource Regen.

---

## 4. Resources

Standard pools: HP, MP, Stamina.
Class bars: Rage, Energy, Focus, Faith, Aether, Heat, Sanity.

Current engine equivalent: `health` (HP), `sif` (structural integrity field),
`power` (energy / output), `sp` (ship points, the battle HP used by
`initializeBattle`).

---

## 5. Damage Types & Resistances

**Damage types** — Physical, Fire, Ice, Lightning, Earth, Wind, Water, Light,
Dark, Arcane, Poison, Bleed, Void, Chaos, True.

**Resistance layers** (use all four):

1. Flat resist — subtract from raw.
2. % resist — multiplier.
3. Absorption — shield-like pool.
4. Immunity / Vulnerability — boolean flags.

Bosses lean on layers 2 and 4, gated behind breakable windows.

---

## 6. Buffs & Debuffs

### 6.1 Buff categories

- **Offensive** — Power Up, Crit Up, Attack Speed Up, Penetration Up,
  Elemental Infuse, Berserk.
- **Defensive** — Shield, Fortify, Damage Reduction, Regen, Barrier, Anti-crit.
- **Utility** — Haste, Stealth, True Sight, Cleanse-over-time,
  Immunity Window, Phase Shift.

### 6.2 Debuff categories

- **Crowd Control (CC)** — Stun, Freeze, Root, Silence, Fear, Charm, Sleep,
  Knockdown, Knockback.
- **DoT / Degens** — Burn, Poison, Bleed, Corruption, Shock, Frostbite.
- **Stat Breaks** — Weaken (-ATK), Shatter (-Armor), Curse (-All), Slow
  (-Speed), Mana Burn.

### 6.3 Stacking rules

Every effect defines: `Stack Mode` (None | Refresh Duration | Additive Stacks |
Replace Stronger | Independent Instances), `Max Stacks`, and `Scaling`
(linear / exponential / diminishing).

### 6.4 Diminishing Returns (CC)

Standard DR ladder, applied per target inside a DR window:

| Application | Duration |
| --- | --- |
| 1st | 100% |
| 2nd | 50% |
| 3rd | 25% |
| 4th | immune (temporary) |

### 6.5 Cleanse & Immunity

Cleanse types: Physical, Magic, Curse, Poison, Bleed, All.
Immunity windows: `Unstoppable` vs `CC Immune` vs `Damage Immune`.

---

## 7. Enemy System

### 7.1 Taxonomy

| Tier | Iface | Baseline vs player |
| --- | --- | --- |
| Trash Mob | `enemyships` | 60–80% |
| Strong / Veteran | `enemyships` | 120–150% |
| Elite | `enemyships` | 200–300% |
| Champion | `enemyships` + affix | 250–400% |
| Dungeon Boss | `enemyships` | 500–1500% |
| World Boss | `enemyships` | 1000–2500% |
| Raid Boss | `enemyships` | 2000–5000% |
| Mythic / Ascended | `enemyships` + rules | 5000%+ |

### 7.2 AI roles

Bruiser, Tank, Assassin, Sniper, Caster, Controller, Summoner,
Healer / Support, Trickster.

### 7.3 Rank multipliers

Normal x1.0 · Veteran x1.2 · Elite x1.5–2.5 · Champion x2.5–4.0 (affix) ·
Boss x5–20 · Raid Boss x20–200 (player count) · Mythic x200+.

### 7.4 How this codebase handles it today

`initializeEnemyships()` in `src/data/enemyships.c++` seeds all 17 hulls from
`enemySeeds[]` and derives `attackpower = power * 2`,
`defencepower = sif * 2`. `initializeBattle()` in `src/game/battle.c++` picks a
random index in `0..16`, rerolls `sp = 60 + rand() % 41` for a fresh copy each
fight, then resolves a turn loop.

**[planned]** A `rank` / `role` field on `enemyships` and a scaling helper that
applies the rank multiplier above.

---

## 8. Boss Framework

### 8.1 Boss metadata (data-driven)

Name, Rank, Level, Tags; HP pools per phase; damage profile (types + weights);
resist / immunity tables; phase triggers; ability kits (cooldowns, priorities);
add spawners; arena rules; enrage rules; loot table; scaling rules.

### 8.2 Phase triggers

HP thresholds (80 / 60 / 40 / 20), time-based (e.g. every 90 s), event-based
("3 crystals destroyed"), behavior-based ("tank swap failed").

### 8.3 Reusable mechanics modules

- **Damage & Pressure** — raid-wide AoE pulse, targeted beam / chain lightning,
  meteors / falling zones, soft enrage ramp.
- **Control & Positioning** — knockback into hazards, pull + root,
  rotating lasers / line sweeps, "don't look" gaze.
- **Puzzle / Objective** — break shield with a specific element, interrupt cast
  cycles, kill adds to stop wipe, carry objects to seals.
- **Team Coordination** — tank swap (debuff stacks), split party (portals),
  soak, spread.

### 8.4 Enrage model

- Soft enrage: +X% damage every 15 s.
- Hard enrage: instant wipe cast at T = Y.

---

## 9. Aggro / Threat

Sources: damage dealt, healing done (weighted), buffs applied, proximity,
taunts.

Modifiers: tank stance +200–400%, DPS stance 0–100%, stealth −100% threat
generation, taunt forces target for N s and sets threat to `top + 1`.

```
Threat = Damage * D_Threat + Healing * H_Threat + FlatThreat + TauntOverride
```

**[planned]** `initializecreatPlayerdata()` in `src/game/player.c++` is the
natural home for a threat accumulator.

---

## 10. Scaling & Difficulty

### 10.1 Axes

Level scaling, party-size scaling (1–40), difficulty tier scaling
(Normal → Mythic), time scaling (events, soft enrages), gear-score
normalization for PvP / instanced play.

### 10.2 Curves

```
HP  = BaseHP  * (1 + 0.18 * LevelDiff) * TierMult * PartyMult
DMG = BaseDMG * (1 + 0.12 * LevelDiff) * TierMult * PartyMult^0.6
PartyMult = 1 + (Players - 1) * 0.65
```

HP scales faster than damage — never one-shot.

### 10.3 PvP normalization toolkit

Stat caps, CC caps + DR, healing reduction, burst dampening window,
resolve bar (anti chain-CC).

---

## 11. Affix System

- **Offensive** — Berserk, Frenzied, Sniper, Arcane Burst.
- **Defensive** — Fortified, Shielded, Reflective.
- **Utility** — Teleporting, Summoner, Illusionist.
- **Hazard / Rules** — toxic pools, fire trails, lightning orbs, no-res,
  permadeath, time pressure.

Stacking: Minor (1–2) + Major (0–1) + Seasonal (0–1).
Never pair `Reflective` with a high DoT aura in early game.

---

## 12. Loot & Rewards

**Categories** — currency, gear (rarities), crafting mats, set items,
relics / artifacts, mounts / pets, cosmetics, titles, quest items.

**Loot table** — base rolls, rarity weights, pity timers, per-week unique
flags, difficulty multipliers, distribution (need / greed / personal).

**Reward multipliers** — difficulty tier, boss rank, clear time, no-death
bonus, luck (bounded; never breaks the economy).

Existing hooks: `src/game/items.c++` (random item generator),
`continueStory()` in `src/game/story.c++`, and the score/kill counters in
`targetscore` (`src/game/score.c++`).

---

## 13. Encounter Templates

**Open world packs** — 3 trash + 1 caster + patrol; rare spawn with a
condition; event wave (5–10 waves + boss).

**Dungeon rooms** — pack + hazards; pack + miniboss; puzzle room (switches);
timed gauntlet corridor.

**Raid** — boss + add cycle; boss + arena morph; two bosses shared arena
(split tanks); council fight (multi-target priorities).

---

## 14. AI Logic

### 14.1 States

`Idle → Patrol → Alert → Engage → Combat → Retreat → Reset`

### 14.2 Combat sub-states

Maintain Range, Close Distance, Burst Window, Defensive Window, Summon Adds,
Cast Ultimate, Phase Transition.

### 14.3 Ability selection (utility score)

Each ability scores on distance fit, target HP, cooldown ready, phase
allowed, player clustering, and threat target (tank vs healer). Pick the
highest score, then apply "no-repeat" lockouts.

---

## 15. Combat Formulas

### 15.1 Hit & crit

```
HitChance      = clamp(Accuracy / (Accuracy + Evasion), 0.05, 0.95)
CritChance     = clamp(BaseCrit + CritRatingScale, 0, CritCap)
CritMultiplier = 1 + CritDamage%
```

### 15.2 Mitigation

```
Mitigation   = Armor / (Armor + K * AttackerLevel)     // K = 50..100
DamageTaken  = RawDamage * (1 - Mitigation) * (1 - Resist%) * VulnerabilityMods
```

### 15.3 DoT tick

```
TickDamage = (DoTPower * SkillCoef) * (1 - MitigationRelevant)
```

### 15.4 Healing

```
Heal = (HealingPower * SkillCoef) * (1 + BonusHeal%) * (1 - HealReductionDebuff)
```

### 15.5 Engine baseline

The turn loop in `initializeBattle()` already implements a stripped version:

```
attackpower       = (charactership.power + charactership.weapon) * rand(1..10)
enemyshipsdefence = enemyshipsArray[i].sif * rand(1..5)
if attackpower > enemyshipsdefence:
    enemyshipsArray[i].sp -= (attackpower - enemyshipsdefence)
```

---

## 16. Item Power & Value Formulas

### 16.1 Power Units

```
StatValuePU = RawStat * StatWeight * ScalingCurve
```

Baseline weights:

| Stat | Weight |
| --- | --- |
| STR / DEX / INT | 1.00 |
| VIT / CON | 0.85 |
| Crit Chance | 2.50 |
| Crit Damage | 1.75 |
| Attack Speed | 2.00 |
| CDR | 2.25 |
| Armor | 0.75 |
| Resistance | 0.90 |
| PvP Resilience | 3.00 |

### 16.2 Scaling curves

```
Linear        ScalingCurve = 1.0
Soft Cap      EffectiveStat = RawStat / (RawStat + SoftCap)
              ScalingCurve  = EffectiveStat * SoftCap
Diminishing   EffectiveValue = Cap * (1 - e^(-RawStat / K))
```

### 16.3 Weapon power

```
WeaponPower_PvE = (BaseDamage * WeaponSpeedFactor
                 + AttributeScaling
                 + ElementalScaling) * PvEMultiplier

WeaponPower_PvP = (NormalizedBaseDamage
                 + AttributeScaling * PvPStatScale) * PvPDampening
```

`PvPStatScale = 0.65`, `PvPDampening = 0.75..0.85`.

### 16.4 Defensive power

```
TDP = (HP * 0.4 + Armor * 0.3 + Resist * 0.3) * SurvivalMultiplier
```

### 16.5 Gear score

```
ItemPower   = Sum(StatValuePU) + ProcValue + SetBonusValue
ProcValue   = EffectMagnitude * Uptime * ImpactWeight
```

Worked example:

```
BurnProcValue = 300 DPS * 0.25 uptime * 1.2 = 90 PU
```

### 16.6 Buff & CC value

```
BuffValue  = StatIncrease * Duration * TargetCount * ContextMultiplier
CCValue    = BaseDuration * ControlWeight * DRMultiplier * TargetImportance
```

### 16.7 Worked item calc

PvE sword:

```
STR +50         -> 50  * 1.00 =  50.0
Crit +5%        -> 5   * 2.50 =  12.5
FireDamage +120 -> 120 * 1.30 = 156.0
Proc            ->               90.0
Total Item Power              = 308.5
```

PvP version applies the conversion table below -> ~185 effective.

---

## 17. PvE vs PvP Conversion

Stat multipliers:

| Stat | PvE | PvP | Note |
| --- | --- | --- | --- |
| Damage | 1.00 | 0.70 | burst control |
| Crit Chance | 1.00 | 0.50 | hard capped |
| Crit Damage | 1.00 | 0.60 | |
| Healing | 1.00 | 0.65 | anti-stall |
| CDR | 1.00 | 0.50 | |
| Armor | 1.00 | 1.10 | survivability |
| CC Duration | 1.00 | 0.40 | DR applied |

### 17.1 Weapon rules

**PvE** — high raw damage, elemental scaling, proc effects, boss bonuses
(+15% vs bosses, burn on hit, lifesteal vs mobs, execute <30% HP, CDR).

**PvP** — normalized base damage, reduced proc RNG, resilience penetration,
anti-heal, capped CC duration, anti-shield damage, resource drain.

### 17.2 Armor rules

**PvE** — armor / magic resist, elemental resist, set bonuses, boss damage
reduction, enrage mitigation, threat generation on tank gear.

**PvP** — Resilience (anti-crit / anti-burst), Tenacity (CC resist), flat
damage reduction, healing-received modifiers, anti-execute protection.

### 17.3 Slots

Head, Chest, Legs, Gloves, Boots, Weapon (main / off-hand), Ring x2, Amulet,
Cloak, Belt, Trinket x2.

### 17.4 Rarity

| Rarity | PvE | PvP |
| --- | --- | --- |
| Common | Leveling | Entry gear |
| Uncommon | Early dungeons | Starter PvP |
| Rare | Build-defining | Ranked viable |
| Epic | Endgame | Competitive |
| Legendary | Unique mechanics | Cosmetic + minor bonus |
| Mythic | Boss-only | Disabled or normalized |

### 17.5 Trinkets & consumables

**PvE trinkets** — on-hit procs, boss phase triggers, damage ramps, summons,
elemental surges.

**PvP trinkets** — CC break (mandatory), anti-burst shield, resource denial,
mobility escape, cleanse.

**PvE consumables** — potions, buff food, elixirs, resistance flasks,
scrolls, summoning items.

**PvP consumables** — limited-use potions, diminishing healing, CC break
items, anti-stealth flares. No stat stacking.

### 17.6 Enhancements

**PvE** — elemental gems, damage runes, proc enchants, boss augments.
**PvP** — flat stat gems, CC resist gems, heal-reduction gems, no proc RNG.

---

## 18. Item Schema

```
ItemData
├── Name
├── Type                    (Weapon / Armor / Trinket)
├── Slot
├── PvEStats
│   ├── BaseStats
│   ├── Procs
│   ├── ScalingRules
│   └── SetLinks
├── PvPStats
│   ├── NormalizedStats
│   ├── Caps
│   ├── PvPModifiers
│   └── DRRules
├── Rarity
├── RequiredLevel
├── BindRules
└── CosmeticData
```

Implemented today as `struct ship_item` (name, description, type, level,
lightlevel, bonus, attackpower, defencepower, itemclass) and
`struct Technology` / `struct upgrads`. See `src/game/items.c++`.

---

## 19. Hard Caps & Safety Limits

| Stat | Cap |
| --- | --- |
| Crit Chance | 40% PvP / 70% PvE |
| Cooldown Reduction | 35% PvP / 60% PvE |
| CC Duration | 50% PvP |
| Damage Reduction | 70% |
| Healing Bonus | 50% PvP |

---

## 20. Master Formulas

### 20.1 Effective combat value

```
EffectiveCombatValue = ( Sum(Stats * Weights * Curves)
                       + Sum(Procs * Uptime)
                       + Sum(Buffs * Duration) )
                     * PvE/PvPMultiplier
                     * EncounterModifier
```

### 20.2 Encounter power budget

```
EncounterPB = Sum(PlayerEffectivePower) * DifficultyFactor
```

Boss abilities must not exceed PB unless they have a telegraph > 1.5 s,
counterplay, or limited frequency.

---

## 21. Tuning Rules (non-negotiable)

- HP scaling must exceed damage scaling.
- Cap the Luck impact — economy safety.
- DR on CC always, in PvP and on bosses.
- Bosses use windows (break shields, stagger), never permanent vulnerability.
- Every new stat must declare: source, cap / soft cap, PvP behaviour, scaling
  curve, and UI exposure.

---

## 22. Sci-Fi Layer: Galaxy / 4X / RTS / Turn-Based

The same framework extends upward into a persistent galaxy.

### 22.1 Layers

```
Player Character  (RPG)   - ground missions, boarding, raids
Fleet Command     (RTS)   - real-time ship combat
Empire Management (4X)    - explore / expand / exploit / exterminate
Galaxy Simulation (MMO)   - persistent sectors, events, economy
```

### 22.2 Galaxy map

```
Galaxy
└── Sectors
    └── Star Systems
        ├── Planets
        ├── Asteroid Fields
        ├── Stations
        └── Anomalies
```

### 22.3 Empire attributes

Industry (production), Science (research), Economy (credits),
Influence (diplomacy), Logistics (fleet supply), Intelligence (espionage),
Stability (rebellion resistance).

### 22.4 Resources

Primary: Credits, Minerals, Gas, Energy, Data.
Advanced: Dark Matter, Antimatter, Nanites, Quantum Cores, Alien Artifacts.
Flow: Planets -> Stations -> Trade Routes -> Empire Storage.

### 22.5 Ship classes

Fighter, Corvette, Frigate, Destroyer, Cruiser, Battleship, Carrier,
Dreadnought, Titan, Flagship.

### 22.6 Space combat

```
HitChance   = Accuracy / (Accuracy + TargetEvasion)
Damage      = WeaponDamage * TechMultiplier * CommanderBonus
FinalDamage = Damage * (1 - ShieldReduction) * (1 - ArmorReduction)
CritChance  = WeaponCrit + CrewSkill + SensorAdvantage
FleetPower  = Sum(ShipCombatValue * CrewSkill * TechMultiplier)
```

### 22.7 Ground combat (turn-based)

Units: Infantry, Mechs, Drones, Vehicles, Heroes.
`Initiative = Speed + TacticalSkill + EquipmentBonus`.
Actions: Move, Attack, Ability, Overwatch, Use item.

### 22.8 Research

```
ResearchTime = TechCost / (ScienceOutput * ScientistSkill)
```

Categories: Physics (lasers, shields, sensors), Engineering (armor, hulls,
engines), Biotech (cloning, cybernetics, gene mods), Psionics (psychic
powers, mind shields, warp travel).

### 22.9 Diplomacy & trade

```
DiplomaticScore = Influence + Reputation + TradeValue + SharedEnemies
TradeIncome     = TradeValue * RouteSecurity * DistanceModifier
```

Actions: Alliance, Trade Agreement, Research Pact, Non-Aggression Pact,
War Declaration, Vassalization.

### 22.10 AI empire logic

```
Priority = ThreatLevel + ResourceNeed + ExpansionOpportunity + DiplomaticRisk
```

Behaviours: expansionist, defensive, economic, aggressive, technological.

### 22.11 Progression & PvP

XP -> Character Level · Fleet Victories -> Commander Rank · Faction Missions
-> Reputation · Territory Control -> Empire Rank.

PvP modes: fleet battles, ground combat, territory wars, guild wars, arena
tournaments — all using normalized scaling.

### 22.12 Victory conditions

Military, Economic, Technological, Diplomatic, Ascension.

### 22.13 Server architecture

```
Galaxy Server
└── Sector Servers
    └── System Instances
        ├── RTS Battles
        └── Ground Combat Instances
```

Persistent: empire state, fleets, trade routes, economy.

### 22.14 Master power

```
TotalPower = CharacterPower + FleetPower + EmpirePower
           + TechnologyPower + EconomicPower
```

**[planned]** The 4X / RTS layering is not yet in `universe.c++`; the current
`space` struct is the 17-node space map that this layer will sit on top of.

---

## Appendix A: Data Schemas

**StatBlock** — Attributes, DerivedStats, Resistances (type -> value),
Resources, Multipliers (PvE / PvP / boss tuning), Tags.

**Effect** — Type, Category, Duration, Stack rules, Magnitudes (flat / % /
curve), Tick rate (DoT), DR category, Cleanse type, Immunity interactions.

**EnemyArchetype** — Role, Preferred range, Aggro style, Ability kit, Loot
profile, Spawn rules.

**BossData** — Phases[], Mechanics modules[], Arena hazards[], Add spawners[],
Enrage rules, Loot tables.

## Appendix B: File Reference

| Area | File |
| --- | --- |
| Types | `include/core/types.h` |
| Globals | `include/core/globals.h` |
| Input / RNG | `src/core/input.c++` |
| Space map | `src/core/universe.c++` |
| Ship tables | `src/data/technology.c++`, `upgrades.c++`, `artifact.c++`, `glyph.c++`, `enchantment.c++` |
| Enemies | `src/data/enemyships.c++` |
| Player | `src/game/player.c++` |
| Battle loop | `src/game/battle.c++` |
| Items | `src/game/items.c++` |
| Story | `src/game/story.c++` |
| Modifiers | `src/game/modifier.c++` |
| Difficulty | `src/game/difficulty.c++` |
| Score | `src/game/score.c++` |
| Save / load | `src/game/save.c++` |
| Campaign menu | `src/game/campaign.c++` |

## Appendix C: Status Legend

- **Implemented** — active in the current engine.
- **[planned]** — specified here, not yet coded.
