// include/data/affixes.h -- elite / mythic affixes.
// See docs/FRAMEWORK.md section 11.

#ifndef SBW_DATA_AFFIXES_H
#define SBW_DATA_AFFIXES_H

// Fill affixArray[] with offensive, defensive, utility and hazard affixes.
void initializeAffixes();

// Look up an affix by name.  Returns index, or -1.
int findAffix(const char *name);

// Roll an affix set for a champion/mythic enemy.
// Writes indices into `out` and returns how many were chosen.
// Applies the Minor(1-2) + Major(0-1) + Seasonal(0-1) stacking rule and
// refuses the unfun Reflective + high-DoT combination.
int rollAffixSet(affixcategory seasonCategory, int out[], int outMax);

// True if two affixes must never appear together.
int affixesConflict(int affixA, int affixB);

#endif /* SBW_DATA_AFFIXES_H */
