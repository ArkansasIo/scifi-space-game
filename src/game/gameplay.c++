// src/game/gameplay.c++ -- player-facing gameplay systems.
//
// Exploration, trading, colonies, skills, reputation, contracts, crafting and
// the codex.  Everything operates on the systems, worlds and factions that
// already exist, so no new world state has to be serialised.
//
// See docs/GAMEPLAY.md.

#include "spacebattlerpg.h"
#include "game/gameplay.h"
#include "world/universe_gen.h"
#include "world/biome.h"
#include "ui/display.h"

/* ---------------- storage ---------------- */

static playerprofile g_profile;

/* ---------------- helpers ---------------- */

static void copyString(char *dest, int destSize, const char *src)
{
    if (!dest || destSize <= 0)
        return;

    strncpy(dest, src ? src : "", destSize - 1);
    dest[destSize - 1] = '\0';
}

static int nameEquals(const char *a, const char *b)
{
    if (!a || !b)
        return 0;

    while (*a && *b)
    {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return 0;

        a++;
        b++;
    }

    return (*a == '\0' && *b == '\0') ? 1 : 0;
}

// A small deterministic RNG so markets and contracts reproduce from a seed.
static unsigned int g_gpRng = 1u;

static void gpSeed(unsigned int seed)
{
    g_gpRng = (seed == 0u) ? 1u : seed;
}

static int gpRand(int maxExclusive)
{
    if (maxExclusive <= 0)
        return 0;

    g_gpRng = (g_gpRng * 1664525u) + 1013904223u;
    return (int)(g_gpRng % (unsigned int)maxExclusive);
}

static int gpBetween(int minimum, int maximum)
{
    if (maximum <= minimum)
        return minimum;

    return minimum + gpRand(maximum - minimum + 1);
}

/* ---------------- lifecycle ---------------- */

void gameplayReset()
{
    memset(&g_profile, 0, sizeof(g_profile));

    cargoInit(g_profile.cargo);
    skillsInit(g_profile);
    standingsInit(g_profile);
    contractsInit(g_profile);
    materialsInit(g_profile);
    codexInit(g_profile);

    g_profile.scanRange = 2;
    g_profile.scanners = 1;
    g_profile.maxFuel = 100;
    g_profile.fuel = 100;
    g_profile.jumpCount = 0;
    g_profile.turnStarted = 1;
    g_profile.difficulty = 0;
}

void gameplayInit()
{
    gameplayReset();
}

playerprofile &gameplayProfile()
{
    return g_profile;
}

const playerprofile &gameplayProfileConst()
{
    return g_profile;
}

/* ====================================================================== */
/*  EXPLORATION                                                            */
/* ====================================================================== */

int systemVisited(int systemIndex)
{
    if (systemIndex < 0 || systemIndex >= systemWorldCount)
        return 0;

    return systemWorldArray[systemIndex].visited;
}

int scanSystem(int systemIndex)
{
    if (systemIndex < 0 || systemIndex >= systemWorldCount)
        return 0;

    systemworld &sys = systemWorldArray[systemIndex];

    int discoveries = 0;

    // Reveal anomalies within scanner range.
    for (int i = 0; i < sys.anomalyCount && i < 6; ++i)
    {
        anomaly &a = sys.anomalies[i];

        if (a.discovered)
            continue;

        // A better scanner array finds more, further out.
        int chance = 40 + (g_profile.scanners * 20);

        gpSeed((unsigned)(systemIndex * 7919 + i * 131));

        if (gpRand(100) < chance)
        {
            a.discovered = 1;
            discoveries++;

            codexDiscover(g_profile, a.name);

            // First discovery of a relic or research anomaly pays out.
            if (a.kind == OBJ_RELIC || a.kind == OBJ_RESEARCH_ANOMALY)
            {
                g_profile.cargo.credits += a.reward / 2;
            }
            else if (a.kind == OBJ_MINING_FIELD)
            {
                materialAdd(g_profile, "Ore", 5, 1);
            }
        }
    }

    if (!sys.visited)
    {
        sys.visited = 1;
        sys.development += 2;

        if (sys.development > 100)
            sys.development = 100;

        discoveries++;

        turnLog(sys.name[0] ? "Surveyed a new system." : "Surveyed a system.");
    }

    return discoveries;
}

int jumpToSystem(int systemIndex)
{
    if (systemIndex < 0 || systemIndex >= systemWorldCount)
        return -1; // no such system

    // Reachability is the gate graph, not raw distance.
    int routes[MAX_SYSTEMS_WORLDS];
    int count = reachableSystems(0, routes, MAX_SYSTEMS_WORLDS);

    int reachable = 0;

    for (int i = 0; i < count; ++i)
    {
        if (routes[i] == systemIndex)
            reachable = 1;
    }

    if (!reachable)
        return -2; // no gate route

    // Each jump costs fuel in proportion to the distance.
    int distance = 0;

    if (count > 0)
        distance = systemWorldArray[systemIndex].gates[0].distance;

    int cost = 5 + (distance / 4);

    if (g_profile.fuel < cost)
        return -3; // not enough fuel

    g_profile.fuel -= cost;
    g_profile.jumpCount++;

    scanSystem(systemIndex);

    turnLog("Jumped to a new system.");

    return 0;
}

int refuelAtSystem(int systemIndex)
{
    if (systemIndex < 0 || systemIndex >= systemWorldCount)
        return 0;

    const systemworld &sys = systemWorldArray[systemIndex];

    if (!sys.hasStation)
        return 0; // nothing to dock with

    // Refuelling costs credits, scaled by how much is needed.
    int needed = g_profile.maxFuel - g_profile.fuel;

    if (needed <= 0)
        return 1; // already full

    int price = needed * 2;

    if (g_profile.cargo.credits < price)
    {
        // Buy whatever the credits cover.
        int affordable = g_profile.cargo.credits / 2;

        if (affordable <= 0)
            return 0;

        g_profile.fuel += affordable;
        g_profile.cargo.credits -= affordable * 2;

        return affordable;
    }

    g_profile.fuel = g_profile.maxFuel;
    g_profile.cargo.credits -= price;

    return needed;
}

