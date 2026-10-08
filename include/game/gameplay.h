// include/game/gameplay.h -- player-facing gameplay systems.
//
// The mechanics that make the game a game rather than a set of screens:
//
//   Exploration   scanning systems, discovering anomalies, first-visit rewards
//   Trading       buy low / sell high across systems, cargo holds, market drift
//   Colonies      settle a world, build, grow, collect, defend
//   Skills        an ability list with cooldowns, costs and effects
//   Reputation    standing with each faction, and what it unlocks
//   Contracts     jobs offered by factions: kill, deliver, survey, escort
//   Crafting      turn salvaged materials into gear
//   Codex         a discovered-lore log
//
// Everything here operates on state already declared elsewhere (systems,
// worlds, factions, items) so nothing new has to be serialised to save a game.
//
// See docs/GAMEPLAY.md.

#ifndef SBW_GAME_GAMEPLAY_H
#define SBW_GAME_GAMEPLAY_H

// MAX_FACTIONS (the size of the standings array) comes from core/globals.h,
// which is where every framework table's capacity lives.
#include "core/globals.h"

/* ---------------- limits ---------------- */

#define MAX_CARGO 12
#define MAX_COLONIES 16
#define MAX_SKILLS 24
#define MAX_CONTRACTS 12
#define MAX_CODEX 64
#define MAX_MATERIALS 16

/* ---------------- cargo ---------------- */

// A stack of commodities in the hold.
struct cargostack
{
    char name[24];
    int quantity;
    int buyPrice; // what it cost, for profit tracking
};

struct cargohold
{
    int capacity; // total units
    int used;
    cargostack stacks[MAX_CARGO];
    int stackCount;
    int credits;
};

/* ---------------- market ---------------- */

// One commodity in a system's market.
struct marketentry
{
    char name[24];
    int price;     // current price per unit
    int basePrice; // the long-run average it drifts back toward
    int stock;
    int demand; // 0..100
};

struct market
{
    char systemName[30];
    int entryCount;
    marketentry entries[12];
    int lastRefreshed; // turn
};

/* ---------------- colonies ---------------- */

enum colonybuilding
{
    BLD_NONE = 0,
    BLD_MINE,
    BLD_FARM,
    BLD_FACTORY,
    BLD_LAB,
    BLD_SHIPYARD,
    BLD_DEFENCE,
    BLD_HABITAT,
    BLD_TRADE_HUB,
    BLD_COUNT
};

struct colony
{
    char name[30];
    int systemIndex;
    int worldIndex;
    int population;
    int development; // 0..100
    int morale;      // 0..100
    int food;        // stockpile
    int minerals;
    int energy;
    int buildingCount;
    colonybuilding buildings[8];
    int buildingLevel[8];
    int foundedTurn;
    int underSiege;
};

/* ---------------- skills ---------------- */

enum skillkind
{
    SKILL_ATTACK = 0,
    SKILL_HEAL,
    SKILL_BUFF,
    SKILL_DEBUFF,
    SKILL_UTILITY,
    SKILL_PASSIVE,
    SKILL_KIND_COUNT
};

struct skill
{
    char name[24];
    char description[64];
    skillkind kind;
    int power;     // damage or healing
    int cost;      // resource cost
    int cooldown;  // turns
    int remaining; // turns until ready
    int level;     // skill rank
    int maxLevel;
    int unlocked;
};

/* ---------------- reputation ---------------- */

struct repstanding
{
    int factionIndex;
    int value; // -10000 .. +10000
    int tier;  // 0 hostile .. 5 exalted
};

/* ---------------- contracts ---------------- */

enum contractkind
{
    CONTRACT_KILL = 0,
    CONTRACT_DELIVER,
    CONTRACT_SURVEY,
    CONTRACT_ESCORT,
    CONTRACT_BOUNTY,
    CONTRACT_KIND_COUNT
};

struct contract
{
    char title[40];
    char description[80];
    contractkind kind;
    int giverFaction;
    int targetSystem;
    int required; // how many / how far
    int progress;
    int rewardCredits;
    int rewardRep;
    int expiresTurn; // 0 = never
    int accepted;
    int complete;
};

/* ---------------- crafting ---------------- */

struct material
{
    char name[24];
    int quantity;
    int tier;
};

// A recipe: inputs and the item it produces.
struct recipe
{
    char name[32];
    int inputMaterial[MAX_MATERIALS];
    int inputCount[MAX_MATERIALS];
    int inputKinds;
    int outputItem; // index into itemArray
    int requiresTurn;
};

/* ---------------- codex ---------------- */

struct codexentry
{
    char title[48];
    char body[120];
    int category; // 0 lore, 1 bestiary, 2 tech, 3 history
    int discovered;
};

/* ---------------- player state ---------------- */

