// include/data/starships.h -- the known starship database.
//
// Ships are built from classes, and every class is a hull plus a slot layout
// that determines what weapons, defences and systems it can carry.
//
//   Hull class -> slots (weapon/defence/system) -> components -> stats
//
// See docs/STARSHIPS.md.

#ifndef SBW_DATA_STARSHIPS_H
#define SBW_DATA_STARSHIPS_H

/* ---------------- limits ---------------- */

#define MAX_SHIPCLASSES 48
#define MAX_COMPONENTS 64
#define MAX_WEAPONS 32

/* ---------------- hull classes ---------------- */

enum hullclass
{
    HULL_FIGHTER = 0,
    HULL_INTERCEPTOR,
    HULL_BOMBER,
    HULL_CORVETTE,
    HULL_FRIGATE,
    HULL_DESTROYER,
    HULL_CRUISER,
    HULL_BATTLESHIP,
    HULL_CARRIER,
    HULL_DREADNOUGHT,
    HULL_TITAN,
    HULL_FLAGSHIP,
    HULL_STATION,
    HULL_COUNT
};

/* ---------------- weapon taxonomy ---------------- */

enum weapontype
{
    WTYPE_NONE = 0,
    WTYPE_LASER,
    WTYPE_PLASMA,
    WTYPE_RAILGUN,
    WTYPE_MISSILE,
    WTYPE_TORPEDO,
    WTYPE_ION,
    WTYPE_PARTICLE_BEAM,
    WTYPE_FLAK,
    WTYPE_POINT_DEFENCE,
    WTYPE_TESLA,
    WTYPE_GRAVITON,
    WTYPE_COUNT
};

/* ---------------- component taxonomy ---------------- */

enum componentkind
{
    COMP_NONE = 0,
    COMP_WEAPON,
    COMP_SHIELD,
    COMP_ARMOR,
    COMP_ENGINE,
    COMP_REACTOR,
    COMP_SENSOR,
    COMP_TARGETING,
    COMP_POINT_DEFENCE,
    COMP_FIGHTER_BAY,
    COMP_REPAIR,
    COMP_CLOAK,
    COMP_AI_CORE,
    COMP_COUNT
};

/* ---------------- damage / defence typing ---------------- */

enum shiptype
{
    SHIP_ARMORED = 0, // resists kinetic, weak to energy
    SHIP_SHIELDED,    // resists energy, weak to ion
    SHIP_HYBRID,
    SHIP_STEALTH,
    SHIP_COUNT
};

/* ---------------- records ---------------- */

struct weaponprofile
{
    char name[32];
    weapontype type;
    int damage;
    int accuracy;    // 0..100
    int range;       // in light-seconds
    int cooldown;    // ticks
    int shieldBonus; // % effectiveness vs shields
    int armorBonus;  // % effectiveness vs armor
    int ammoCost;
    int powerDraw;
};

struct component
{
    char name[32];
    componentkind kind;
    int tier; // 1..5
    int cost;
    int mass;
    int hp;
    int powerDraw;
    int powerOutput;
    int shieldValue;
    int armorValue;
    int speedValue;
    int sensorValue;
    int accuracyValue;
    int repairValue;
    damagetype damageType;
};

struct shipclassdef
{
    char name[32];
    hullclass hull;
    shiptype type;
    int tier;
    int cost;

    /* hull stats */
    int hullHP;
    int shieldHP;
    int armorRating;
    int evasion; // 0..100
    int speed;
    int maneuver;

    /* power and crew */
    int reactorOutput;
    int crewRequired;
    int crewCapacity;
    int supplyUse;
    int maintenance;

    /* hardpoints */
    int weaponSlots;
    int defenceSlots;
    int systemSlots;
    int fighterCapacity;

    /* role and flavour */
    char role[24];
    int buildTurns;
    char description[120];
};

/* ---------------- tables ---------------- */

extern shipclassdef shipClassTable[MAX_SHIPCLASSES];
extern component componentTable[MAX_COMPONENTS];
extern weaponprofile weaponTable[MAX_WEAPONS];

extern int shipClassCount;
extern int componentCount;
extern int weaponRecordCount;

/* ---------------- lifecycle ---------------- */

void initializeStarships();
void initializeComponents();
void initializeWeapons();

/* ---------------- lookups ---------------- */

const char *hullClassName(hullclass hull);
const char *weaponTypeName(weapontype type);
const char *componentKindName(componentkind kind);
const char *shipTypeName(shiptype type);

int findShipClass(const char *name);
int findComponent(const char *name);
int findWeapon(const char *name);

const shipclassdef *shipClassAt(int index);
const component *componentAt(int index);
const weaponprofile *weaponAt(int index);

// Every ship of a given hull class.  Returns how many were written.
int shipsOfHull(hullclass hull, int out[], int outMax);

// Every component of a kind.  Returns how many were written.
int componentsOfKind(componentkind kind, int out[], int outMax);

/* ---------------- derived ship maths ---------------- */

// Total build cost of a hull with the given components fitted.
int fitCost(int shipClassIndex, const int componentIndices[], int count);

// Total power draw minus output; negative means the reactor is overloaded.
int fitPowerBalance(int shipClassIndex, const int componentIndices[], int count);

// Combat value of a fitted ship, used by fleet power and encounter budgets.
int fittedShipValue(int shipClassIndex, const int componentIndices[], int count);

// Does the fit break any rule (slots, power, crew)?
// Returns 0 when legal, or a positive error code.
int validateFit(int shipClassIndex, const int componentIndices[], int count);

// Effective damage of a weapon against a target ship type.
int weaponDamageVs(const weaponprofile &weapon, shiptype target);

// Best weapon a hull can mount, by raw damage.
int bestWeaponForHull(hullclass hull);

// A sane default loadout for a hull, written as component indices.
// Returns how many components were chosen.
int defaultLoadout(int shipClassIndex, int out[], int outMax);

/* ---------------- fleet composition ---------------- */

// Supply cost of a fleet of these hulls.
int fleetHullSupply(const int shipClassIndices[], int count);

// Recommended escort count for a capital ship.
int recommendedEscorts(hullclass hull);

// Role text for a hull ("screen", "line", "capital", "support").
const char *hullRoleName(hullclass hull);

/* ---------------- presentation ---------------- */

void showShipClassList();
void showShipClassDetail(int shipClassIndex);
void showComponentList();
void showWeaponList();
void showShipComparison(int classA, int classB);

#endif /* SBW_DATA_STARSHIPS_H */
