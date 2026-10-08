// src/game/galaxy.c++ -- the 4X layer: systems, planets, fleets, trade and
// diplomacy.  See docs/FRAMEWORK.md s.22.2 - s.22.10.

#include "spacebattlerpg.h"

/* ---------------- systems and planets ---------------- */

void clearStarSystem(starsystem &sys)
{
    memset(&sys, 0, sizeof(sys));

    sys.ownerFaction = -1;
}

void rollPlanet(planet &p, const char *name, int systemTier)
{
    memset(&p, 0, sizeof(p));

    if (name)
    {
        strncpy(p.name, name, sizeof(p.name) - 1);
        p.name[sizeof(p.name) - 1] = '\0';
    }

    // A planet's rolls scale with how deep in the galaxy it sits.
    p.size = 1 + randomNumber(10);
    p.habitability = 1 + randomNumber(10);
    p.resources = systemTier + randomNumber(10);
    p.defense = systemTier + randomNumber(5);
    p.stability = 50 + randomNumber(50);
    p.buildingCount = 0;
}

void initializeGalaxy()
{
    static const char *systemNames[MAX_SYSTEMS] = {
        "Aaamazzara", "Altair IV", "Aurelia", "Bajor", "Benthos",
        "Borg Prime", "Cait", "Cardassia Prime", "Cygnia Minor", "Daran V",
        "Duronom", "Dytallix B", "Efros", "El-Adrel IV", "Epsilon Caneris III",
        "Ferenginar", "Finnea Prime"};

    static const char *planetSuffixes[3] = {
        "Prime", "Minor", "Reach"};

    systemCount = 0;

    for (int i = 0; i < MAX_SYSTEMS; ++i)
    {
        starsystem &sys = systemArray[i];
        clearStarSystem(sys);

        strncpy(sys.name, systemNames[i], sizeof(sys.name) - 1);
        sys.name[sizeof(sys.name) - 1] = '\0';

        sys.sectorIndex = i / 4;
        sys.planetCount = 1 + randomNumber(3);

        if (sys.planetCount > 6)
            sys.planetCount = 6;

        for (int p = 0; p < sys.planetCount; ++p)
        {
            char planetName[30];
            snprintf(planetName, sizeof(planetName), "%s %s",
                     systemNames[i], planetSuffixes[p % 3]);

            rollPlanet(sys.planets[p], planetName, i / 3);
        }

        sys.hasAsteroidField = (randomNumber(100) <= 40) ? 1 : 0;
        sys.hasStation = (randomNumber(100) <= 60) ? 1 : 0;
        sys.hasAnomaly = (randomNumber(100) <= 20) ? 1 : 0;
        sys.ownerFaction = randomNumber(factionCount) - 1;

        systemCount++;
    }
}

/* ---------------- factions ---------------- */

int findFaction(const char *name)
{
    return findFactionByName(name);
}

/* ---------------- diplomacy and trade ---------------- */

int diplomaticScore(const faction &who, int reputation, int tradeValue,
                    int sharedEnemies)
{
    // DiplomaticScore = Influence + Reputation + TradeValue + SharedEnemies
    return who.attributes.influence + reputation + tradeValue + sharedEnemies;
}

int tradeIncome(int tradeValue, int routeSecurity, int distanceModifier)
{
    if (tradeValue <= 0)
        return 0;

    // TradeIncome = TradeValue * RouteSecurity% * DistanceModifier%
    int income = (tradeValue * routeSecurity) / 100;
    income = (income * distanceModifier) / 100;

    return income;
}

/* ---------------- planet output ---------------- */

int planetCreditOutput(const planet &p)
{
    return p.resources * 2 + p.stability / 10;
}

int planetMineralOutput(const planet &p)
{
    return p.resources * 3;
}

int planetResearchOutput(const planet &p)
{
    return p.size + p.habitability / 2 + p.buildingCount;
}

void addStockpile(stockpile &into, const stockpile &amount)
{
    into.credits += amount.credits;
    into.minerals += amount.minerals;
    into.gas += amount.gas;
    into.energy += amount.energy;
    into.data += amount.data;
    into.darkmatter += amount.darkmatter;
    into.antimatter += amount.antimatter;
    into.nanites += amount.nanites;
    into.quantumcores += amount.quantumcores;
    into.alienartifacts += amount.alienartifacts;
}

void tickEmpireEconomy()
{
    stockpile income;
    memset(&income, 0, sizeof(income));

    for (int i = 0; i < systemCount; ++i)
    {
        const starsystem &sys = systemArray[i];

        if (sys.ownerFaction != playerFactionIndex())
            continue;

        for (int p = 0; p < sys.planetCount && p < 6; ++p)
        {
            income.credits += planetCreditOutput(sys.planets[p]);
            income.minerals += planetMineralOutput(sys.planets[p]);
            income.data += planetResearchOutput(sys.planets[p]);
        }

        if (sys.hasStation)
            income.gas += 5 + randomNumber(10);
    }

    addStockpile(empireStorage, income);
}

int empirePriority(const faction &ai, int threatLevel, int resourceNeed,
                   int expansionOpportunity, int diplomaticRisk)
{
    (void)ai;

    // Priority = ThreatLevel + ResourceNeed + ExpansionOpportunity + Risk
    return threatLevel + resourceNeed + expansionOpportunity + diplomaticRisk;
}

/* ---------------- fleets ---------------- */

void clearFleet(fleet &f)
{
    memset(&f, 0, sizeof(f));

    f.commandCapacity = 10;
    f.morale = 100;
    f.sensorRange = 5;
}

void recalcFleet(fleet &f)
{
    f.supplyUse = fleetSupplyUse(f);
}

int fleetSupplyUse(const fleet &f)
{
    int supply = 0;

    for (int i = 0; i < f.shipCount && i < MAX_SHIPS; ++i)
        supply += f.ships[i].supplyUse;

    return supply;
}

int fleetWithinCommandCapacity(const fleet &f)
{
    return (fleetSupplyUse(f) <= f.commandCapacity) ? 1 : 0;
}
