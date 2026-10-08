# Architecture

**Project:** spacebattlerpg 2.0.0 (krypton v1.0)
**Status:** reflects the codebase as built

---

## 1. Layers

Three layers, each usable without the one above it.

```
┌──────────────────────────────────────────────────────────────┐
│  PRESENTATION      ui/  ·  client/                           │
│  Display, navigation, HUD, title screens, CLI                │
├──────────────────────────────────────────────────────────────┤
│  ENGINES           engine/  ·  audio/  ·  server/  ·  db/    │
│  Stateful subsystems behind one lifecycle contract           │
├──────────────────────────────────────────────────────────────┤
│  LIBRARY           library/                                  │
│  Pure functions, no global state, testable in isolation      │
├──────────────────────────────────────────────────────────────┤
│  TYPES             core/types.h                              │
│  Structs, enums, constants. Data only.                       │
└──────────────────────────────────────────────────────────────┘
```

**Rule: dependencies point downward only.** `library/` never includes
`engine/`; `engine/` never includes `ui/`. A violation is a compile error,
which is why the layering has held.

---

## 2. Module Map

| Directory | Contents | State |
| --- | --- | --- |
| `core/` | types, globals, input, space map | working |
| `data/` | Content tables: ships, gear, bosses, effects, factions | working |
| `library/` | Stat maths, effects, item value, scaling, Lua bridge | working |
| `engine/` | Kernel + 6 subsystems | working |
| `audio/` | Cue dispatcher and manifest | working (null backend) |
| `db/` | MySQL schema and CSV export | CSV working |
| `ui/` | Display, navigation, HUD, title screens | working |
| `world/` | Universe generation, biomes, classes | working |
| `game/` | Game logic: combat, threat, loot, AI, turns, scheduler | working |
| `server/` | Sessions, accounts, protocol | accounts working |
| `client/` | Thin client and CLI | headers only |
| `scripts/` | Lua content | stub |
| `config/` | ini files | working |
| `docs/` | This documentation set | — |

---

## 3. The Engine Kernel

Every engine implements the same five-phase contract:

```c
struct subsystem
{
    const char *name;
    subsystemid id;
    int initialized;
    int running;
    int priority;

    int  (*init)();
    int  (*start)();
    int  (*update)(int ticks);
    void (*stop)();
    void (*shutdown)();
};
```

The kernel registers them by **priority** and drives the lifecycle. Shutdown
runs in **reverse** priority order, so dependents come down before their
dependencies.

| Subsystem | Priority | Owns |
| --- | --- | --- |
| Audio | 10 | Cue dispatch, bus volumes |
| Effect | 15 | Live effect lists, DR records |
| Combat | 20 | Fight state machine, turn pipeline |
| Battle | 25 | Encounter sequencing, waves, phases |
| AI | 30 | Brains, state machine, utility scoring |
| Loot | 35 | Drop rolls, pity timers |
| Progression | 40 | XP, levels, attribute spend |

Priority is **dependency order, not importance**. The effect engine starts
before combat because combat applies effects.

---

## 4. Header Ordering

`include/spacebattlerpg.h` is the only header a source file includes. Order
matters in exactly one place:

```c
// 1. standard library
// 2. core/          types, globals, input
// 3. data/          content tables
// 4. library/       pure maths
// 5. audio/ db/     leaf subsystems
// 6. game/          game logic  ← BEFORE engine/
// 7. engine/        engines
// 8. ui/            presentation
// 9. game/gamelogic.h  (ties it together)
```

**Why `game/` precedes `engine/`:** the engine headers operate on runtime
types declared in `game/` — `aibrain`, `lootresult`, `encounterinstance`. If
`engine/` came first, those would be incomplete types.

Every module header has its own include guard, so individual inclusion is
safe. `db/db.h` explicitly includes `core/types.h` so it resolves standalone;
without that, opening it directly in an editor shows phantom `player is
undefined` errors.

---

## 5. Data Flow

### 5.1 Stat resolution

