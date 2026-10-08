// src/data/starships.c++ -- the known starship database.
//
// Hull classes from fighter to flagship, the weapons they mount, and the
// components that fit into their slots.  See docs/STARSHIPS.md.

#include "spacebattlerpg.h"
#include "data/starships.h"

/* ---------------- storage ---------------- */

shipclassdef shipClassTable[MAX_SHIPCLASSES];
component componentTable[MAX_COMPONENTS];
weaponprofile weaponTable[MAX_WEAPONS];

int shipClassCount = 0;
int componentCount = 0;
int weaponRecordCount = 0;

/* ---------------- labels ---------------- */

static const char *hullNames[HULL_COUNT] = {
    "Fighter", "Interceptor", "Bomber", "Corvette", "Frigate",
    "Destroyer", "Cruiser", "Battleship", "Carrier", "Dreadnought",
    "Titan", "Flagship", "Station"};

static const char *weaponTypeNames[WTYPE_COUNT] = {
    "None", "Laser", "Plasma", "Railgun", "Missile", "Torpedo",
    "Ion Cannon", "Particle Beam", "Flak", "Point Defence", "Tesla",
    "Graviton"};

static const char *componentKindNames[COMP_COUNT] = {
    "None", "Weapon", "Shield", "Armor", "Engine", "Reactor",
    "Sensor", "Targeting", "Point Defence", "Fighter Bay", "Repair",
    "Cloak", "AI Core"};

static const char *shipTypeNames[SHIP_COUNT] = {
    "Armored", "Shielded", "Hybrid", "Stealth"};

const char *hullClassName(hullclass hull)
{
    if (hull < 0 || hull >= HULL_COUNT)
        return "Unknown";

    return hullNames[hull];
}

const char *weaponTypeName(weapontype type)
{
    if (type < 0 || type >= WTYPE_COUNT)
        return "Unknown";

    return weaponTypeNames[type];
}

const char *componentKindName(componentkind kind)
{
    if (kind < 0 || kind >= COMP_COUNT)
        return "Unknown";

    return componentKindNames[kind];
}

const char *shipTypeName(shiptype type)
{
    if (type < 0 || type >= SHIP_COUNT)
        return "Unknown";

    return shipTypeNames[type];
}

const char *hullRoleName(hullclass hull)
{
    switch (hull)
    {
    case HULL_FIGHTER:
    case HULL_INTERCEPTOR:
    case HULL_BOMBER:
        return "screen";
    case HULL_CORVETTE:
    case HULL_FRIGATE:
        return "escort";
    case HULL_DESTROYER:
    case HULL_CRUISER:
        return "line";
    case HULL_BATTLESHIP:
    case HULL_CARRIER:
    case HULL_DREADNOUGHT:
        return "capital";
    case HULL_TITAN:
    case HULL_FLAGSHIP:
        return "super-capital";
    case HULL_STATION:
        return "installation";
    default:
        return "unknown";
    }
}

/* ---------------- hull table ---------------- */

