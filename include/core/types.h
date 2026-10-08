// include/core/types.h
// Fundamental data types used across the whole game.

#ifndef SBW_CORE_TYPES_H
#define SBW_CORE_TYPES_H

/* Standard Line Length */
#define MAXLEN 255

/* Standard Terminal Sizes */
#define MAXROW 24
#define MAXCOL 80

/* Standard Page Size */
#define MAXLINES 66

/* An item of technology the player can be given (weapon / shield / armour). */
struct Technology
{
    char name[30];      // Name
    char type[18];      // The Type
    char itemclass[10]; // Class
    int level;
    int lightlevel;
    int attackpower;
    int defencepower;
    int bonus; // The Bonus
};

/* An upgrade of a piece of technology. */
struct upgrads
{
    char name[30];
    char type[18];
    char itemclass[10];
    int level;
    int lightlevel;
    int bonus;
    int attackpower;
    int defencepower;
};

/* Ship artifacts / glyphs / enchantments all share the same shape. */
struct ship_item
{
    char name[30];
    char description[500]; // Description
    char type[18];
    int level;
    int lightlevel;
    int bonus;
    int attackpower;
    int defencepower;
    char itemclass[10];
};

/* An enemy ship. */
struct enemyships
{
    char name[30];
    int sp; // ship points
    int health;
    int power; // power
    int sif;   // structural integrity field
    int attackpower;
    int defencepower;
};

/* A named class of enemy ship. */
struct enemyshipclass
{
    char shipclass[500];
    int attackpower;
    int defencepower;
};

/* The player's ship. */
struct player
{
    char name[30];
    int power; // power
    int sif;   // structural integrity field
    int sp;    // ship points
    int exp;   // experience
    int health;
    int level;
    int lightlevel;
    int attackpower;
    int defencepower;
    int weapon; // weapon bonus
    int armour; // armour bonus
    int shield; // shield bonus
};

/* A location in space.  -1 means "you cannot move that way". */
struct space
{
    char description[500];
    char name[30];
    int forward;
    int backward;
    int starboard; // right
    int port;      // left
};

/* A story beat (act / chapter / episode / stage / mission / quest). */
struct storynode
{
    char description[500];
    char name[30];
    int level;
    int bonus;
};

/* Difficulty mode. */
struct difficultmode
{
    char name[30];
    int level;
};

/* Target score for the current run. */
struct targetscore
{
    int score;
    int actions_taken;
    int damage_taken;
    int kills;
    int damage_done;
};

/* ======================================================================= */
/*  RPG / MMO FRAMEWORK                                                     */
/*                                                                          */
/*  The types below back the design described in docs/FRAMEWORK.md.  They   */
/*  are additive: nothing above this line changes, so the existing battle,  */
/*  item and story code keeps working untouched.                            */
/* ======================================================================= */

/* ---------- enums: damage, resources, effect taxonomy ---------- */

/* Every damage type in the resistance model. */
enum damagetype
{
    DMG_PHYSICAL = 0,
    DMG_FIRE,
    DMG_ICE,
    DMG_LIGHTNING,
    DMG_EARTH,
    DMG_WIND,
    DMG_WATER,
    DMG_LIGHT,
    DMG_DARK,
    DMG_ARCANE,
    DMG_POISON,
    DMG_BLEED,
    DMG_VOID,
    DMG_CHAOS,
    DMG_TRUE,
    DMG_TYPE_COUNT
};

/* Multi-bar resource pools. */
enum resourcetype
{
    RES_HP = 0,
    RES_MP,
    RES_STAMINA,
    RES_RAGE,
    RES_ENERGY,
    RES_FOCUS,
    RES_FAITH,
    RES_AETHER,
    RES_HEAT,
    RES_SANITY,
    RES_COUNT
};

/* Buff / debuff category. */
enum effectcategory
{
    EFF_OFFENSIVE = 0,
    EFF_DEFENSIVE,
    EFF_UTILITY,
    EFF_CROWD_CONTROL,
    EFF_DOT,
    EFF_STAT_BREAK
};

/* How an effect behaves when reapplied. */
enum stackmode
{
    STACK_NONE = 0,
    STACK_REFRESH,
    STACK_ADDITIVE,
    STACK_REPLACE_STRONGER,
    STACK_INDEPENDENT
};

/* What a cleanse removes. */
enum cleansetype
{
    CLEANSE_NONE = 0,
    CLEANSE_PHYSICAL,
    CLEANSE_MAGIC,
    CLEANSE_CURSE,
    CLEANSE_POISON,
    CLEANSE_BLEED,
    CLEANSE_ALL
};

/* ---------- enums: enemies ---------- */

/* Difficulty tier of an enemy. */
enum enemytier
{
    TIER_TRASH = 0,
    TIER_VETERAN,
    TIER_ELITE,
    TIER_CHAMPION,
    TIER_DUNGEON_BOSS,
    TIER_WORLD_BOSS,
    TIER_RAID_BOSS,
    TIER_MYTHIC,
    TIER_COUNT
};

