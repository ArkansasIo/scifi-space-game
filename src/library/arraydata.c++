// src/library/arraydata.c++ -- defines every framework array and sub-array
// declared "extern" in include/core/globals.h.
//
// This is the counterpart to src/core/universe.c++: universe.c++ owns the
// original [17] tables, this file owns the RPG / MMO framework tables.

#include "spacebattlerpg.h"

/* ---------------- label tables ---------------- */

const char *damageTypeNames[MAX_DAMAGE_TYPES] = {
    "Physical", "Fire", "Ice", "Lightning", "Earth",
    "Wind", "Water", "Light", "Dark", "Arcane",
    "Poison", "Bleed", "Void", "Chaos", "True"};

const char *resourceNames[MAX_RESOURCES] = {
    "HP", "MP", "Stamina", "Rage", "Energy",
    "Focus", "Faith", "Aether", "Heat", "Sanity"};

const char *tierNames[MAX_ENEMY_TIERS] = {
    "Trash", "Veteran", "Elite", "Champion",
    "Dungeon Boss", "World Boss", "Raid Boss", "Mythic"};

const char *roleNames[MAX_ENEMY_ROLES] = {
    "Bruiser", "Tank", "Assassin", "Sniper", "Caster",
    "Controller", "Summoner", "Support", "Trickster"};

const char *slotNames[MAX_GEAR_SLOTS] = {
    "Head", "Chest", "Legs", "Gloves", "Boots",
    "Main Hand", "Off Hand", "Ring 1", "Ring 2", "Amulet",
    "Cloak", "Belt", "Trinket 1", "Trinket 2"};

const char *rarityNames[MAX_RARITIES] = {
    "Common", "Uncommon", "Rare", "Epic", "Legendary", "Mythic"};

/* ---------------- sub-arrays ---------------- */

// Rank multipliers (x100) from the taxonomy table in the design doc.
int tierMultiplier[MAX_ENEMY_TIERS] = {
    100,  /* Trash        x1.00 */
    120,  /* Veteran      x1.20 */
    200,  /* Elite        x1.50-2.50 -> midpoint 2.0 */
    325,  /* Champion     x2.50-4.00 -> midpoint 3.25 */
    900,  /* Dungeon Boss x5-13 */
    1500, /* World Boss   x10-20 */
    3500, /* Raid Boss    x20-50 (party scaled) */
    6000  /* Mythic       x200+ rule-driven */
};

// Baseline strength vs a player, as a percentage.
int tierBaselinePercent[MAX_ENEMY_TIERS] = {
    70,   /* Trash        60-80 */
    135,  /* Veteran      120-150 */
    250,  /* Elite        200-300 */
    325,  /* Champion     250-400 */
    800,  /* Dungeon Boss 500-1500 */
    1500, /* World Boss   1000-2500 */
    3000, /* Raid Boss    2000-5000 */
    5000  /* Mythic       5000+ */
};

// Budget weight per equipped slot: bigger slots carry more item power.
int slotWeight[MAX_GEAR_SLOTS] = {
    100, /* Head */
    120, /* Chest */
    110, /* Legs */
    90,  /* Gloves */
    90,  /* Boots */
    160, /* Main Hand */
    80,  /* Off Hand */
    60,  /* Ring 1 */
    60,  /* Ring 2 */
    70,  /* Amulet */
    70,  /* Cloak */
    70,  /* Belt */
    85,  /* Trinket 1 */
    85   /* Trinket 2 */
};

// Progression value multiplier per rarity (x100).
int rarityMultiplier[MAX_RARITIES] = {
    100, /* Common */
    115, /* Uncommon */
    135, /* Rare */
    160, /* Epic */
    200, /* Legendary */
    250  /* Mythic */
};

/* ---------------- framework table definitions ---------------- */

// Stat caps (PvE / PvP hard caps, soft cap, DR constant).
statcap statCapArray[MAX_CAPS];

// Effects, enemies, bosses, gear, loot, encounters.
effectdefinition effectArray[MAX_EFFECTS];
enemyarchetype archetypeArray[MAX_ARCHETYPES];
affix affixArray[MAX_AFFIXES];
bossdata bossArray[MAX_BOSSES];
itemdata itemArray[MAX_ITEMS];
gearsets setArray[MAX_SETS];
loottable lootArray[MAX_LOOT_TABLES];
encountertemplate encounterArray[MAX_ENCOUNTERS];

// 4X / galaxy layer.
faction factionArray[MAX_FACTIONS];
starsystem systemArray[MAX_SYSTEMS];
fleet playerFleet;
stockpile empireStorage;
empireattributes empireAttributes;

/* ---------------- populated counts ---------------- */

int statCapCount = 0;
int effectCount = 0;
int archetypeCount = 0;
int affixCount = 0;
int bossCount = 0;
int itemCount = 0;
int setCount = 0;
int lootCount = 0;
int encounterCount = 0;
int factionCount = 0;
int systemCount = 0;

/* ---------------- initializers ---------------- */

void initializeLabelArrays()
{
    // The label arrays are statically initialised above.  This function
    // exists so callers have one explicit place to hook in if labels ever
    // need to become runtime-loaded.
}

void initializeSubArrays()
{
    // tierMultiplier, tierBaselinePercent, slotWeight and rarityMultiplier
    // are statically initialised above.  Kept as a function for symmetry
    // with initializeLabelArrays() and so tests can reset them.
}

void initializeFrameworkArrays()
{
    initializeLabelArrays();
    initializeSubArrays();

    initializeStatCaps();
    initializeStatWeights();
    initializeEffects();
    initializeArchetypes();
    initializeAffixes();
    initializeBosses();
    initializeGear();
    initializeSets();
    initializeLootTables();
    initializeEncounters();
    initializeFactions();
}

/* ---------------- lookups ---------------- */

const char *damageTypeName(damagetype type)
{
    if (type < 0 || type >= MAX_DAMAGE_TYPES)
        return "Unknown";
    return damageTypeNames[type];
}

const char *resourceName(resourcetype res)
{
    if (res < 0 || res >= MAX_RESOURCES)
        return "Unknown";
    return resourceNames[res];
}

const char *tierName(enemytier tier)
{
    if (tier < 0 || tier >= MAX_ENEMY_TIERS)
        return "Unknown";
    return tierNames[tier];
}

const char *roleName(enemyrole role)
{
    if (role < 0 || role >= MAX_ENEMY_ROLES)
        return "Unknown";
    return roleNames[role];
}

const char *slotName(gearslot slot)
{
    if (slot < 0 || slot >= MAX_GEAR_SLOTS)
        return "Unknown";
    return slotNames[slot];
}

const char *rarityName(gearrarity rarity)
{
    if (rarity < 0 || rarity >= MAX_RARITIES)
        return "Unknown";
    return rarityNames[rarity];
}

int tierMultiplierFor(enemytier tier)
{
    if (tier < 0 || tier >= MAX_ENEMY_TIERS)
        return 100;
    return tierMultiplier[tier];
}

int tierBaselineFor(enemytier tier)
{
    if (tier < 0 || tier >= MAX_ENEMY_TIERS)
        return 100;
    return tierBaselinePercent[tier];
}

int slotWeightFor(gearslot slot)
{
    if (slot < 0 || slot >= MAX_GEAR_SLOTS)
        return 100;
    return slotWeight[slot];
}

int rarityMultiplierFor(gearrarity rarity)
{
    if (rarity < 0 || rarity >= MAX_RARITIES)
        return 100;
    return rarityMultiplier[rarity];
}