// name, hull, type, tier, cost, hullHP, shieldHP, armor, evasion, speed,
// maneuver, reactor, crewReq, crewCap, supply, maintenance,
// wSlots, dSlots, sSlots, fighters, role, buildTurns, description
static const shipclassdef hullSeeds[] = {
    {"Wasp Interceptor", HULL_FIGHTER, SHIP_SHIELDED, 1, 120,
     80, 120, 4, 85, 180, 90, 60, 1, 1, 1, 1, 1, 1, 0, 0,
     "screen", 1, "Fast, fragile, and cheap. Screens the line."},

    {"Raptor Interceptor", HULL_INTERCEPTOR, SHIP_SHIELDED, 2, 260,
     140, 220, 6, 80, 200, 95, 110, 2, 2, 2, 2, 2, 1, 1, 0,
     "screen", 2, "Anti-fighter specialist with high evasion."},

    {"Hammer Bomber", HULL_BOMBER, SHIP_ARMORED, 2, 320,
     220, 80, 14, 45, 120, 55, 140, 3, 3, 3, 3, 2, 1, 1, 0,
     "screen", 2, "Carries torpedoes. Deadly against capitals."},

    {"Sabre Corvette", HULL_CORVETTE, SHIP_HYBRID, 2, 420,
     320, 260, 12, 65, 150, 70, 200, 6, 8, 4, 4, 3, 2, 1, 0,
     "escort", 3, "Balanced patrol craft. Common pirate hull."},

    {"Bulwark Frigate", HULL_FRIGATE, SHIP_ARMORED, 3, 780,
     620, 380, 22, 50, 120, 50, 340, 14, 18, 7, 6, 4, 3, 2, 0,
     "escort", 4, "Anti-fighter escort with point-defence arrays."},

    {"Lancer Destroyer", HULL_DESTROYER, SHIP_HYBRID, 3, 1250,
     980, 720, 30, 40, 105, 42, 620, 30, 40, 11, 10, 6, 4, 3, 0,
     "line", 6, "Front-line gun platform. Spine-mounted railgun."},

    {"Vigil Cruiser", HULL_CRUISER, SHIP_SHIELDED, 4, 2400,
     1800, 1700, 40, 32, 92, 34, 1200, 70, 95, 18, 18, 8, 6, 4, 1,
     "line", 9, "Independent operator. Long endurance, deep magazine."},

    {"Sovereign Battleship", HULL_BATTLESHIP, SHIP_HYBRID, 5, 5200,
     4200, 3600, 62, 22, 78, 26, 2600, 180, 240, 34, 36, 12, 9, 6, 2,
     "capital", 14, "The line breaker. Broadside of particle beams."},

    {"Aegis Carrier", HULL_CARRIER, SHIP_SHIELDED, 5, 5800,
     3600, 4200, 48, 18, 70, 22, 2800, 160, 420, 30, 40, 8, 10, 8, 24,
     "capital", 16, "Projects fighter wings across the battlespace."},

    {"Obliterator Dreadnought", HULL_DREADNOUGHT, SHIP_ARMORED, 6, 11000,
     8800, 6400, 90, 12, 62, 18, 5200, 420, 560, 60, 80, 18, 14, 10, 4,
     "capital", 24, "Siege hull. Built to crack stations and worlds."},

    {"Colossus Titan", HULL_TITAN, SHIP_ARMORED, 7, 24000,
     18000, 12000, 120, 8, 50, 12, 9000, 900, 1100, 95, 160, 24, 20, 14, 8,
     "super-capital", 40, "A fleet in one hull. Rarely seen, never forgotten."},

    {"Divine Throne Ship", HULL_FLAGSHIP, SHIP_HYBRID, 8, 48000,
     34000, 26000, 140, 10, 55, 14, 16000, 1600, 2000, 150, 280, 30, 24, 18, 12,
     "super-capital", 60, "The Divine Order's command vessel. Raid target."},

    {"Orbital Fortress", HULL_STATION, SHIP_ARMORED, 4, 3600,
     7200, 2400, 70, 0, 0, 0, 1800, 120, 300, 20, 30, 14, 10, 8, 6,
     "installation", 20, "Static defence. Cannot move, very hard to kill."}};

static const int hullSeedCount =
    (int)(sizeof(hullSeeds) / sizeof(hullSeeds[0]));

/* ---------------- weapon table ---------------- */

