// include/core/globals.h
// Declarations of the global game objects.  They are DEFINED exactly once,
// in src/core/universe.c++.

#ifndef SBW_CORE_GLOBALS_H
#define SBW_CORE_GLOBALS_H

#include "types.h"

extern space       spaceArray[17];
extern Technology  TechnologyArray[17];
extern upgrads     upgradeArray[17];
extern ship_item   artifactArray[17];
extern ship_item   glyphsArray[17];
extern ship_item   enchantmentArray[17];
extern player      charactership;
extern enemyships  enemyshipsArray[17];

extern difficultmode difficultMode;
extern targetscore targetScore;

/* ---------------- RPG / MMO framework arrays ---------------- */
/*
 * Each table has a fixed compile-time size (MAX_* below) so the engine stays
 * allocation-free, exactly like the original [17] data tables.  The arrays are
 * DEFINED once in src/core/universe.c++ and filled by the initialize* helpers
 * in src/data/ and src/library/.
 */

#define MAX_DAMAGE_TYPES   DMG_TYPE_COUNT   /* 15 */
#define MAX_RESOURCES      RES_COUNT        /* 10 */
#define MAX_EFFECTS        32
#define MAX_ENEMY_TIERS    TIER_COUNT       /* 8  */
#define MAX_ENEMY_ROLES    ROLE_COUNT       /* 9  */
#define MAX_ARCHETYPES     24
#define MAX_AFFIXES        16
#define MAX_BOSSES         8
#define MAX_GEAR_SLOTS     SLOT_COUNT       /* 14 */
#define MAX_RARITIES       RARITY_COUNT     /* 6  */
#define MAX_ITEMS          64
#define MAX_SETS           8
#define MAX_LOOT_TABLES    12
#define MAX_ENCOUNTERS     16
#define MAX_CAPS           24
#define MAX_FACTIONS       8
#define MAX_SYSTEMS        17
#define MAX_SHIPS          24

/* Identity / label tables. */
extern const char *damageTypeNames[MAX_DAMAGE_TYPES];
extern const char *resourceNames[MAX_RESOURCES];
extern const char *tierNames[MAX_ENEMY_TIERS];
extern const char *roleNames[MAX_ENEMY_ROLES];
extern const char *slotNames[MAX_GEAR_SLOTS];
extern const char *rarityNames[MAX_RARITIES];

/* Sub-arrays: one row per tier / slot / rarity, used by the scaling helpers. */
extern int tierMultiplier[MAX_ENEMY_TIERS];
extern int tierBaselinePercent[MAX_ENEMY_TIERS];
extern int slotWeight[MAX_GEAR_SLOTS];
extern int rarityMultiplier[MAX_RARITIES];

/* Framework tables. */
extern statcap            statCapArray[MAX_CAPS];
extern effectdefinition   effectArray[MAX_EFFECTS];
extern enemyarchetype     archetypeArray[MAX_ARCHETYPES];
extern affix              affixArray[MAX_AFFIXES];
extern bossdata           bossArray[MAX_BOSSES];
extern itemdata           itemArray[MAX_ITEMS];
extern gearsets           setArray[MAX_SETS];
extern loottable          lootArray[MAX_LOOT_TABLES];
extern encountertemplate  encounterArray[MAX_ENCOUNTERS];
extern faction            factionArray[MAX_FACTIONS];
extern starsystem         systemArray[MAX_SYSTEMS];
extern fleet              playerFleet;
extern stockpile          empireStorage;
extern empireattributes   empireAttributes;

/* Counts actually populated by the initializers (<= MAX_*). */
extern int statCapCount;
extern int effectCount;
extern int archetypeCount;
extern int affixCount;
extern int bossCount;
extern int itemCount;
extern int setCount;
extern int lootCount;
extern int encounterCount;
extern int factionCount;
extern int systemCount;

#endif /* SBW_CORE_GLOBALS_H */
