# Lua Scripts

Scripts in this folder are loaded by the embedded scripting layer
(`include/library/lua.h`, `src/library/lua.c++`).

## Status

Scripting is **stubbed by default**: the API always compiles and hook dispatch
works, but no Lua VM is linked in. To go live:

1. Build the engine with `-DSBW_ENABLE_LUA` (see `build.ps1`).
2. Provide a Lua library and link it (e.g. `-llua54`).
3. Replace the stub bodies in `src/library/lua.c++` with real `lua_*` calls.

Until then, `luaIsEnabled()` returns `0` and `luaRunFile()` reports
`scripting disabled: rebuild with -DSBW_ENABLE_LUA`.

## Layout

```
scripts/
├── README.md            this file
├── init.lua             entry point, runs on startup
├── hooks.lua            example event hook registrations
├── items/
│   ├── ashen_blade.lua  worked example: one item, PvE + PvP tables
│   └── void_lance.lua   worked example: second item
├── effects/
│   └── burn.lua         worked example: a DoT debuff
└── bosses/
    └── ash_tyrant.lua   worked example: 3-phase raid boss
```

## API

| Function | Purpose |
| --- | --- |
| `luaStartup()` | create the VM, register the engine API |
| `luaShutdown()` | tear the VM down |
| `luaIsEnabled()` | 1 when a real VM is running |
| `luaRunString(chunk)` | run a chunk of source |
| `luaRunFile(path)` | run a script file |
| `luaCallFunction(name)` | call a global function |
| `luaLoadItem(name)` | load a scripted item into `itemArray[]` |
| `luaLoadItemDirectory(dir)` | load every item script in a folder |
| `luaLoadEffect(name)` | load a scripted effect |
| `luaLoadBoss(name)` | load a scripted boss |
| `luaRegisterHook(event, fn)` | run `fn` when an event fires |
| `luaFireEvent(event, a, b)` | fire an event to all hooks |
| `luaRegisterEngineTables()` | expose the data tables to scripts |

## Exposed engine tables

`stats`, `effects`, `archetypes`, `affixes`, `bosses`, `items`, `loot`,
`encounters`, `factions`.

Each is a read-only table mirroring the C++ array of the same name, so a
script can read `items[3].name` or `bosses[i].phases[1].hpThreshold`.

## Event ids

| Constant | Value | Fired when |
| --- | --- | --- |
| `SBW_EVENT_COMBAT_START` | 1 | a battle begins |
| `SBW_EVENT_DAMAGE_DEALT` | 2 | any damage is applied |
| `SBW_EVENT_ENEMY_KILLED` | 3 | an enemy is destroyed |
| `SBW_EVENT_LOOT_ROLLED` | 4 | a loot table is rolled |
| `SBW_EVENT_LEVEL_UP` | 5 | a character gains a level |
| `SBW_EVENT_BOSS_PHASE` | 6 | a boss changes phase |
| `SBW_EVENT_PVP_FLAG` | 7 | a player flags for PvP |

## Example

```lua
-- hooks.lua
onEvent(SBW_EVENT_ENEMY_KILLED, function(enemyId, overkill)
  score.add(100 + overkill * 2)
end)
```
