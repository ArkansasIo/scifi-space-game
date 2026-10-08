# Lua Scripting

**Module:** `include/library/lua.h` · `src/library/lua.c++`
**Scripts:** `scripts/`
**Status:** stub — API works, hook dispatch works, no VM linked

---

## 1. Purpose

Content is data, not code. Scripts let a designer add items, effects and
bosses, and hook game events, without a recompile.

---

## 2. Enabling It

The API compiles and works with **no external dependency**. With scripting
disabled, `luaIsEnabled()` returns 0 and `luaRunFile()` fails with:

```
scripting disabled: rebuild with -DSBW_ENABLE_LUA
```

To go live:

1. Build with `-DSBW_ENABLE_LUA`
2. Link a Lua library (e.g. `-llua54`)
3. Replace the bodies marked `/* Real implementation: … */` in
   `src/library/lua.c++` with real `lua_*` calls

There are exactly **six** such seams, so the swap is mechanical.

---

## 3. Script Layout

```
scripts/
├── README.md            API reference and event table
├── init.lua             entry point, runs on startup
├── hooks.lua            example event hook registrations
├── items/
│   ├── ashen_blade.lua  PvE + PvP stat tables
│   └── void_lance.lua   PvP-oriented weapon
├── effects/
│   └── burn.lua         a DoT debuff
└── bosses/
    └── ash_tyrant.lua   3-phase raid boss
```

---

## 4. API

| Function | Purpose |
| --- | --- |
| `luaStartup()` | Create the VM, register the engine API |
| `luaShutdown()` | Tear the VM down |
| `luaIsEnabled()` | 1 when a real VM is running |
| `luaRunString(chunk)` | Run a chunk of source |
| `luaRunFile(path)` | Run a script file |
| `luaCallFunction(name)` | Call a global function |
| `luaLoadItem(name)` | Load a scripted item into `itemArray[]` |
| `luaLoadItemDirectory(dir)` | Load every item script in a folder |
| `luaLoadEffect(name)` | Load a scripted effect |
| `luaLoadBoss(name)` | Load a scripted boss |
| `luaRegisterHook(event, fn)` | Run `fn` when an event fires |
| `luaFireEvent(event, a, b)` | Fire an event to all hooks |
| `luaRegisterEngineTables()` | Expose the data tables to scripts |
| `luaGetNumber(name)` / `luaSetNumber(name, v)` | Global numeric bridge |
| `luaLastError()` | Last error string (never null) |

---

## 5. Exposed Tables

Scripts can read the engine tables directly:

`stats`, `effects`, `archetypes`, `affixes`, `bosses`, `items`, `loot`,
`encounters`, `factions`.

Each mirrors the C++ array of the same name, so a script reads
`items[3].name` or `bosses[i].phases[1].hpThreshold`. They are **read-only**:
writing goes through the load functions, which validate before committing.

---

## 6. Events

| Constant | Value | Fired when |
| --- | --- | --- |
| `SBW_EVENT_COMBAT_START` | 1 | A battle begins |
| `SBW_EVENT_DAMAGE_DEALT` | 2 | Any damage is applied |
| `SBW_EVENT_ENEMY_KILLED` | 3 | An enemy is destroyed |
| `SBW_EVENT_LOOT_ROLLED` | 4 | A loot table is rolled |
| `SBW_EVENT_LEVEL_UP` | 5 | A character gains a level |
| `SBW_EVENT_BOSS_PHASE` | 6 | A boss changes phase |
| `SBW_EVENT_PVP_FLAG` | 7 | A player flags for PvP |

### 6.1 Hook registration

```lua
onEvent(SBW_EVENT_ENEMY_KILLED, function(enemyId, overkill)
  score.add(100 + overkill * 2)
end)
```

Up to **8 hooks per event**, dispatched in registration order.

### 6.2 Hooks register even while scripting is disabled

A deliberate choice: `luaRegisterHook()` records the function name in its
slot regardless of VM state, so content that registers during startup still
dispatches correctly once the VM comes up. `luaFireEvent()` returns the
**number of hooks that ran** — 0 with no VM, the real count otherwise.