// name, type, damage, accuracy, range, cooldown, shieldBonus, armorBonus,
// ammo, power
static const weaponprofile weaponSeeds[] = {
    {"Pulse Laser", WTYPE_LASER, 40, 82, 3, 2, 90, 110, 0, 6},
    {"Heavy Laser", WTYPE_LASER, 90, 76, 4, 3, 95, 120, 0, 14},
    {"Plasma Lance", WTYPE_PLASMA, 160, 68, 5, 4, 120, 90, 0, 26},
    {"Fusion Cannon", WTYPE_PLASMA, 300, 60, 6, 6, 130, 85, 0, 48},
    {"Light Railgun", WTYPE_RAILGUN, 120, 74, 7, 4, 70, 150, 2, 22},
    {"Spinal Railgun", WTYPE_RAILGUN, 420, 62, 9, 8, 60, 180, 4, 60},
    {"Hornet Missile", WTYPE_MISSILE, 70, 88, 6, 3, 100, 120, 3, 12},
    {"Swarm Missiles", WTYPE_MISSILE, 55, 92, 5, 2, 105, 115, 2, 10},
    {"Heavy Torpedo", WTYPE_TORPEDO, 480, 66, 8, 9, 80, 200, 6, 70},
    {"Ion Cannon", WTYPE_ION, 110, 78, 5, 3, 200, 40, 0, 24},
    {"Ion Disruptor", WTYPE_ION, 220, 70, 6, 5, 240, 30, 0, 44},
    {"Particle Beam", WTYPE_PARTICLE_BEAM, 260, 80, 7, 5, 140, 140, 0, 52},
    {"Beam Lance", WTYPE_PARTICLE_BEAM, 520, 70, 10, 9, 150, 130, 0, 95},
    {"Flak Battery", WTYPE_FLAK, 45, 86, 4, 2, 70, 80, 1, 8},
    {"Point Defence Grid", WTYPE_POINT_DEFENCE, 30, 94, 2, 1, 60, 60, 1, 6},
    {"Tesla Arc", WTYPE_TESLA, 95, 84, 5, 3, 160, 70, 0, 20},
    {"Graviton Driver", WTYPE_GRAVITON, 380, 58, 9, 8, 110, 170, 0, 88},
    {"Singularity Projector", WTYPE_GRAVITON, 700, 50, 11, 12, 120, 190, 0, 140}};

static const int weaponSeedCount =
    (int)(sizeof(weaponSeeds) / sizeof(weaponSeeds[0]));

/* ---------------- component table ---------------- */

