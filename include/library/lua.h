// include/library/lua.h -- the embedded Lua scripting layer.
//
// The engine exposes its data tables and helpers to Lua so content (items,
// effects, bosses, encounters) can be authored as scripts and hot-reloaded
// without a recompile.
//
// This build is self-contained: when SBW_ENABLE_LUA is not defined the API
// below is still compiled and callable, but the VM is a stub that reports
// "scripting disabled" instead of loading scripts.  Define SBW_ENABLE_LUA
// and link a Lua library to go live.

#ifndef SBW_LIBRARY_LUA_H
#define SBW_LIBRARY_LUA_H

/* ---------------- lifecycle ---------------- */

// Create the Lua state and register the engine API.  Returns 1 on success.
int luaStartup();

// Tear the Lua state down.
void luaShutdown();

// True when a real Lua VM is compiled in and running.
int luaIsEnabled();

// Last error message produced by any lua* call (never null).
const char *luaLastError();

/* ---------------- script execution ---------------- */

// Run a chunk of Lua source.  Returns 1 on success.
int luaRunString(const char *chunk);

// Run a script file from disk.  Returns 1 on success.
int luaRunFile(const char *path);

// Call a global Lua function with no arguments; returns 1 on success.
int luaCallFunction(const char *name);

/* ---------------- scripted content ---------------- */

// Load a scripted item definition by name into itemArray[itemCount].
// Returns the new index, or -1.
int luaLoadItem(const char *scriptName);

// Load every item script under the given directory.  Returns how many loaded.
int luaLoadItemDirectory(const char *dir);

// Load a scripted effect definition.  Returns the index, or -1.
int luaLoadEffect(const char *scriptName);

// Load a scripted boss definition.  Returns the index, or -1.
int luaLoadBoss(const char *scriptName);

/* ---------------- hooks ---------------- */

// Register a Lua function to run when an event fires.
// Event ids are the SBW_EVENT_* constants below.
int luaRegisterHook(int eventId, const char *functionName);

// Fire an event, calling every registered hook in order.
// Returns how many hooks ran.
int luaFireEvent(int eventId, int argA, int argB);

/* ---------------- event ids ---------------- */

#define SBW_EVENT_COMBAT_START   1
#define SBW_EVENT_DAMAGE_DEALT   2
#define SBW_EVENT_ENEMY_KILLED   3
#define SBW_EVENT_LOOT_ROLLED    4
#define SBW_EVENT_LEVEL_UP       5
#define SBW_EVENT_BOSS_PHASE     6
#define SBW_EVENT_PVP_FLAG       7
#define SBW_EVENT_COUNT          8

/* ---------------- value bridge ---------------- */

// Read a global number from Lua (0 if absent).
int luaGetNumber(const char *name);

// Read a global string from Lua ("" if absent).
const char *luaGetString(const char *name);

// Set a global number in Lua.
void luaSetNumber(const char *name, int value);

// Register the engine's tables so scripts can read them.
// Exposes: stats, effects, archetypes, affixes, bosses, items, loot,
// encounters, factions.
void luaRegisterEngineTables();

#endif /* SBW_LIBRARY_LUA_H */