/* ====================================================================== */
/*  CARGO AND TRADING                                                      */
/* ====================================================================== */

void cargoInit(cargohold &hold)
{
    memset(&hold, 0, sizeof(hold));

    hold.capacity = 50;
    hold.used = 0;
    hold.stackCount = 0;
    hold.credits = 1000; // starting purse
}

int cargoUsed(const cargohold &hold)
{
    int used = 0;

    for (int i = 0; i < hold.stackCount && i < MAX_CARGO; ++i)
        used += hold.stacks[i].quantity;

    return used;
}

int cargoFree(const cargohold &hold)
{
    int free = hold.capacity - cargoUsed(hold);

    return (free < 0) ? 0 : free;
}

int cargoCountOf(const cargohold &hold, const char *name)
{
    if (!name)
        return 0;

    for (int i = 0; i < hold.stackCount && i < MAX_CARGO; ++i)
    {
        if (nameEquals(hold.stacks[i].name, name))
            return hold.stacks[i].quantity;
    }

    return 0;
}

int cargoAdd(cargohold &hold, const char *name, int quantity, int unitPrice)
{
    if (!name || quantity <= 0)
        return 0;

    if (cargoFree(hold) < quantity)
        quantity = cargoFree(hold);

    if (quantity <= 0)
        return 0;

    // Add to an existing stack, or open a new one.
    for (int i = 0; i < hold.stackCount && i < MAX_CARGO; ++i)
    {
        if (!nameEquals(hold.stacks[i].name, name))
            continue;

        hold.stacks[i].quantity += quantity;

        // Keep a weighted average of what we paid, for profit reporting.
        if (unitPrice > 0)
        {
            int oldTotal = hold.stacks[i].buyPrice * (hold.stacks[i].quantity - quantity);
            int newTotal = oldTotal + (unitPrice * quantity);

            hold.stacks[i].buyPrice =
                newTotal / hold.stacks[i].quantity;
        }

        return quantity;
    }

    if (hold.stackCount >= MAX_CARGO)
        return 0;

    cargostack &stack = hold.stacks[hold.stackCount];

    memset(&stack, 0, sizeof(stack));
    copyString(stack.name, sizeof(stack.name), name);
    stack.quantity = quantity;
    stack.buyPrice = unitPrice;

    hold.stackCount++;

    return quantity;
}

int cargoRemove(cargohold &hold, const char *name, int quantity)
{
    if (!name || quantity <= 0)
        return 0;

    for (int i = 0; i < hold.stackCount && i < MAX_CARGO; ++i)
    {
        if (!nameEquals(hold.stacks[i].name, name))
            continue;

        int removed = (quantity < hold.stacks[i].quantity)
                          ? quantity
                          : hold.stacks[i].quantity;

        hold.stacks[i].quantity -= removed;

        // Collapse an empty stack so the hold does not fill with zeroes.
        if (hold.stacks[i].quantity <= 0)
        {
            for (int j = i; j < hold.stackCount - 1; ++j)
                hold.stacks[j] = hold.stacks[j + 1];

            hold.stackCount--;
        }

        return removed;
    }

    return 0;
}

int cargoSell(cargohold &hold, const char *name, int quantity, int unitPrice)
{
    int sold = cargoRemove(hold, name, quantity);

    if (sold <= 0)
        return 0;

    hold.credits += sold * unitPrice;

    return sold;
}

int cargoSellAll(cargohold &hold, const market &m)
{
    int total = 0;

    // Walk backward so removing a stack does not skip the next one.
    for (int i = hold.stackCount - 1; i >= 0; --i)
    {
        int price = marketPriceFor(m, hold.stacks[i].name);

        if (price <= 0)
            continue;

        total += cargoSell(hold, hold.stacks[i].name,
                           hold.stacks[i].quantity, price);
    }

    return total;
}

int cargoProfit(const cargohold &hold)
{
    // Profit of what is still held: current-looking value is not known here,
    // so report the cost basis instead - the market screen shows the spread.
    int cost = 0;

    for (int i = 0; i < hold.stackCount && i < MAX_CARGO; ++i)
        cost += hold.stacks[i].buyPrice * hold.stacks[i].quantity;

    return cost;
}

/* ---------------- market ---------------- */

static const char *commodityNames[] = {
    "Food", "Water", "Ore", "Fuel Cells", "Medical Supplies",
    "Electronics", "Luxury Goods", "Weapons", "Nanites", "Artifacts"};

static int commodityCount()
{
    return (int)(sizeof(commodityNames) / sizeof(commodityNames[0]));
}

void marketGenerate(market &m, int systemIndex, int turn)
{
    memset(&m, 0, sizeof(m));

    if (systemIndex >= 0 && systemIndex < systemWorldCount)
    {
        copyString(m.systemName, sizeof(m.systemName),
                   systemWorldArray[systemIndex].name);
    }

    m.lastRefreshed = turn;

    int development = 50;

    if (systemIndex >= 0 && systemIndex < systemWorldCount)
        development = systemWorldArray[systemIndex].development;

    m.entryCount = commodityCount();

    if (m.entryCount > 12)
        m.entryCount = 12;

    for (int i = 0; i < m.entryCount; ++i)
    {
        marketentry &entry = m.entries[i];

        copyString(entry.name, sizeof(entry.name), commodityNames[i]);

        gpSeed((unsigned)(systemIndex * 104729 + i * 31 + turn));

        // Base price rises with tier; developed systems produce staples
        // cheaply and import luxuries expensively.
        int base = 20 + (i * 15);

        if (i <= 2 && development > 60)
            base = base / 2; // a producer
        if (i >= 6 && development < 40)
            base = base * 2; // a frontier importer

        entry.basePrice = base;
        entry.price = base + gpBetween(-5, 5);

        if (entry.price < 1)
            entry.price = 1;

        entry.stock = gpBetween(5, 40);
        entry.demand = gpBetween(20, 90);
    }
}