/* Combat role an enemy fills. */
enum enemyrole
{
    ROLE_BRUISER = 0,
    ROLE_TANK,
    ROLE_ASSASSIN,
    ROLE_SNIPER,
    ROLE_CASTER,
    ROLE_CONTROLLER,
    ROLE_SUMMONER,
    ROLE_SUPPORT,
    ROLE_TRICKSTER,
    ROLE_COUNT
};

/* Affix category for elites and mythic runs. */
enum affixcategory
{
    AFFIX_OFFENSIVE = 0,
    AFFIX_DEFENSIVE,
    AFFIX_UTILITY,
    AFFIX_HAZARD,
    AFFIX_RULE
};

/* ---------- enums: gear ---------- */

/* Where a piece of gear is worn. */
enum gearslot
{
    SLOT_HEAD = 0,
    SLOT_CHEST,
    SLOT_LEGS,
    SLOT_GLOVES,
    SLOT_BOOTS,
    SLOT_WEAPON_MAIN,
    SLOT_WEAPON_OFF,
    SLOT_RING_1,
    SLOT_RING_2,
    SLOT_AMULET,
    SLOT_CLOAK,
    SLOT_BELT,
    SLOT_TRINKET_1,
    SLOT_TRINKET_2,
    SLOT_COUNT
};

/* Progression tier of an item. */
enum gearrarity
{
    RARITY_COMMON = 0,
    RARITY_UNCOMMON,
    RARITY_RARE,
    RARITY_EPIC,
    RARITY_LEGENDARY,
    RARITY_MYTHIC,
    RARITY_COUNT
};

/* The three broad gear families. */
enum gearfamily
{
    FAMILY_WEAPON = 0,
    FAMILY_ARMOR,
    FAMILY_TRINKET,
    FAMILY_COUNT
};

/* Which rule set a numeric context is resolved under. */
enum combatcontext
{
    CTX_PVE = 0,
    CTX_PVP,
    CTX_COUNT
};

/* ---------- stat model ---------- */

/* Persistent growth stats.  One field per primary attribute. */
struct coreattributes
{
    int str;
    int dex;
    int con;
    int intel;
    int wis;
    int vit;
    int spi;
    int lck;
    int wil;
    int cha;
};

/* Secondary stats derived from primaries, gear and passives. */
struct subattributes
{
    int physicalpower;
    int spellpower;
    int healingpower;
    int accuracy;
    int evasion;
    int armorpen;
    int magicpen;
    int critchance;
    int critdamage;
    int attackspeed;
    int castspeed;
    int maxhp;
    int hpregen;
    int shieldpower;
    int armor;
    int magicresist;
    int damagereduction;
    int tenacity;
    int poise;
    int controlpower;
    int manaefficiency;
};

/* Final runtime values consumed by the damage / heal formulas. */
struct combatstats
{
    int baseattack;
    int skillpower;
    int critchance;
    int critdamage;
    int vulnerabilitydmg;
    int dotpower;
    int executedmg;
    int mitigation;
    int blockchance;
    int blockvalue;
    int parrychance;
    int dodgechance;
    int movespeed;
    int cdr;
    int gcd;
};

/* One resistance layer entry per damage type. */
struct resistancetable
{
    int flat[DMG_TYPE_COUNT];
    int percent[DMG_TYPE_COUNT];
    int absorption[DMG_TYPE_COUNT];
    int immune[DMG_TYPE_COUNT];
    int vulnerable[DMG_TYPE_COUNT];
};

/* Full stat block used by players, mobs and bosses alike. */
struct statblock
{
    coreattributes attributes;
    subattributes sub;
    combatstats combat;
    resistancetable resist;
    int resource[RES_COUNT];
    int resourcemax[RES_COUNT];
    int pvemultiplier;
    int pvpmultiplier;
    int encountermultiplier;
    char tags[64];
};

/* ---------- effects ---------- */

/* A buff or debuff definition. */
struct effectdefinition
{
    char name[30];
    effectcategory category;
    damagetype damageType;
    stackmode stackMode;
    int maxStacks;
    int duration;      /* ticks */
    int magnitude;     /* flat or percent, per category */
    int tickRate;      /* 0 = no tick */
    int drCategory;    /* -1 = not subject to DR */
    cleansetype cleanse;
    int immunityWindow;
};

/* A live instance of an effect on a target. */
struct activeeffect
{
    int definitionIndex;
    int stacks;
    int remaining;
    int magnitude;
    int sourceId;
};

/* ---------- enemies ---------- */

/* Data-driven enemy archetype used to spawn encounters. */
enum aiState
{
    AI_IDLE = 0,
    AI_PATROL,
    AI_ALERT,
    AI_ENGAGE,
    AI_COMBAT,
    AI_RETREAT,
    AI_RESET
};

