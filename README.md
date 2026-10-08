# spacebattlerpg

Space Battle Wars Simulation Program 2.0.0 (krypton v1.0 engine).

A single-player, terminal-based sci-fi RPG / MMO simulation: turn-based ship
combat, RPG character progression, a 4X strategic layer, and data-driven
content that designers can edit without a recompile.

**Documentation:** [`docs/INDEX.md`](docs/INDEX.md) — start here.

| | |
| --- | --- |
| Design | [GDD](docs/GDD.md) · [FRAMEWORK](docs/FRAMEWORK.md) |
| Systems | [GAMELOGIC](docs/GAMELOGIC.md) · [UNIVERSE](docs/UNIVERSE.md) · [BIOMES](docs/BIOMES.md) · [STARSHIPS](docs/STARSHIPS.md) · [TURNS](docs/TURNS.md) · [SCHEDULER](docs/SCHEDULER.md) |
| Interface | [UI](docs/UI.md) · [CLIENT](docs/CLIENT.md) · [PROTOCOL](docs/PROTOCOL.md) · [SERVER](docs/SERVER.md) · [ACCOUNTS](docs/ACCOUNTS.md) |
| Data / infra | [ARCHITECTURE](docs/ARCHITECTURE.md) · [DATABASE](docs/DATABASE.md) · [LUA](docs/LUA.md) |

---

## Build

Requires a C++17 compiler. Built and tested with clang++ 22 (mingw target) on
Windows.

```powershell
.\build.ps1
```

`build.ps1` is the single source of truth for the file list. It compiles:

```
main.c++

core/      universe.c++, input.c++

data/      technology.c++, upgrades.c++, artifact.c++, glyph.c++,
           enchantment.c++, enemyships.c++
           stats.c++, effects.c++, archetypes.c++, affixes.c++,
           bosses.c++, gear.c++, loot.c++, encounters.c++, factions.c++
           starships.c++

library/   arraydata.c++, stats.c++, effects.c++, itemvalue.c++,
           scaling.c++, lua.c++

engine/    subsystem.c++, combat_engine.c++, battle_engine.c++,
           effect_engine.c++, ai_engine.c++, loot_engine.c++,
           progression_engine.c++

audio/     audio.c++, manifest.c++

db/        schema.c++, db.c++

world/     universe_gen.c++, biome.c++

server/    account.c++

ui/        display.c++, titlescreen.c++, hud.c++

game/      player.c++, battle.c++, items.c++, story.c++, campaign.c++,
           difficulty.c++, modifier.c++, score.c++, save.c++,
           combat.c++, threat.c++, encounter.c++, loot.c++, ai.c++,
           progression.c++, galaxy.c++, turn.c++, scheduler.c++,
           menus.c++, gamelogic.c++
```

Or manually, passing the same list:

```powershell
clang++ -std=c++17 -Wall -Wextra -Iinclude -o spacebattlerpg.exe @sources
```

Note the `-Iinclude` flag: the master header is found via the include path, not
by relative path.

### Optional features

Both compile as working stubs with no external dependency. Define these to go
live:

| Define | Requires | Effect |
| --- | --- | --- |
| `SBW_ENABLE_LUA` | a Lua library (`-llua54`) | real script execution |
| `SBW_ENABLE_MYSQL` | `libmysqlclient` | live MySQL persistence |
| `SBW_ENABLE_AUDIO` | an audio backend | real sound rendering |

The CSV / Excel export in `src/db/` and the audio cue manifest work fully
without any of these.

### Troubleshooting

If the link step fails with a permission error, a previous run is still
holding `spacebattlerpg.exe`. Close the game, or:

```powershell
Get-Process spacebattlerpg -ErrorAction SilentlyContinue | Stop-Process -Force
```

## Run

```powershell
.\spacebattlerpg.exe
```

Startup flow: bootloader banner → loading screen → title menu → story
opening → explore loop.

Controls: `F` forward, `B` backward, `S` starboard, `P` port, `C` stats,
`I` inventory, `M` map menu, `V` empire menu, `T` turn menu, `Q` quit.

### Menus

| Key | Menu | Sub-menus |
| --- | --- | --- |
| `M` | Map | Universe overview · Sectors · Systems · Current system · Interstellar objects · Jump routes · Biome list · Class tree |
| `V` | Empire | Character sheet · Empire dashboard · Fleet · Ship database · Boss roster · Item browser |
| `T` | Turn | Status · History · Steps · Scheduled jobs · Advance one turn |

Every sub-menu is a numbered list: digits pick, `0` returns to the parent.

## Layout