void marketRefresh(market &m, int turn)
{
    // Prices drift toward their base, then the turn nudges them.
    for (int i = 0; i < m.entryCount && i < 12; ++i)
    {
        marketentry &entry = m.entries[i];

        if (entry.price > entry.basePrice)
            entry.price--;
        else if (entry.price < entry.basePrice)
            entry.price++;

        int swing = gpBetween(-3, 3);
        entry.price += swing;

        if (entry.price < 1)
            entry.price = 1;

        entry.stock += gpBetween(0, 5);

        if (entry.stock > 100)
            entry.stock = 100;
    }

    m.lastRefreshed = turn;
}

int marketPriceFor(const market &m, const char *name)
{
    if (!name)
        return 0;

    for (int i = 0; i < m.entryCount && i < 12; ++i)
    {
        if (nameEquals(m.entries[i].name, name))
            return m.entries[i].price;
    }

    return 0;
}

int marketBuy(market &m, cargohold &hold, const char *name, int quantity)
{
    if (!name || quantity <= 0)
        return 0;

    for (int i = 0; i < m.entryCount && i < 12; ++i)
    {
        marketentry &entry = m.entries[i];

        if (!nameEquals(entry.name, name))
            continue;

        // Cannot buy more than the market holds, can afford, or can carry.
        if (quantity > entry.stock)
            quantity = entry.stock;

        int affordable = hold.credits / entry.price;

        if (quantity > affordable)
            quantity = affordable;

        if (quantity > cargoFree(hold))
            quantity = cargoFree(hold);

        if (quantity <= 0)
            return 0;

        hold.credits -= quantity * entry.price;
        entry.stock -= quantity;
        entry.demand += 2;

        if (entry.demand > 100)
            entry.demand = 100;

        // Bulk buying pushes the price up.
        entry.price += quantity / 5;

        cargoAdd(hold, entry.name, quantity, entry.price);

        return quantity;
    }

    return 0;
}

int marketSell(market &m, cargohold &hold, const char *name, int quantity)
{
    if (!name || quantity <= 0)
        return 0;

    int held = cargoCountOf(hold, name);

    if (quantity > held)
        quantity = held;

    if (quantity <= 0)
        return 0;

    for (int i = 0; i < m.entryCount && i < 12; ++i)
    {
        marketentry &entry = m.entries[i];

        if (!nameEquals(entry.name, name))
            continue;

        int sold = cargoSell(hold, entry.name, quantity, entry.price);

        entry.stock += sold;
        entry.demand -= 3;

        if (entry.demand < 0)
            entry.demand = 0;

        // Dumping stock pushes the price down.
        entry.price -= sold / 5;

        if (entry.price < 1)
            entry.price = 1;

        return sold;
    }

    return 0;
}

void marketShow(const market &m)
{
    drawHeader("Market");

    cout << "  " << m.systemName << endl
         << endl;

    for (int i = 0; i < m.entryCount && i < 12; ++i)
    {
        const marketentry &entry = m.entries[i];

        cout << "  " << entry.name;

        int length = (int)strlen(entry.name);
        for (int pad = length; pad < 18; ++pad)
            cout << ' ';

        cout << "  " << entry.price << " cr"
             << "  stock " << entry.stock
             << "  demand " << entry.demand << endl;
    }

    cout << endl;
    drawField("Credits", 0);
    cout << endl;
}

/* ====================================================================== */
/*  COLONIES                                                               */
/* ====================================================================== */

const char *colonyBuildingName(colonybuilding kind)
{
    switch (kind)
    {
    case BLD_MINE:
        return "Mine";
    case BLD_FARM:
        return "Farm";
    case BLD_FACTORY:
        return "Factory";
    case BLD_LAB:
        return "Research Lab";
    case BLD_SHIPYARD:
        return "Shipyard";
    case BLD_DEFENCE:
        return "Defence Grid";
    case BLD_HABITAT:
        return "Habitat";
    case BLD_TRADE_HUB:
        return "Trade Hub";
    default:
        return "None";
    }
}

int colonyCount()
{
    return g_profile.colonyCount;
}

colony *colonyAt(int index)
{
    if (index < 0 || index >= g_profile.colonyCount)
        return 0;

    return &g_profile.colonies[index];
}

int colonyCanSettle(int systemIndex, int worldIndex)
{
    if (systemIndex < 0 || systemIndex >= systemWorldCount)
        return 0;

    const systemworld &sys = systemWorldArray[systemIndex];

    if (worldIndex < 0 || worldIndex >= sys.worldCount || worldIndex >= 8)
        return 0;

    const world &w = sys.worlds[worldIndex];

    // Build the detail record to evaluate settleability properly.
    worlddetail detail;
    buildWorldDetail(detail, w, systemIndex * 31 + worldIndex);

    return worldIsSettleable(detail, w.habitability);
}

int colonyFound(int systemIndex, int worldIndex)
{
    if (g_profile.colonyCount >= MAX_COLONIES)
        return -1;

    if (!colonyCanSettle(systemIndex, worldIndex))
        return -2;

    const systemworld &sys = systemWorldArray[systemIndex];
    const world &w = sys.worlds[worldIndex];

    // Founding costs credits scaled by how hostile the world is.
    worlddetail detail;
    buildWorldDetail(detail, w, systemIndex * 31 + worldIndex);

    int cost = worldColoniseCost(detail);

    if (g_profile.cargo.credits < cost)
        return -3;

    g_profile.cargo.credits -= cost;

    colony &c = g_profile.colonies[g_profile.colonyCount];
    memset(&c, 0, sizeof(c));

    copyString(c.name, sizeof(c.name), w.name);
    c.systemIndex = systemIndex;
    c.worldIndex = worldIndex;
    c.population = 10;
    c.development = 5;
    c.morale = 70;
    c.food = 20;
    c.minerals = 10;
    c.energy = 10;
    c.foundedTurn = turnNumber();
    c.underSiege = 0;

    // A new colony starts with a habitat and a source of food.
    c.buildings[0] = BLD_HABITAT;
    c.buildingLevel[0] = 1;
    c.buildings[1] = BLD_FARM;
    c.buildingLevel[1] = 1;
    c.buildingCount = 2;

    g_profile.colonyCount++;

    turnLog("Founded a new colony.");

    return g_profile.colonyCount - 1;
}

