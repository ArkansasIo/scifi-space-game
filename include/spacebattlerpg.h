// include/spacebattlerpg.h
// Master header.  Include this from every source file.
//
//   Space Battle Wars Simulation Program 2.0.0
//   krypton v1.0 engine

#ifndef SPACEBATTLERPG_H
#define SPACEBATTLERPG_H

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <string>

using namespace std;

#include "core/types.h"
#include "core/globals.h"
#include "core/input.h"

/* Data tables (ship technology, artifacts, glyphs, enchantments, enemies). */
#include "data/technology.h"
#include "data/upgrades.h"
#include "data/artifact.h"
#include "data/glyph.h"
#include "data/enchantment.h"
#include "data/enemyships.h"

/* RPG / MMO framework data tables. */
#include "data/stats.h"
#include "data/effects.h"
#include "data/archetypes.h"
#include "data/affixes.h"
#include "data/bosses.h"
#include "data/gear.h"
#include "data/loot.h"
#include "data/encounters.h"
#include "data/factions.h"

/* The framework library: arrays, stat math, effects, item value, scaling. */
#include "library/library.h"

/* Audio: cue dispatcher and sound manifest. */
#include "audio/audio.h"
#include "audio/manifest.h"

/* Persistence: MySQL schema and Excel/CSV export. */
#include "db/db.h"
#include "db/schema.h"

/* Original game modules. */
#include "world/universe.h"

#include "game/player.h"
#include "game/battle.h"
#include "game/items.h"
#include "game/story.h"
#include "game/campaign.h"
#include "game/difficulty.h"
#include "game/modifier.h"
#include "game/score.h"
#include "game/save.h"

/* Framework game logic (defines aibrain, lootresult, encounterinstance...). */
#include "game/combat.h"
#include "game/threat.h"
#include "game/ai.h"
#include "game/loot.h"
#include "game/encounter.h"
#include "game/progression.h"
#include "game/galaxy.h"

/* The engine kernel and its subsystems.  Included after game/ so the engine
   headers can see the shared runtime types they operate on. */
#include "engine/subsystem.h"
#include "engine/combat_engine.h"
#include "engine/battle_engine.h"
#include "engine/effect_engine.h"
#include "engine/ai_engine.h"
#include "engine/loot_engine.h"
#include "engine/progression_engine.h"

/* Presentation. */
#include "ui/display.h"
#include "ui/titlescreen.h"
#include "ui/hud.h"

#include "game/gamelogic.h"

#endif /* SPACEBATTLERPG_H */