// name, kind, tier, cost, mass, hp, powerDraw, powerOut, shield, armor,
// speed, sensor, accuracy, repair, damage type
static const component componentSeeds[] = {
    /* reactors */
    {"Fission Core I", COMP_REACTOR, 1, 120, 40, 40, 0, 200, 0, 0, 0, 0, 0, 0, DMG_TRUE},
    {"Fusion Core II", COMP_REACTOR, 2, 320, 70, 70, 0, 520, 0, 0, 0, 0, 0, 0, DMG_TRUE},
    {"Antimatter Core III", COMP_REACTOR, 3, 780, 110, 120, 0, 1200, 0, 0, 0, 0, 0, 0, DMG_TRUE},
    {"Quantum Core IV", COMP_REACTOR, 4, 1800, 160, 180, 0, 2800, 0, 0, 0, 0, 0, 0, DMG_TRUE},

    /* shields */
    {"Deflector I", COMP_SHIELD, 1, 100, 30, 30, 20, 0, 200, 0, 0, 0, 0, 0, DMG_TRUE},
    {"Deflector II", COMP_SHIELD, 2, 280, 55, 60, 45, 0, 560, 0, 0, 0, 0, 0, DMG_TRUE},
    {"Aegis Shield III", COMP_SHIELD, 3, 700, 90, 100, 90, 0, 1400, 0, 0, 0, 0, 0, DMG_TRUE},
    {"Aegis Shield IV", COMP_SHIELD, 4, 1600, 140, 160, 170, 0, 3200, 0, 0, 0, 0, 0, DMG_TRUE},

    /* armor */
    {"Ablative Plate I", COMP_ARMOR, 1, 90, 60, 120, 0, 0, 0, 60, 0, 0, 0, 0, DMG_TRUE},
    {"Ablative Plate II", COMP_ARMOR, 2, 240, 100, 260, 0, 0, 0, 150, 0, 0, 0, 0, DMG_TRUE},
    {"Crystal Weave III", COMP_ARMOR, 3, 620, 150, 480, 0, 0, 0, 320, 0, 0, 0, 0, DMG_TRUE},
    {"Neutronium Plate IV", COMP_ARMOR, 4, 1500, 220, 900, 0, 0, 0, 700, 0, 0, 0, 0, DMG_TRUE},

    /* engines */
    {"Ion Drive I", COMP_ENGINE, 1, 80, 50, 50, 40, 0, 0, 0, 40, 0, 0, 0, DMG_TRUE},
    {"Ion Drive II", COMP_ENGINE, 2, 220, 80, 90, 80, 0, 0, 0, 95, 0, 0, 0, DMG_TRUE},
    {"Warp Drive III", COMP_ENGINE, 3, 560, 120, 150, 150, 0, 0, 0, 180, 0, 0, 0, DMG_TRUE},

    /* sensors */
    {"Sensor Array I", COMP_SENSOR, 1, 70, 20, 30, 15, 0, 0, 0, 0, 60, 0, 0, DMG_TRUE},
    {"Deep Scanner II", COMP_SENSOR, 2, 200, 35, 50, 35, 0, 0, 0, 0, 150, 0, 0, DMG_TRUE},
    {"Quantum Sensor III", COMP_SENSOR, 3, 520, 55, 80, 70, 0, 0, 0, 0, 320, 0, 0, DMG_TRUE},

    /* targeting */
    {"Targeting Computer I", COMP_TARGETING, 1, 90, 15, 25, 20, 0, 0, 0, 0, 0, 40, 0, DMG_TRUE},
    {"Targeting Computer II", COMP_TARGETING, 2, 260, 25, 45, 45, 0, 0, 0, 0, 0, 95, 0, DMG_TRUE},
    {"Fire Control III", COMP_TARGETING, 3, 640, 40, 70, 90, 0, 0, 0, 0, 0, 200, 0, DMG_TRUE},

    /* point defence */
    {"PD Turret I", COMP_POINT_DEFENCE, 1, 110, 25, 40, 25, 0, 0, 0, 0, 40, 30, 0, DMG_PHYSICAL},
    {"PD Grid II", COMP_POINT_DEFENCE, 2, 300, 45, 70, 55, 0, 0, 0, 0, 90, 70, 0, DMG_PHYSICAL},

    /* fighter bays */
    {"Fighter Bay I", COMP_FIGHTER_BAY, 2, 400, 120, 150, 60, 0, 0, 0, 0, 30, 0, 0, DMG_TRUE},
    {"Fighter Bay II", COMP_FIGHTER_BAY, 3, 950, 190, 260, 120, 0, 0, 0, 0, 60, 0, 0, DMG_TRUE},

    /* repair */
    {"Repair Drone Bay I", COMP_REPAIR, 2, 340, 70, 90, 60, 0, 0, 40, 0, 0, 0, 60, DMG_TRUE},
    {"Repair Drone Bay II", COMP_REPAIR, 3, 820, 110, 150, 120, 0, 0, 90, 0, 0, 0, 150, DMG_TRUE},

    /* cloak */
    {"Stealth Field II", COMP_CLOAK, 2, 600, 80, 80, 120, 0, 0, 20, 0, 0, 0, 0, DMG_TRUE},

    /* ai core */
    {"AI Core I", COMP_AI_CORE, 2, 480, 40, 60, 70, 0, 0, 0, 0, 50, 50, 0, DMG_TRUE},
    {"AI Core II", COMP_AI_CORE, 3, 1200, 70, 110, 140, 0, 0, 0, 0, 120, 120, 0, DMG_TRUE},

    /* weapons as components */
    {"Pulse Laser Turret", COMP_WEAPON, 1, 150, 30, 40, 6, 0, 0, 0, 0, 0, 0, 0, DMG_PHYSICAL},
    {"Heavy Laser Turret", COMP_WEAPON, 2, 380, 60, 75, 14, 0, 0, 0, 0, 0, 0, 0, DMG_PHYSICAL},
    {"Plasma Lance Turret", COMP_WEAPON, 3, 900, 95, 120, 26, 0, 0, 0, 0, 0, 0, 0, DMG_FIRE},
    {"Railgun Battery", COMP_WEAPON, 2, 340, 70, 90, 22, 0, 0, 0, 0, 0, 0, 0, DMG_PHYSICAL},
    {"Spinal Railgun", COMP_WEAPON, 4, 1600, 180, 220, 60, 0, 0, 0, 0, 0, 0, 0, DMG_PHYSICAL},
    {"Missile Rack", COMP_WEAPON, 2, 300, 55, 70, 12, 0, 0, 0, 0, 0, 0, 0, DMG_PHYSICAL},
    {"Torpedo Tube", COMP_WEAPON, 3, 720, 100, 110, 70, 0, 0, 0, 0, 0, 0, 0, DMG_PHYSICAL},
    {"Ion Cannon Mount", COMP_WEAPON, 3, 780, 90, 130, 24, 0, 0, 0, 0, 0, 0, 0, DMG_LIGHTNING},
    {"Particle Beam Emitter", COMP_WEAPON, 4, 1450, 140, 180, 52, 0, 0, 0, 0, 0, 0, 0, DMG_ARCANE},
    {"Graviton Driver", COMP_WEAPON, 5, 2600, 220, 280, 88, 0, 0, 0, 0, 0, 0, 0, DMG_VOID}};

