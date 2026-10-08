// include/world/biome.h -- biomes, sub-biomes, planet detail and the
// class / subclass taxonomy for characters and worlds.
//
// Every world gets a biome band, and every biome has sub-biomes.  A biome
// drives: what resources a planet yields, what it costs to colonise, what
// hazards it presents, and what units it can produce.
//
// See docs/BIOMES.md.

#ifndef SBW_WORLD_BIOME_H
#define SBW_WORLD_BIOME_H

// buildWorldDetail() reads a full `world` (temperature, water coverage,
// gravity, resources), so this header needs the real definition rather than a
// forward declaration.  core/types.h provides statblock and resourcetype.
#include "core/types.h"
#include "world/universe_gen.h"

/* ---------------- biome taxonomy ---------------- */

enum biome
{
    BIOME_NONE = 0,

    /* rocky / barren */
    BIOME_BARREN_ROCK,
    BIOME_CRATERED,
    BIOME_DUST_FLAT,

    /* hot */
    BIOME_VOLCANIC,
    BIOME_LAVA_SEA,
    BIOME_MAGMA_CAVERN,

    /* cold */
    BIOME_ICE_SHEET,
    BIOME_TUNDRA,
    BIOME_FROZEN_OCEAN,

    /* temperate */
    BIOME_GRASSLAND,
    BIOME_FOREST,
    BIOME_JUNGLE,
    BIOME_SAVANNA,
    BIOME_WETLAND,

    /* dry */
    BIOME_DESERT,
    BIOME_DUNES,
    BIOME_BADLANDS,

    /* aquatic */
    BIOME_OCEAN,
    BIOME_REEF,
    BIOME_ARCHIPELAGO,

    /* exotic */
    BIOME_CRYSTALLINE,
    BIOME_FUNGAL,
    BIOME_GAS_ENVELOPE,
    BIOME_CRYSTAL_CLOUD,
    BIOME_HIVE_WORLD,

    /* artificial */
    BIOME_ECUMENOPOLIS,
    BIOME_MACHINE_WORLD,
    BIOME_RUINED_WORLD,

    BIOME_COUNT
};

/* ---------------- sub-biome / feature ---------------- */

enum biomefeature
{
    FEATURE_NONE = 0,
    FEATURE_RIVER,
    FEATURE_MOUNTAIN_RANGE,
    FEATURE_CANYON,
    FEATURE_GEOTHERMAL,
    FEATURE_CRYSTAL_FIELD,
    FEATURE_RUINS,
    FEATURE_CRATER,
    FEATURE_ICE_CAVERN,
    FEATURE_UNDERWATER_VENT,
    FEATURE_FLOATING_ISLES,
    FEATURE_SUBTERRANEAN_SEA,
    FEATURE_COUNT
};;

/* ---------------- biome record ---------------- */

struct biominfo
{
    char name[32];
    biome parent;             // BIOME_NONE for a top-level biome
    int minTemperature;       // kelvin band it appears in
    int maxTemperature;
    int minWater;             // water coverage band
    int maxWater;
    int habitabilityBonus;
    int foodYield;            // 0..100
    int oreYield;
    int energyYield;
    int researchYield;
    int coloniseCost;
    int hazard;               // 0..100
    int unitType;             // what it can raise, index into UNITTYPE_*
};

/* ---------------- planet detail ---------------- */

// Per-world detail beyond the orbital record: its biome make-up and yields.
struct worlddetail
{
    biome primaryBiome;
    biome secondaryBiome;
    int featureCount;
    biomefeature features[4];
    int foodYield;
    int oreYield;
    int energyYield;
    int researchYield;
    int colonised;
    int population;
    int terrainDifficulty;    // 0..100, slows construction
    int hazardRating;         // 0..100, drains units
};

/* ---------------- character class taxonomy ---------------- */

