// src/library/scaling.c++ -- level, party, difficulty and PvP scaling.
// See docs/FRAMEWORK.md section 10 and 22.

#include "spacebattlerpg.h"

int partyMultiplier(int players)
{
    if (players < 1)
        players = 1;

    // PartyMult = 1 + (Players - 1) * 0.65, returned x100.
    return 100 + ((players - 1) * 65);
}

int scaleBossHP(int baseHP, int levelDiff, enemytier tier, int players)
{
    if (baseHP <= 0)
        return 0;

    // (1 + 0.18 * LevelDiff), x100.
    int levelFactor = 100 + (18 * levelDiff);
    if (levelFactor < 10)
        levelFactor = 10;

    int tierFactor = tierMultiplierFor(tier);
    int partyFactor = partyMultiplier(players);

    long long hp = (long long)baseHP * levelFactor / 100;
    hp = hp * tierFactor / 100;
    hp = hp * partyFactor / 100;

    return (int)hp;
}

int scaleBossDamage(int baseDamage, int levelDiff, enemytier tier, int players)
{
    if (baseDamage <= 0)
        return 0;

    // (1 + 0.12 * LevelDiff), x100.  Damage deliberately climbs slower than
    // HP so that scaling never produces one-shots.
    int levelFactor = 100 + (12 * levelDiff);
    if (levelFactor < 10)
        levelFactor = 10;

    int tierFactor = tierMultiplierFor(tier);

    // PartyMult^0.6, approximated in integer maths as a damping curve.
    int partyFactor = partyMultiplier(players);
    partyFactor = 100 + ((partyFactor - 100) * 6) / 10;

    long long damage = (long long)baseDamage * levelFactor / 100;
    damage = damage * tierFactor / 100;
    damage = damage * partyFactor / 100;

    return (int)damage;
}

int encounterPowerBudget(int totalPlayerPower, int difficultyFactorPercent)
{
    if (totalPlayerPower <= 0)
        return 0;

    return (totalPlayerPower * difficultyFactorPercent) / 100;
}

int difficultyFactor(const difficultmode &mode)
{
    // Difficulty is modelled as a percentage ramp above the first tier.
    if (mode.level <= 0)
        return 100;

    return 100 + (mode.level * 15);
}

int normalizeForPvP(int value, int pvpCap, int pvpPercent)
{
    int normalized = (value * pvpPercent) / 100;

    if (pvpCap > 0 && normalized > pvpCap)
        normalized = pvpCap;

    if (normalized < 0)
        normalized = 0;

    return normalized;
}

int totalPower(int characterPower, int fleetPower, int empirePower,
               int technologyPower, int economicPower)
{
    // TotalPower = Character + Fleet + Empire + Technology + Economic
    return characterPower + fleetPower + empirePower + technologyPower + economicPower;
}

int fleetPower(const fleet &f)
{
    int total = 0;

    for (int i = 0; i < f.shipCount && i < MAX_SHIPS; ++i)
    {
        const shipinstance &ship = f.ships[i];

        // FleetPower line = ShipCombatValue * CrewSkill% * TechMultiplier%
        int combatValue = ship.firepower + (ship.hull / 2) + (ship.shields / 2);
        int value = (combatValue * ship.crewSkill) / 100;
        value = (value * ship.techLevel) / 100;

        total += value;
    }

    return total;
}

int applyTierMultiplier(int base, enemytier tier)
{
    if (base <= 0)
        return 0;

    return (base * tierMultiplierFor(tier)) / 100;
}