This means the game's own flow doesn't need to check `luaIsEnabled()` before
every event. Events are free when scripting is off.

### 6.3 Where events fire

| Event | Call site |
| --- | --- |
| `COMBAT_START` | `initializeBattle()`, `beginCombat()` |
| `ENEMY_KILLED` | `reportEnemyDestroyed()`, `runCombatToCompletion()` |
| `LOOT_ROLLED` | `rollLoot()` |
| `LEVEL_UP` | `grantExperience()`, `progressionGrantXP()` |
| `BOSS_PHASE` | `advanceBossPhase()` |
| `PVP_FLAG` | `setPvPEnabled()` |

---

## 7. Script Content

### 7.1 An item

```lua
item = {
  name = "Ashen Blade",
  family = "weapon",
  slot = "main_hand",
  rarity = "epic",
  requiredLevel = 40,

  pve = {
    { label = "ATK",         raw = 1200, weight = 100 },
    { label = "Fire Damage", raw = 120,  weight = 130 },
    { label = "STR",         raw = 50,   weight = 100 },
    { label = "Crit Chance", raw = 5,    weight = 250 },
  },

  pvp = {
    { label = "ATK",       raw = 650, weight = 100 },
    { label = "PvP Power", raw = 10,  weight = 300 },
    { label = "Anti-Heal", raw = 20,  weight = 150 },
  },

  procs  = { { name = "Burn on hit", magnitude = 300, uptime = 25, impact = 120 } },
  pvpProcsEnabled = false,
}

itemIndex = registerItem(item)
```

The same weapon carries **two stat tables**. The PvE table is the power
fantasy; the PvP table is normalised, dampened and proc-free. This is the
design rule from `FRAMEWORK.md` §17, expressed in content.

### 7.2 An effect

```lua
effect = {
  name = "Burn",
  category = "dot",
  damageType = "fire",
  stackMode = "refresh",
  maxStacks = 1,
  duration = 12,
  magnitude = 300,
  tickRate = 1,
  drCategory = -1,      -- not subject to CC diminishing returns
  cleanse = "magic",
}
```

### 7.3 A boss

```lua
boss = {
  name = "The Ash Tyrant",
  tier = "raid_boss",
  level = 70,
  hpPool = 2500000,
  phases = {
    { name = "Cinders",    hpThreshold = 100, abilityCount = 4 },
    { name = "Pyroclasm",  hpThreshold = 60,  abilityCount = 6, addSpawnerIndex = 0 },
    { name = "Final Conflagration", hpThreshold = 25, abilityCount = 8 },
  },
  softEnrageRate = 12,   -- +12% damage per 15s
  hardEnrageTime = 480,  -- wipe at 8 minutes
}
```

---

## 8. API Summary

```c
int  luaStartup();
void luaShutdown();
int  luaIsEnabled();
const char *luaLastError();

int  luaRunString(const char *chunk);
int  luaRunFile(const char *path);
int  luaCallFunction(const char *name);

int  luaLoadItem(const char *scriptName);
int  luaLoadItemDirectory(const char *dir);
int  luaLoadEffect(const char *scriptName);
int  luaLoadBoss(const char *scriptName);

int  luaRegisterHook(int eventId, const char *functionName);
int  luaFireEvent(int eventId, int argA, int argB);

int  luaGetNumber(const char *name);
const char *luaGetString(const char *name);
void luaSetNumber(const char *name, int value);
void luaRegisterEngineTables();
```

---

## 9. Open Questions

- **No sandboxing.** Once enabled, a script has whatever the VM grants it.
  A content pipeline should run untrusted scripts with a restricted
  environment.
- **`registerItem` / `registerEffect` / `registerBoss` are not implemented.**
  The scripts call them; the C++ side currently only has the
  `luaLoadItem(name)` path.
- **No hot reload.** `luaRunFile` re-runs a file, but nothing watches for
  changes or unwinds previously registered content.
- **String returns.** `luaGetString()` always returns `""` — reading a string
  back out of the VM is not wired up.