enum classrole
{
    CLASSROLE_TANK = 0,
    CLASSROLE_HEALER,
    CLASSROLE_DPS_MELEE,
    CLASSROLE_DPS_RANGED,
    CLASSROLE_DPS_MAGIC,
    CLASSROLE_SUPPORT,
    CLASSROLE_CONTROLLER,
    CLASSROLE_COUNT
};;

enum characterclass
{
    CLS_NONE = 0,

    /* Warrior line */
    CLS_WARRIOR,
    CLS_GUARDIAN,
    CLS_BERSERKER,
    CLS_CHAMPION,

    /* Scout line */
    CLS_SCOUT,
    CLS_RANGER,
    CLS_ASSASSIN,
    CLS_SNIPER,

    /* Engineer line */
    CLS_ENGINEER,
    CLS_TECHNICIAN,
    CLS_MECHANIC,
    CLS_ARCHITECT,

    /* Psionic line */
    CLS_PSIONIC,
    CLS_MYSTIC,
    CLS_TELEPATH,
    CLS_WARLOCK,

    /* Officer line */
    CLS_OFFICER,
    CLS_COMMANDER,
    CLS_ADMIRAL,
    CLS_DIPLOMAT,

    CLS_COUNT
};;

struct classinfo
{
    char name[32];
    characterclass parent;     // CLS_NONE for a base class
    classrole role;
    int baseStr, baseDex, baseCon, baseInt, baseWis;
    int baseVit, baseSpi, baseLck, baseWil, baseCha;
    int hpPerLevel;
    int powerPerLevel;
    int attackPerLevel;
    int defencePerLevel;
    int resourceType;          // resourcetype enum
    char signatureSkill[32];
};

/* ---------------- tables ---------------- */

extern biominfo biomeTable[BIOME_COUNT];
extern classinfo classTable[CLS_COUNT];

/* ---------------- biome API ---------------- */

void initializeBiomes();

const biominfo *biomeInfo(biome b);
const char *biomeName(biome b);
const char *biomeFeatureName(biomefeature f);
int biomeCount();

// The best biome for a world's temperature and water coverage.
biome selectBiome(int temperature, int waterCoverage, int habitability);

// A secondary biome, chosen as a neighbouring band.
biome selectSecondaryBiome(biome primary, int seed);

// Roll the features of a world.
int rollBiomeFeatures(biomefeature out[], int outMax, int seed);

// Build a world's detail record from its class, temperature and water.
void buildWorldDetail(worlddetail &d, const struct world &w, int seed);

// Aggregate yields for a planet, honouring its detail if present.
int worldFoodYield(const worlddetail &d);
int worldOreYield(const worlddetail &d);
int worldEnergyYield(const worlddetail &d);
int worldResearchYield(const worlddetail &d);

// Cost to settle a world, before terrain modifiers.
int worldColoniseCost(const worlddetail &d);

// Can this world support a colony at all?
int worldIsSettleable(const worlddetail &d, int habitability);

/* ---------------- class API ---------------- */

void initializeClasses();

const classinfo *classInfo(characterclass c);
const char *className(characterclass c);
const char *classRoleName(classrole r);
int classCount();

// Find a class by name (case-insensitive).  Returns CLS_NONE if absent.
characterclass findClass(const char *name);

// Every class whose parent is `parent`.  Returns how many were written.
int classChildren(characterclass parent, characterclass out[], int outMax);

// Can `from` advance to `to`?  (Direct parent/child relationship.)
int classCanAdvance(characterclass from, characterclass to);

// Apply a class template to a stat block.
void applyClass(struct statblock &stats, characterclass c);

// The role a class fills.
classrole classRoleOf(characterclass c);

/* ---------------- presentation ---------------- */

void showBiomeList();
void showBiomeDetail(biome b);
void showClassTree();
void showClassDetail(characterclass c);
void showWorldDetail(const worlddetail &d);

#endif /* SBW_WORLD_BIOME_H */