```
spacebattlerpg/
├── build.ps1                  Build script (Ctrl+Shift+B in VS Code)
├── .vscode/                   Editor config: include paths, build task
├── config/                    game.ini, database.ini
├── docs/                      Design and reference documentation (17 files)
├── scripts/                   Lua content: init, hooks, items/, effects/, bosses/
├── include/                   Headers (69)
│   ├── spacebattlerpg.h       Master header - include this from every source
│   ├── core/                  types, globals, input
│   ├── data/                  Ship tables, content tables, starships
│   ├── library/               arraydata, stats, effects, itemvalue, scaling, lua
│   ├── engine/                Subsystem contract + 6 engines
│   ├── audio/                 Cue dispatcher and sound manifest
│   ├── db/                    MySQL schema and CSV export
│   ├── world/                 universe_gen, biome, universe
│   ├── server/                protocol, server core, accounts
│   ├── client/                thin client and CLI
│   ├── ui/                    display, navigation, titlescreen, hud
│   └── game/                  Game modules + framework logic + menus
└── src/                       Implementation (62)
    ├── main.c++               Entry point and game loop
    ├── core/                  Global data, space map, input helpers
    ├── data/                  Ship tables and framework content tables
    ├── library/               Stat math, effects, item value, scaling, Lua
    ├── engine/                Kernel + combat / battle / effect / ai /
    │                          loot / progression engines
    ├── audio/                 Cue dispatcher and sound manifest
    ├── db/                    MySQL schema and CSV export
    ├── world/                 Procedural universe generation, biomes
    ├── server/                Accounts, registration and authentication
    ├── ui/                    Display, title/boot/loading, HUD
    └── game/                  Game modules + menus + turn + scheduler
```

## Architecture

Three layers, each usable without the one above it:

**1. `include/core/types.h`** — every struct, enum and constant. Data only.

**2. `src/library/`** — pure functions with no global state. Stat derivation,
combat formulas, effect stacking, item value, scaling curves. Testable in
isolation.

**3. `src/engine/`** — stateful subsystems behind one contract
(`subsystem.h`: init → start → update → stop → shutdown). The kernel
(`subsystem.c++`) registers them by priority and drives the lifecycle.

`src/game/` holds the original game modules plus the framework game logic
(threat, encounters, loot, AI, progression, galaxy, gamelogic).

### Data flow

```
attributes → sub-attributes → combat stats
                  ↓
         damage / heal formulas
                  ↓
   buffs + debuffs + diminishing returns
                  ↓
         PvE / PvP context multiplier
                  ↓
          encounter modifier
```

Baseline damage resolution:

```
hit roll → crit roll → mitigation → resistance → apply
```

## Persistence

The data tables live in fixed C++ arrays (`src/library/arraydata.c++`) and can
be mirrored to MySQL (24 tables, `src/db/schema.c++`) or exported to
spreadsheet-ready CSV for balance work. Config in `config/database.ini`.

## Scripting

Content is data, not code. `scripts/` holds Lua definitions for items,
effects and bosses, plus an event-hook file. Events (`SBW_EVENT_*` in
`library/lua.h`) fire from combat, loot and progression. See
`scripts/README.md`.

## Headers

`include/spacebattlerpg.h` is the only header a source file needs to include.
It pulls in the standard library, then `core/` (types, globals, input), the
data tables, the framework library, audio, db, the game modules, the engine
headers, and the UI. Order matters in one place: `game/` comes before
`engine/` so the engine headers can see the runtime types they operate on
(`aibrain`, `lootresult`, `encounterinstance`).

Each module header has its own include guard, so including them individually
is safe — and `include/db/db.h` explicitly includes `core/types.h` so it
resolves standalone.

## Status

| Area | State |
| --- | --- |
| Original game (map, battle, items, story, score, save) | working |
| Stat model, effects, item value, scaling | working |
| Engine kernel + 6 engines | working |
| UI: display, navigation, boot, title, loading, HUD | working |
| Menus: Map / Empire / Turn with sub-menus | working |
| Procedural universe (32 systems, moons, gates, anomalies) | working |
| Biomes (28), classes (20), starships (13 hulls) | working |
| Turn system + scheduler (8 jobs) | working |
| 12 acts / 50 chapters | working |
| Audio cue dispatcher + manifest | working (null backend) |
| MySQL / CSV persistence | CSV working; MySQL needs `SBW_ENABLE_MYSQL` |
| Lua scripting | stub; hook dispatch works, needs `SBW_ENABLE_LUA` |
| Accounts (register / login / logout) | working |
| Server core, protocol, client | headers only |
| 4X layer (systems, planets, fleets, diplomacy) | seeded, not yet simulated |

The build is clean under `-Wall -Wextra`: zero warnings, zero errors.
