# Space Battle Wars -- Game Design Document (GDD)

**Version:** 2.0.0 · **Engine:** krypton v1.0

This is the high-level design. For the systems reference see
[FRAMEWORK.md](FRAMEWORK.md); for the function-by-function breakdown see
[GAMELOGIC.md](GAMELOGIC.md); for the repository layout see
[ARCHITECTURE.md](ARCHITECTURE.md).

---

## 1. High Concept

A single-player, terminal-based **sci-fi RPG / MMO simulation** that combines:

- **Turn-based tactical combat** between capital ships
- **RPG character progression** with attributes, gear, effects and bosses
- **A 4X strategic layer** of empires, systems, planets and diplomacy
- **Live-content scripting** so items, effects and bosses are data, not code

You are a ship captain in a contested galaxy. You travel a 17-node space map,
fight fleets, collect technology, and grow from a lone frigate into a fleet
commander.

## 2. Pillars

1. **Every number has a reason.** Nothing is hardcoded without a weight, a cap
   and a scaling rule (docs/FRAMEWORK.md s.21).
2. **PvE and PvP never share a balance curve.** The same weapon carries two
   distinct stat tables.
3. **Data over code.** Stats, effects, bosses, loot and encounters are tables;
   Lua can add more without a rebuild.
4. **Runs are short, progression is long.** A run is a scoreable session; the
   character persists across runs.
5. **The console is a constraint, not a limitation.** 80x24 is the design
   target -- every panel is built to fit it.

## 3. Core Loop

```
   Title screen
        │
        ▼
   Boot + loading (config, tables, scripts)
        │
        ▼
   Explore space map ──────────────┐
        │                          │
        ├──▶ Encounter  ──▶ Combat ─┤
        │        │                  │
        │        └──▶ Loot / XP ────┤
        │                          │
        ├──▶ Item offer (yes/no) ───┤
        │                          │
        ├──▶ Story beat ────────────┤
        │                          │
        ▼                          │
   Boss / endgame ◀────────────────┘
        │
        ▼
   Score + save
```

## 4. Player Fantasy Progression

| Stage | Level | Ship | Content |
| --- | --- | --- | --- |
| Scout | 1-10 | Fighter / Corvette | Trash mobs, open-world packs |
| Captain | 11-25 | Frigate / Destroyer | Veterans, first dungeon boss |
| Commander | 26-45 | Cruiser / Battleship | Elites, champion affixes, sets |
| Admiral | 46-70 | Carrier / Dreadnought | World bosses, raid content |
| Legend | 71-100 | Titan / Flagship | Mythic bosses, ascension victory |

## 5. Combat Design

Turn-based, resolved in the order: `hit roll -> crit roll -> mitigation ->
resistance -> apply`.

- **Tempo** is the interesting axis: CDR, cast speed and initiative decide who
  acts and how often.
- **CC is always diminishing** -- no perma-stun, ever (FRAMEWORK.md s.6.4).
- **Bosses use windows**, not permanent vulnerability (s.21).
- **Damage scales slower than HP** so fights get longer, not spikier (s.10.2).

## 6. Encounter Design

Three families, all data-driven (FRAMEWORK.md s.13):

- **Open world** -- packs, rare spawns, event waves.
- **Dungeon rooms** -- hazards, minibosses, puzzles, timed gauntlets.
- **Raids** -- add cycles, arena morphs, split-tank fights, councils.

Every boss declares phases, mechanics, arena hazards, enrage rules and loot.

## 7. Economy and Loot

- Rarity ladder: Common -> Uncommon -> Rare -> Epic -> Legendary -> Mythic.
- **Pity timers** guarantee progress; **bounded luck** protects the economy.
- **Reward multipliers** come from difficulty, boss rank, clear time and a
  no-death bonus.
- Mythic PvP effects are normalized or cosmetic only.

## 8. PvE vs PvP

PvE is the power fantasy; PvP is the fairness contract.

| Axis | PvE | PvP |
| --- | --- | --- |
| Damage | 100% | 70% |
| Crit chance | 100% | 50% (capped 40%) |
| Healing | 100% | 65% |
| CC duration | 100% | 40% + DR |
| Procs | on | normalized or off |

Full table: FRAMEWORK.md s.17.

## 9. Strategic Layer (4X)

Empire attributes: Industry, Science, Economy, Influence, Logistics,
Intelligence, Stability.

Victory paths: Military, Economic, Technological, Diplomatic, Ascension.

Diplomacy and trade run on the same stat framework as combat, so a single
balance pass covers both.

## 10. Content Pipeline

```
   Excel / Google Sheets  ◀──▶  CSV  ◀──▶  MySQL  ◀──▶  C++ tables
                                    ▲
                                    │
                              Lua scripts
```

Designers edit spreadsheets or Lua; the engine imports either. See
[DATABASE.md](DATABASE.md) and [LUA.md](LUA.md).

## 11. Telemetry and Live Balance

Events (combat start, damage, kill, loot, level up, boss phase, PvP flag) are
dispatched through the scripting layer and persisted to the `Telemetry` table.
That closes the loop: play -> measure -> retune -> ship.

## 12. Accessibility / UX

- Plain-text output, no colour dependency.
- Numbered menus and single-key shortcuts everywhere.
- Every screen fits 80x24 (docs/FRAMEWORK.md, `MAXROW` / `MAXCOL`).
- Save / load from the title screen.

## 13. Out of Scope (v2.0.0)

Real-time fleet combat, 3D rendering, networking, voice, and a graphical
client. The 4X layer is specified and seeded but not yet simulated.
