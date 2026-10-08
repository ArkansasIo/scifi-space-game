// include/data/archetypes.h -- enemy archetypes (AI roles).
// See docs/FRAMEWORK.md sections 7.1 and 7.2.

#ifndef SBW_DATA_ARCHETYPES_H
#define SBW_DATA_ARCHETYPES_H

// Fill archetypeArray[] with one entry per role / tier combination.
void initializeArchetypes();

// Find an archetype by role and tier.  Returns index, or -1.
int findArchetype(enemyrole role, enemytier tier);

// Pick a random archetype appropriate for a tier.  Returns index, or -1.
int randomArchetypeForTier(enemytier tier);

// Baseline HP / damage multiplier for an archetype's tier.
int archetypeHPMultiplier(int archetypeIndex);
int archetypeDamageMultiplier(int archetypeIndex);

#endif /* SBW_DATA_ARCHETYPES_H */