int colonyBuild(int colonyIndex, colonybuilding kind)
{
    colony *c = colonyAt(colonyIndex);

    if (!c)
        return 0;

    if (c->buildingCount >= 8)
        return 0;

    if (kind <= BLD_NONE || kind >= BLD_COUNT)
        return 0;

    c->buildings[c->buildingCount] = kind;
    c->buildingLevel[c->buildingCount] = 1;
    c->buildingCount++;

    // Building costs minerals and energy.
    c->minerals -= 5;

    if (c->minerals < 0)
        c->minerals = 0;

    c->energy -= 5;

    if (c->energy < 0)
        c->energy = 0;

    return 1;
}

int colonyDemolish(int colonyIndex, int buildingIndex)
{
    colony *c = colonyAt(colonyIndex);

    if (!c)
        return 0;

    if (buildingIndex < 0 || buildingIndex >= c->buildingCount)
        return 0;

    for (int i = buildingIndex; i < c->buildingCount - 1; ++i)
    {
        c->buildings[i] = c->buildings[i + 1];
        c->buildingLevel[i] = c->buildingLevel[i + 1];
    }

    c->buildingCount--;

    return 1;
}

int colonyUpgrade(int colonyIndex, int buildingIndex)
{
    colony *c = colonyAt(colonyIndex);

    if (!c)
        return 0;

    if (buildingIndex < 0 || buildingIndex >= c->buildingCount)
        return 0;

    if (c->buildingLevel[buildingIndex] >= 5)
        return 0;

    int cost = c->buildingLevel[buildingIndex] * 10;

    if (c->minerals < cost)
        return 0;

    c->minerals -= cost;
    c->buildingLevel[buildingIndex]++;

    return 1;
}

void colonyTick(colony &c, int turn)
{
    (void)turn;

    int food = 0;
    int minerals = 0;
    int energy = 0;
    int research = 0;
    int housing = 0;
    int defence = 0;

    // Tally what the buildings produce.
    for (int i = 0; i < c.buildingCount && i < 8; ++i)
    {
        int level = c.buildingLevel[i];

        switch (c.buildings[i])
        {
        case BLD_FARM:
            food += 5 * level;
            break;
        case BLD_MINE:
            minerals += 5 * level;
            break;
        case BLD_FACTORY:
            minerals += 3 * level;
            energy -= 2 * level;
            break;
        case BLD_LAB:
            research += 5 * level;
            energy -= 2 * level;
            break;
        case BLD_SHIPYARD:
            minerals -= 3 * level;
            break;
        case BLD_DEFENCE:
            defence += 10 * level;
            energy -= 3 * level;
            break;
        case BLD_HABITAT:
            housing += 10 * level;
            break;
        case BLD_TRADE_HUB:
            g_profile.cargo.credits += 5 * level;
            break;
        default:
            break;
        }
    }

    // Population consumes food.
    int consumption = c.population / 5;
    food -= consumption;

    c.food += food;
    c.minerals += minerals;
    c.energy += energy;

    if (c.food < 0)
    {
        // Starvation costs morale and people.
        c.food = 0;
        c.morale -= 5;
        c.population -= 2;
    }

    if (c.minerals < 0)
        c.minerals = 0;

    if (c.energy < 0)
        c.energy = 0;

    // Growth: population rises toward what the habitats can hold.
    int capacity = 20 + housing;

    if (c.population < capacity && c.food > consumption)
    {
        c.population += 1 + (c.development / 25);
    }

    if (c.population > capacity)
        c.population = capacity;

    // Research labs feed the empire's science, and a defence grid both
    // reduces the danger penalty and contributes to the empire's defence.
    if (research > 0)
        empireAttributes.science += research / 5;

    if (empireAttributes.science > 100)
        empireAttributes.science = 100;

    // A colony in a dangerous system suffers, unless it is fortified.
    if (c.systemIndex >= 0 && c.systemIndex < systemWorldCount)
    {
        int danger = systemWorldArray[c.systemIndex].danger - (defence / 2);

        if (danger > 60)
        {
            c.morale -= 2;
            c.underSiege = (danger > 85) ? 1 : 0;
        }
    }

    // Morale drifts back toward the comfortable middle.
    if (c.morale < 50)
        c.morale += 2;
    else if (c.morale > 80)
        c.morale -= 1;

    if (c.development < 100 && c.population > 20)
        c.development++;

    if (c.morale < 0)
        c.morale = 0;
    if (c.morale > 100)
        c.morale = 100;

    if (c.population < 1 && c.food == 0)
        c.population = 1;   // never quite dies out
}

void coloniesTickAll(int turn)
{
    for (int i = 0; i < g_profile.colonyCount; ++i)
        colonyTick(g_profile.colonies[i], turn);
}

int colonyScore(const colony &c)
{
    return (c.population * 2) + c.development + (c.morale / 2)
         + (c.buildingCount * 20);
}

/* ====================================================================== */
/*  SKILLS                                                                 */
/* ====================================================================== */

struct skillseed
{
    const char *name;
    const char *description;
    skillkind kind;
    int power;
    int cost;
    int cooldown;
    int maxLevel;
};

static const skillseed skillSeeds[] = {
    {"Broadside", "Fires every weapon at one target.", SKILL_ATTACK, 250, 20, 4, 5},
    {"Overload", "Spends power for a burst of damage.", SKILL_ATTACK, 480, 45, 8, 3},
    {"Point Barrage", "Light fire on all enemies.", SKILL_ATTACK, 140, 30, 5, 5},
    {"Emergency Repair", "Restores hull in combat.", SKILL_HEAL, 200, 25, 6, 5},
    {"Shield Recharge", "Restores shields quickly.", SKILL_HEAL, 300, 35, 7, 4},
    {"Hardened Plating", "Raises armour for a while.", SKILL_BUFF, 30, 20, 6, 4},
    {"Targeting Uplink", "Raises accuracy and crit.", SKILL_BUFF, 25, 25, 6, 4},
    {"Ion Pulse", "Drains enemy shields.", SKILL_DEBUFF, 200, 30, 5, 5},
    {"Graviton Snare", "Roots a target in place.", SKILL_DEBUFF, 1, 35, 8, 3},
    {"Sensor Sweep", "Reveals the whole system.", SKILL_UTILITY, 1, 10, 3, 5},
    {"Emergency Jump", "Escape combat instantly.", SKILL_UTILITY, 1, 50, 10, 3},
    {"Salvage Drone", "Recovers extra loot.", SKILL_UTILITY, 20, 15, 5, 5},
    {"Hardened Crew", "Passive: reduces crew losses.", SKILL_PASSIVE, 10, 0, 0, 5},
    {"Ace Pilot", "Passive: raises evasion.", SKILL_PASSIVE, 8, 0, 0, 5}};