static const int componentSeedCount =
    (int)(sizeof(componentSeeds) / sizeof(componentSeeds[0]));

/* ---------------- initializers ---------------- */

void initializeStarships()
{
    shipClassCount = 0;

    for (int i = 0; i < hullSeedCount && i < MAX_SHIPCLASSES; ++i)
    {
        shipClassTable[shipClassCount] = hullSeeds[i];
        shipClassCount++;
    }
}

void initializeWeapons()
{
    weaponRecordCount = 0;

    for (int i = 0; i < weaponSeedCount && i < MAX_WEAPONS; ++i)
    {
        weaponTable[weaponRecordCount] = weaponSeeds[i];
        weaponRecordCount++;
    }
}

void initializeComponents()
{
    componentCount = 0;

    for (int i = 0; i < componentSeedCount && i < MAX_COMPONENTS; ++i)
    {
        componentTable[componentCount] = componentSeeds[i];
        componentCount++;
    }
}

/* ---------------- lookups ---------------- */

int findShipClass(const char *name)
{
    if (!name)
        return -1;

    for (int i = 0; i < shipClassCount; ++i)
    {
        if (strcmp(shipClassTable[i].name, name) == 0)
            return i;
    }

    return -1;
}

int findComponent(const char *name)
{
    if (!name)
        return -1;

    for (int i = 0; i < componentCount; ++i)
    {
        if (strcmp(componentTable[i].name, name) == 0)
            return i;
    }

    return -1;
}

int findWeapon(const char *name)
{
    if (!name)
        return -1;

    for (int i = 0; i < weaponRecordCount; ++i)
    {
        if (strcmp(weaponTable[i].name, name) == 0)
            return i;
    }

    return -1;
}

const shipclassdef *shipClassAt(int index)
{
    if (index < 0 || index >= shipClassCount)
        return &shipClassTable[0];

    return &shipClassTable[index];
}

const component *componentAt(int index)
{
    if (index < 0 || index >= componentCount)
        return 0;

    return &componentTable[index];
}

const weaponprofile *weaponAt(int index)
{
    if (index < 0 || index >= weaponRecordCount)
        return 0;

    return &weaponTable[index];
}

int shipsOfHull(hullclass hull, int out[], int outMax)
{
    if (!out || outMax <= 0)
        return 0;

    int written = 0;

    for (int i = 0; i < shipClassCount && written < outMax; ++i)
    {
        if (shipClassTable[i].hull == hull)
        {
            out[written] = i;
            written++;
        }
    }

    return written;
}

int componentsOfKind(componentkind kind, int out[], int outMax)
{
    if (!out || outMax <= 0)
        return 0;

    int written = 0;

    for (int i = 0; i < componentCount && written < outMax; ++i)
    {
        if (componentTable[i].kind == kind)
        {
            out[written] = i;
            written++;
        }
    }

    return written;
}