```
core attributes          (persistent growth)
      ↓ derive
sub-attributes           (physical power, armor, tenacity, ...)
      ↓ roll up
combat stats             (final runtime values)
      ↓ formulas
damage / heal
      ↓ modifiers
buffs + debuffs + diminishing returns
      ↓ context
PvE / PvP multiplier
      ↓ encounter
encounter modifier
```

Baseline resolution:

```
hit roll → crit roll → mitigation → resistance → apply
```

Every stage is a function in `library/` or `game/combat.c++`, and each is
callable in isolation — which is what makes the numbers testable without
running a battle.

### 5.2 Content flow

```
C++ tables  ──▶  engine       (runtime)
    ▲
    │ dbSaveEverything / csvExportAll
    │
  MySQL  ◀──▶  CSV  ◀──▶  spreadsheets
    ▲
    │ luaLoadItem / luaLoadEffect / luaLoadBoss
    │
  Lua scripts
```

Content is data. Adding a weapon, an effect or a boss is a table row or a
script, not a compile.

---

## 6. Conventions

### 6.1 Style

- C++17, `-Wall -Wextra`, **zero warnings**
- 4-space indent, brace on its own line
- `snake_case` structs (legacy), `camelCase` functions, `PascalCase` constants
- One module = one header + one source, both with the `SBW_` guard prefix
- Fixed-size arrays throughout; no dynamic allocation anywhere

### 6.2 No dynamic allocation

Every table has a compile-time cap (`MAX_ITEMS`, `MAX_EFFECTS`,
`MAX_SESSIONS`…). The engine never calls `new` or `malloc`.

This is why the arrays use `#define` sizes rather than `std::vector`: the
project predates the framework layer and the whole engine is built to be
allocation-free and predictable.

### 6.3 Determinism

Randomness goes through either:
- `randomNumber(max)` for gameplay rolls (1..max)
- the private LCG in `universe_gen.c++` for generation

Generation is **seeded**, so a universe is reproducible. Gameplay rolls are
not, deliberately — a seeded combat roll would be exploitable.

### 6.4 Error handling

No exceptions. Functions return the result, or `-1` for failure, or a
positive error code where the caller needs to distinguish causes
(`validateFit`, `accountLogin`).

Error text comes from one place per domain: `protoErrorText()`,
`accountResultText()`, `luaLastError()`, `dbLastError()`, `audioLastError()`.

---

## 7. Optional Features

Three subsystems compile as working stubs and go live with a define:

| Define | Requires | Effect |
| --- | --- | --- |
| `SBW_ENABLE_LUA` | a Lua library | Real script execution |
| `SBW_ENABLE_MYSQL` | `libmysqlclient` | Live MySQL persistence |
| `SBW_ENABLE_AUDIO` | an audio backend | Real sound rendering |

The stub-to-live seam is one function per subsystem, marked in the source:

```c
/* Real implementation: luaL_dostring(state, chunk). */
```

This pattern means the whole project builds and runs with zero external
dependencies, and each feature is opted into independently.

---

## 8. Build

`build.ps1` holds the file list and is the single source of truth:

```powershell
clang++ -std=c++17 -Wall -Wextra -Iinclude -o spacebattlerpg.exe @sources
```

`-Iinclude` is required: the master header resolves via the include path, not
a relative path.

**Locking:** a running `spacebattlerpg.exe` holds the output file, so a
rebuild fails at link. Kill it first:

```powershell
Get-Process spacebattlerpg -ErrorAction SilentlyContinue | Stop-Process -Force
```

---

## 9. Known Gaps

| Gap | Impact |
| --- | --- |
| New modules not in `build.ps1` | Universe, biome, turn, scheduler, starship, server and client files compile but aren't linked yet |
| `main.c++` doesn't call the new systems | `generateUniverse()`, `turnInit()`, `schedulerInstallDefaults()` not invoked at boot |
| `server.c++`, `client.c++`, `protocol.c++`, `navigation.c++` | Headers written, sources pending |
| 4X layer | Seeded but not simulated — no per-turn faction expansion |
| Account hashing | Placeholder KDF, must be replaced before public use |

The build is clean at every stage that exists: **zero warnings, zero errors**.
