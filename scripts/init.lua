-- scripts/init.lua
-- Entry point.  Run once at startup, after luaStartup() succeeds.

-- Engine tables are available as: stats, effects, archetypes, affixes,
-- bosses, items, loot, encounters, factions.

-- Load content.
-- luaLoadItemDirectory("scripts/items")
-- luaRunFile("scripts/effects/burn.lua")
-- luaRunFile("scripts/bosses/ash_tyrant.lua")

-- Load behaviour hooks.
-- luaRunFile("scripts/hooks.lua")

luaSetNumber("SBW_SCRIPT_VERSION", 1)
