# File: build.ps1
# Build Space Battle Wars RPG.
# Usage:  .\build.ps1

$ErrorActionPreference = "Stop"

$sources = @(
    "src/main.c++",

    "src/core/universe.c++",
    "src/core/input.c++",

    "src/data/technology.c++",
    "src/data/upgrades.c++",
    "src/data/artifact.c++",
    "src/data/glyph.c++",
    "src/data/enchantment.c++",
    "src/data/enemyships.c++",

    "src/data/stats.c++",
    "src/data/effects.c++",
    "src/data/archetypes.c++",
    "src/data/affixes.c++",
    "src/data/bosses.c++",
    "src/data/gear.c++",
    "src/data/loot.c++",
    "src/data/encounters.c++",
    "src/data/factions.c++",

    "src/library/arraydata.c++",
    "src/library/stats.c++",
    "src/library/effects.c++",
    "src/library/itemvalue.c++",
    "src/library/scaling.c++",
    "src/library/lua.c++",

    "src/engine/subsystem.c++",
    "src/engine/combat_engine.c++",
    "src/engine/battle_engine.c++",
    "src/engine/effect_engine.c++",
    "src/engine/ai_engine.c++",
    "src/engine/loot_engine.c++",
    "src/engine/progression_engine.c++",

    "src/audio/audio.c++",
    "src/audio/manifest.c++",

    "src/db/schema.c++",
    "src/db/db.c++",

    "src/ui/display.c++",
    "src/ui/titlescreen.c++",
    "src/ui/hud.c++",

    "src/game/player.c++",
    "src/game/battle.c++",
    "src/game/items.c++",
    "src/game/story.c++",
    "src/game/campaign.c++",
    "src/game/difficulty.c++",
    "src/game/modifier.c++",
    "src/game/score.c++",
    "src/game/save.c++",

    "src/game/combat.c++",
    "src/game/threat.c++",
    "src/game/encounter.c++",
    "src/game/loot.c++",
    "src/game/ai.c++",
    "src/game/progression.c++",
    "src/game/galaxy.c++",
    "src/game/gamelogic.c++"
)

clang++ -std=c++17 -Wall -Wextra -Iinclude -o spacebattlerpg.exe @sources

if ($LASTEXITCODE -eq 0) {
    Write-Output "Build OK -> spacebattlerpg.exe"
} else {
    Write-Output "Build FAILED"
    exit $LASTEXITCODE
}