struct playerprofile
{
    cargohold cargo;
    colony colonies[MAX_COLONIES];
    int colonyCount;
    skill skills[MAX_SKILLS];
    int skillCount;
    repstanding standings[MAX_FACTIONS];
    int standingCount;
    contract contracts[MAX_CONTRACTS];
    int contractCount;
    material materials[MAX_MATERIALS];
    int materialCount;
    codexentry codex[MAX_CODEX];
    int codexCount;
    int scanRange;
    int scanners;
    int fuel;
    int maxFuel;
    int jumpCount;
    int turnStarted;
    int difficulty;
};

/* ---------------- lifecycle ---------------- */

void gameplayInit();
void gameplayReset();
playerprofile &gameplayProfile();
const playerprofile &gameplayProfileConst();

/* ---------------- exploration ---------------- */

// Scan a system: reveals anomalies, grants first-visit rewards.
// Returns how many new discoveries.
int scanSystem(int systemIndex);

// Have we visited this system?
int systemVisited(int systemIndex);

// Jump to a system if it is reachable and there is fuel.
// Returns 0 on success, or a negative reason code.
int jumpToSystem(int systemIndex);

// Refuel at a station, if one is present.
int refuelAtSystem(int systemIndex);

/* ---------------- cargo and trading ---------------- */

void cargoInit(cargohold &hold);
int cargoUsed(const cargohold &hold);
int cargoFree(const cargohold &hold);
int cargoAdd(cargohold &hold, const char *name, int quantity, int unitPrice);
int cargoRemove(cargohold &hold, const char *name, int quantity);
int cargoCountOf(const cargohold &hold, const char *name);
int cargoSell(cargohold &hold, const char *name, int quantity, int unitPrice);
int cargoSellAll(cargohold &hold, const market &m);
int cargoProfit(const cargohold &hold);

// Build the market for a system, seeded by the system index.
void marketGenerate(market &m, int systemIndex, int turn);
void marketRefresh(market &m, int turn);
int marketPriceFor(const market &m, const char *name);
int marketBuy(market &m, cargohold &hold, const char *name, int quantity);
int marketSell(market &m, cargohold &hold, const char *name, int quantity);
void marketShow(const market &m);

/* ---------------- colonies ---------------- */

int colonyFound(int systemIndex, int worldIndex);
int colonyCount();
colony *colonyAt(int index);
int colonyBuild(int colonyIndex, colonybuilding kind);
int colonyDemolish(int colonyIndex, int buildingIndex);
void colonyTick(colony &c, int turn);
void coloniesTickAll(int turn);
int colonyUpgrade(int colonyIndex, int buildingIndex);
int colonyCanSettle(int systemIndex, int worldIndex);
const char *colonyBuildingName(colonybuilding kind);
int colonyScore(const colony &c);

/* ---------------- skills ---------------- */

void skillsInit(playerprofile &profile);
int skillUnlock(playerprofile &profile, const char *name);
int skillUpgrade(playerprofile &profile, const char *name);
skill *skillFind(playerprofile &profile, const char *name);
int skillUse(playerprofile &profile, const char *name);
void skillsTick(playerprofile &profile);
void skillShow(const skill &s);
void skillsShowAll(const playerprofile &profile);

/* ---------------- reputation ---------------- */

void standingsInit(playerprofile &profile);
int standingValue(const playerprofile &profile, int factionIndex);
int standingTier(const playerprofile &profile, int factionIndex);
int standingAdjust(playerprofile &profile, int factionIndex, int delta);
const char *standingTierName(int tier);
void standingsShow(const playerprofile &profile);

/* ---------------- contracts ---------------- */

void contractsInit(playerprofile &profile);
int contractOffer(playerprofile &profile, int giverFaction, int turn);
int contractAccept(playerprofile &profile, int index);
int contractProgress(playerprofile &profile, int index, int amount);
int contractComplete(playerprofile &profile, int index);
int contractAbandon(playerprofile &profile, int index);
int contractsTick(playerprofile &profile, int turn);
const char *contractKindName(contractkind kind);
void contractsShow(const playerprofile &profile);

/* ---------------- materials and crafting ---------------- */

void materialsInit(playerprofile &profile);
int materialAdd(playerprofile &profile, const char *name, int quantity, int tier);
int materialCountOf(const playerprofile &profile, const char *name);
int recipeCount();
const recipe *recipeAt(int index);
int canCraft(const playerprofile &profile, int recipeIndex);
int craftItem(playerprofile &profile, int recipeIndex);
void materialsShow(const playerprofile &profile);
void recipesShow();

/* ---------------- codex ---------------- */

void codexInit(playerprofile &profile);
int codexDiscover(playerprofile &profile, const char *title);
int codexHas(const playerprofile &profile, const char *title);
void codexShow(const playerprofile &profile, int category);

/* ---------------- aggregate ---------------- */

// Advance every gameplay system one turn.
void gameplayTick(int turn);

// A one-line summary of the player's overall standing, for the HUD.
void gameplaySummarise(char out[], int outSize);

// Total score contribution from exploration, trading, colonies and contracts.
int gameplayScore(const playerprofile &profile);

#endif /* SBW_GAME_GAMEPLAY_H */