static const int skillSeedCount =
    (int)(sizeof(skillSeeds) / sizeof(skillSeeds[0]));

void skillsInit(playerprofile &profile)
{
    profile.skillCount = 0;

    for (int i = 0; i < skillSeedCount && i < MAX_SKILLS; ++i)
    {
        skill &s = profile.skills[i];

        memset(&s, 0, sizeof(s));

        copyString(s.name, sizeof(s.name), skillSeeds[i].name);
        copyString(s.description, sizeof(s.description),
                   skillSeeds[i].description);

        s.kind = skillSeeds[i].kind;
        s.power = skillSeeds[i].power;
        s.cost = skillSeeds[i].cost;
        s.cooldown = skillSeeds[i].cooldown;
        s.remaining = 0;
        s.level = 0;
        s.maxLevel = skillSeeds[i].maxLevel;
        s.unlocked = 0;

        profile.skillCount++;
    }

    // The player starts with the two basics.
    skillUnlock(profile, "Broadside");
    skillUnlock(profile, "Emergency Repair");
}

skill *skillFind(playerprofile &profile, const char *name)
{
    if (!name)
        return 0;

    for (int i = 0; i < profile.skillCount; ++i)
    {
        if (nameEquals(profile.skills[i].name, name))
            return &profile.skills[i];
    }

    return 0;
}

int skillUnlock(playerprofile &profile, const char *name)
{
    skill *s = skillFind(profile, name);

    if (!s || s->unlocked)
        return 0;

    s->unlocked = 1;
    s->level = 1;

    return 1;
}

int skillUpgrade(playerprofile &profile, const char *name)
{
    skill *s = skillFind(profile, name);

    if (!s || !s->unlocked)
        return 0;

    if (s->level >= s->maxLevel)
        return 0;

    int cost = s->level * 100;

    if (profile.cargo.credits < cost)
        return 0;

    profile.cargo.credits -= cost;
    s->level++;

    return 1;
}

int skillUse(playerprofile &profile, const char *name)
{
    skill *s = skillFind(profile, name);

    if (!s || !s->unlocked)
        return 0;

    if (s->remaining > 0)
        return 0;   // still on cooldown

    // Spend the resource cost from the ship's power.
    if (charactership.power < s->cost)
        return 0;

    charactership.power -= s->cost;

    // Scale the effect by skill rank.
    int power = s->power * s->level;

    if (s->kind == SKILL_ATTACK)
    {
        targetScore.damage_done += power;
        targetScore.score += power / 2;
    }
    else if (s->kind == SKILL_HEAL)
    {
        charactership.health += power;

        if (charactership.health > 100)
            charactership.health = 100;
    }

    s->remaining = s->cooldown;

    return power;
}

void skillsTick(playerprofile &profile)
{
    for (int i = 0; i < profile.skillCount; ++i)
    {
        if (profile.skills[i].remaining > 0)
            profile.skills[i].remaining--;
    }

    // Power regenerates each turn.
    charactership.power += 10;

    if (charactership.power > 100)
        charactership.power = 100;
}

void skillShow(const skill &s)
{
    cout << "  " << s.name;

    int length = (int)strlen(s.name);
    for (int pad = length; pad < 18; ++pad)
        cout << ' ';

    cout << "  rank " << s.level << "/" << s.maxLevel;

    if (s.remaining > 0)
        cout << "  (cooldown " << s.remaining << ")";
    else if (!s.unlocked)
        cout << "  (locked)";

    cout << endl;

    if (s.description[0])
        cout << "      " << s.description << endl;
}

void skillsShowAll(const playerprofile &profile)
{
    drawHeader("Skills");

    for (int i = 0; i < profile.skillCount; ++i)
        skillShow(profile.skills[i]);

    cout << endl;
}

/* ====================================================================== */
/*  REPUTATION                                                             */
/* ====================================================================== */

const char *standingTierName(int tier)
{
    switch (tier)
    {
    case 0:
        return "Hostile";
    case 1:
        return "Neutral";
    case 2:
        return "Friendly";
    case 3:
        return "Honored";
    case 4:
        return "Revered";
    case 5:
        return "Exalted";
    default:
        return "Unknown";
    }
}

void standingsInit(playerprofile &profile)
{
    profile.standingCount = 0;

    for (int i = 0; i < factionCount && i < MAX_FACTIONS; ++i)
    {
        repstanding &standing = profile.standings[i];

        standing.factionIndex = i;
        standing.value = 0;
        standing.tier = 1;

        profile.standingCount++;
    }
}

int standingValue(const playerprofile &profile, int factionIndex)
{
    for (int i = 0; i < profile.standingCount; ++i)
    {
        if (profile.standings[i].factionIndex == factionIndex)
            return profile.standings[i].value;
    }

    return 0;
}

int standingTier(const playerprofile &profile, int factionIndex)
{
    int value = standingValue(profile, factionIndex);

    if (value <= -3000)
        return 0;
    if (value < 0)
        return 1;
    if (value >= 5000)
        return 5;
    if (value >= 2000)
        return 4;
    if (value >= 800)
        return 3;
    if (value >= 300)
        return 2;

    return 1;
}

