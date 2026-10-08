# Documentation Index

Design and reference documentation for **spacebattlerpg 2.0.0**
(krypton v1.0 engine).

Start with [`GDD.md`](GDD.md) for the design, [`FRAMEWORK.md`](FRAMEWORK.md)
for the systems reference, and [`ARCHITECTURE.md`](ARCHITECTURE.md) for how
the code is put together.

---

## Design

| Document | Covers |
| --- | --- |
| [`GDD.md`](GDD.md) | High concept, pillars, core loop, progression, combat and encounter design |
| [`FRAMEWORK.md`](FRAMEWORK.md) | The full systems spec: stats, effects, damage, scaling, item value, PvE/PvP |

## Systems

| Document | Covers |
| --- | --- |
| [`GAMELOGIC.md`](GAMELOGIC.md) | Function-by-function reference for `src/game/` |
| [`UNIVERSE.md`](UNIVERSE.md) | Procedural galaxy generation: stars, worlds, moons, gates, anomalies |
| [`BIOMES.md`](BIOMES.md) | 28 biomes, terrain features, world detail, and the 20-class taxonomy |
| [`STARSHIPS.md`](STARSHIPS.md) | 13 hulls, 18 weapons, 34 components, fit validation and fleet maths |
| [`TURNS.md`](TURNS.md) | The 8-phase turn machine and calendar |
| [`SCHEDULER.md`](SCHEDULER.md) | Cron-style scheduled jobs |

## Interface

| Document | Covers |
| --- | --- |
| [`UI.md`](UI.md) | Top bar, left nav, menus and sub-menus, the 80×24 layout |
| [`CLIENT.md`](CLIENT.md) | The thin client cache, command wrappers and CLI shell |
| [`PROTOCOL.md`](PROTOCOL.md) | The line protocol: verbs, framing, error codes |
| [`SERVER.md`](SERVER.md) | Sessions, dispatch, authorization, the update loop |
| [`ACCOUNTS.md`](ACCOUNTS.md) | Registration, authentication, progression persistence |

## Data and infrastructure

| Document | Covers |
| --- | --- |
| [`ARCHITECTURE.md`](ARCHITECTURE.md) | Layers, module map, engine kernel, conventions, known gaps |
| [`DATABASE.md`](DATABASE.md) | The 24-table MySQL schema and CSV/Excel export |
| [`LUA.md`](LUA.md) | Scripting: API, events, content format |

---

## Quick Reference

### Where things live

| I want to change… | Look in |
| --- | --- |
| A stat's weight or cap | `src/data/stats.c++` |
| A buff or debuff | `src/data/effects.c++` |
| A ship, weapon or component | `src/data/starships.c++` |
| An item or set | `src/data/gear.c++` |
| A boss | `src/data/bosses.c++` |
| A drop table | `src/data/loot.c++` |
| A biome or class | `src/world/biome.c++` |
| How worlds are generated | `src/world/universe_gen.c++` |
| A combat formula | `src/library/scaling.c++`, `src/game/combat.c++` |
| What happens each turn | `src/game/turn.c++` |
| What runs on a schedule | `src/game/scheduler.c++` |
| A UI screen | `src/ui/` |
| Matchmaking or sessions | `src/server/` |

### Build and run

```powershell
.\build.ps1
.\spacebattlerpg.exe
```

### The one rule

**PvE and PvP never share a balance curve.** The same weapon carries two
stat tables; the same stat converts differently per context. Everything in
`FRAMEWORK.md` §17 follows from this.

### The one convention

`include/spacebattlerpg.h` is the only header a source file includes, and
`build.ps1` is the only build path. Keeping those two true is what makes the
project navigable at its current size.

---

## Documentation Status

Every document above is written against the code as it exists. Where
something is specified but not yet built, the document says so inline and
lists it under **Open Questions** — and `ARCHITECTURE.md` §9 collects the
project-wide gaps in one place.

The build is clean throughout: **zero warnings, zero errors** under
`-Wall -Wextra`.
