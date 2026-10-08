// include/engine/effect_engine.h -- the effect engine.
//
// Owns the live effect list on every combatant, drives periodic ticks
// (DoT / HoT), resolves stacking, and applies cleanses and immunity windows.

#ifndef SBW_ENGINE_EFFECT_ENGINE_H
#define SBW_ENGINE_EFFECT_ENGINE_H

// A tracked DR record, so the CC ladder is per-target and per-category.
struct drrecord
{
    int entityId;
    int drCategory;
    int count;
    int resetAtTick;
};

// The engine's per-tick context.
struct effectruntime
{
    int currentTick;
    drrecord dr[32];
    int drCount;
    int dotsApplied;
    int effectsExpired;
    int cleansesApplied;
};

/* ---------------- lifecycle ---------------- */

int effectEngineInit();
int effectEngineStart();
int effectEngineUpdate(int ticks);
void effectEngineStop();
void effectEngineShutdown();

/* ---------------- runtime ---------------- */

void clearEffectRuntime(effectruntime &rt);

// Apply an effect to a combatant's list, recording DR for CC.
int effectApply(effectruntime &rt, activeeffect list[], int listMax,
                int definitionIndex, int entityId);

// Advance every effect one tick, applying DoT/HoT and expiry.
int effectTick(effectruntime &rt, activeeffect list[], int listMax);

// Total DoT damage a list deals this tick.
int effectDoTDamage(const activeeffect list[], int listMax, int mitigationBp);

// Total HoT healing a list restores this tick.
int effectHoTHeal(const activeeffect list[], int listMax);

/* ---------------- diminishing returns ---------------- */

// Look up (or create) a DR record.  Returns its index.
int drRecordFor(effectruntime &rt, int entityId, int drCategory);

// The duration multiplier this application should get, then advance the count.
int drConsume(effectruntime &rt, int entityId, int drCategory);

// Reset records whose window has elapsed.  Returns the number reset.
int drExpire(effectruntime &rt, int windowTicks);

/* ---------------- cleanses & immunity ---------------- */

int effectCleanse(activeeffect list[], int listMax, cleansetype cleanse);

// Is the target currently immune to this category of effect?
int effectImmune(const effectruntime &rt, int entityId, effectcategory category);

// Apply the immunity window an effect grants after it ends.
void effectGrantImmunity(effectruntime &rt, int entityId, int windowTicks);

/* ---------------- queries ---------------- */

// Count live effects of a category on a list.
int effectCountCategory(const activeeffect list[], int listMax,
                        effectcategory category);

// Total magnitude of every effect in a category (for buff math).
int effectCategoryMagnitude(const activeeffect list[], int listMax,
                            effectcategory category);

#endif /* SBW_ENGINE_EFFECT_ENGINE_H */