/* ---------------- fit maths ---------------- */

int fitCost(int shipClassIndex, const int componentIndices[], int count)
{
    const shipclassdef *hull = shipClassAt(shipClassIndex);

    if (shipClassIndex < 0 || shipClassIndex >= shipClassCount)
        return 0;

    int cost = hull->cost;

    for (int i = 0; i < count; ++i)
    {
        const component *c = componentAt(componentIndices[i]);

        if (c)
            cost += c->cost;
    }

    return cost;
}

int fitPowerBalance(int shipClassIndex, const int componentIndices[], int count)
{
    const shipclassdef *hull = shipClassAt(shipClassIndex);

    if (shipClassIndex < 0 || shipClassIndex >= shipClassCount)
        return 0;

    int balance = hull->reactorOutput;

    for (int i = 0; i < count; ++i)
    {
        const component *c = componentAt(componentIndices[i]);

        if (!c)
            continue;

        balance += c->powerOutput;
        balance -= c->powerDraw;
    }

    return balance;
}

int fittedShipValue(int shipClassIndex, const int componentIndices[], int count)
{
    const shipclassdef *hull = shipClassAt(shipClassIndex);

    if (shipClassIndex < 0 || shipClassIndex >= shipClassCount)
        return 0;

    // Offence: hull base, plus weapons.
    int value = hull->hullHP / 4;
    value += hull->shieldHP / 5;
    value += hull->armorRating * 3;
    value += hull->weaponSlots * 40;

    // Defence and systems from the fitted components.
    for (int i = 0; i < count; ++i)
    {
        const component *c = componentAt(componentIndices[i]);

        if (!c)
            continue;

        value += c->hp / 2;
        value += c->shieldValue / 3;
        value += c->armorValue / 3;
        value += c->sensorValue / 4;
        value += c->accuracyValue / 4;
        value += c->repairValue / 3;
        value += c->powerOutput / 6;
    }

    return value;
}

int validateFit(int shipClassIndex, const int componentIndices[], int count)
{
    const shipclassdef *hull = shipClassAt(shipClassIndex);

    if (shipClassIndex < 0 || shipClassIndex >= shipClassCount)
        return 1;

    int usedWeapon = 0;
    int usedDefence = 0;
    int usedSystem = 0;

    for (int i = 0; i < count; ++i)
    {
        const component *c = componentAt(componentIndices[i]);

        if (!c)
            return 2; // invalid component index

        switch (c->kind)
        {
        case COMP_WEAPON:
        case COMP_POINT_DEFENCE:
            usedWeapon++;
            break;

        case COMP_SHIELD:
        case COMP_ARMOR:
        case COMP_ENGINE:
            usedDefence++;
            break;

        default:
            usedSystem++;
            break;
        }
    }

    if (usedWeapon > hull->weaponSlots)
        return 3; // too many weapons

    if (usedDefence > hull->defenceSlots)
        return 4; // too many defences

    if (usedSystem > hull->systemSlots)
        return 5; // too many systems

    if (fitPowerBalance(shipClassIndex, componentIndices, count) < 0)
        return 6; // reactor overloaded

    return 0;
}

int weaponDamageVs(const weaponprofile &weapon, shiptype target)
{
    int damage = weapon.damage;

    switch (target)
    {
    case SHIP_ARMORED:
        // Armor is weak to energy, strong against kinetic.
        if (weapon.type == WTYPE_LASER || weapon.type == WTYPE_PLASMA || weapon.type == WTYPE_ION || weapon.type == WTYPE_PARTICLE_BEAM)
            damage = (damage * 120) / 100;
        else if (weapon.type == WTYPE_RAILGUN)
            damage = (damage * 80) / 100;
        break;

    case SHIP_SHIELDED:
        // Shields are weak to ion, strong against energy.
        if (weapon.type == WTYPE_ION)
            damage = (damage * 200) / 100;
        else if (weapon.type == WTYPE_LASER || weapon.type == WTYPE_PLASMA)
            damage = (damage * 85) / 100;
        else if (weapon.type == WTYPE_RAILGUN)
            damage = (damage * 130) / 100;
        break;

    case SHIP_HYBRID:
        break;

    case SHIP_STEALTH:
        // Stealth hulls are hard to hit, not hard to hurt.
        damage = (damage * 110) / 100;
        break;

    default:
        break;
    }

    return damage;
}

