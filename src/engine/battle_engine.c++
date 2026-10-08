// src/engine/battle_engine.c++ -- the battle engine.
//
// Sequences fights into an encounter: intro, waves, per-fight resolution,
// boss phases, rewards and completion.  Where the combat engine answers
// "who wins this fight", the battle engine answers "what happens next".

#include "spacebattlerpg.h"
#include "engine/battle_engine.h"
#include "engine/combat_engine.h"
#include "ui/display.h"

static int g_battleRunning = 0;

/* ---------------- lifecycle ---------------- */

int battleEngineInit()
{
    return 1;
}

int battleEngineStart()
{
    g_battleRunning = 1;
    return 1;
}

int battleEngineUpdate(int ticks)
{
    (void)ticks;
    return g_battleRunning;
}

void battleEngineStop()
{
    g_battleRunning = 0;
}

void battleEngineShutdown()
{
    g_battleRunning = 0;
}

/* ---------------- sequencing ---------------- */

void clearBattleRuntime(battleruntime &rt)
{
    memset(&rt, 0, sizeof(rt));

    rt.state = BAT_IDLE;
    rt.bossPhaseIndex = -1;
    rt.encounterIndex = -1;
}

void beginBattle(battleruntime &rt, int encounterIndex, int playerLevel,
                 int partySize)
{
    clearBattleRuntime(rt);

    if (encounterIndex < 0 || encounterIndex >= encounterCount)
        return;

    if (partySize < 1)
        partySize = 1;

    const encountertemplate &tmpl = encounterArray[encounterIndex];

    rt.encounterIndex = encounterIndex;
    rt.waveCount = (tmpl.waveCount > 0) ? tmpl.waveCount : 1;
    rt.wave = 0;
    rt.enemiesRemaining = tmpl.trashCount + tmpl.casterCount + tmpl.minibossCount;

    if (rt.enemiesRemaining <= 0 && tmpl.bossIndex >= 0)
        rt.enemiesRemaining = 1;

    rt.state = BAT_INTRO;

    // Level and party scale the encounter's power budget.
    int budget = encounterPowerBudget(playerLevel * 100 * partySize, 100);
    (void)budget;
}

const char *battleStateName(battlestate state)
{
    switch (state)
    {
    case BAT_IDLE:
        return "Idle";
    case BAT_INTRO:
        return "Intro";
    case BAT_WAVE:
        return "Wave";
    case BAT_FIGHT:
        return "Fight";
    case BAT_REWARD:
        return "Reward";
    case BAT_BOSS_PHASE:
        return "Boss Phase";
    case BAT_COMPLETE:
        return "Complete";
    case BAT_LOST:
        return "Lost";
    default:
        return "Unknown";
    }
}

void tickBattle(battleruntime &rt)
{
    rt.ticksInState++;

    switch (rt.state)
    {
    case BAT_INTRO:
        rt.wave++;
        rt.state = BAT_WAVE;
        break;

    case BAT_WAVE:
        cout << "  Wave " << rt.wave << " of " << rt.waveCount
             << " incoming." << endl;
        rt.state = BAT_FIGHT;
        break;

    case BAT_FIGHT:
        // A single fight is resolved by the combat engine.
        if (rt.enemiesRemaining > 0)
            rt.enemiesRemaining--;

        if (rt.enemiesRemaining <= 0)
            rt.state = BAT_REWARD;
        break;

    case BAT_REWARD:
        grantWaveRewards(rt);

        if (rt.wave >= rt.waveCount)
        {
            rt.state = BAT_COMPLETE;
        }
        else
        {
            rt.wave++;
            rt.state = BAT_WAVE;
        }
        break;

    case BAT_BOSS_PHASE:
        rt.bossPhaseIndex++;
        rt.state = BAT_FIGHT;
        break;

    case BAT_COMPLETE:
    case BAT_LOST:
    case BAT_IDLE:
    default:
        break;
    }
}

int battleComplete(const battleruntime &rt)
{
    return (rt.state == BAT_COMPLETE) ? 1 : 0;
}

/* ---------------- rewards ---------------- */

int grantWaveRewards(battleruntime &rt)
{
    // A cleared wave is worth score and a little experience.
    int reward = 50 * rt.wave;

    targetScore.score += reward;
    rt.scoreAwarded += reward;
    rt.rewardsGranted++;

    if (charactership.level < 100)
        charactership.exp += 10 * rt.wave;

    luaFireEvent(SBW_EVENT_LEVEL_UP, charactership.level, 0);

    return reward;
}

int grantFinalRewards(battleruntime &rt, int clearTimePercent, int noDeath)
{
    enemytier tier = TIER_DUNGEON_BOSS;

    if (rt.encounterIndex >= 0 && rt.encounterIndex < encounterCount)
    {
        const encountertemplate &tmpl = encounterArray[rt.encounterIndex];

        if (tmpl.bossIndex >= 0 && tmpl.bossIndex < bossCount)
            tier = bossArray[tmpl.bossIndex].tier;
    }

    int multiplier = lootRewardMultiplier(currentDifficultyTier(), tier,
                                          clearTimePercent, noDeath);

    int bonus = (rt.scoreAwarded * (multiplier - 100)) / 100;

    if (bonus > 0)
    {
        targetScore.score += bonus;
        rt.scoreAwarded += bonus;
    }

    return bonus;
}

/* ---------------- boss phases ---------------- */

int advanceBossPhase(battleruntime &rt, const bossdata &boss, int hpPercent)
{
    const bossphase *phase = currentBossPhase(boss, hpPercent);

    if (!phase)
        return -1;

    // Find the phase's index so the runtime stays in sync.
    int index = -1;

    for (int i = 0; i < boss.phaseCount && i < 4; ++i)
    {
        if (&boss.phases[i] == phase)
        {
            index = i;
            break;
        }
    }

    if (index > rt.bossPhaseIndex)
    {
        rt.bossPhaseIndex = index;
        rt.state = BAT_BOSS_PHASE;

        cout << "  " << boss.name << " enters phase: "
             << phase->name << endl;

        audioPlaySFX(SFX_BOSS_PHASE);
        luaFireEvent(SBW_EVENT_BOSS_PHASE, index, hpPercent);

        return index;
    }

    return -1;
}

int applySoftEnrage(battleruntime &rt, const bossdata &boss)
{
    if (boss.softEnrageRate <= 0)
        return 0;

    // +softEnrageRate% damage for every 15 seconds elapsed.
    int intervals = rt.ticksInState / 15;

    if (intervals <= 0)
        return 0;

    return 100 + (intervals * boss.softEnrageRate);
}

int hardEnrageReached(const battleruntime &rt, const bossdata &boss)
{
    if (boss.hardEnrageTime <= 0)
        return 0;

    return (rt.ticksInState >= boss.hardEnrageTime) ? 1 : 0;
}