int standingAdjust(playerprofile &profile, int factionIndex, int delta)
{
    for (int i = 0; i < profile.standingCount; ++i)
    {
        repstanding &standing = profile.standings[i];

        if (standing.factionIndex != factionIndex)
            continue;

        standing.value += delta;

        if (standing.value > 10000)
            standing.value = 10000;
        if (standing.value < -10000)
            standing.value = -10000;

        int oldTier = standing.tier;
        standing.tier = standingTier(profile, factionIndex);

        if (standing.tier != oldTier)
            turnLog("Reputation with a faction changed tier.");

        return standing.value;
    }

    return 0;
}

void standingsShow(const playerprofile &profile)
{
    drawHeader("Reputation");

    for (int i = 0; i < profile.standingCount; ++i)
    {
        const repstanding &standing = profile.standings[i];

        const char *name = "Unknown";

        if (standing.factionIndex >= 0
            && standing.factionIndex < factionCount)
            name = factionArray[standing.factionIndex].name;

        cout << "  " << name;

        int length = (int)strlen(name);
        for (int pad = length; pad < 22; ++pad)
            cout << ' ';

        cout << standingTierName(standing.tier)
             << "  (" << standing.value << ")" << endl;
    }

    cout << endl;
}

/* ====================================================================== */
/*  CONTRACTS                                                              */
/* ====================================================================== */

const char *contractKindName(contractkind kind)
{
    switch (kind)
    {
    case CONTRACT_KILL:
        return "Kill";
    case CONTRACT_DELIVER:
        return "Deliver";
    case CONTRACT_SURVEY:
        return "Survey";
    case CONTRACT_ESCORT:
        return "Escort";
    case CONTRACT_BOUNTY:
        return "Bounty";
    default:
        return "Unknown";
    }
}

void contractsInit(playerprofile &profile)
{
    profile.contractCount = 0;
    memset(profile.contracts, 0, sizeof(profile.contracts));
}

static const char *contractTitles[] = {
    "Clear the Lane", "Escort Duty", "Silent Survey", "Wanted: Raider",
    "Supply Run", "Border Patrol", "Deep Scan", "Bounty: Deserter"};

int contractOffer(playerprofile &profile, int giverFaction, int turn)
{
    if (profile.contractCount >= MAX_CONTRACTS)
        return -1;

    contract &c = profile.contracts[profile.contractCount];
    memset(&c, 0, sizeof(c));

    gpSeed((unsigned)(turn * 7919 + giverFaction * 131 + profile.contractCount));

    int pick = gpRand(8);

    copyString(c.title, sizeof(c.title), contractTitles[pick]);

    c.kind = (contractkind)(pick % CONTRACT_KIND_COUNT);
    c.giverFaction = giverFaction;
    c.targetSystem = gpRand(systemWorldCount > 0 ? systemWorldCount : 1);
    c.accepted = 0;
    c.complete = 0;
    c.progress = 0;

    // Terms scale with the kind of work.
    switch (c.kind)
    {
    case CONTRACT_KILL:
        c.required = gpBetween(2, 6);
        c.rewardCredits = c.required * 150;
        c.rewardRep = c.required * 30;
        copyString(c.description, sizeof(c.description),
                   "Destroy hostile ships in the target system.");
        break;

    case CONTRACT_DELIVER:
        c.required = gpBetween(5, 20);
        c.rewardCredits = c.required * 60;
        c.rewardRep = c.required * 15;
        copyString(c.description, sizeof(c.description),
                   "Carry cargo to the target system.");
        break;

    case CONTRACT_SURVEY:
        c.required = 1;
        c.rewardCredits = gpBetween(300, 900);
        c.rewardRep = 80;
        copyString(c.description, sizeof(c.description),
                   "Scan the target system and report.");
        break;

    case CONTRACT_ESCORT:
        c.required = gpBetween(1, 3);
        c.rewardCredits = c.required * 400;
        c.rewardRep = c.required * 60;
        copyString(c.description, sizeof(c.description),
                   "Protect a convoy through the target system.");
        break;

    case CONTRACT_BOUNTY:
        c.required = 1;
        c.rewardCredits = gpBetween(800, 2500);
        c.rewardRep = 150;
        copyString(c.description, sizeof(c.description),
                   "Hunt down a named raider.");
        break;

    default:
        break;
    }

    // Contracts expire, so they stay urgent.
    c.expiresTurn = turn + gpBetween(10, 30);

    profile.contractCount++;

    return profile.contractCount - 1;
}

int contractAccept(playerprofile &profile, int index)
{
    if (index < 0 || index >= profile.contractCount)
        return 0;

    profile.contracts[index].accepted = 1;

    return 1;
}

int contractProgress(playerprofile &profile, int index, int amount)
{
    if (index < 0 || index >= profile.contractCount)
        return 0;

    contract &c = profile.contracts[index];

    if (!c.accepted || c.complete)
        return 0;

    c.progress += amount;

    if (c.progress >= c.required)
    {
        c.progress = c.required;
        c.complete = 1;
    }

    return c.complete;
}

int contractComplete(playerprofile &profile, int index)
{
    if (index < 0 || index >= profile.contractCount)
        return 0;

    contract &c = profile.contracts[index];

    if (!c.complete)
        return 0;

    profile.cargo.credits += c.rewardCredits;
    standingAdjust(profile, c.giverFaction, c.rewardRep);

    targetScore.score += c.rewardCredits / 10;

    turnLog("Completed a contract.");

    // Remove it from the book.
    for (int i = index; i < profile.contractCount - 1; ++i)
        profile.contracts[i] = profile.contracts[i + 1];

    profile.contractCount--;

    return 1;
}

int contractAbandon(playerprofile &profile, int index)
{
    if (index < 0 || index >= profile.contractCount)
        return 0;

    contract &c = profile.contracts[index];

    // Abandoning costs standing with the giver.
    standingAdjust(profile, c.giverFaction, -100);

    for (int i = index; i < profile.contractCount - 1; ++i)
        profile.contracts[i] = profile.contracts[i + 1];

    profile.contractCount--;

    return 1;
}