struct enemyarchetype
{
    char name[30];
    enemytier tier;
    enemyrole role;
    int preferredRange;
    int aggroRadius;
    int abilityCount;
    int lootProfile;
    int spawnWeight;
};

/* A modifier attached to an elite / champion / mythic enemy. */
struct affix
{
    char name[30];
    affixcategory category;
    int magnitude;
    int stackCost;
};

/* ---------- boss ---------- */

/* A single boss phase. */
struct bossphase
{
    char name[30];
    int hpThreshold;   /* enter at this % of max HP */
    int timeLimit;     /* 0 = none */
    int abilityCount;
    int addSpawnerIndex;
    int immuneMask;    /* bitfield of damagetype */
};

/* Boss metadata: phases, arena rules, enrage, loot. */
struct bossdata
{
    char name[30];
    enemytier tier;
    int level;
    int hpPool;
    int phaseCount;
    bossphase phases[4];
    int arenaHazardCount;
    int softEnrageRate;  /* % damage per 15s */
    int hardEnrageTime;  /* seconds, 0 = none */
    int lootTableIndex;
    int partyScaling;
    char tags[64];
};

/* ---------- gear ---------- */

/* One line of gear score, useful for tooltips and auditing. */
struct statline
{
    char label[24];
    int raw;
    int weight;
    int valuePU;
};

/* A complete item, valued separately for PvE and PvP. */
struct itemdata
{
    char name[30];
    gearfamily family;
    gearslot slot;
    gearrarity rarity;
    int requiredLevel;
    int bindRule;
    statline pveLines[8];
    int pveLineCount;
    statline pvpLines[8];
    int pvpLineCount;
    int procValuePU;
    int setBonusValuePU;
    int itemPowerPvE;
    int itemPowerPvP;
};

/* A set: the slots it covers and its tier bonuses. */
struct gearsets
{
    char name[30];
    int pieceCount;
    gearslot pieces[8];
    int bonus2;
    int bonus4;
    int bonus6;
};

/* ---------- loot ---------- */

struct lootentry
{
    char name[30];
    gearrarity rarity;
    int weight;
    int minLevel;
};

struct loottable
{
    char name[30];
    int entryCount;
    lootentry entries[12];
    int pityTimer;
    int uniquePerWeek;
    int difficultyMultiplier;
    int distributionRule;
};

/* ---------- encounters ---------- */

enum encounterkind
{
    ENC_PACK = 0,
    ENC_RARE_SPAWN,
    ENC_EVENT_WAVE,
    ENC_DUNGEON_ROOM,
    ENC_GAUNTLET,
    ENC_RAID_BOSS,
    ENC_COUNCIL,
    ENC_COUNT
};

struct encountertemplate
{
    char name[30];
    encounterkind kind;
    int trashCount;
    int casterCount;
    int minibossCount;
    int waveCount;
    int hasHazard;
    int hasPuzzle;
    int timed;
    int bossIndex;
};

/* ---------- sci-fi: empire / fleet / galaxy ---------- */

/* Strategic attributes of a 4X empire. */
struct empireattributes
{
    int industry;
    int science;
    int economy;
    int influence;
    int logistics;
    int intelligence;
    int stability;
};

/* Primary and advanced strategic resources. */
struct stockpile
{
    int credits;
    int minerals;
    int gas;
    int energy;
    int data;
    int darkmatter;
    int antimatter;
    int nanites;
    int quantumcores;
    int alienartifacts;
};

/* A planet inside a star system. */
struct planet
{
    char name[30];
    int size;
    int habitability;
    int resources;
    int defense;
    int stability;
    int buildingCount;
};

/* A star system: planets, stations, anomalies. */
struct starsystem
{
    char name[30];
    int sectorIndex;
    int planetCount;
    planet planets[6];
    int hasAsteroidField;
    int hasStation;
    int hasAnomaly;
    int ownerFaction;
};

/* A single ship in a fleet. */
struct shipinstance
{
    char name[30];
    int shipClass;
    int hull;
    int shields;
    int firepower;
    int crewSkill;
    int techLevel;
    int supplyUse;
};

/* A fleet: flagship, capitals, escorts. */
struct fleet
{
    char name[30];
    int shipCount;
    shipinstance ships[24];
    int commandCapacity;
    int morale;
    int sensorRange;
    int supplyUse;
};

/* A faction and its diplomatic posture. */
struct faction
{
    char name[30];
    empireattributes attributes;
    stockpile resources;
    int reputation;
    int atWar;
};

/* A 4X victory path. */
enum victorytype
{
    VICTORY_MILITARY = 0,
    VICTORY_ECONOMIC,
    VICTORY_TECHNOLOGICAL,
    VICTORY_DIPLOMATIC,
    VICTORY_ASCENSION,
    VICTORY_COUNT
};

/* ---------- shared tuning limits ---------- */

/* Soft / hard caps, one row per stat. */
struct statcap
{
    char name[24];
    int pveCap;
    int pvpCap;
    int softCap;
    int curveK;
};

#endif /* SBW_CORE_TYPES_H */