int bestWeaponForHull(hullclass hull)
{
    // Capital hulls want heavy weapons; screen hulls want fast ones.
    const char *preferred = "Pulse Laser";

    switch (hull)
    {
    case HULL_FIGHTER:
    case HULL_INTERCEPTOR:
        preferred = "Pulse Laser";
        break;
    case HULL_BOMBER:
        preferred = "Heavy Torpedo";
        break;
    case HULL_CORVETTE:
    case HULL_FRIGATE:
        preferred = "Hornet Missile";
        break;
    case HULL_DESTROYER:
        preferred = "Light Railgun";
        break;
    case HULL_CRUISER:
        preferred = "Heavy Laser";
        break;
    case HULL_BATTLESHIP:
        preferred = "Particle Beam";
        break;
    case HULL_CARRIER:
        preferred = "Flak Battery";
        break;
    case HULL_DREADNOUGHT:
        preferred = "Spinal Railgun";
        break;
    case HULL_TITAN:
    case HULL_FLAGSHIP:
        preferred = "Beam Lance";
        break;
    case HULL_STATION:
        preferred = "Point Defence Grid";
        break;
    default:
        break;
    }

    int index = findWeapon(preferred);

    return (index < 0) ? 0 : index;
}

int defaultLoadout(int shipClassIndex, int out[], int outMax)
{
    if (!out || outMax <= 0)
        return 0;

    const shipclassdef *hull = shipClassAt(shipClassIndex);

    if (shipClassIndex < 0 || shipClassIndex >= shipClassCount)
        return 0;

    int written = 0;

    // Rules of thumb: one reactor and engine, then proportionally split the
    // remaining slots between weapons, shields and systems.
    const char *plan[8];
    int planCount = 0;

    plan[planCount++] = "Fusion Core II";
    plan[planCount++] = "Ion Drive II";
    plan[planCount++] = "Deflector II";
    plan[planCount++] = "Ablative Plate II";

    if (hull->weaponSlots >= 4)
        plan[planCount++] = "Heavy Laser Turret";
    else
        plan[planCount++] = "Pulse Laser Turret";

    if (hull->fighterCapacity > 0)
        plan[planCount++] = "Fighter Bay I";

    if (hull->tier >= 5)
        plan[planCount++] = "Targeting Computer II";
    else
        plan[planCount++] = "Targeting Computer I";

    if (hull->defenceSlots >= 4)
        plan[planCount++] = "PD Grid II";

    for (int i = 0; i < planCount && written < outMax; ++i)
    {
        int index = findComponent(plan[i]);

        if (index >= 0)
        {
            out[written] = index;
            written++;
        }
    }

    return written;
}

/* ---------------- fleet composition ---------------- */

int fleetHullSupply(const int shipClassIndices[], int count)
{
    if (!shipClassIndices || count <= 0)
        return 0;

    int supply = 0;

    for (int i = 0; i < count; ++i)
    {
        const shipclassdef *hull = shipClassAt(shipClassIndices[i]);

        if (shipClassIndices[i] >= 0 && shipClassIndices[i] < shipClassCount)
            supply += hull->supplyUse;
    }

    return supply;
}

int recommendedEscorts(hullclass hull)
{
    switch (hull)
    {
    case HULL_DREADNOUGHT:
        return 12;
    case HULL_BATTLESHIP:
    case HULL_CARRIER:
        return 8;
    case HULL_TITAN:
    case HULL_FLAGSHIP:
        return 20;
    case HULL_STATION:
        return 6;
    default:
        return 2;
    }
}

/* ---------------- presentation ---------------- */

