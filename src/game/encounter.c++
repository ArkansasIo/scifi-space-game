// src/game/encounter.c++ -- encounter generation from templates.
// See docs/FRAMEWORK.md s.7, s.8 and s.13.

#include "spacebattlerpg.h"

void clearEncounter(encounterinstance &enc)
{
    memset(&enc, 0, sizeof(enc));

    enc.bossIndex = -1;
    enc.kind = ENC_PACK;
}

// Seed one unit in the instance, scaled to the party and the tier.
static void seedUnit(encounterinstance &enc, int archetypeIndex,
                     enemytier tier, int playerLevel)
{
    if (enc.unitCount >= MAX_ARCHETYPES)
        return;

    encounterunit &unit = enc.units[enc.unitCount];

    unit.archetypeIndex = archetypeIndex;
    unit.tier = (int)tier;

    if (archetypeIndex >= 0 && archetypeIndex < archetypeCount)
    {
        unit.role = (int)archetypeArray[archetypeIndex].role;

        // Scale HP and damage from the archetype's tier multipliers.
        int baseHP = 100 + playerLevel * 10;
        int baseDamage = 10 + playerLevel;

        unit.scaledHP = (baseHP * archetypeHPMultiplier(archetypeIndex)) / 100;
        unit.scaledDamage =
            (baseDamage * archetypeDamageMultiplier(archetypeIndex)) / 100;
    }
    else
    {
        unit.role = ROLE_BRUISER;
        unit.scaledHP = 100 + playerLevel * 10;
        unit.scaledDamage = 10 + playerLevel;
    }

    unit.affixIndex = -1;
    unit.spawnSlot = enc.unitCount;

    enc.unitCount++;
}

// Fill an instance from a template.
static void fillFromTemplate(encounterinstance &enc,
                             const encountertemplate &tmpl,
                             int playerLevel, enemytier tier)
{
    strncpy(enc.name, tmpl.name, sizeof(enc.name) - 1);
    enc.name[sizeof(enc.name) - 1] = '\0';

    enc.kind = tmpl.kind;
    enc.waveCount = tmpl.waveCount > 0 ? tmpl.waveCount : 1;
    enc.timed = tmpl.timed;
    enc.bossIndex = tmpl.bossIndex;

    // Trash.
    for (int i = 0; i < tmpl.trashCount; ++i)
    {
        int archetype = randomArchetypeForTier(TIER_TRASH);
        seedUnit(enc, archetype, TIER_TRASH, playerLevel);
    }

    // Casters.
    for (int i = 0; i < tmpl.casterCount; ++i)
    {
        int archetype = findArchetype(ROLE_CASTER, TIER_VETERAN);

        if (archetype < 0)
            archetype = randomArchetypeForTier(TIER_VETERAN);

        seedUnit(enc, archetype, TIER_VETERAN, playerLevel);
    }

    // Minibosses.
    for (int i = 0; i < tmpl.minibossCount; ++i)
    {
        int archetype = randomArchetypeForTier(TIER_ELITE);
        seedUnit(enc, archetype, TIER_ELITE, playerLevel);
    }

    // The boss itself.
    if (tmpl.bossIndex >= 0 && tmpl.bossIndex < bossCount)
    {
        int archetype = findArchetype(ROLE_TANK, tier);

        if (archetype < 0)
            archetype = randomArchetypeForTier(tier);

        seedUnit(enc, archetype, tier, playerLevel);
    }
}

void buildEncounter(encounterinstance &enc, int templateIndex, int playerLevel,
                    int partySize, enemytier tier)
{
    clearEncounter(enc);

    if (templateIndex < 0 || templateIndex >= encounterCount)
        return;

    fillFromTemplate(enc, encounterArray[templateIndex], playerLevel, tier);

    // EncounterPB = Sum(PlayerPower) * DifficultyFactor
    int playerPower = playerLevel * 100 * partySize;
    enc.powerBudget = encounterPowerBudget(playerPower,
                                           difficultyFactor(difficultMode));
}

void buildOpenWorldPack(encounterinstance &enc, int playerLevel)
{
    int packIndex = findEncounter("Standard Patrol");

    if (packIndex < 0)
        packIndex = randomEncounterOfKind(ENC_PACK);

    buildEncounter(enc, packIndex, playerLevel, 1, TIER_TRASH);
}

void buildDungeonRoom(encounterinstance &enc, int playerLevel, int roomSeed)
{
    // Four room shapes, selected by seed so a run is reproducible from it.
    static const char *rooms[4] = {
        "Hazard Cache", "Miniboss Room", "Puzzle Chamber", "Timed Gauntlet"};

    int index = roomSeed % 4;
    if (index < 0)
        index = -index;

    int roomIndex = findEncounter(rooms[index]);

    if (roomIndex < 0)
        roomIndex = randomEncounterOfKind(ENC_DUNGEON_ROOM);

    buildEncounter(enc, roomIndex, playerLevel, 1, TIER_ELITE);
}

void buildRaidEncounter(encounterinstance &enc, int bossIndex, int partySize)
{
    if (bossIndex < 0 || bossIndex >= bossCount)
    {
        clearEncounter(enc);
        return;
    }

    int raidIndex = findEncounter("Boss and Adds");

    if (raidIndex < 0)
        raidIndex = randomEncounterOfKind(ENC_RAID_BOSS);

    const bossdata &boss = bossArray[bossIndex];

    buildEncounter(enc, raidIndex, boss.level, partySize, boss.tier);
    enc.bossIndex = bossIndex;
}

/*
 * bossShouldEnrage() and softEnrageBonus() are defined once, in
 * src/data/bosses.c++, alongside the boss table they read.  They are derived
 * purely from bossdata (hard enrage time and soft enrage rate), so there is
 * no reason for a second copy here.
 */
