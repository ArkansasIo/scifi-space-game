// src/engine/progression_engine.c++ -- the progression engine.
//
// Character, fleet, faction and empire growth.  Growth is applied to a
// stat block through accumulate-and-derive, never by patching raw numbers.

#include "spacebattlerpg.h"
#include "engine/progression_engine.h"

static int g_progressionRunning = 0;

/* ---------------- lifecycle ---------------- */

int progressionEngineInit()
{
    return 1;
}

int progressionEngineStart()
{
    g_progressionRunning = 1;
    return 1;
}

int progressionEngineUpdate(int ticks)
{
    (void)ticks;
    return g_progressionRunning;
}

void progressionEngineStop()
{
    g_progressionRunning = 0;
}

void progressionEngineShutdown()
{
    g_progressionRunning = 0;
}

/* ---------------- runtime ---------------- */

void clearProgression(progressionstate &state)
{
    memset(&state, 0, sizeof(state));

    state.level = 1;
    state.techLevel = 1;
}

int progressionXPToNext(const progressionstate &state)
{
    // A gently super-linear curve: 100 * level^1.5, approximated.
    int level = state.level > 0 ? state.level : 1;

    return 100 + (level * level * 10);
}

int progressionGrantXP(progressionstate &state, int amount)
{
    if (amount <= 0)
        return 0;

    state.experience += amount;

    int gained = 0;
    int guard = 0;

    while (state.experience >= progressionXPToNext(state) && guard < 200)
    {
        state.experience -= progressionXPToNext(state);

        state.level++;
        state.pendingAttributePoints += 3;
        state.pendingSkillPoints += 1;
        gained++;

        charactership.level = state.level;

        cout << "  Level up! You are now level " << state.level << "." << endl;

        audioPlaySFX(SFX_LEVEL_UP);
        luaFireEvent(SBW_EVENT_LEVEL_UP, state.level, 0);

        guard++;
    }

    return gained;
}

int progressionSpendAttribute(coreattributes &attrs, int attributeId)
{
    // Attributes are addressed by the enum order in docs/FRAMEWORK.md s.1.
    switch (attributeId)
    {
    case 0:
        attrs.str++;
        return 1;
    case 1:
        attrs.dex++;
        return 1;
    case 2:
        attrs.con++;
        return 1;
    case 3:
        attrs.intel++;
        return 1;
    case 4:
        attrs.wis++;
        return 1;
    case 5:
        attrs.vit++;
        return 1;
    case 6:
        attrs.spi++;
        return 1;
    case 7:
        attrs.lck++;
        return 1;
    case 8:
        attrs.wil++;
        return 1;
    case 9:
        attrs.cha++;
        return 1;
    default:
        return 0;
    }
}

int progressionSpendSkill(progressionstate &state, int skillId)
{
    if (state.pendingSkillPoints <= 0)
        return 0;

    if (skillId < 0)
        return 0;

    state.pendingSkillPoints--;
    return 1;
}

/* ---------------- derived power ---------------- */

int progressionCharacterPower(const statblock &stats)
{
    // Character power is a weighted roll-up, in the same units as item power.
    int power = stats.combat.baseattack;
    power += stats.sub.maxhp / 10;
    power += stats.sub.armor / 4;
    power += stats.sub.magicresist / 4;
    power += stats.combat.critchance / 100;
    power += stats.combat.critdamage / 10;

    return power;
}

int progressionFleetRank(int fleetVictories)
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

int progressionReputationTier(int reputation)
{
    if (reputation >= 5000)
        return 5; // exalted
    if (reputation >= 2000)
        return 4; // revered
    if (reputation >= 800)
        return 3; // honored
    if (reputation >= 300)
        return 2; // friendly
    if (reputation >= 0)
        return 1; // neutral

    return 0; // hostile
}

int progressionEmpireRank(int systemsControlled)
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

/* ---------------- applied growth ---------------- */

void progressionApplyLevel(statblock &stats, int newLevel)
{
    // Each level grants a small, even spread so no single stat runs away.
    if (newLevel <= 1)
        return;

    coreattributes &a = stats.attributes;

    a.str += newLevel / 2;
    a.dex += newLevel / 3;
    a.con += newLevel / 2;
    a.vit += newLevel / 2;
    a.intel += newLevel / 3;
    a.wis += newLevel / 4;
    a.spi += newLevel / 4;
    a.lck += newLevel / 5;
    a.wil += newLevel / 4;
    a.cha += newLevel / 5;

    deriveSubAttributes(stats);
    deriveCombatStats(stats);
    applyStatCaps(stats, isPvPEnabled() ? CTX_PVP : CTX_PVE);
}

void progressionBuildStats(statblock &stats, int level, int classId)
{
    clearStatBlock(stats);

    // Class baselines, then level growth on top.
    switch (classId)
    {
    case 0: // marine
        stats.attributes.str = 12;
        stats.attributes.con = 10;
        stats.attributes.vit = 10;
        break;

    case 1: // engineer
        stats.attributes.intel = 12;
        stats.attributes.wis = 10;
        stats.attributes.spi = 10;
        break;

    case 2:  // pilot
        stats.attributes.dex = 12;
        stats.attributes.wil = 10;
        stats.attributes.lck = 8;
        break;

    default: // commander
        stats.attributes.cha = 12;
        stats.attributes.wil = 10;
        stats.attributes.vit = 8;
        break;
    }

    progressionApplyLevel(stats, level);
}

/* ---------------- reporting ---------------- */

void progressionShowSummary(const progressionstate &state)
{
    drawHeader("Progression");

    drawField("Level", state.level);
    drawField("Experience", state.experience);
    drawField("XP to next", progressionXPToNext(state));
    drawField("Attribute points", state.pendingAttributePoints);
    drawField("Skill points", state.pendingSkillPoints);
    drawField("Character power", state.characterPower);
    drawField("Fleet rank", state.fleetRank);
    drawField("Reputation", state.factionReputation);
    drawField("Empire rank", state.empireRank);
    drawField("Tech level", state.techLevel);

    cout << endl;
}