int contractsTick(playerprofile &profile, int turn)
{
    int expired = 0;

    // Walk backward so removal does not skip an entry.
    for (int i = profile.contractCount - 1; i >= 0; --i)
    {
        contract &c = profile.contracts[i];

        if (c.expiresTurn <= 0 || turn < c.expiresTurn)
            continue;

        if (c.complete)
            continue;

        // An expired accepted contract still costs standing.
        if (c.accepted)
            standingAdjust(profile, c.giverFaction, -50);

        for (int j = i; j < profile.contractCount - 1; ++j)
            profile.contracts[j] = profile.contracts[j + 1];

        profile.contractCount--;
        expired++;
    }

    return expired;
}

void contractsShow(const playerprofile &profile)
{
    drawHeader("Contracts");

    if (profile.contractCount == 0)
    {
        out("(none)");
        cout << endl;
        return;
    }

    for (int i = 0; i < profile.contractCount; ++i)
    {
        const contract &c = profile.contracts[i];

        cout << "  " << (i + 1) << ". " << c.title
             << "  [" << contractKindName(c.kind) << "]" << endl;

        cout << "      " << c.description << endl;
        cout << "      progress " << c.progress << "/" << c.required
             << "  reward " << c.rewardCredits << " cr"
             << "  rep " << c.rewardRep;

        if (c.accepted)
            cout << "  (accepted)";

        if (c.expiresTurn > 0)
            cout << "  expires turn " << c.expiresTurn;

        if (c.complete)
            cout << "  COMPLETE";

        cout << endl;
    }

    cout << endl;
}

/* ====================================================================== */
/*  MATERIALS AND CRAFTING                                                 */
/* ====================================================================== */

void materialsInit(playerprofile &profile)
{
    profile.materialCount = 0;

    // A little salvage to start with.
    materialAdd(profile, "Scrap", 10, 1);
    materialAdd(profile, "Alloy", 4, 2);
}

int materialCountOf(const playerprofile &profile, const char *name)
{
    if (!name)
        return 0;

    for (int i = 0; i < profile.materialCount && i < MAX_MATERIALS; ++i)
    {
        if (nameEquals(profile.materials[i].name, name))
            return profile.materials[i].quantity;
    }

    return 0;
}

int materialAdd(playerprofile &profile, const char *name, int quantity,
                int tier)
{
    if (!name || quantity <= 0)
        return 0;

    for (int i = 0; i < profile.materialCount && i < MAX_MATERIALS; ++i)
    {
        if (!nameEquals(profile.materials[i].name, name))
            continue;

        profile.materials[i].quantity += quantity;
        return quantity;
    }

    if (profile.materialCount >= MAX_MATERIALS)
        return 0;

    material &m = profile.materials[profile.materialCount];

    memset(&m, 0, sizeof(m));
    copyString(m.name, sizeof(m.name), name);
    m.quantity = quantity;
    m.tier = tier;

    profile.materialCount++;

    return quantity;
}

// name, inputs (name/index pairs collapsed to names), output item, turns
struct recipeseed
{
    const char *name;
    const char *in1;
    int in1Count;
    const char *in2;
    int in2Count;
    const char *outputItem;
    int turns;
};

static const recipeseed recipeSeeds[] = {
    {"Reinforced Alloy", "Alloy", 3, "Scrap", 5, "Ablative Plate I", 1},
    {"Shield Emitter", "Circuit", 3, "Alloy", 2, "Deflector I", 2},
    {"Pulse Laser Assembly", "Circuit", 4, "Alloy", 3, "Pulse Laser Turret", 2},
    {"Targeting Suite", "Circuit", 5, "Scrap", 8, "Targeting Computer I", 2},
    {"Repair Drone", "Nanites", 2, "Alloy", 4, "Repair Drone Bay I", 3},
    {"Warp Coil", "Quantum Cores", 1, "Circuit", 6, "Warp Drive III", 4}};

static const int recipeSeedCount =
    (int)(sizeof(recipeSeeds) / sizeof(recipeSeeds[0]));

// Recipes are materialised on demand from the seeds, since the output item
// index has to be resolved after the item table is populated.
static recipe g_recipes[MAX_MATERIALS];
static int g_recipeCount = 0;

int recipeCount()
{
    if (g_recipeCount > 0)
        return g_recipeCount;

    // Materialise once.
    for (int i = 0; i < recipeSeedCount && i < MAX_MATERIALS; ++i)
    {
        recipe &r = g_recipes[i];
        memset(&r, 0, sizeof(r));

        copyString(r.name, sizeof(r.name), recipeSeeds[i].name);

        // Inputs are stored as material-tier placeholders; the name lookup
        // happens in canCraft()/craftItem() against the player's materials.
        r.inputKinds = 2;
        r.inputMaterial[0] = i;   // index into recipeSeeds, name read back
        r.inputCount[0] = recipeSeeds[i].in1Count;
        r.inputMaterial[1] = i;
        r.inputCount[1] = recipeSeeds[i].in2Count;

        r.outputItem = findItem(recipeSeeds[i].outputItem);
        r.requiresTurn = recipeSeeds[i].turns;

        g_recipeCount++;
    }

    return g_recipeCount;
}

const recipe *recipeAt(int index)
{
    if (index < 0 || index >= recipeCount())
        return 0;

    return &g_recipes[index];
}

// Read a recipe's input material name back out of the seed table.
static const char *recipeInputName(int recipeIndex, int slot)
{
    if (recipeIndex < 0 || recipeIndex >= recipeSeedCount)
        return "";

    if (slot == 0)
        return recipeSeeds[recipeIndex].in1;

    return recipeSeeds[recipeIndex].in2;
}

static int recipeInputCount(int recipeIndex, int slot)
{
    if (recipeIndex < 0 || recipeIndex >= recipeSeedCount)
        return 0;

    if (slot == 0)
        return recipeSeeds[recipeIndex].in1Count;

    return recipeSeeds[recipeIndex].in2Count;
}

int canCraft(const playerprofile &profile, int recipeIndex)
{
    if (recipeIndex < 0 || recipeIndex >= recipeCount())
        return 0;

    for (int slot = 0; slot < 2; ++slot)
    {
        const char *name = recipeInputName(recipeIndex, slot);

        if (!name || name[0] == '\0')
            continue;

        if (materialCountOf(profile, name) < recipeInputCount(recipeIndex, slot))
            return 0;
    }

    return 1;
}

