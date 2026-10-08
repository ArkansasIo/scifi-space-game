// include/library/arraydata.h -- defines every framework array and sub-array
// declared "extern" in include/core/globals.h, plus the label / lookup
// sub-arrays (damage types, resources, tiers, roles, slots, rarities).
//
// Sub-arrays are the small fixed-size rows the scaling helpers index by enum,
// for example tierMultiplier[TIER_ELITE] or slotWeight[SLOT_CHEST].

#ifndef SBW_LIBRARY_ARRAYDATA_H
#define SBW_LIBRARY_ARRAYDATA_H

// Fill the label tables (const char * names indexed by enum).
void initializeLabelArrays();

// Fill the numeric sub-arrays: tierMultiplier, tierBaselinePercent,
// slotWeight, rarityMultiplier.
void initializeSubArrays();

// Define + fill every framework table in one call.
void initializeFrameworkArrays();

// Human-readable lookup helpers (bounds-checked, never return null).
const char *damageTypeName(damagetype type);
const char *resourceName(resourcetype res);
const char *tierName(enemytier tier);
const char *roleName(enemyrole role);
const char *slotName(gearslot slot);
const char *rarityName(gearrarity rarity);

// Sub-array lookups.
int tierMultiplierFor(enemytier tier);
int tierBaselineFor(enemytier tier);
int slotWeightFor(gearslot slot);
int rarityMultiplierFor(gearrarity rarity);

#endif /* SBW_LIBRARY_ARRAYDATA_H */
