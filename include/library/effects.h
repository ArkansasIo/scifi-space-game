// include/library/effects.h -- buffs, debuffs, stacking and diminishing
// returns.  See docs/FRAMEWORK.md section 6.

#ifndef SBW_LIBRARY_EFFECTS_H
#define SBW_LIBRARY_EFFECTS_H

// Reset a live effect instance.
void clearActiveEffect(activeeffect &effect);

// Apply an effect (by index into effectArray) to a target's live list.
// Honours the definition's stack mode.  Returns the slot used, or -1.
int applyEffect(activeeffect list[], int listMax, int definitionIndex,
                int sourceId);

// Advance one tick.  Returns the number of effects that expired.
int tickEffects(activeeffect list[], int listMax);

// Duration multiplier from the CC diminishing-returns ladder:
// 1st 100%, 2nd 50%, 3rd 25%, 4th 0% (immune).  drCount is 0-based.
int diminishingReturnMultiplier(int drCount);

// Apply DR to a raw duration in ticks using the ladder above.
int applyDiminishingReturns(int baseDuration, int drCount);

// True if the cleanse type removes the given effect.
bool cleanseMatches(cleansetype cleanse, const effectdefinition &def);

// Remove every effect a cleanse type covers.  Returns how many were removed.
int cleanseEffects(activeeffect list[], int listMax, cleansetype cleanse);

// Total magnitude contributed by all stacks of one definition.
int totalMagnitude(const activeeffect list[], int listMax, int definitionIndex);

#endif /* SBW_LIBRARY_EFFECTS_H */
