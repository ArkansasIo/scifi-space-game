// include/data/effects.h -- the buff / debuff catalogue.
// See docs/FRAMEWORK.md section 6.

#ifndef SBW_DATA_EFFECTS_H
#define SBW_DATA_EFFECTS_H

// Fill effectArray[] with every buff and debuff.
void initializeEffects();

// Look up an effect by name.  Returns index, or -1 if not found.
int findEffect(const char *name);

// Count all effects in a category.
int countEffectsInCategory(effectcategory category);

// Fill a fresh list of effects belonging to an expandable category.
// Returns how many were written.
int listEffectsInCategory(effectcategory category, int out[], int outMax);

#endif /* SBW_DATA_EFFECTS_H */
