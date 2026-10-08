// src/engine/effect_engine.c++ -- the effect engine.
//
// Owns the live effect list on every combatant and the per-target diminishing
// returns records that gate crowd control.  See docs/FRAMEWORK.md s.6.

#include "spacebattlerpg.h"
#include "engine/effect_engine.h"

static int g_effectRunning = 0;

/* ---------------- lifecycle ---------------- */

int effectEngineInit()
{
    return 1;
}

int effectEngineStart()
{
    g_effectRunning = 1;
    return 1;
}

int effectEngineUpdate(int ticks)
{
    (void)ticks;
    return g_effectRunning;
}

void effectEngineStop()
{
    g_effectRunning = 0;
}

void effectEngineShutdown()
{
    g_effectRunning = 0;
}

/* ---------------- runtime ---------------- */

void clearEffectRuntime(effectruntime &rt)
{
    memset(&rt, 0, sizeof(rt));

    for (int i = 0; i < 32; ++i)
        rt.dr[i].drCategory = -1;
}

/* ---------------- apply / tick ---------------- */

int effectApply(effectruntime &rt, activeeffect list[], int listMax,
                int definitionIndex, int entityId)
{
    if (definitionIndex < 0 || definitionIndex >= effectCount)
        return -1;

    const effectdefinition &def = effectArray[definitionIndex];

    // Crowd control is gated by the diminishing-returns ladder.
    int drMultiplier = 100;

    if (def.drCategory >= 0)
    {
        drMultiplier = drConsume(rt, entityId, def.drCategory);

            if (drMultiplier <= 0)
                return -1;  // immune right now
        }

        int slot = applyEffect(list, listMax, definitionIndex, entityId);

    if (slot < 0)
        return -1;

    // Scale the duration by the DR ladder.
    if (drMultiplier < 100)
    {
        list[slot].remaining = (list[slot].remaining * drMultiplier) / 100;

        if (list[slot].remaining <= 0)
            list[slot].remaining = 1;
    }

    luaFireEvent(SBW_EVENT_DAMAGE_DEALT, definitionIndex, 0);

    return slot;
}

int effectTick(effectruntime &rt, activeeffect list[], int listMax)
{
    int expired = tickEffects(list, listMax);
    rt.effectsExpired += expired;

    return expired;
}

int effectDoTDamage(const activeeffect list[], int listMax, int mitigationBp)
{
    if (!list || listMax <= 0)
        return 0;

    int total = 0;

    for (int i = 0; i < listMax; ++i)
    {
        int index = list[i].definitionIndex;
        if (index < 0 || index >= effectCount)
            continue;

        const effectdefinition &def = effectArray[index];

        if (def.category != EFF_DOT || def.tickRate <= 0)
            continue;

        // A DoT ticks for its magnitude, per stack.
        int tick = list[i].magnitude * list[i].stacks;
        tick = computeDoTTick(tick, 100, mitigationBp);

        total += tick;
    }

    return total;
}

int effectHoTHeal(const activeeffect list[], int listMax)
{
    if (!list || listMax <= 0)
        return 0;

    int total = 0;

    for (int i = 0; i < listMax; ++i)
    {
        int index = list[i].definitionIndex;
        if (index < 0 || index >= effectCount)
            continue;

        const effectdefinition &def = effectArray[index];

        // A defensive tick that restores rather than drains.
        if (def.category != EFF_DEFENSIVE || def.tickRate <= 0)
            continue;

        if (def.magnitude > 0)
            total += def.magnitude * list[i].stacks;
    }

    return total;
}

/* ---------------- diminishing returns ---------------- */

int drRecordFor(effectruntime &rt, int entityId, int drCategory)
{
    for (int i = 0; i < rt.drCount; ++i)
    {
        if (rt.dr[i].entityId == entityId && rt.dr[i].drCategory == drCategory)
            return i;
    }

    if (rt.drCount >= 32)
        return -1;

    int index = rt.drCount;
    rt.dr[index].entityId = entityId;
    rt.dr[index].drCategory = drCategory;
    rt.dr[index].count = 0;
    rt.dr[index].resetAtTick = rt.currentTick;

    rt.drCount++;

    return index;
}

int drConsume(effectruntime &rt, int entityId, int drCategory)
{
    int index = drRecordFor(rt, entityId, drCategory);

    if (index < 0)
        return 100;

    int multiplier = diminishingReturnMultiplier(rt.dr[index].count);

    // Advance the ladder; it resets on a timer, not on a successful break.
    rt.dr[index].count++;
    rt.dr[index].resetAtTick = rt.currentTick + 30;

    return multiplier;
}

int drExpire(effectruntime &rt, int windowTicks)
{
    (void)windowTicks;  // each record carries its own resetAtTick

    int reset = 0;

    for (int i = 0; i < rt.drCount; ++i)
    {
        if (rt.currentTick >= rt.dr[i].resetAtTick)
        {
            if (rt.dr[i].count > 0)
                reset++;

            rt.dr[i].count = 0;
            rt.dr[i].resetAtTick = 0;
        }
    }

    return reset;
}

/* ---------------- cleanses and immunity ---------------- */

int effectCleanse(activeeffect list[], int listMax, cleansetype cleanse)
{
    int removed = cleanseEffects(list, listMax, cleanse);

    if (removed > 0)
        audioPlaySFX(SFX_BUFF_APPLIED);

    return removed;
}

int effectImmune(const effectruntime &rt, int entityId, effectcategory category)
{
    // Only crowd control is subject to immunity windows in this model.
    if (category != EFF_CROWD_CONTROL)
        return 0;

    for (int i = 0; i < rt.drCount; ++i)
    {
        if (rt.dr[i].entityId == entityId && rt.dr[i].count >= 3)
            return 1;
    }

    return 0;
}

void effectGrantImmunity(effectruntime &rt, int entityId, int windowTicks)
{
    int index = drRecordFor(rt, entityId, 1);

    if (index < 0)
        return;

    rt.dr[index].count = 3; // the immune step of the ladder
    rt.dr[index].resetAtTick = rt.currentTick + windowTicks;
}

/* ---------------- queries ---------------- */

int effectCountCategory(const activeeffect list[], int listMax,
                        effectcategory category)
{
    if (!list || listMax <= 0)
        return 0;

    int count = 0;

    for (int i = 0; i < listMax; ++i)
    {
        int index = list[i].definitionIndex;
        if (index < 0 || index >= effectCount)
            continue;

        if (effectArray[index].category == category)
            count++;
    }

    return count;
}

int effectCategoryMagnitude(const activeeffect list[], int listMax,
                            effectcategory category)
{
    if (!list || listMax <= 0)
        return 0;

    int total = 0;

    for (int i = 0; i < listMax; ++i)
    {
        int index = list[i].definitionIndex;
        if (index < 0 || index >= effectCount)
            continue;

        if (effectArray[index].category == category)
            total += list[i].magnitude * list[i].stacks;
    }

    return total;
}
