// src/game/progression.c++ -- character, fleet, faction and empire growth.
// See docs/FRAMEWORK.md s.22.11.

#include "spacebattlerpg.h"

int xpForLevel(int level)
{
    if (level <= 0)
        level = 1;

    // 100 * level^1.5, approximated without floating point.
    return 100 + (level * level * 10);
}

int grantExperience(player &who, int amount)
{
    if (amount <= 0)
        return 0;

    who.exp += amount;

    int gained = 0;
    int guard = 0;

    while (who.exp >= xpForLevel(who.level) && guard < 200)
    {
        who.exp -= xpForLevel(who.level);
        who.level++;

        // Level growth feeds straight back into the ship's power.
        who.power += 5;
        who.sif += 3;
        who.health += 5;
        who.attackpower = who.power * 2;
        who.defencepower = who.sif * 2;

        gained++;
        guard++;

        audioPlaySFX(SFX_LEVEL_UP);
        luaFireEvent(SBW_EVENT_LEVEL_UP, who.level, 0);
    }

    return gained;
}

int spendAttributePoint(coreattributes &attrs, int attributeId, int points)
{
    if (points <= 0)
        return 0;

    int spent = 0;

    for (int i = 0; i < points; ++i)
    {
        switch (attributeId)
        {
        case 0:
            attrs.str++;
            spent++;
            break;
        case 1:
            attrs.dex++;
            spent++;
            break;
        case 2:
            attrs.con++;
            spent++;
            break;
        case 3:
            attrs.intel++;
            spent++;
            break;
        case 4:
            attrs.wis++;
            spent++;
            break;
        case 5:
            attrs.vit++;
            spent++;
            break;
        case 6:
            attrs.spi++;
            spent++;
            break;
        case 7:
            attrs.lck++;
            spent++;
            break;
        case 8:
            attrs.wil++;
            spent++;
            break;
        case 9:
            attrs.cha++;
            spent++;
            break;
        default:
            return spent;
        }
    }

    return spent;
}

int commanderRank(int fleetVictories)
{
    if (fleetVictories >= 200)
        return 10;
    if (fleetVictories >= 100)
        return 8;
    if (fleetVictories >= 50)
        return 6;
    if (fleetVictories >= 25)
        return 4;
    if (fleetVictories >= 10)
        return 3;
    if (fleetVictories >= 5)
        return 2;

    return 1;
}

int reputationTier(int reputation)
{
    if (reputation >= 5000)
        return 5;
    if (reputation >= 2000)
        return 4;
    if (reputation >= 800)
        return 3;
    if (reputation >= 300)
        return 2;
    if (reputation >= 0)
        return 1;

    return 0;
}

int empireRank(int systemsControlled)
{
    if (systemsControlled >= 40)
        return 6;
    if (systemsControlled >= 25)
        return 5;
    if (systemsControlled >= 15)
        return 4;
    if (systemsControlled >= 8)
        return 3;
    if (systemsControlled >= 3)
        return 2;

    return 1;
}

victorytype leadingVictory(const faction &who, int systemsControlled,
                           int techLevel)
{
    // The empire is closest to whichever path its attributes favour most.
    if (systemsControlled >= 30)
        return VICTORY_MILITARY;

    if (who.attributes.economy >= 90)
        return VICTORY_ECONOMIC;

    if (techLevel >= 8 || who.attributes.science >= 90)
        return VICTORY_TECHNOLOGICAL;

    if (who.attributes.influence >= 90)
        return VICTORY_DIPLOMATIC;

    if (who.resources.alienartifacts >= 3)
        return VICTORY_ASCENSION;

    return VICTORY_MILITARY;
}

int researchTime(int techCost, int scienceOutput, int scientistSkill)
{
    if (techCost <= 0)
        return 0;

    // ResearchTime = TechCost / (ScienceOutput * ScientistSkill)
    int rate = (scienceOutput * scientistSkill) / 100;

    if (rate <= 0)
        rate = 1;

    return techCost / rate;
}