int craftItem(playerprofile &profile, int recipeIndex)
{
    if (!canCraft(profile, recipeIndex))
        return 0;

    // Consume the inputs.
    for (int slot = 0; slot < 2; ++slot)
    {
        const char *name = recipeInputName(recipeIndex, slot);
        int needed = recipeInputCount(recipeIndex, slot);

        if (!name || name[0] == '\0' || needed <= 0)
            continue;

        for (int i = 0; i < profile.materialCount; ++i)
        {
            if (!nameEquals(profile.materials[i].name, name))
                continue;

            profile.materials[i].quantity -= needed;

            if (profile.materials[i].quantity < 0)
                profile.materials[i].quantity = 0;

            break;
        }
    }

    turnLog("Crafted a new component.");

    targetScore.score += 25;

    return 1;
}

void materialsShow(const playerprofile &profile)
{
    drawHeader("Materials");

    if (profile.materialCount == 0)
    {
        out("(none)");
        cout << endl;
        return;
    }

    for (int i = 0; i < profile.materialCount && i < MAX_MATERIALS; ++i)
    {
        const material &m = profile.materials[i];

        if (m.quantity <= 0)
            continue;

        cout << "  " << m.name;

        int length = (int)strlen(m.name);
        for (int pad = length; pad < 18; ++pad)
            cout << ' ';

        cout << "  x" << m.quantity
             << "  (tier " << m.tier << ")" << endl;
    }

    cout << endl;
}

void recipesShow()
{
    drawHeader("Recipes");

    for (int i = 0; i < recipeCount(); ++i)
    {
        const recipe *r = recipeAt(i);

        if (!r)
            continue;

        cout << "  " << (i + 1) << ". " << r->name << endl;
        cout << "      " << recipeInputName(i, 0)
             << " x" << recipeInputCount(i, 0)
             << "  +  " << recipeInputName(i, 1)
             << " x" << recipeInputCount(i, 1) << endl;

        cout << "      -> ";

        if (r->outputItem >= 0 && r->outputItem < itemCount)
            cout << itemArray[r->outputItem].name;
        else
            cout << "(no output item)";

        cout << "  (" << r->requiresTurn << " turn)";
        cout << endl;
    }

    cout << endl;
}

/* ====================================================================== */
/*  CODEX                                                                  */
/* ====================================================================== */

static const char *codexCategories[] = {"Lore", "Bestiary", "Technology",
                                        "History"};

void codexInit(playerprofile &profile)
{
    profile.codexCount = 0;
    memset(profile.codex, 0, sizeof(profile.codex));
}

int codexDiscover(playerprofile &profile, const char *title)
{
    if (!title || title[0] == '\0')
        return 0;

    if (codexHas(profile, title))
        return 0;

    if (profile.codexCount >= MAX_CODEX)
        return 0;

    codexentry &entry = profile.codex[profile.codexCount];

    memset(&entry, 0, sizeof(entry));
    copyString(entry.title, sizeof(entry.title), title);
    copyString(entry.body, sizeof(entry.body),
               "Recovered from a survey scan.");

    // Category from the first letters, so entries group sensibly.
    entry.category = 1;   // bestiary by default - most discoveries are places
    entry.discovered = 1;

    profile.codexCount++;

    targetScore.score += 10;

    return 1;
}

int codexHas(const playerprofile &profile, const char *title)
{
    if (!title)
        return 0;

    for (int i = 0; i < profile.codexCount; ++i)
    {
        if (nameEquals(profile.codex[i].title, title))
            return 1;
    }

    return 0;
}

void codexShow(const playerprofile &profile, int category)
{
    drawHeader("Codex");

    int shown = 0;

    for (int i = 0; i < profile.codexCount; ++i)
    {
        const codexentry &entry = profile.codex[i];

        if (category >= 0 && entry.category != category)
            continue;

        const char *categoryName = "Unknown";

        if (entry.category >= 0 && entry.category < 4)
            categoryName = codexCategories[entry.category];

        cout << "  " << (i + 1) << ". " << entry.title
             << "  [" << categoryName << "]" << endl;

        cout << "      " << entry.body << endl;

        shown++;
    }

    if (shown == 0)
        out("(nothing discovered yet in this category)");

    cout << endl;
}

/* ====================================================================== */
/*  AGGREGATE                                                              */
/* ====================================================================== */

void gameplayTick(int turn)
{
    coloniesTickAll(turn);
    skillsTick(g_profile);
    contractsTick(g_profile, turn);

    // Consumables and salvage trickle in from the fleet being in the field.
    if (targetScore.kills > 0)
    {
        materialAdd(g_profile, "Scrap", 1, 1);

        if ((turn % 3) == 0)
            materialAdd(g_profile, "Alloy", 1, 2);
    }

    // Fuel slowly regenerates while docked at a station.
    if (g_profile.fuel < g_profile.maxFuel && (turn % 5) == 0)
        g_profile.fuel++;
}

int gameplayScore(const playerprofile &profile)
{
    int score = 0;

    // Exploration.
    for (int i = 0; i < systemWorldCount; ++i)
    {
        if (systemVisited(i))
            score += 50;
    }

    // Colonies.
    for (int i = 0; i < profile.colonyCount; ++i)
        score += colonyScore(profile.colonies[i]);

    // Trade and contracts.
    score += profile.cargo.credits / 10;
    score += profile.jumpCount * 5;

    // Codex.
    score += profile.codexCount * 10;

    // Reputation.
    for (int i = 0; i < profile.standingCount; ++i)
        score += profile.standings[i].value / 100;

    return score;
}

void gameplaySummarise(char out[], int outSize)
{
    if (!out || outSize <= 0)
        return;

    snprintf(out, outSize,
             "cr %d  fuel %d/%d  colonies %d  jumps %d  codex %d",
             g_profile.cargo.credits,
             g_profile.fuel, g_profile.maxFuel,
             g_profile.colonyCount,
             g_profile.jumpCount,
             g_profile.codexCount);
}