void showShipClassList()
{
    drawHeader("Ship Classes");

    for (int i = 0; i < shipClassCount; ++i)
    {
        const shipclassdef &s = shipClassTable[i];

        cout << "  " << i << ". " << s.name;

        int length = (int)strlen(s.name);
        for (int pad = length; pad < 24; ++pad)
            cout << ' ';

        cout << hullClassName(s.hull)
             << "  T" << s.tier
             << "  cost " << s.cost
             << "  hp " << s.hullHP << endl;
    }

    cout << endl;
}

void showShipClassDetail(int shipClassIndex)
{
    const shipclassdef *s = shipClassAt(shipClassIndex);

    if (shipClassIndex < 0 || shipClassIndex >= shipClassCount)
    {
        out("No such ship class.");
        return;
    }

    drawHeader(s->name);

    drawFieldText("Hull", hullClassName(s->hull));
    drawFieldText("Type", shipTypeName(s->type));
    drawFieldText("Role", s->role);
    drawField("Tier", s->tier);
    drawField("Cost", s->cost);

    cout << endl
         << "  Hull:" << endl;
    drawField("Hull HP", s->hullHP);
    drawField("Shield HP", s->shieldHP);
    drawField("Armor", s->armorRating);
    drawField("Evasion", s->evasion);
    drawField("Speed", s->speed);

    cout << endl
         << "  Systems:" << endl;
    drawField("Reactor output", s->reactorOutput);
    drawField("Crew required", s->crewRequired);
    drawField("Supply use", s->supplyUse);
    drawField("Maintenance", s->maintenance);

    cout << endl
         << "  Hardpoints:" << endl;
    drawField("Weapon slots", s->weaponSlots);
    drawField("Defence slots", s->defenceSlots);
    drawField("System slots", s->systemSlots);
    drawField("Fighter capacity", s->fighterCapacity);

    cout << endl;
    outWrapped(s->description);
    cout << endl;
}

void showComponentList()
{
    drawHeader("Components");

    for (int i = 0; i < componentCount; ++i)
    {
        const component &c = componentTable[i];

        cout << "  " << i << ". " << c.name;

        int length = (int)strlen(c.name);
        for (int pad = length; pad < 26; ++pad)
            cout << ' ';

        cout << componentKindName(c.kind)
             << "  T" << c.tier
             << "  cost " << c.cost
             << "  pwr " << c.powerDraw << endl;
    }

    cout << endl;
}

void showWeaponList()
{
    drawHeader("Weapons");

    for (int i = 0; i < weaponRecordCount; ++i)
    {
        const weaponprofile &w = weaponTable[i];

        cout << "  " << i << ". " << w.name;

        int length = (int)strlen(w.name);
        for (int pad = length; pad < 22; ++pad)
            cout << ' ';

        cout << weaponTypeName(w.type)
             << "  dmg " << w.damage
             << "  acc " << w.accuracy
             << "  rng " << w.range
             << "  cd " << w.cooldown << endl;
    }

    cout << endl;
}

void showShipComparison(int classA, int classB)
{
    const shipclassdef *a = shipClassAt(classA);
    const shipclassdef *b = shipClassAt(classB);

    drawHeader("Comparison");

    cout << "  " << a->name;

    int lengthA = (int)strlen(a->name);
    int lengthB = (int)strlen(b->name);
    int gap = lengthA - lengthB;

    for (int pad = 0; pad < 24 - lengthA + gap; ++pad)
        cout << ' ';

    cout << b->name << endl;

    cout << "  hull " << a->hullHP << " / " << b->hullHP << endl;
    cout << "  shield " << a->shieldHP << " / " << b->shieldHP << endl;
    cout << "  armor " << a->armorRating << " / " << b->armorRating << endl;
    cout << "  speed " << a->speed << " / " << b->speed << endl;
    cout << "  evasion " << a->evasion << " / " << b->evasion << endl;
    cout << "  slots " << a->weaponSlots << " / " << b->weaponSlots << endl;
    cout << "  cost " << a->cost << " / " << b->cost << endl;

    cout << endl;
}
